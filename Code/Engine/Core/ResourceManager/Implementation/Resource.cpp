#include <PCH.h>
#include <Core/ResourceManager/Resource.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezResourceBase, 1, ezRTTINoAllocator);

EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_CORE_DLL void IncreaseResourceRefCount(ezResourceBase* pResource)
{
  pResource->m_iReferenceCount.Increment();
}

EZ_CORE_DLL void DecreaseResourceRefCount(ezResourceBase* pResource)
{
  pResource->m_iReferenceCount.Decrement();
}

ezResourceBase::ezResourceBase(DoUpdate ResourceUpdateThread, ezUInt8 uiQualityLevelsLoadable)
{
  m_Flags.AddOrRemove(ezResourceFlags::UpdateOnMainThread, ResourceUpdateThread == DoUpdate::OnMainThread);

  m_uiUniqueIDHash = 0;
  m_iReferenceCount = 0;
  m_uiResourceChangeCounter = 0;
  m_LoadingState = ezResourceState::Unloaded;
  m_uiQualityLevelsLoadable = uiQualityLevelsLoadable;
  m_uiQualityLevelsDiscardable = 0;
  m_Priority = ezResourcePriority::Normal;
  m_DueDate = ezTime::Seconds(60.0 * 60.0 * 24.0 * 365.0 * 1000.0);
}

void ezResourceBase::SetResourceDescription(const char * szDescription)
{
  m_sResourceDescription = szDescription;
}

void ezResourceBase::SetDueDate(ezTime date /* = ezTime::Seconds(60.0 * 60.0 * 24.0 * 365.0 * 1000.0) */)
{
  if (m_DueDate != date)
  {
    m_DueDate = date;

    ezResourceEvent e;
    e.m_pResource = this;
    e.m_EventType = ezResourceEventType::ResourceDueDateChanged;
    ezResourceManager::BroadcastResourceEvent(e);
  }
}

void ezResourceBase::SetPriority(ezResourcePriority priority)
{
  m_Priority = priority;

  ezResourceEvent e;
  e.m_pResource = this;
  e.m_EventType = ezResourceEventType::ResourcePriorityChanged;
  ezResourceManager::BroadcastResourceEvent(e);
}

void ezResourceBase::SetUniqueID(const char* szUniqueID, bool bIsReloadable)
{
  m_UniqueID = szUniqueID;
  m_uiUniqueIDHash = ezHashing::MurmurHash(szUniqueID);
  SetIsReloadable(bIsReloadable);

  ezResourceEvent e;
  e.m_pResource = this;
  e.m_EventType = ezResourceEventType::ResourceCreated;
  ezResourceManager::BroadcastResourceEvent(e);
}

void ezResourceBase::CallUnloadData(Unload WhatToUnload)
{
  EZ_LOG_BLOCK("ezResource::UnloadData", GetResourceID().GetData());

  ezResourceEvent e;
  e.m_pResource = this;
  e.m_EventType = ezResourceEventType::ResourceContentUnloading;
  ezResourceManager::BroadcastResourceEvent(e);

  ezResourceLoadDesc ld = UnloadData(WhatToUnload);

  EZ_ASSERT_DEV(ld.m_State != ezResourceState::Invalid, "UnloadData() did not return a valid resource load state");
  EZ_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "UnloadData() did not fill out m_uiQualityLevelsDiscardable correctly");
  EZ_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "UnloadData() did not fill out m_uiQualityLevelsLoadable correctly");

  m_LoadingState = ld.m_State;
  m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
  m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;
}

void ezResourceBase::CallUpdateContent(ezStreamReader* Stream)
{
  EZ_LOG_BLOCK("ezResource::UpdateContent", GetResourceID().GetData());

  ezResourceLoadDesc ld = UpdateContent(Stream);

  EZ_ASSERT_DEV(ld.m_State != ezResourceState::Invalid, "UpdateContent() did not return a valid resource load state");
  EZ_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "UpdateContent() did not fill out m_uiQualityLevelsDiscardable correctly");
  EZ_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "UpdateContent() did not fill out m_uiQualityLevelsLoadable correctly");

  if (ld.m_State == ezResourceState::LoadedResourceMissing)
  {
    ReportResourceIsMissing();
  }

  IncResourceChangeCounter();

  m_LoadingState = ld.m_State;
  m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
  m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;

  ezResourceEvent e;
  e.m_pResource = this;
  e.m_EventType = ezResourceEventType::ResourceContentUpdated;
  ezResourceManager::BroadcastResourceEvent(e);

  ezLog::Debug("Updated {0} - '{1}'({2}, {3}) ", GetDynamicRTTI()->GetTypeName(), GetResourceDescription(), (int)GetPriority(), GetLoadingDeadline(ezTime::Now()).GetSeconds());
}

ezTime ezResourceBase::GetLoadingDeadline(ezTime tNow) const
{
  ezTime DueDate = tNow;

  ezTime tDelay;

  if (GetLoadingState() != ezResourceState::Loaded)
  {
    if (!GetBaseResourceFlags().IsAnySet(ezResourceFlags::ResourceHasFallback))
    {
      DueDate = ezTime::Seconds(0.0);
      tDelay = ezTime::Seconds(1.0); // to get a sorting by priority
    }
    else
    {
      tDelay += ezMath::Min((tNow - GetLastAcquireTime()) / 10.0, ezTime::Seconds(10.0));
    }
  }
  else
  {
    tDelay += ezTime::Seconds(GetNumQualityLevelsDiscardable() * 10.0);

    tDelay += (tNow - GetLastAcquireTime()) * 2.0;
  }

  DueDate += tDelay * ((double) GetPriority() + 1.0);

  return ezMath::Min(DueDate, m_DueDate);
}

ezResourceTypeLoader* ezResourceBase::GetDefaultResourceTypeLoader() const
{
  return ezResourceManager::GetDefaultResourceLoader();
}

void ezResourceBase::ReportResourceIsMissing()
{
  ezLog::Error("Missing Resource of Type '{2}': '{0}' ('{1}')", GetResourceID(), m_sResourceDescription, GetDynamicRTTI()->GetTypeName());
}

EZ_STATICLINK_FILE(Core, Core_ResourceManager_Implementation_Resource);

