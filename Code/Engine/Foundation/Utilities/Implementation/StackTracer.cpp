
#include <PCH.h>
#include <Foundation/Utilities/StackTracer.h>
#include <Foundation/Configuration/Startup.h>

// Include inline file
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)

# if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
  #include <Foundation/Utilities/Implementation/Win/StackTracer_uwp.h>
# else
  #include <Foundation/Utilities/Implementation/Win/StackTracer_win.h>
#endif

#elif EZ_ENABLED(EZ_PLATFORM_OSX) || EZ_ENABLED(EZ_PLATFORM_LINUX)
  #include <Foundation/Utilities/Implementation/Posix/StackTracer_posix.h>
#else
  #error "StackTracer is not implemented on current platform"
#endif

EZ_BEGIN_SUBSYSTEM_DECLARATION(Foundation, StackTracer)

BEGIN_SUBSYSTEM_DEPENDENCIES
"Time"
END_SUBSYSTEM_DEPENDENCIES

ON_CORE_STARTUP
{
  ezPlugin::s_PluginEvents.AddEventHandler(ezStackTracer::OnPluginEvent);
}

ON_CORE_SHUTDOWN
{
  ezPlugin::s_PluginEvents.RemoveEventHandler(ezStackTracer::OnPluginEvent);
}

EZ_END_SUBSYSTEM_DECLARATION

EZ_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_StackTracer);

