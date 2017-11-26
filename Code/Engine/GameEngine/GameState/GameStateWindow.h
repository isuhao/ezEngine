﻿#pragma once

#include <GameEngine/Basics.h>
#include <GameEngine/Declarations.h>
#include <System/Window/Window.h>

/// \brief A window class that expands a little on ezWindow. Default type used by ezGameState to create a window.
class EZ_GAMEENGINE_DLL ezGameStateWindow : public ezWindow
{
public:
  ezGameStateWindow(const ezWindowCreationDesc& windowdesc);
  ~ezGameStateWindow();

private:
  virtual void OnResizeMessage(const ezSizeU32& newWindowSize) override;

};
