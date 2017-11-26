#pragma once

#include <PhysXPlugin/Basics.h>
#include <Core/Messages/ScriptFunctionMessage.h>
#include <Core/World/World.h>
#include <GameEngine/Surfaces/SurfaceResource.h>

struct ezPhysicsHitResult;
typedef ezComponentManager<class ezPxRaycastInteractComponent, ezBlockStorageType::FreeList> ezPxRaycastInteractComponentManager;

struct EZ_PHYSXPLUGIN_DLL ezPxRaycastInteractComponent_Execute : public ezScriptFunctionMessage
{
  EZ_DECLARE_MESSAGE_TYPE(ezPxRaycastInteractComponent_Execute, ezScriptFunctionMessage);
};

class EZ_PHYSXPLUGIN_DLL ezPxRaycastInteractComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPxRaycastInteractComponent, ezComponent, ezPxRaycastInteractComponentManager);

public:
  ezPxRaycastInteractComponent();
  ~ezPxRaycastInteractComponent();

  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

  void Execute(ezPxRaycastInteractComponent_Execute& msg);

  // ************************************* PROPERTIES ***********************************

  ezUInt8 m_uiCollisionLayer = 0;
  float m_fMaxDistance = 1.0f;
  ezString m_sUserMessage;

protected:
  virtual void SendMessage(const ezPhysicsHitResult& hit);


};



