#include <PCH.h>
#include <EditorPluginAssets/PropertyAnimAsset/PropertyAnimAsset.h>
#include <EditorPluginAssets/PropertyAnimAsset/PropertyAnimObjectManager.h>
#include <EditorPluginAssets/PropertyAnimAsset/PropertyAnimObjectAccessor.h>
#include <Core/WorldSerializer/ResourceHandleWriter.h>
#include <GameEngine/Resources/PropertyAnimResource.h>
#include <ToolsFoundation/Object/DocumentObjectVisitor.h>
#include <Core/World/GameObject.h>
#include <ToolsFoundation/Command/TreeCommands.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPropertyAnimationTrack, 1, ezRTTIDefaultAllocator<ezPropertyAnimationTrack>);
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ObjectPath", m_sObjectSearchSequence),
    EZ_MEMBER_PROPERTY("ComponentType", m_sComponentType),
    EZ_MEMBER_PROPERTY("Property", m_sPropertyPath),
    EZ_ENUM_MEMBER_PROPERTY("Target", ezPropertyAnimTarget, m_Target),
    EZ_MEMBER_PROPERTY("FloatCurve", m_FloatCurve),
    EZ_MEMBER_PROPERTY("Gradient", m_ColorGradient),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPropertyAnimationTrackGroup, 1, ezRTTIDefaultAllocator<ezPropertyAnimationTrackGroup>);
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("FPS", m_uiFramesPerSecond)->AddAttributes(new ezDefaultValueAttribute(60)),
    EZ_ARRAY_MEMBER_PROPERTY("Tracks", m_Tracks)->AddFlags(ezPropertyFlags::PointerOwner),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPropertyAnimAssetDocument, 1, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE

ezPropertyAnimationTrackGroup::~ezPropertyAnimationTrackGroup()
{
  for (ezPropertyAnimationTrack* pTrack : m_Tracks)
  {
    EZ_DEFAULT_DELETE(pTrack);
  }
}

ezPropertyAnimAssetDocument::ezPropertyAnimAssetDocument(const char* szDocumentPath)
  : ezSimpleAssetDocument<ezPropertyAnimationTrackGroup, ezGameObjectContextDocument>(EZ_DEFAULT_NEW(ezPropertyAnimObjectManager), szDocumentPath, true, true)
{
  m_GameObjectContextEvents.AddEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::GameObjectContextEventHandler, this));
  m_pAccessor = EZ_DEFAULT_NEW(ezPropertyAnimObjectAccessor, this, GetCommandHistory());

  GetCommandHistory()->m_Events.AddEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::CommandHistoryEventHandler, this));
}

ezPropertyAnimAssetDocument::~ezPropertyAnimAssetDocument()
{
  m_GameObjectContextEvents.RemoveEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::GameObjectContextEventHandler, this));
  GetCommandHistory()->m_Events.RemoveEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::CommandHistoryEventHandler, this));

  GetObjectManager()->m_StructureEvents.RemoveEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::TreeStructureEventHandler, this));
  GetObjectManager()->m_PropertyEvents.RemoveEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::TreePropertyEventHandler, this));
}

ezObjectAccessorBase* ezPropertyAnimAssetDocument::GetObjectAccessor() const
{
  return m_pAccessor.Borrow();
}

ezUInt64 ezPropertyAnimAssetDocument::GetAnimationDurationTicks() const
{
  if (m_uiCachedAnimationDuration != 0)
    return m_uiCachedAnimationDuration;

  const ezPropertyAnimationTrackGroup* pProp = GetProperties();

  // minimum duration
  m_uiCachedAnimationDuration = 480; // 1/10th second

  for (ezUInt32 i = 0; i < pProp->m_Tracks.GetCount(); ++i)
  {
    const ezPropertyAnimationTrack* pTrack = pProp->m_Tracks[i];

    for (const auto& cp : pTrack->m_FloatCurve.m_ControlPoints)
    {
      m_uiCachedAnimationDuration = ezMath::Max(m_uiCachedAnimationDuration, (ezUInt64)cp.m_iTick);
    }

    for (const auto& cp : pTrack->m_ColorGradient.m_ColorCPs)
    {
      m_uiCachedAnimationDuration = ezMath::Max<ezInt64>(m_uiCachedAnimationDuration, cp.m_iTick);
    }

    for (const auto& cp : pTrack->m_ColorGradient.m_AlphaCPs)
    {
      m_uiCachedAnimationDuration = ezMath::Max<ezInt64>(m_uiCachedAnimationDuration, cp.m_iTick);
    }

    for (const auto& cp : pTrack->m_ColorGradient.m_IntensityCPs)
    {
      m_uiCachedAnimationDuration = ezMath::Max<ezInt64>(m_uiCachedAnimationDuration, cp.m_iTick);
    }
  }

  if (m_uiLastAnimationDuration != m_uiCachedAnimationDuration)
  {
    m_uiLastAnimationDuration = m_uiCachedAnimationDuration;

    ezPropertyAnimAssetDocumentEvent e;
    e.m_pDocument = this;
    e.m_Type = ezPropertyAnimAssetDocumentEvent::Type::AnimationLengthChanged;
    m_PropertyAnimEvents.Broadcast(e);
  }

  return m_uiCachedAnimationDuration;
}


ezTime ezPropertyAnimAssetDocument::GetAnimationDurationTime() const
{
  const ezInt64 ticks = GetAnimationDurationTicks();

  return ezTime::Seconds(ticks / 4800.0);
}

bool ezPropertyAnimAssetDocument::SetScrubberPosition(ezUInt64 uiTick)
{
  if (!m_bPlayAnimation)
  {
    const ezUInt32 uiTicksPerFrame = 4800 / GetProperties()->m_uiFramesPerSecond;
    uiTick = (ezUInt64)ezMath::Round((double)uiTick, (double)uiTicksPerFrame);
  }
  uiTick = ezMath::Clamp<ezUInt64>(uiTick, 0, GetAnimationDurationTicks());

  if (m_uiScrubberTickPos == uiTick)
    return false;

  m_uiScrubberTickPos = uiTick;
  ApplyAnimation();

  ezPropertyAnimAssetDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = ezPropertyAnimAssetDocumentEvent::Type::ScrubberPositionChanged;
  m_PropertyAnimEvents.Broadcast(e);

  return true;
}

ezStatus ezPropertyAnimAssetDocument::InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const char* szPlatform, const ezAssetFileHeader& AssetHeader, bool bTriggeredManually)
{
  const ezPropertyAnimationTrackGroup* pProp = GetProperties();

  ezResourceHandleWriteContext HandleContext;
  HandleContext.BeginWritingToStream(&stream);

  ezPropertyAnimResourceDescriptor desc;
  desc.m_AnimationDuration = GetAnimationDurationTime();

  for (ezUInt32 i = 0; i < pProp->m_Tracks.GetCount(); ++i)
  {
    const ezPropertyAnimationTrack* pTrack = pProp->m_Tracks[i];

    if (pTrack->m_Target == ezPropertyAnimTarget::Color)
    {
      auto& anim = desc.m_ColorAnimations.ExpandAndGetRef();
      anim.m_sObjectSearchSequence = pTrack->m_sObjectSearchSequence;
      anim.m_sComponentType = pTrack->m_sComponentType;
      anim.m_sPropertyPath = pTrack->m_sPropertyPath;
      anim.m_Target = pTrack->m_Target;
      pTrack->m_ColorGradient.FillGradientData(anim.m_Gradient);
      anim.m_Gradient.SortControlPoints();
    }
    else
    {
      auto& anim = desc.m_FloatAnimations.ExpandAndGetRef();
      anim.m_sObjectSearchSequence = pTrack->m_sObjectSearchSequence;
      anim.m_sComponentType = pTrack->m_sComponentType;
      anim.m_sPropertyPath = pTrack->m_sPropertyPath;
      anim.m_Target = pTrack->m_Target;
      pTrack->m_FloatCurve.ConvertToRuntimeData(anim.m_Curve);
      anim.m_Curve.SortControlPoints();
      anim.m_Curve.ApplyTangentModes();
      anim.m_Curve.ClampTangents();
    }
  }

  // sort animation tracks by object path for better cache reuse at runtime
  {
    desc.m_FloatAnimations.Sort([](const ezFloatPropertyAnimEntry& lhs, const ezFloatPropertyAnimEntry& rhs) -> bool
    {
      const ezInt32 res = lhs.m_sObjectSearchSequence.Compare(rhs.m_sObjectSearchSequence);
      if (res < 0)
        return true;
      if (res > 0)
        return false;

      return lhs.m_sComponentType < rhs.m_sComponentType;
    });

    desc.m_ColorAnimations.Sort([](const ezColorPropertyAnimEntry& lhs, const ezColorPropertyAnimEntry& rhs) -> bool
    {
      const ezInt32 res = lhs.m_sObjectSearchSequence.Compare(rhs.m_sObjectSearchSequence);
      if (res < 0)
        return true;
      if (res > 0)
        return false;

      return lhs.m_sComponentType < rhs.m_sComponentType;
    });
  }

  desc.Save(stream);

  HandleContext.EndWritingToStream(&stream);

  return ezStatus(EZ_SUCCESS);
}


void ezPropertyAnimAssetDocument::InitializeAfterLoading()
{
  // Filter needs to be set before base class init as that one sends the doc.
  // (Local mirror ignores temporaries, i.e. only mirrors the asset itself)
  m_ObjectMirror.SetFilterFunction([this](const ezDocumentObject* pObject, const char* szProperty) -> bool
  {
    return !static_cast<ezPropertyAnimObjectManager*>(GetObjectManager())->IsTemporary(pObject, szProperty);
  });
  // (Remote IPC mirror only sends temporaries, i.e. the context)
  m_Mirror.SetFilterFunction([this](const ezDocumentObject* pObject, const char* szProperty) -> bool
  {
    return static_cast<ezPropertyAnimObjectManager*>(GetObjectManager())->IsTemporary(pObject, szProperty);
  });
  SUPER::InitializeAfterLoading();
  // Important to do these after base class init as we want our subscriptions to happen after the mirror of the base class.
  GetObjectManager()->m_StructureEvents.AddEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::TreeStructureEventHandler, this));
  GetObjectManager()->m_PropertyEvents.AddEventHandler(ezMakeDelegate(&ezPropertyAnimAssetDocument::TreePropertyEventHandler, this));
  // Subscribe here as otherwise base init will fire a context changed event when we are not set up yet.
  //RebuildMapping();
}

void ezPropertyAnimAssetDocument::GameObjectContextEventHandler(const ezGameObjectContextEvent& e)
{
  switch (e.m_Type)
  {
  case ezGameObjectContextEvent::Type::ContextAboutToBeChanged:
    static_cast<ezPropertyAnimObjectManager*>(GetObjectManager())->SetAllowStructureChangeOnTemporaries(true);
    break;
  case ezGameObjectContextEvent::Type::ContextChanged:
    static_cast<ezPropertyAnimObjectManager*>(GetObjectManager())->SetAllowStructureChangeOnTemporaries(false);
    RebuildMapping();
    break;
  }
}

void ezPropertyAnimAssetDocument::CommandHistoryEventHandler(const ezCommandHistoryEvent& e)
{
  switch (e.m_Type)
  {
  case ezCommandHistoryEvent::Type::UndoEnded:
  case ezCommandHistoryEvent::Type::RedoEnded:
    ClearCachedAnimationDuration();
    GetAnimationDurationTicks();
    break;
  }
}

void ezPropertyAnimAssetDocument::TreeStructureEventHandler(const ezDocumentObjectStructureEvent& e)
{
  auto pManager = static_cast<ezPropertyAnimObjectManager*>(GetObjectManager());
  if (e.m_pPreviousParent && pManager->IsTemporary(e.m_pPreviousParent, e.m_sParentProperty))
    return;
  if (e.m_pNewParent && pManager->IsTemporary(e.m_pNewParent, e.m_sParentProperty))
    return;

  if (e.m_pObject->GetType() == ezGetStaticRTTI<ezPropertyAnimationTrack>())
  {
    switch (e.m_EventType)
    {
    case ezDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case ezDocumentObjectStructureEvent::Type::AfterObjectMoved:
      AddTrack(e.m_pObject->GetGuid());
      return;
    case ezDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
    case ezDocumentObjectStructureEvent::Type::BeforeObjectMoved:
      RemoveTrack(e.m_pObject->GetGuid());
      return;
    }
  }
  else
  {
    ApplyAnimation();
  }
}

void ezPropertyAnimAssetDocument::TreePropertyEventHandler(const ezDocumentObjectPropertyEvent& e)
{
  auto pManager = static_cast<ezPropertyAnimObjectManager*>(GetObjectManager());
  if (pManager->IsTemporary(e.m_pObject))
    return;

  if (e.m_pObject->GetType() == ezGetStaticRTTI<ezPropertyAnimationTrack>())
  {
    if (e.m_EventType == ezDocumentObjectPropertyEvent::Type::PropertySet)
    {
      RemoveTrack(e.m_pObject->GetGuid());
      AddTrack(e.m_pObject->GetGuid());
      return;
    }
  }
  else
  {
    ApplyAnimation();
  }
}

void ezPropertyAnimAssetDocument::RebuildMapping()
{
  while (!m_TrackTable.IsEmpty())
  {
    RemoveTrack(m_TrackTable.GetIterator().Key());
  }
  EZ_ASSERT_DEBUG(m_PropertyTable.IsEmpty() && m_TrackTable.IsEmpty(), "All tracks should be removed.");

  const ezAbstractProperty* pTracksProp = ezGetStaticRTTI<ezPropertyAnimationTrackGroup>()->FindPropertyByName("Tracks");
  EZ_ASSERT_DEBUG(pTracksProp, "Name of property ezPropertyAnimationTrackGroup::m_Tracks has changed.");
  ezHybridArray<ezVariant, 16> values;
  m_pAccessor->GetValues(GetPropertyObject(), pTracksProp, values);
  for (const ezVariant& value : values)
  {
    AddTrack(value.Get<ezUuid>());
  }
}

void ezPropertyAnimAssetDocument::RemoveTrack(const ezUuid& track)
{
  auto& keys = *m_TrackTable.GetValue(track);
  for (const PropertyKey& key : keys)
  {
    PropertyValue& value = *m_PropertyTable.GetValue(key);
    value.m_Tracks.RemoveSwap(track);
    ApplyAnimation(key, value);
    if (value.m_Tracks.IsEmpty())
      m_PropertyTable.Remove(key);
  }
  m_TrackTable.Remove(track);
}

void ezPropertyAnimAssetDocument::AddTrack(const ezUuid& track)
{
  EZ_ASSERT_DEV(!m_TrackTable.Contains(track), "Track already exists.");
  auto& keys = m_TrackTable[track];
  const ezDocumentObject* pContext = GetContextObject();
  if (!pContext)
    return;

  {
    ezDocumentObjectVisitor visitor(GetObjectManager(), "Children", "TempObjects");
    auto pTrack = GetTrack(track);
    ezHybridArray<const ezDocumentObject*, 8> input;
    input.PushBack(pContext);
    ezHybridArray<const ezDocumentObject*, 8> output;

    // Find objects that match the search path
    ezStringBuilder sObjectSearchSequence = pTrack->m_sObjectSearchSequence;
    ezHybridArray<ezStringView, 4> names;
    sObjectSearchSequence.Split(false, names, "/");
    for (const ezStringView& sName : names)
    {
      for (const ezDocumentObject* pObj : input)
      {
        visitor.Visit(pContext, false, [&output, &sName](const ezDocumentObject* pObject) -> bool
        {
          const auto& sObjectName = pObject->GetTypeAccessor().GetValue("Name").Get<ezString>();
          if (sObjectName == sName)
          {
            output.PushBack(pObject);
            return false;
          }
          return true;
        });
      }
      input.Clear();
      input.Swap(output);
    }

    // Test found objects for component
    for (const ezDocumentObject* pObject : input)
    {
      // Could also be the root object in which case we found nothing.
      if (pObject->GetType() == ezGetStaticRTTI<ezGameObject>())
      {
        if (pTrack->m_sComponentType.IsEmpty())
        {
          // We are animating the game object directly
          output.PushBack(pObject);
        }
        else
        {
          const ezInt32 iComponents = pObject->GetTypeAccessor().GetCount("Components");
          for (ezInt32 i = 0; i < iComponents; i++)
          {
            ezVariant value = pObject->GetTypeAccessor().GetValue("Components", i);
            auto pChild = GetObjectManager()->GetObject(value.Get<ezUuid>());
            if (pTrack->m_sComponentType == pChild->GetType()->GetTypeName())
            {
              output.PushBack(pChild);
              continue; //#TODO: break on found component?
            }
          }
        }
      }
    }
    input.Clear();
    input.Swap(output);

    // Test found objects / components for property
    for (const ezDocumentObject* pObject : input)
    {
      if (ezAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(pTrack->m_sPropertyPath))
      {
        //#TODO: Support property path (sub-objects and indices into arrays / maps)
        PropertyKey key;
        key.m_Object = pObject->GetGuid();
        key.m_pProperty = pProp;
        keys.PushBack(key);
      }
    }
  }

  for (const PropertyKey& key : keys)
  {
    if (!m_PropertyTable.Contains(key))
    {
      PropertyValue value;
      EZ_VERIFY(m_pAccessor->GetValue(GetObjectManager()->GetObject(key.m_Object), key.m_pProperty,
        value.m_InitialValue, key.m_Index).Succeeded(), "Computed key invalid, does not resolve to a value.");
      m_PropertyTable.Insert(key, value);
    }

    PropertyValue& value = *m_PropertyTable.GetValue(key);
    value.m_Tracks.PushBack(track);
    ApplyAnimation(key, value);
  }
}

void ezPropertyAnimAssetDocument::ApplyAnimation()
{
  for (auto it = m_PropertyTable.GetIterator(); it.IsValid(); ++it)
  {
    ApplyAnimation(it.Key(), it.Value());
  }
}

void ezPropertyAnimAssetDocument::ApplyAnimation(const PropertyKey& key, const PropertyValue& value)
{
  ezVariant animValue = value.m_InitialValue;
  for (const ezUuid& track : value.m_Tracks)
  {
    auto pTrack = GetTrack(track);
    const ezRTTI* pPropRtti = key.m_pProperty->GetSpecificType();

    //#TODO apply pTrack to animValue
    switch (pTrack->m_Target)
    {
    case ezPropertyAnimTarget::Number:
      {
        if (pPropRtti->GetVariantType() >= ezVariantType::Bool && pPropRtti->GetVariantType() <= ezVariantType::Double)
        {
          ezVariant value = pTrack->m_FloatCurve.Evaluate(m_uiScrubberTickPos);
          animValue = value.ConvertTo(animValue.GetType());
        }
      }
      break;
    case ezPropertyAnimTarget::VectorX:
    case ezPropertyAnimTarget::VectorY:
    case ezPropertyAnimTarget::VectorZ:
    case ezPropertyAnimTarget::VectorW:
      {
        if (pPropRtti->GetVariantType() >= ezVariantType::Vector2 && pPropRtti->GetVariantType() <= ezVariantType::Vector4U)
        {
          double fValue = pTrack->m_FloatCurve.Evaluate(m_uiScrubberTickPos);
          EZ_CHECK_AT_COMPILETIME_MSG(ezPropertyAnimTarget::VectorX == 1, "Need to fix enum index code below");
          ezReflectionUtils::SetComponent(animValue, (ezUInt32)pTrack->m_Target - 1, fValue);
        }
      }
      break;
    case ezPropertyAnimTarget::Color:
      {
        if (pPropRtti->GetVariantType() == ezVariantType::Color || pPropRtti->GetVariantType() == ezVariantType::ColorGamma)
        {
          ezVariant value = pTrack->m_ColorGradient.Evaluate(m_uiScrubberTickPos);
          animValue = value.ConvertTo(animValue.GetType());
        }
      }
      break;
    }
  }
  ezDocumentObject* pObj = GetObjectManager()->GetObject(key.m_Object);
  ezVariant oldValue;
  EZ_VERIFY(m_pAccessor->GetValue(pObj, key.m_pProperty, oldValue, key.m_Index).Succeeded(), "Retrieving old value failed.");
  if (oldValue != animValue)
    GetObjectManager()->SetValue(pObj, key.m_pProperty->GetPropertyName(), animValue, key.m_Index);
}

void ezPropertyAnimAssetDocument::SetPlayAnimation(bool play)
{
  if (m_bPlayAnimation == play)
    return;

  if (m_uiScrubberTickPos >= GetAnimationDurationTicks())
    m_uiScrubberTickPos = 0;

  m_bPlayAnimation = play;
  if (!m_bPlayAnimation)
  {
    // During playback we do not round to frames, so we need to round it again on stop.
    SetScrubberPosition(GetScrubberPosition());
  }
  m_LastFrameTime = ezTime::Now();

  ezPropertyAnimAssetDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = ezPropertyAnimAssetDocumentEvent::Type::PlaybackChanged;
  m_PropertyAnimEvents.Broadcast(e);
}

void ezPropertyAnimAssetDocument::SetRepeatAnimation(bool repeat)
{
  if (m_bRepeatAnimation == repeat)
    return;

  m_bRepeatAnimation = repeat;

  ezPropertyAnimAssetDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = ezPropertyAnimAssetDocumentEvent::Type::PlaybackChanged;
  m_PropertyAnimEvents.Broadcast(e);
}

void ezPropertyAnimAssetDocument::ExecuteAnimationPlaybackStep()
{
  const ezTime currentTime = ezTime::Now();
  const ezTime tDiff = (currentTime - m_LastFrameTime) * GetSimulationSpeed();
  const ezUInt64 uiTicks = (ezUInt64)(tDiff.GetSeconds() * 4800.0);
  // Accumulate further if we render too fast and round ticks to zero.
  if (uiTicks == 0)
    return;

  m_LastFrameTime = currentTime;
  const ezUInt64 uiNewPos = GetScrubberPosition() + uiTicks;
  SetScrubberPosition(uiNewPos);

  if (uiNewPos > GetAnimationDurationTicks())
  {
    SetPlayAnimation(false);

    if (m_bRepeatAnimation)
      SetPlayAnimation(true);
  }
}

const ezPropertyAnimationTrack* ezPropertyAnimAssetDocument::GetTrack(const ezUuid& track) const
{
  return const_cast<ezPropertyAnimAssetDocument*>(this)->GetTrack(track);
}

ezPropertyAnimationTrack* ezPropertyAnimAssetDocument::GetTrack(const ezUuid& track)
{
  auto obj = m_Context.GetObjectByGUID(track);
  EZ_ASSERT_DEBUG(obj.m_pType == ezGetStaticRTTI<ezPropertyAnimationTrack>(), "Track guid does not resolve to a track, "
    "either the track is not yet created in the mirror or already destroyed. Make sure callbacks are executed in the right order.");
  auto pTrack = static_cast<ezPropertyAnimationTrack*>(obj.m_pObject);
  return pTrack;
}

ezUuid ezPropertyAnimAssetDocument::FindTrack(const ezDocumentObject* pObject, const ezAbstractProperty* pProp, ezVariant index, ezPropertyAnimTarget::Enum target) const
{
  PropertyKey key;
  key.m_Object = pObject->GetGuid();
  key.m_pProperty = pProp;
  key.m_Index = index;
  if (const PropertyValue* value = m_PropertyTable.GetValue(key))
  {
    for (const ezUuid& track : value->m_Tracks)
    {
      auto pTrack = GetTrack(track);
      if (pTrack->m_Target == target)
        return track;
    }
  }
  return ezUuid();
}

ezUuid ezPropertyAnimAssetDocument::CreateTrack(const ezDocumentObject* pObject, const ezAbstractProperty* pProp, ezVariant index, ezPropertyAnimTarget::Enum target)
{
  ezObjectCommandAccessor accessor(GetCommandHistory());
  const ezRTTI* pObjType = ezGetStaticRTTI<ezGameObject>();
  const ezAbstractProperty* pName = pObjType->FindPropertyByName("Name");
  ezStringBuilder sObjectSearchSequence;
  ezStringBuilder sComponentType;
  ezStringBuilder sPropertyPath = pProp->GetPropertyName();
  const ezDocumentObject* pObj = pObject;
  while (pObj != GetContextObject())
  {
    if (pObj->GetType() == ezGetStaticRTTI<ezGameObject>())
    {
      ezString sName = m_pAccessor->Get<ezString>(pObj, pName);
      if (!sName.IsEmpty())
      {
        if (!sObjectSearchSequence.IsEmpty())
          sObjectSearchSequence.Prepend("/");
        sObjectSearchSequence.Prepend(sName);
      }
    }
    else if (pObj->GetType()->IsDerivedFrom(ezGetStaticRTTI<ezComponent>()))
    {
      sComponentType = pObj->GetType()->GetTypeName();
    }
    else
    {
      if (!sPropertyPath.IsEmpty())
        sPropertyPath.Prepend("/");
      sPropertyPath.Prepend(pObj->GetParentPropertyType()->GetPropertyName());
    }
    pObj = pObj->GetParent();
  }

  const ezRTTI* pTrackType = ezGetStaticRTTI<ezPropertyAnimationTrack>();
  ezUuid newTrack;
  EZ_VERIFY(accessor.AddObject(GetPropertyObject(), ezGetStaticRTTI<ezPropertyAnimationTrackGroup>()->FindPropertyByName("Tracks"),
    -1, pTrackType, newTrack).Succeeded(), "Adding track failed.");
  const ezDocumentObject* pTrackObj = accessor.GetObject(newTrack);
  ezVariant value = sObjectSearchSequence.GetData();
  EZ_VERIFY(accessor.SetValue(pTrackObj, pTrackType->FindPropertyByName("ObjectPath"), value).Succeeded(), "Adding track failed.");
  value = sComponentType.GetData();
  EZ_VERIFY(accessor.SetValue(pTrackObj, pTrackType->FindPropertyByName("ComponentType"), value).Succeeded(), "Adding track failed.");
  value = sPropertyPath.GetData();
  EZ_VERIFY(accessor.SetValue(pTrackObj, pTrackType->FindPropertyByName("Property"), value).Succeeded(), "Adding track failed.");
  value = (int)target;
  EZ_VERIFY(accessor.SetValue(pTrackObj, pTrackType->FindPropertyByName("Target"), value).Succeeded(), "Adding track failed.");

  return newTrack;
}

ezUuid ezPropertyAnimAssetDocument::FindCurveCp(const ezUuid& trackGuid, ezInt64 tickX)
{
  auto pTrack = GetTrack(trackGuid);
  ezInt32 iIndex = -1;
  for (ezUInt32 i = 0; i < pTrack->m_FloatCurve.m_ControlPoints.GetCount(); i++)
  {
    if (pTrack->m_FloatCurve.m_ControlPoints[i].m_iTick == tickX)
    {
      iIndex = (ezInt32)i;
      break;
    }
  }
  if (iIndex == -1)
    return ezUuid();
  const ezAbstractProperty* pCurveProp = ezGetStaticRTTI<ezPropertyAnimationTrack>()->FindPropertyByName("FloatCurve");
  const ezDocumentObject* trackObject = GetObjectManager()->GetObject(trackGuid);
  ezUuid curveGuid = m_pAccessor->Get<ezUuid>(trackObject, pCurveProp);
  const ezAbstractProperty* pControlPointsProp = ezGetStaticRTTI<ezSingleCurveData>()->FindPropertyByName("ControlPoints");
  const ezDocumentObject* curveObject = GetObjectManager()->GetObject(curveGuid);
  ezUuid cpGuid = m_pAccessor->Get<ezUuid>(curveObject, pControlPointsProp, iIndex);
  return cpGuid;
}

ezUuid ezPropertyAnimAssetDocument::InsertCurveCpAt(const ezUuid& track, ezInt64 tickX, double newPosY)
{
  ezCommandHistory* history = GetCommandHistory();
  history->StartTransaction("Insert Control Point");

  const ezDocumentObject* trackObject = GetObjectManager()->GetObject(track);
  const ezVariant curveGuid = trackObject->GetTypeAccessor().GetValue("FloatCurve");

  ezAddObjectCommand cmdAdd;
  cmdAdd.m_Parent = curveGuid.Get<ezUuid>();
  cmdAdd.m_NewObjectGuid.CreateNewUuid();
  cmdAdd.m_sParentProperty = "ControlPoints";
  cmdAdd.m_pType = ezGetStaticRTTI<ezCurveControlPointData>();
  cmdAdd.m_Index = -1;

  history->AddCommand(cmdAdd);

  ezSetObjectPropertyCommand cmdSet;
  cmdSet.m_Object = cmdAdd.m_NewObjectGuid;

  cmdSet.m_sProperty = "Tick";
  cmdSet.m_NewValue = tickX;
  history->AddCommand(cmdSet);

  cmdSet.m_sProperty = "Value";
  cmdSet.m_NewValue = newPosY;
  history->AddCommand(cmdSet);

  cmdSet.m_sProperty = "LeftTangent";
  cmdSet.m_NewValue = ezVec2(-0.1f, 0.0f);
  history->AddCommand(cmdSet);

  cmdSet.m_sProperty = "RightTangent";
  cmdSet.m_NewValue = ezVec2(+0.1f, 0.0f);
  history->AddCommand(cmdSet);

  history->FinishTransaction();

  ClearCachedAnimationDuration();

  return cmdAdd.m_NewObjectGuid;
}

