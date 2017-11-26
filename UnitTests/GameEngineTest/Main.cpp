﻿#include <PCH.h>
#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/TestSetup.h>
#include <RendererCore/Textures/TextureUtils.h>

int main(int argc, char **argv)
{
  ezTestSetup::InitTestFramework("GameEngineTest", "GameEngine Tests", argc, (const char**) argv);

  ezTextureUtils::s_bForceFullQualityAlways = true; // never allow to use low-res textures

  // *** Add additional output handlers and configurations here. ***
  ezTestFramework::s_bCallstackOnAssert = true;
  while (ezTestSetup::RunTests() == ezTestAppRun::Continue)
  {
  }

  const ezInt32 iFailedTests = ezTestSetup::GetFailedTestCount();

  ezTestSetup::DeInitTestFramework();
  return iFailedTests;
}
