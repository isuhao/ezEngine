﻿#pragma once

#include <GameEngine/Basics.h>
#include <Foundation/Types/Bitflags.h>

class ezWindowBase;
class ezWorld;
class ezWorldModule;
class ezGameApplication;


/// \brief The type of application that is currently active
enum class ezGameApplicationType
{
  StandAlone,             ///< The application is stand-alone (e.g. ezPlayer or a custom game)
  EmbeddedInTool,         ///< The application is embedded into a tool, such as the editor
#ifdef BUILDSYSTEM_ENABLE_MIXEDREALITY_SUPPORT
  StandAloneMixedReality, ///< The application is stand-alone and when available would like to use mixed reality functionality.
  EmbeddedInToolMixedReality, ///< Only used by the EditorEngineProcess in remote mode
#endif
};


