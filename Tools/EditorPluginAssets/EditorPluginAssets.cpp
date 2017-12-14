﻿#include <PCH.h>
#include <EditorPluginAssets/EditorPluginAssets.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetObjects.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>
#include <Foundation/Reflection/Reflection.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <EditorFramework/Actions/AssetActions.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <EditorPluginAssets/MeshAsset/MeshAssetObjects.h>
#include <EditorPluginAssets/Actions/EditorPluginAssetsActions.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <EditorFramework/Actions/ViewActions.h>
#include <EditorPluginAssets/MaterialAsset/MaterialAsset.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <EditorPluginAssets/VisualShader/VisualShaderActions.h>
#include <EditorPluginAssets/TextureAsset/TextureAssetWindow.moc.h>
#include <GuiFoundation/Action/EditActions.h>
#include <EditorPluginAssets/TextureCubeAsset/TextureCubeAssetObjects.h>
#include <EditorPluginAssets/TextureCubeAsset/TextureCubeAssetWindow.moc.h>
#include <EditorPluginAssets/VisualScriptAsset/VisualScriptActions.h>
#include <EditorPluginAssets/DecalAsset/DecalAsset.h>
#include <EditorFramework/Actions/GameObjectContextActions.h>
#include <EditorFramework/Actions/GameObjectDocumentActions.h>
#include <EditorFramework/Actions/QuadViewActions.h>
#include <EditorFramework/Actions/GameObjectSelectionActions.h>
#include <EditorFramework/Actions/TransformGizmoActions.h>

void OnLoadPlugin(bool bReloading)
{
  ezQtEditorApp::GetSingleton()->AddRuntimePluginDependency("EditorPluginAssets", "ezEnginePluginAssets");

  // Texture2D Asset
  {
    ezPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(ezTextureAssetProperties::PropertyMetaStateEventHandler);

    ezTextureAssetActions::RegisterActions();

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("TextureAssetMenuBar");
      ezProjectActions::MapActions("TextureAssetMenuBar");
      ezStandardMenus::MapActions("TextureAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("TextureAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("TextureAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("TextureAssetToolBar");
      ezDocumentActions::MapActions("TextureAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("TextureAssetToolBar", "");
      ezAssetActions::MapActions("TextureAssetToolBar", true);
      ezTextureAssetActions::MapActions("TextureAssetToolBar", "");
    }
  }

  // TextureCube Asset
  {
    ezPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(ezTextureCubeAssetProperties::PropertyMetaStateEventHandler);

    ezTextureCubeAssetActions::RegisterActions();

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("TextureCubeAssetMenuBar");
      ezProjectActions::MapActions("TextureCubeAssetMenuBar");
      ezStandardMenus::MapActions("TextureCubeAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("TextureCubeAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("TextureCubeAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("TextureCubeAssetToolBar");
      ezDocumentActions::MapActions("TextureCubeAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("TextureCubeAssetToolBar", "");
      ezAssetActions::MapActions("TextureCubeAssetToolBar", true);
      ezTextureAssetActions::MapActions("TextureCubeAssetToolBar", "");
    }
  }

  // Material Asset
  {
    ezPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(ezMaterialAssetProperties::PropertyMetaStateEventHandler);

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("MaterialAssetMenuBar");
      ezProjectActions::MapActions("MaterialAssetMenuBar");
      ezStandardMenus::MapActions("MaterialAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("MaterialAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("MaterialAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("MaterialAssetMenuBar", "Menu.Edit");
      ezEditActions::MapActions("MaterialAssetMenuBar", "Menu.Edit", false, false);
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("MaterialAssetToolBar");
      ezDocumentActions::MapActions("MaterialAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("MaterialAssetToolBar", "");
      ezAssetActions::MapActions("MaterialAssetToolBar", true);

      ezVisualShaderActions::RegisterActions();
      ezVisualShaderActions::MapActions("MaterialAssetToolBar");
    }

    // View Tool Bar
    {
      ezActionMapManager::RegisterActionMap("MaterialAssetViewToolBar");
      ezViewActions::MapActions("MaterialAssetViewToolBar", "", ezViewActions::RenderMode | ezViewActions::ActivateRemoteProcess);
    }
  }

  // Render Pipeline Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("RenderPipelineAssetMenuBar");
      ezProjectActions::MapActions("RenderPipelineAssetMenuBar");
      ezStandardMenus::MapActions("RenderPipelineAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("RenderPipelineAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("RenderPipelineAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("RenderPipelineAssetToolBar");
      ezDocumentActions::MapActions("RenderPipelineAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("RenderPipelineAssetToolBar", "");
      ezAssetActions::MapActions("RenderPipelineAssetToolBar", true);
    }
  }

  // Mesh Asset
  {
    ezPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(ezMeshAssetProperties::PropertyMetaStateEventHandler);

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("MeshAssetMenuBar");
      ezProjectActions::MapActions("MeshAssetMenuBar");
      ezStandardMenus::MapActions("MeshAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("MeshAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("MeshAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("MeshAssetToolBar");
      ezDocumentActions::MapActions("MeshAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("MeshAssetToolBar", "");
      ezAssetActions::MapActions("MeshAssetToolBar", true);
    }

    // View Tool Bar
    {
      ezActionMapManager::RegisterActionMap("MeshAssetViewToolBar");
      ezViewActions::MapActions("MeshAssetViewToolBar", "", ezViewActions::RenderMode | ezViewActions::ActivateRemoteProcess);
    }
  }

  // Surface Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("SurfaceAssetMenuBar");
      ezProjectActions::MapActions("SurfaceAssetMenuBar");
      ezStandardMenus::MapActions("SurfaceAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("SurfaceAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("SurfaceAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("SurfaceAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("SurfaceAssetToolBar");
      ezDocumentActions::MapActions("SurfaceAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("SurfaceAssetToolBar", "");
      ezAssetActions::MapActions("SurfaceAssetToolBar", true);
    }
  }

  // Collection Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("CollectionAssetMenuBar");
      ezProjectActions::MapActions("CollectionAssetMenuBar");
      ezStandardMenus::MapActions("CollectionAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("CollectionAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("CollectionAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("CollectionAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("CollectionAssetToolBar");
      ezDocumentActions::MapActions("CollectionAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("CollectionAssetToolBar", "");
      ezAssetActions::MapActions("CollectionAssetToolBar", true);
    }
  }

  // ColorGradient Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("ColorGradientAssetMenuBar");
      ezProjectActions::MapActions("ColorGradientAssetMenuBar");
      ezStandardMenus::MapActions("ColorGradientAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("ColorGradientAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("ColorGradientAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("ColorGradientAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("ColorGradientAssetToolBar");
      ezDocumentActions::MapActions("ColorGradientAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("ColorGradientAssetToolBar", "");
      ezAssetActions::MapActions("ColorGradientAssetToolBar", true);
    }
  }

  // Curve1D Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("Curve1DAssetMenuBar");
      ezProjectActions::MapActions("Curve1DAssetMenuBar");
      ezStandardMenus::MapActions("Curve1DAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("Curve1DAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("Curve1DAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("Curve1DAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("Curve1DAssetToolBar");
      ezDocumentActions::MapActions("Curve1DAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("Curve1DAssetToolBar", "");
      ezAssetActions::MapActions("Curve1DAssetToolBar", true);
    }
  }

  // PropertyAnim Asset
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("PropertyAnimAssetMenuBar");
      ezProjectActions::MapActions("PropertyAnimAssetMenuBar");
      ezStandardMenus::MapActions("PropertyAnimAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Scene | ezStandardMenuTypes::View | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("PropertyAnimAssetMenuBar", "Menu.File", false);
      ezDocumentActions::MapToolsActions("PropertyAnimAssetMenuBar", "Menu.Tools");
      ezCommandHistoryActions::MapActions("PropertyAnimAssetMenuBar", "Menu.Edit");
      ezGameObjectSelectionActions::MapActions("PropertyAnimAssetMenuBar", "Menu.Edit");
      ezGameObjectDocumentActions::MapMenuActions("PropertyAnimAssetMenuBar", "Menu.View");
      ezGameObjectDocumentActions::MapMenuSimulationSpeed("PropertyAnimAssetMenuBar", "Menu.Scene");
      ezTransformGizmoActions::MapMenuActions("PropertyAnimAssetMenuBar", "Menu.Edit");
      ezRotateGizmoAction::MapActions("PropertyAnimAssetMenuBar", "Menu.Edit/Gizmo.Menu");
      ezScaleGizmoAction::MapActions("PropertyAnimAssetMenuBar", "Menu.Edit/Gizmo.Menu");
      ezTranslateGizmoAction::MapActions("PropertyAnimAssetMenuBar", "Menu.Edit/Gizmo.Menu");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("PropertyAnimAssetToolBar");
      ezDocumentActions::MapActions("PropertyAnimAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("PropertyAnimAssetToolBar", "");
      ezAssetActions::MapActions("PropertyAnimAssetToolBar", true);
      ezGameObjectContextActions::MapToolbarActions("PropertyAnimAssetToolBar", "");
      ezGameObjectDocumentActions::MapToolbarActions("PropertyAnimAssetToolBar", "");
      ezTransformGizmoActions::MapToolbarActions("PropertyAnimAssetToolBar", "");
      ezRotateGizmoAction::MapActions("PropertyAnimAssetToolBar", "");
      ezScaleGizmoAction::MapActions("PropertyAnimAssetToolBar", "");
      ezTranslateGizmoAction::MapActions("PropertyAnimAssetToolBar", "");
    }

    // View Tool Bar
    {
      ezActionMapManager::RegisterActionMap("PropertyAnimAssetViewToolBar");
      ezViewActions::MapActions("PropertyAnimAssetViewToolBar", "", ezViewActions::PerspectiveMode | ezViewActions::RenderMode | ezViewActions::UsageHint | ezViewActions::ActivateRemoteProcess);
      ezQuadViewActions::MapActions("PropertyAnimAssetViewToolBar", "");
    }

    // SceneGraph Context Menu
    {
      ezActionMapManager::RegisterActionMap("PropertyAnimAsset_ScenegraphContextMenu");
      ezGameObjectSelectionActions::MapContextMenuActions("PropertyAnimAsset_ScenegraphContextMenu", "");
      ezGameObjectContextActions::MapContextMenuActions("PropertyAnimAsset_ScenegraphContextMenu", "");
    }
  }

  // Visual Script Asset
  {
    ezVisualScriptActions::RegisterActions();

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("VisualScriptAssetMenuBar");
      ezProjectActions::MapActions("VisualScriptAssetMenuBar");
      ezStandardMenus::MapActions("VisualScriptAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("VisualScriptAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("VisualScriptAssetMenuBar", "Menu.Edit");
      ezEditActions::MapActions("VisualScriptAssetMenuBar", "Menu.Edit", false, false);
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("VisualScriptAssetToolBar");
      ezDocumentActions::MapActions("VisualScriptAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("VisualScriptAssetToolBar", "");
      ezAssetActions::MapActions("VisualScriptAssetToolBar", true);
      ezVisualScriptActions::MapActions("VisualScriptAssetToolBar", "");
    }
  }

  // Decal Asset
  {
    ezPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(ezDecalAssetProperties::PropertyMetaStateEventHandler);

    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("DecalAssetMenuBar");
      ezProjectActions::MapActions("DecalAssetMenuBar");
      ezStandardMenus::MapActions("DecalAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
      ezDocumentActions::MapActions("DecalAssetMenuBar", "Menu.File", false);
      ezCommandHistoryActions::MapActions("DecalAssetMenuBar", "Menu.Edit");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("DecalAssetToolBar");
      ezDocumentActions::MapActions("DecalAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("DecalAssetToolBar", "");
      ezAssetActions::MapActions("DecalAssetToolBar", true);
    }

    // View Tool Bar
    {
      ezActionMapManager::RegisterActionMap("DecalAssetViewToolBar");
      ezViewActions::MapActions("DecalAssetViewToolBar", "", ezViewActions::RenderMode | ezViewActions::ActivateRemoteProcess);
    }
  }

  // General editor plugin actions
  {
    ezAssetPluginActions::RegisterActions();
    ezAssetPluginActions::MapActions("SettingsTabMenuBar");
  }
}

void OnUnloadPlugin(bool bReloading)
{
  ezAssetPluginActions::UnregisterActions();

  ezPropertyMetaState::GetSingleton()->m_Events.RemoveEventHandler(ezMeshAssetProperties::PropertyMetaStateEventHandler);
  ezPropertyMetaState::GetSingleton()->m_Events.RemoveEventHandler(ezTextureAssetProperties::PropertyMetaStateEventHandler);
  ezPropertyMetaState::GetSingleton()->m_Events.RemoveEventHandler(ezDecalAssetProperties::PropertyMetaStateEventHandler);
}

ezPlugin g_Plugin(false, OnLoadPlugin, OnUnloadPlugin);

EZ_DYNAMIC_PLUGIN_IMPLEMENTATION(EZ_EDITORPLUGINASSETS_DLL, ezEditorPluginAssets);
