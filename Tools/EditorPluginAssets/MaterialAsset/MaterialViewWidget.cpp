﻿#include <PCH.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorPluginAssets/MaterialAsset/MaterialViewWidget.moc.h>
#include <EditorPluginAssets/MaterialAsset/MaterialAssetWindow.moc.h>
#include <EditorFramework/InputContexts/OrbitCameraContext.h>

ezQtMaterialViewWidget::ezQtMaterialViewWidget(QWidget* pParent, ezQtMaterialAssetDocumentWindow* pOwnerWindow, ezEngineViewConfig* pViewConfig)
  : ezQtEngineViewWidget(pParent, pOwnerWindow, pViewConfig)
{
  setAcceptDrops(true);

  m_pOrbitCameraContext = EZ_DEFAULT_NEW(ezOrbitCameraContext, pOwnerWindow, this);
  m_pOrbitCameraContext->SetCamera(&m_pViewConfig->m_Camera);
  m_pOrbitCameraContext->SetOrbitVolume(ezVec3(0), ezVec3(0.0f), ezVec3(-0.2, 0, 0));

  m_InputContexts.PushBack(m_pOrbitCameraContext.Borrow());
}

ezQtMaterialViewWidget::~ezQtMaterialViewWidget()
{
}
