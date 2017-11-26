#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <Foundation/Strings/String.h>

class ezPhysicsWorldModuleInterface;

struct EZ_PARTICLEPLUGIN_DLL ezParticleRaycastHitReaction
{
  typedef ezUInt8 StorageType;

  enum Enum
  {
    Bounce,
    Die,

    Default = Bounce
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleRaycastHitReaction);

class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_Raycast : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_Raycast, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_Raycast();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject) const override;

  virtual void Save(ezStreamWriter& stream) const override;
  virtual void Load(ezStreamReader& stream) override;

  ezEnum<ezParticleRaycastHitReaction> m_Reaction;
  ezUInt8 m_uiCollisionLayer;
  ezString m_sOnCollideEvent;

  /// \todo On hit something: bounce, stick, die
  /// \todo Collision Filter
};


class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Raycast : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Raycast, ezParticleBehavior);

public:

  virtual void AfterPropertiesConfigured(bool bFirstTime) override;
  virtual void CreateRequiredStreams() override;

  ezEnum<ezParticleRaycastHitReaction> m_Reaction;
  ezUInt8 m_uiCollisionLayer;
  ezTempHashedString m_sOnCollideEvent;

protected:
  virtual void Process(ezUInt64 uiNumElements) override;

  ezPhysicsWorldModuleInterface* m_pPhysicsModule;

  ezProcessingStream* m_pStreamPosition;
  ezProcessingStream* m_pStreamLastPosition;
  ezProcessingStream* m_pStreamVelocity;
};

