#pragma once

#include <Core/Basics.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>
#include <Core/ResourceManager/Implementation/Declarations.h>
#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/ResourceManager/ResourceManager.h>

/// \brief The class from which all resource types need to derive.
///
/// Pass the resources own type as the first template parameter.
/// Pass the type name of the resource descriptor struct as the second template parameter.
/// This may be any custom struct that stores the required information for creating a resource.
template<typename SELF, typename SELF_DESCRIPTOR>
class ezResource : public ezResourceBase
{
public:
  typedef SELF_DESCRIPTOR DescriptorType;

  /// \brief Unfortunately this has to be called manually from within dynamic plugins during core engine shutdown.
  ///
  /// Without this, the dynamic plugin might still be referenced by the core engine during later shutdown phases and will crash, because memory
  /// and code is still referenced, that is already unloaded.
  static void CleanupDynamicPluginReferences()
  {
    s_TypeFallbackResource.Invalidate();
    s_TypeMissingResource.Invalidate();

    if (s_bAddedManagerEventHandler)
    {
      s_bAddedManagerEventHandler = false;
      ezResourceManager::s_ManagerEvents.RemoveEventHandler(ManagerEventHandler);
    }
  }

  /// \brief Sets the fallback resource that can be used while this resource is not yet loaded.
  ///
  /// By default there is no fallback resource, so all resource will block the application when requested for the first time.
  void SetFallbackResource(const ezTypedResourceHandle<SELF>& hResource)
  {
    m_hFallback = hResource;
    m_Flags.AddOrRemove(ezResourceFlags::ResourceHasFallback, m_hFallback.IsValid());
  }

  static void SetTypeFallbackResource(const ezTypedResourceHandle<SELF>& hResource)
  {
    if (!s_bAddedManagerEventHandler && !s_TypeFallbackResource.IsValid() && !s_TypeMissingResource.IsValid() && hResource.IsValid())
    {
      s_bAddedManagerEventHandler = true;
      ezResourceManager::s_ManagerEvents.AddEventHandler(ManagerEventHandler);
    }

    s_TypeFallbackResource = hResource;
  }

  static void SetTypeMissingResource(const ezTypedResourceHandle<SELF>& hResource)
  {
    if (!s_bAddedManagerEventHandler && !s_TypeFallbackResource.IsValid() && !s_TypeMissingResource.IsValid() && hResource.IsValid())
    {
      s_bAddedManagerEventHandler = true;
      ezResourceManager::s_ManagerEvents.AddEventHandler(ManagerEventHandler);
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    if (hResource.IsValid())
    {
      ezResourceLock<SELF> lock(hResource, ezResourceAcquireMode::NoFallback);
      // if this fails, the 'missing resource' is missing itself
    }
#endif

    if (hResource.IsValid())
    {
      // this is not thread safe (though we are only setting one flag), but this function should only be called during startup, where it is single-threaded anyway
      ezResourceLock<SELF> pFallback(hResource, ezResourceAcquireMode::PointerOnly);
      pFallback->m_Flags.Add(ezResourceFlags::IsMissingFallback);
    }

    s_TypeMissingResource = hResource;
  }

  static const ezTypedResourceHandle<SELF>& GetTypeFallbackResource() { return s_TypeFallbackResource; }
  static const ezTypedResourceHandle<SELF>& GetTypeMissingResource() { return s_TypeMissingResource; }

protected:

  friend class ezResourceManager;

  ezResource(DoUpdate ResourceUpdateThread, ezUInt8 uiQualityLevelsLoadable) : ezResourceBase(ResourceUpdateThread, uiQualityLevelsLoadable)
  {
  }

  ~ezResource() { }

private:

  static void ManagerEventHandler(const ezResourceManagerEvent& e)
  {
    if (e.m_EventType == ezResourceManagerEventType::ManagerShuttingDown)
    {
      CleanupDynamicPluginReferences();
    }

    if (e.m_EventType == ezResourceManagerEventType::ClearResourceFallbacks)
    {
      s_TypeFallbackResource.Invalidate();
      s_TypeMissingResource.Invalidate();
    }
  }

  static bool s_bAddedManagerEventHandler;
  static ezTypedResourceHandle<SELF> s_TypeFallbackResource;
  static ezTypedResourceHandle<SELF> s_TypeMissingResource;

  void CallCreateResource(const SELF_DESCRIPTOR& descriptor)
  {
    ezResourceLoadDesc ld = CreateResource(descriptor);

    EZ_ASSERT_DEV(ld.m_State != ezResourceState::Invalid, "CreateResource() did not return a valid resource load state");
    EZ_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "CreateResource() did not fill out m_uiQualityLevelsDiscardable correctly");
    EZ_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "CreateResource() did not fill out m_uiQualityLevelsLoadable correctly");

    IncResourceChangeCounter();

    m_LoadingState = ld.m_State;
    m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
    m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;

    // Update Memory Usage
    {
      ezResourceBase::MemoryUsage MemUsage;
      MemUsage.m_uiMemoryCPU = 0xFFFFFFFF;
      MemUsage.m_uiMemoryGPU = 0xFFFFFFFF;
      UpdateMemoryUsage(MemUsage);

      EZ_ASSERT_DEV(MemUsage.m_uiMemoryCPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its CPU memory usage", GetResourceID());
      EZ_ASSERT_DEV(MemUsage.m_uiMemoryGPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its GPU memory usage", GetResourceID());

      m_MemoryUsage = MemUsage;
    }

    ezResourceEvent e;
    e.m_pResource = this;
    e.m_EventType = ezResourceEventType::ResourceContentUpdated;
    ezResourceManager::BroadcastResourceEvent(e);

    ezLog::Debug("Created {0} - '{1}' ", GetDynamicRTTI()->GetTypeName(), GetResourceDescription());
  }

  /// \brief Override this function to implement resource creation. This is called by ezResourceManager::CreateResource.
  ///
  /// 'Creating' a resource is different from 'loading' a resource. Most resource types are never created. For example a mesh
  /// or a sound might not support being created and thus your implementation for such resources should be empty, except for an assert
  /// that always fires.
  /// However, some resources can be created (or even can only be created, but not loaded). For example textures might support to be
  /// created as render targets. In this case the descriptor needs to provide all the necessary information.
  /// Another example would be a procedurally generated mesh. The descriptor needs to provide all the parameters, such that 'CreateResource'
  /// can set up the resource.
  /// Note that created resources should always set its loading state to 'Loaded' and its current and max quality to 1, otherwise
  /// the resource manager might try to load even more into the resource afterwards.
  /// However, since this might be a valid use case for some resource types, it is not enforced by the resource manager.
  virtual ezResourceLoadDesc CreateResource(const SELF_DESCRIPTOR& descriptor)
  {
    EZ_REPORT_FAILURE("The resource type '{0}' does not support resource creation", GetDynamicRTTI()->GetTypeName());

    return ezResourceLoadDesc();
  }

  ezTypedResourceHandle<SELF> m_hFallback;
};

template<typename SELF, typename SELF_DESCRIPTOR>
bool ezResource<SELF, SELF_DESCRIPTOR>::s_bAddedManagerEventHandler = false;

template<typename SELF, typename SELF_DESCRIPTOR>
ezTypedResourceHandle<SELF> ezResource<SELF, SELF_DESCRIPTOR>::s_TypeFallbackResource;

template<typename SELF, typename SELF_DESCRIPTOR>
ezTypedResourceHandle<SELF> ezResource<SELF, SELF_DESCRIPTOR>::s_TypeMissingResource;


