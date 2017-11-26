﻿#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>

struct ezForwardRenderShadingQuality
{
  typedef ezInt8 StorageType;

  enum Enum
  {
    Normal,
    Simplified,

    Default = Normal,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_NO_LINKAGE, ezForwardRenderShadingQuality);

/// \brief A standard forward render pass that renders into the color target.
class EZ_RENDERERCORE_DLL ezForwardRenderPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezForwardRenderPass, ezRenderPipelinePass);

public:
  ezForwardRenderPass(const char* szName = "ForwardRenderPass");
  ~ezForwardRenderPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs,
    ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs,
    const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:

  virtual void SetupPermutationVars(const ezRenderViewContext& renderViewContext);

  ezPassThroughNodePin m_PinColor;
  ezPassThroughNodePin m_PinDepthStencil;
  ezInputNodePin m_PinSSAO;

  ezEnum<ezForwardRenderShadingQuality> m_ShadingQuality;
  bool m_bWriteDepth;
  bool m_bApplySSAOToDirectLighting;

  ezTexture2DResourceHandle m_hWhiteTexture;
};
