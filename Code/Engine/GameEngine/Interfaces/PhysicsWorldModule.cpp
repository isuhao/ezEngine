#include <PCH.h>
#include <GameEngine/Interfaces/PhysicsWorldModule.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPhysicsWorldModuleInterface, 1, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_IMPLEMENT_MESSAGE_TYPE(ezPhysicsAddImpulseMsg);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPhysicsAddImpulseMsg, 1, ezRTTIDefaultAllocator<ezPhysicsAddImpulseMsg>);
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_IMPLEMENT_MESSAGE_TYPE(ezPhysicsAddForceMsg);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezPhysicsAddForceMsg, 1, ezRTTIDefaultAllocator<ezPhysicsAddForceMsg>);
EZ_END_DYNAMIC_REFLECTED_TYPE

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_MESSAGE_TYPE(ezBuildStaticMeshMessage);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezBuildStaticMeshMessage, 1, ezRTTIDefaultAllocator<ezBuildStaticMeshMessage>)
EZ_END_DYNAMIC_REFLECTED_TYPE
