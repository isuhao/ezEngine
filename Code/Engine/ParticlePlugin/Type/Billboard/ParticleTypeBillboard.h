#pragma once

#include <ParticlePlugin/Type/ParticleType.h>
#include <RendererFoundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <ParticlePlugin/Type/Billboard/BillboardRenderer.h>

typedef ezTypedResourceHandle<class ezTexture2DResource> ezTexture2DResourceHandle;
struct ezBillboardParticleData;

class EZ_PARTICLEPLUGIN_DLL ezParticleTypeBillboardFactory : public ezParticleTypeFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleTypeBillboardFactory, ezParticleTypeFactory);

public:
  virtual const ezRTTI* GetTypeType() const override;
  virtual void CopyTypeProperties(ezParticleType* pObject) const override;

  virtual void Save(ezStreamWriter& stream) const override;
  virtual void Load(ezStreamReader& stream) override;

  ezEnum<ezParticleTypeRenderMode> m_RenderMode;
  ezString m_sTexture;
  ezUInt8 m_uiNumSpritesX = 1;
  ezUInt8 m_uiNumSpritesY = 1;
  ezString m_sTintColorParameter;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleTypeBillboard : public ezParticleType
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleTypeBillboard, ezParticleType);

public:
  ezParticleTypeBillboard();
  ~ezParticleTypeBillboard();

  virtual void CreateRequiredStreams() override;

  ezEnum<ezParticleTypeRenderMode> m_RenderMode;
  ezTexture2DResourceHandle m_hTexture;
  ezUInt8 m_uiNumSpritesX = 1;
  ezUInt8 m_uiNumSpritesY = 1;
  ezTempHashedString m_sTintColorParameter;

  virtual void ExtractTypeRenderData(const ezView& view, ezExtractedRenderData* pExtractedRenderData, const ezTransform& instanceTransform, ezUInt64 uiExtractedFrame) const override;

protected:
  virtual void Process(ezUInt64 uiNumElements) override {}

  ezProcessingStream* m_pStreamLifeTime = nullptr;
  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamSize = nullptr;
  ezProcessingStream* m_pStreamColor = nullptr;
  ezProcessingStream* m_pStreamRotationSpeed = nullptr;

  mutable ezArrayPtr<ezBillboardParticleData> m_ParticleData;
};


