#include <PCH.h>
#include <ParticlePlugin/Initializer/ParticleInitializer_BoxPosition.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>
#include <Foundation/Math/Random.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <Foundation/SimdMath/SimdTransform.h>
#include <Foundation/Profiling/Profiling.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleInitializerFactory_BoxPosition, 1, ezRTTIDefaultAllocator<ezParticleInitializerFactory_BoxPosition>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Size", m_vSize)->AddAttributes(new ezDefaultValueAttribute(ezVec3(0, 0, 0))),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleInitializer_BoxPosition, 1, ezRTTIDefaultAllocator<ezParticleInitializer_BoxPosition>)
EZ_END_DYNAMIC_REFLECTED_TYPE

ezParticleInitializerFactory_BoxPosition::ezParticleInitializerFactory_BoxPosition()
{
  m_vSize.Set(0, 0, 0);
}

const ezRTTI* ezParticleInitializerFactory_BoxPosition::GetInitializerType() const
{
  return ezGetStaticRTTI<ezParticleInitializer_BoxPosition>();
}

void ezParticleInitializerFactory_BoxPosition::CopyInitializerProperties(ezParticleInitializer* pInitializer0) const
{
  ezParticleInitializer_BoxPosition* pInitializer = static_cast<ezParticleInitializer_BoxPosition*>(pInitializer0);

  pInitializer->m_vSize = m_vSize.CompMax(ezVec3::ZeroVector());
}

void ezParticleInitializerFactory_BoxPosition::Save(ezStreamWriter& stream) const
{
  const ezUInt8 uiVersion = 1;
  stream << uiVersion;

  stream << m_vSize;
}

void ezParticleInitializerFactory_BoxPosition::Load(ezStreamReader& stream)
{
  ezUInt8 uiVersion = 0;
  stream >> uiVersion;

  stream >> m_vSize;
}


void ezParticleInitializer_BoxPosition::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, true);
}

void ezParticleInitializer_BoxPosition::InitializeElements(ezUInt64 uiStartIndex, ezUInt64 uiNumElements)
{
  EZ_PROFILE("PFX: Box Position");

  ezSimdVec4f* pPosition = m_pStreamPosition->GetWritableData<ezSimdVec4f>();

  ezRandom& rng = GetRNG();

  if (m_vSize.IsZero())
  {
    ezSimdVec4f pos;
    pos.Load<3>(&GetOwnerSystem()->GetTransform().m_vPosition.x);

    for (ezUInt64 i = uiStartIndex; i < uiStartIndex + uiNumElements; ++i)
    {
      pPosition[i] = pos;
    }
  }
  else
  {
    ezTransform ownerTransform = GetOwnerSystem()->GetTransform();

    ezSimdVec4f pos;
    ezSimdTransform transform;
    transform.m_Position.Load<3>(&ownerTransform.m_vPosition.x);
    transform.m_Rotation.m_v.Load<4>(&ownerTransform.m_qRotation.v.x);
    transform.m_Scale.Load<3>(&ownerTransform.m_vScale.x);

    float p0[4];
    p0[3] = 0;

    for (ezUInt64 i = uiStartIndex; i < uiStartIndex + uiNumElements; ++i)
    {
      p0[0] = (float)(rng.DoubleMinMax(-m_vSize.x, m_vSize.x) * 0.5);
      p0[1] = (float)(rng.DoubleMinMax(-m_vSize.y, m_vSize.y) * 0.5);
      p0[2] = (float)(rng.DoubleMinMax(-m_vSize.z, m_vSize.z) * 0.5);

      pos.Load<4>(p0);

      pPosition[i] = transform.TransformPosition(pos);
    }
  }
}



EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Initializer_ParticleInitializer_BoxPosition);

