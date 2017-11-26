﻿#include <PCH.h>
#include <RecastPlugin/WorldModule/RecastWorldModule.h>
#include <Core/World/World.h>
#include <ThirdParty/Recast/DetourCrowd.h>

EZ_IMPLEMENT_WORLD_MODULE(ezRecastWorldModule);

ezRecastWorldModule::ezRecastWorldModule(ezWorld* pWorld)
  : ezWorldModule(pWorld)
{
}

ezRecastWorldModule::~ezRecastWorldModule()
{
}

void ezRecastWorldModule::Initialize()
{
  {
    auto updateDesc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezRecastWorldModule::UpdateCrowd, this);
    updateDesc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::PostAsync;
    updateDesc.m_bOnlyUpdateWhenSimulating = true;
    updateDesc.m_fPriority = 0.0f;

    RegisterUpdateFunction(updateDesc);
  }
}

void ezRecastWorldModule::Deinitialize()
{
  dtFreeCrowd(m_pCrowd);
  m_pCrowd = nullptr;
}

void ezRecastWorldModule::SetNavMesh(dtNavMesh* pNavMesh)
{
  m_pNavMesh = pNavMesh;
  m_pCrowd = dtAllocCrowd();
  m_pCrowd->init(100, 0.5f, pNavMesh);
}

void ezRecastWorldModule::UpdateCrowd(const UpdateContext& ctxt)
{
  m_NavMeshPointsOfInterest.IncreaseCheckVisibiblityTimeStamp(GetWorld()->GetClock().GetAccumulatedTime());

  if (m_pCrowd)
  {
    m_pCrowd->update((float)GetWorld()->GetClock().GetTimeDiff().GetSeconds(), nullptr);
  }
}

