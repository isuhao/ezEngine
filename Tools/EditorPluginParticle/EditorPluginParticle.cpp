﻿#include <PCH.h>
#include <EditorPluginParticle/EditorPluginParticle.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>
#include <Foundation/Reflection/Reflection.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <EditorFramework/Actions/AssetActions.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <EditorFramework/Actions/ViewActions.h>
#include <EditorPluginParticle/Actions/ParticleActions.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>

void OnLoadPlugin(bool bReloading)
{
  ezQtEditorApp::GetSingleton()->AddRuntimePluginDependency("EditorPluginParticle", "ezParticlePlugin");
  ezQtEditorApp::GetSingleton()->AddRuntimePluginDependency("EditorPluginParticle", "ezEnginePluginParticle");

  ezParticleActions::RegisterActions();

  // Particle Effect Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("ParticleEffectAssetMenuBar");
      ezProjectActions::MapActions("ParticleEffectAssetMenuBar");
      ezStandardMenus::MapActions("ParticleEffectAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("ParticleEffectAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("ParticleEffectAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("ParticleEffectAssetToolBar");
      ezDocumentActions::MapActions("ParticleEffectAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("ParticleEffectAssetToolBar", "");
      ezAssetActions::MapActions("ParticleEffectAssetToolBar", true);
      ezParticleActions::MapActions("ParticleEffectAssetToolBar", "");
    }

    // View Tool Bar
    {
      ezActionMapManager::RegisterActionMap("ParticleEffectAssetViewToolBar");
      //ezViewActions::MapActions("ParticleEffectAssetViewToolBar", "", false, true, false);
    }
  }
}

void OnUnloadPlugin(bool bReloading)
{
}

ezPlugin g_Plugin(false, OnLoadPlugin, OnUnloadPlugin);

EZ_DYNAMIC_PLUGIN_IMPLEMENTATION(EZ_EDITORPLUGINPARTICLE_DLL, ezEditorPluginParticle);
