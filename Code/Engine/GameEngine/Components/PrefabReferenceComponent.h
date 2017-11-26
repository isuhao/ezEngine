﻿#pragma once

#include <GameEngine/Basics.h>
#include <Core/World/World.h>
#include <Core/World/Component.h>
#include <GameEngine/Prefabs/PrefabResource.h>

class ezPrefabReferenceComponent;

class EZ_GAMEENGINE_DLL ezPrefabReferenceComponentManager : public ezComponentManager<ezPrefabReferenceComponent, ezBlockStorageType::Compact>
{
public:
  ezPrefabReferenceComponentManager(ezWorld* pWorld);
  ~ezPrefabReferenceComponentManager();

  virtual void Initialize() override;

  void Update(const ezWorldModule::UpdateContext& context);

  void AddToUpdateList(ezPrefabReferenceComponent* pComponent);

private:
  void ResourceEventHandler(const ezResourceEvent& e);

  ezDeque<ezComponentHandle> m_PrefabComponentsToUpdate;
};

class EZ_GAMEENGINE_DLL ezPrefabReferenceComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPrefabReferenceComponent, ezComponent, ezPrefabReferenceComponentManager);

public:
  ezPrefabReferenceComponent();
  ~ezPrefabReferenceComponent();

  virtual void Deinitialize() override;

  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

  // ************************************* PROPERTIES ***********************************

  void SetPrefabFile(const char* szFile);
  const char* GetPrefabFile() const;

  void SetPrefab(const ezPrefabResourceHandle& hPrefab);
  EZ_ALWAYS_INLINE const ezPrefabResourceHandle& GetPrefab() const { return m_hPrefab; }

  void InstantiatePrefab();

protected:

  // ************************************* FUNCTIONS *****************************

private:
  void ResourceEventHandler(const ezResourceEvent& e);

  ezPrefabResourceHandle m_hPrefab;
  bool m_bRequiresInstantiation;
};
