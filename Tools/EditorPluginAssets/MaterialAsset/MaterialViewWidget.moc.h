﻿#pragma once

#include <Foundation/Basics.h>
#include <EditorFramework/DocumentWindow/EngineViewWidget.moc.h>
#include <Foundation/Types/UniquePtr.h>

class ezQtMaterialAssetDocumentWindow;
class ezOrbitCameraContext;

class ezQtMaterialViewWidget : public ezQtEngineViewWidget
{
  Q_OBJECT
public:
  ezQtMaterialViewWidget(QWidget* pParent, ezQtMaterialAssetDocumentWindow* pOwnerWindow, ezEngineViewConfig* pViewConfig);
  ~ezQtMaterialViewWidget();

private:
  ezUniquePtr<ezOrbitCameraContext> m_pOrbitCameraContext;
};
