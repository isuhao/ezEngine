﻿#include <PCH.h>

#ifdef EZ_SUPPORT_EZARCHIVE

ezArchiveReader::ezArchiveReader(ezStreamReader& stream) : ezChunkStreamReader(stream), m_InputStream(stream)
{
  m_bStreamFinished = true;
  m_pLog = nullptr;
}

ezArchiveReader::~ezArchiveReader()
{
  EZ_ASSERT_DEV(m_bStreamFinished, "The stream is left in an invalid state. EndStream() has to be called before it may be destroyed.");
}

void ezArchiveReader::RegisterTypeSerializer(const ezRTTI* pRttiBase, ezArchiveSerializer* pSerializer)
{
  EZ_ASSERT_DEV(m_bStreamFinished, "Cannot change the registered type serializers while reading the archive.");

  m_TypeSerializers[pRttiBase] = pSerializer;
}

ezUInt32 ezArchiveReader::GetStoredTypeVersion(const ezRTTI* pRtti) const
{
  return m_StoredVersion.GetValueOrDefault(pRtti, 0xFFFFFFFF);
}

ezArchiveSerializer* ezArchiveReader::GetTypeSerializer(const ezRTTI* pRttiBase)
{
  bool bExisted = false;
  auto it = m_TypeSerializers.FindOrAdd(pRttiBase, &bExisted); // this will insert nullptr if not found

  if (bExisted)
    return it.Value();

  // if we have a base type use that serializer, if not, nullptr will stay in there
  if (pRttiBase->GetParentType() != nullptr)
  {
    it.Value() = GetTypeSerializer(pRttiBase->GetParentType());
  }

  return it.Value();
}

void ezArchiveReader::SetObjectReferenceMapping(ezUInt32 uiObjectID, void* pNewObject)
{
  SetObjectReferenceMapping(uiObjectID, pNewObject, 0xFFFFFFFF);
}

void ezArchiveReader::SetObjectReferenceMapping(ezUInt32 uiObjectID, void* pNewObject, ezUInt32 uiDataSize)
{
  m_ObjectIDtoPointer[uiObjectID].m_pObject = pNewObject;
  m_ObjectIDtoPointer[uiObjectID].m_uiDataSize = uiDataSize;
}

void* ezArchiveReader::GetObjectReferenceMapping(ezUInt32 uiObjectID) const
{
  return m_ObjectIDtoPointer[uiObjectID].m_pObject;
}



ezUInt32 ezArchiveReader::ReadObjectReference(void** ppReference)
{
  EZ_ASSERT_DEV(!m_bStreamFinished, "The stream is in an invalid state.");

  ObjectMapping om;
  om.m_pObjectWaiting = ppReference;

  *this >> om.m_ObjectID;

  if (ppReference == nullptr)
    return om.m_ObjectID;

  *ppReference = nullptr;

#if EZ_DISABLED(EZ_COMPILE_FOR_DEBUG)
  {
    // in debug mode we do not do this shortcut, to be able to find code that uses the reference before
    // it is 'officially' available

    // if the mapping is already available, do not queue it

    void* pMapped = m_ObjectIDtoPointer[om.m_ObjectID].m_pObject;

    if (pMapped != nullptr)
    {
      *ppReference = pMapped;
      return om.m_ObjectID;
    }
  }
#endif

  m_ObjectsToMap.PushBack(om);

  return om.m_ObjectID;
}

void ezArchiveReader::FinishReferenceMapping()
{
  ezUInt32 uiRestored = 0;

  for (ezUInt32 i = 0; i < m_ObjectsToMap.GetCount(); ++i)
  {
    ObjectMapping& om = m_ObjectsToMap[i];

    *om.m_pObjectWaiting = m_ObjectIDtoPointer[om.m_ObjectID].m_pObject; // the pointer might still be nullptr, we don't care

    if (m_ObjectIDtoPointer[om.m_ObjectID].m_pObject != nullptr)
      ++uiRestored;
  }

  if (uiRestored > 0)
    ezLog::Success(m_pLog, "References restored: {0} of {1}", uiRestored, m_ObjectsToMap.GetCount());

  if (m_ObjectsToMap.GetCount() != uiRestored)
    ezLog::Warning(m_pLog, "References remaining invalid: {0} of {1}", m_ObjectsToMap.GetCount() - uiRestored, m_ObjectsToMap.GetCount());

  m_ObjectsToMap.Clear();
}

void ezArchiveReader::CallOnDeserialized()
{
  EZ_ASSERT_DEV(m_bStreamFinished, "This function may only be called after reading from the archive has finished, ie. after EndStream() has been called.");

  for (ezUInt32 i = 0; i < m_DeserializedReflected.GetCount(); ++i)
  {
    m_DeserializedReflected[i]->OnDeserialized();
  }
}

void ezArchiveReader::BeginStream()
{
  EZ_ASSERT_DEV(m_bStreamFinished, "The stream is in an invalid state.");

  m_bStreamFinished = false;

  ezString sArchiveTag;
  m_InputStream >> sArchiveTag;

  EZ_ASSERT_DEV(sArchiveTag == "ezArchive", "The given file is either not an ezArchive, or it has been corrupted.");

  ezUInt8 uiVersion = 0;
  m_InputStream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= ezArchiveVersion::CurrentVersion, "The ezArchive version {0} is not supported, expected version {1} or lower.", uiVersion, ezArchiveVersion::CurrentVersion);

  ezUInt32 uiMaxReferences = 0;
  m_InputStream >> uiMaxReferences;

  ezLog::Info(m_pLog, "Archive Version: {0}, Max references: {1}", uiVersion, uiMaxReferences);

  m_ObjectIDtoPointer.SetCount(uiMaxReferences);

  // Read the RTTI information
  {
    ezUInt32 uiRttiCount = 0;
    m_InputStream >> uiRttiCount;

    ezLog::Info(m_pLog, "Types: {0}", uiRttiCount);

    m_Types.Reserve(uiRttiCount);

    for (ezUInt32 i = 0; i < uiRttiCount; ++i)
    {
      RttiData ri;
      m_InputStream >> ri.m_sTypeName;
      m_InputStream >> ri.m_uiTypeVersion;
      m_InputStream >> ri.m_uiObjectCount;

      ri.m_pRTTI = ezRTTI::FindTypeByName(ri.m_sTypeName.GetData());

      m_Types.PushBack(ri);

      ezLog::Dev(m_pLog, "Type {0}: '{1}', Version = {2}, Object Count = {3}", i, ri.m_sTypeName, ri.m_uiTypeVersion, ri.m_uiObjectCount);

      if (ri.m_pRTTI == nullptr)
      {
        ezLog::SeriousWarning(m_pLog, "Stored type '{0}' is unknown. Objects of this type will be skipped during deserialization.", ri.m_sTypeName);
      }
      else
      {
        m_StoredVersion[ri.m_pRTTI] = ri.m_uiTypeVersion;
      }
    }
  }

  ezUInt32 uiChunkFileSize = 0;
  m_InputStream >> uiChunkFileSize;

  ezLog::Dev(m_pLog, "Chunk file Size: {0} Bytes", uiChunkFileSize);

  // now the input stream is at the proper position for the chunk file to take over
  ezChunkStreamReader::BeginStream();
}

void ezArchiveReader::EndStream()
{
  EZ_ASSERT_DEV(!m_bStreamFinished, "The stream is in an invalid state.");

  ezChunkStreamReader::EndStream();

  // fix up all waiting object references
  FinishReferenceMapping();

  m_bStreamFinished = true;
}

ezReflectedClass* ezArchiveReader::ReadReflectedObject()
{
  void* pObject = ReadTypedObject();

  if (pObject == nullptr)
    return nullptr;

  ezReflectedClass* pReflected = (ezReflectedClass*) pObject;

  EZ_ASSERT_DEBUG(pReflected->GetDynamicRTTI()->IsDerivedFrom<ezReflectedClass>(), "Invalid type was created during deserialization.");

  m_DeserializedReflected.PushBack(pReflected);

  pReflected->Deserialize(*this);

  return pReflected;
}

void* ezArchiveReader::ReadTypedObject(const ezRTTI** out_pRtti, ezUInt16* out_pTypeID)
{
  EZ_ASSERT_DEV(!m_bStreamFinished, "The stream is in an invalid state.");

  // read the object's type ID
  ezUInt16 uiTypeID = 0;
  *this >> uiTypeID;

  if (out_pTypeID)
    *out_pTypeID = uiTypeID;

  // read the object's reference ID, if this is known already, the object has been read before
  ezUInt32 uiObjectID = 0;
  *this >> uiObjectID;

  // read the size of the object's serialized data, can be used to skip the object
  ezUInt32 uiObjectDataSize = 0;
  *this >> uiObjectDataSize;

  void* pObject = GetObjectReferenceMapping(uiObjectID);

  // object is not yet known, try to create it
  const ezRTTI* pRtti = m_Types[uiTypeID].m_pRTTI;

  if (out_pRtti)
    *out_pRtti = pRtti;

  ezLog::Debug(m_pLog, "Reading object of Type '{0}' (v{1}), ID = {2}, Size = {3}", m_Types[uiTypeID].m_sTypeName, m_Types[uiTypeID].m_uiTypeVersion, uiObjectID, uiObjectDataSize);

  if (pObject != nullptr)
  {
    ezLog::Warning(m_pLog, "Duplicate object found, skipping object.");

    const ezUInt32 uiPrevSize = m_ObjectIDtoPointer[uiObjectID].m_uiDataSize;

    if (uiPrevSize != 0xFFFFFFFF && uiPrevSize != uiObjectDataSize)
    {
      ezLog::Error(m_pLog, "Duplicate object size mismatch. Originally deserialized size was {0} bytes, duplicate's size is {1} bytes", uiPrevSize, uiObjectDataSize);
    }

    // object already known and deserialized, do not read it a second time
    SkipBytes(uiObjectDataSize);

    return pObject;
  }

  if (pRtti != nullptr)
  {
    ezArchiveSerializer* pSerializer = GetTypeSerializer(pRtti);

    if (pSerializer)
    {
      pObject = pSerializer->Deserialize(*this, pRtti);
    }
    else
    {
      if (pRtti->GetAllocator()->CanAllocate())
        pObject = pRtti->GetAllocator()->Allocate();
    }

    // it is valid for pObject to remain nullptr

    SetObjectReferenceMapping(uiObjectID, pObject, uiObjectDataSize);
  }

  // if the type could not be created, just skip the rest of the object
  if (pObject == nullptr)
  {
    ezLog::SeriousWarning(m_pLog, "Object of Type '{0}' could not be created, skipping object.", m_Types[uiTypeID].m_sTypeName);

    SkipBytes(uiObjectDataSize);
  }


  return pObject;
}


#endif


EZ_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_ArchiveReader);

