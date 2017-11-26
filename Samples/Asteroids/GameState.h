#pragma once

#include "Level.h"
#include <Core/Input/VirtualThumbStick.h>
#include <GameEngine/GameState/GameState.h>

class AsteroidGameState : public ezGameState
{
  EZ_ADD_DYNAMIC_REFLECTION(AsteroidGameState, ezGameState);

public:
  AsteroidGameState();

  virtual void ProcessInput() override;

protected:
  virtual void ConfigureInputActions() override;

private:
  virtual void OnActivation(ezWorld* pWorld) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;

  void CreateGameLevel();
  void DestroyLevel();

  virtual float CanHandleThis(ezGameApplicationType AppType, ezWorld* pWorld) const override;

  Level* m_pLevel;
};
