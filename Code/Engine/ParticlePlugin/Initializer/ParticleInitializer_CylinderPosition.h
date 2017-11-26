﻿#pragma once

#include <ParticlePlugin/Initializer/ParticleInitializer.h>
#include <Foundation/Types/VarianceTypes.h>

class ezParticleInitializerFactory_CylinderPosition : public ezParticleInitializerFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleInitializerFactory_CylinderPosition, ezParticleInitializerFactory);

public:
  ezParticleInitializerFactory_CylinderPosition();

  virtual const ezRTTI* GetInitializerType() const override;
  virtual void CopyInitializerProperties(ezParticleInitializer* pInitializer) const override;

  virtual void Save(ezStreamWriter& stream) const override;
  virtual void Load(ezStreamReader& stream) override;

public:
  float m_fRadius;
  float m_fHeight;
  bool m_bSpawnOnSurface;
  bool m_bSetVelocity;
  ezVarianceTypeFloat m_Speed;
};


class EZ_PARTICLEPLUGIN_DLL ezParticleInitializer_CylinderPosition : public ezParticleInitializer
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleInitializer_CylinderPosition, ezParticleInitializer);

public:

  float m_fRadius;
  float m_fHeight;
  bool m_bSpawnOnSurface;
  bool m_bSetVelocity;
  ezVarianceTypeFloat m_Speed;

protected:
  virtual void CreateRequiredStreams() override;
  virtual void InitializeElements(ezUInt64 uiStartIndex, ezUInt64 uiNumElements) override;

  ezProcessingStream* m_pStreamPosition;
  ezProcessingStream* m_pStreamVelocity;
};
