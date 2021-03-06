#include <PCH.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessViewContext.h>
#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Components/CameraComponent.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessMessages.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessApp.h>

ezEngineProcessViewContext::ezEngineProcessViewContext(ezEngineProcessDocumentContext* pContext)
  : m_pDocumentContext(pContext)
{
  m_uiViewID = 0xFFFFFFFF;
}

ezEngineProcessViewContext::~ezEngineProcessViewContext()
{
  ezRenderWorld::DeleteView(m_hView);
  m_hView.Invalidate();

  if (m_Window.m_hWnd != 0)
  {
    static_cast<ezGameApplication*>(ezApplication::GetApplicationInstance())->RemoveWindow(&m_Window);
  }
}

void ezEngineProcessViewContext::SetViewID(ezUInt32 id)
{
  EZ_ASSERT_DEBUG(m_uiViewID == 0xFFFFFFFF, "View ID may only be set once");
  m_uiViewID = id;
}

void ezEngineProcessViewContext::HandleViewMessage(const ezEditorEngineViewMsg* pMsg)
{
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezViewRedrawMsgToEngine>())
  {
    const ezViewRedrawMsgToEngine* pMsg2 = static_cast<const ezViewRedrawMsgToEngine*>(pMsg);

    SetCamera(pMsg2);

    if (pMsg2->m_uiWindowWidth > 0 && pMsg2->m_uiWindowHeight > 0)
    {
      HandleWindowUpdate(reinterpret_cast<ezWindowHandle>(pMsg2->m_uiHWND), pMsg2->m_uiWindowWidth, pMsg2->m_uiWindowHeight);
      Redraw(true);
    }
  }
#elif EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
  EZ_REPORT_FAILURE("This code path should never be executed on UWP.");
#elif
#   error "Unsupported platform."
#endif
}

void ezEngineProcessViewContext::SendViewMessage(ezEditorEngineViewMsg* pViewMsg)
{
  pViewMsg->m_DocumentGuid = GetDocumentContext()->GetDocumentGuid();
  pViewMsg->m_uiViewID = m_uiViewID;

  GetDocumentContext()->SendProcessMessage(pViewMsg);
}

void ezEngineProcessViewContext::HandleWindowUpdate(ezWindowHandle hWnd, ezUInt16 uiWidth, ezUInt16 uiHeight)
{
  EZ_LOG_BLOCK("ezEngineProcessViewContext::HandleWindowUpdate");

  if (m_Window.m_hWnd != 0)
  {
    if (m_Window.m_uiWidth == uiWidth && m_Window.m_uiHeight == uiHeight)
      return;

    static_cast<ezGameApplication*>(ezApplication::GetApplicationInstance())->RemoveWindow(&m_Window);
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  m_Window.m_hWnd = hWnd;
  m_Window.m_uiWidth = uiWidth;
  m_Window.m_uiHeight = uiHeight;

  ezLog::Debug("Creating Swapchain with size {0} * {1}", uiWidth, uiHeight);

  auto hPrimarySwapChain = static_cast<ezGameApplication*>(ezApplication::GetApplicationInstance())->AddWindow(&m_Window);
  const ezGALSwapChain* pPrimarySwapChain = pDevice->GetSwapChain(hPrimarySwapChain);
  EZ_ASSERT_DEV(pPrimarySwapChain != nullptr, "Failed to init swapchain");

  auto hSwapChainRTV = pDevice->GetDefaultRenderTargetView(pPrimarySwapChain->GetBackBufferTexture());

  ezGALRenderTagetSetup BackBufferRenderTargetSetup;
  BackBufferRenderTargetSetup.SetRenderTarget(0, hSwapChainRTV);

  SetupRenderTarget(BackBufferRenderTargetSetup, uiWidth, uiHeight);
}

void ezEngineProcessViewContext::SetupRenderTarget(ezGALRenderTagetSetup& renderTargetSetup, ezUInt16 uiWidth, ezUInt16 uiHeight)
{
  EZ_LOG_BLOCK("ezEngineProcessViewContext::SetupRenderTarget");

  // setup view
  {
    if (m_hView.IsInvalidated())
    {
      m_hView = CreateView();
    }

    ezView* pView = nullptr;
    if (ezRenderWorld::TryGetView(m_hView, pView))
    {
      pView->SetRenderTargetSetup(renderTargetSetup);
      pView->SetViewport(ezRectFloat(0.0f, 0.0f, (float)uiWidth, (float)uiHeight));
    }
  }
}

void ezEngineProcessViewContext::Redraw(bool bRenderEditorGizmos)
{
  auto pState = ezGameApplication::GetGameApplicationInstance()->GetGameStateForWorld(GetDocumentContext()->GetWorld());

  if (pState != nullptr)
  {
    pState->AddAllMainViews();
  }
  // setting to only update one view ?
  //else

  ezView* pView = nullptr;
  if (ezRenderWorld::TryGetView(m_hView, pView))
  {
    const ezTag& tagEditor = ezTagRegistry::GetGlobalRegistry().RegisterTag("Editor");

    if (!bRenderEditorGizmos)
    {
      // exclude all editor objects from rendering in proper game views
      pView->m_ExcludeTags.Set(tagEditor);
    }
    else
    {
      pView->m_ExcludeTags.Remove(tagEditor);
    }

    ezRenderWorld::AddMainView(m_hView);
  }
}

bool ezEngineProcessViewContext::FocusCameraOnObject(ezCamera& camera, const ezBoundingBoxSphere& objectBounds, float fFov, const ezVec3& vViewDir)
{
  ezVec3 vDir = vViewDir;
  bool bChanged = false;
  ezVec3 vCameraPos = camera.GetCenterPosition();
  ezVec3 vCenterPos = objectBounds.GetSphere().m_vCenter;

  const float fDist = ezMath::Max(0.1f, objectBounds.GetSphere().m_fRadius) / ezMath::Sin(ezAngle::Degree(fFov / 2));
  vDir.Normalize();
  ezVec3 vNewCameraPos = vCenterPos - vDir * fDist;
  if (!vNewCameraPos.IsEqual(vCameraPos, 0.01f))
  {
    vCameraPos = vNewCameraPos;
    bChanged = true;
  }

  if (bChanged)
  {
    camera.SetCameraMode(ezCameraMode::PerspectiveFixedFovX, fFov, 0.1f, 1000.0f);
    camera.LookAt(vNewCameraPos, vCenterPos, ezVec3(0.0f, 0.0f, 1.0f));
  }

  return bChanged;
}

void ezEngineProcessViewContext::SetCamera(const ezViewRedrawMsgToEngine* pMsg)
{
  bool bModeFromCamera = false;

  ezView* pView = nullptr;
  if (ezRenderWorld::TryGetView(m_hView, pView) && pView->GetWorld() != nullptr)
  {
    pView->SetCameraUsageHint(pMsg->m_CameraUsageHint);

    bool bResetDefaultPipeline = true;

    if (const ezCameraComponentManager* pCameraManager = pView->GetWorld()->GetComponentManager<ezCameraComponentManager>())
    {
      // Regardless of the rendering mode, if we have a camera with the same usage hint as the view than use the parameters of that camera.
      if (const ezCameraComponent* pCamera = pCameraManager->GetCameraByUsageHint(pMsg->m_CameraUsageHint))
      {
        bResetDefaultPipeline = !pCamera->GetRenderPipeline().IsValid();
        bModeFromCamera = true;

        pCamera->ApplySettingsToView(pView);
      }
    }

    if (pMsg->m_uiRenderMode == ezViewRenderMode::None)
    {
      if (bResetDefaultPipeline)
      {
        pView->SetRenderPipelineResource(CreateDefaultRenderPipeline());
      }
    }
    else
    {
      pView->SetRenderPipelineResource(CreateDebugRenderPipeline());
    }
  }

  if (m_Camera.GetCameraMode() != ezCameraMode::Stereo)
  {
    ezCameraMode::Enum cameraMode = (ezCameraMode::Enum)pMsg->m_iCameraMode;
    if (!bModeFromCamera || cameraMode == ezCameraMode::OrthoFixedWidth || cameraMode == ezCameraMode::OrthoFixedHeight)
    {
      m_Camera.SetCameraMode(cameraMode, pMsg->m_fFovOrDim, pMsg->m_fNearPlane, pMsg->m_fFarPlane);
    }
    m_Camera.LookAt(pMsg->m_vPosition, pMsg->m_vPosition + pMsg->m_vDirForwards, pMsg->m_vDirUp);
  }

  if (pView)
  {
    bool bUseDepthPrePass = pMsg->m_uiRenderMode != ezViewRenderMode::WireframeColor && pMsg->m_uiRenderMode != ezViewRenderMode::WireframeMonochrome;
    pView->SetRenderPassProperty("DepthPrePass", "Active", bUseDepthPrePass);
    pView->SetRenderPassProperty("AOPass", "Active", bUseDepthPrePass); // Also disable SSAO to save some performance

    pView->SetRenderPassProperty("EditorRenderPass", "ViewRenderMode", pMsg->m_uiRenderMode);
    pView->SetRenderPassProperty("EditorPickingPass", "ViewRenderMode", pMsg->m_uiRenderMode);

    // by default this stuff is disabled, derived classes can enable it
    pView->SetRenderPassProperty("EditorSelectionPass", "Active", false);
    pView->SetExtractorProperty("EditorShapeIconsExtractor", "Active", false);
  }
}

ezRenderPipelineResourceHandle ezEngineProcessViewContext::CreateDefaultRenderPipeline()
{
  return ezEditorEngineProcessApp::GetSingleton()->CreateDefaultMainRenderPipeline();
}

ezRenderPipelineResourceHandle ezEngineProcessViewContext::CreateDebugRenderPipeline()
{
  return ezEditorEngineProcessApp::GetSingleton()->CreateDefaultDebugRenderPipeline();
}
