#pragma once

#include <RendererCore/ShaderCompiler/ShaderCompiler.h>

struct ID3D11ShaderReflectionConstantBuffer;

class ezShaderCompilerHLSL : public ezShaderProgramCompiler
{
  EZ_ADD_DYNAMIC_REFLECTION(ezShaderCompilerHLSL, ezShaderProgramCompiler);

public:
  virtual void GetSupportedPlatforms(ezHybridArray<ezString, 4>& Platforms) override
  {
    Platforms.PushBack("DX11_SM40_93");
    Platforms.PushBack("DX11_SM40");
    Platforms.PushBack("DX11_SM41");
    Platforms.PushBack("DX11_SM50");
  }

  virtual ezResult Compile(ezShaderProgramData& inout_Data, ezLogInterface* pLog) override;

private:
  void ReflectShaderStage(ezShaderProgramData& inout_Data, ezGALShaderStage::Enum Stage);
  ezShaderConstantBufferLayout* ReflectConstantBufferLayout(ezShaderStageBinary& pStageBinary, ID3D11ShaderReflectionConstantBuffer* pConstantBufferReflection);
};




