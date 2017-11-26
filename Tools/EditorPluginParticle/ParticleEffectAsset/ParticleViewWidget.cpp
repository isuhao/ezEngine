﻿#include <PCH.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleViewWidget.moc.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectAssetWindow.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <EditorFramework/InputContexts/OrbitCameraContext.h>
#include <QBoxLayout>

ezQtParticleViewWidget::ezQtParticleViewWidget(QWidget* pParent, ezQtParticleEffectAssetDocumentWindow* pOwnerWindow, ezEngineViewConfig* pViewConfig)
  : ezQtEngineViewWidget(pParent, pOwnerWindow, pViewConfig)
{
  setAcceptDrops(true);

  m_pOrbitCameraContext = EZ_DEFAULT_NEW(ezOrbitCameraContext, pOwnerWindow, this);
  m_pOrbitCameraContext->SetCamera(&m_pViewConfig->m_Camera);
  m_pOrbitCameraContext->SetOrbitVolume(ezVec3(0), ezVec3(5.0f), ezVec3(-2, 0, 0.5f));

  m_InputContexts.PushBack(m_pOrbitCameraContext.Borrow());
}

ezQtParticleViewWidget::~ezQtParticleViewWidget()
{
}
