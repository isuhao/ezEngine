#include <PCH.h>
#include <SampleGamePlugin/SampleGamePlugin.h>

void OnLoadPlugin(bool bReloading)
{
}

void OnUnloadPlugin(bool bReloading)  
{
}

ezPlugin g_Plugin(false, OnLoadPlugin, OnUnloadPlugin);

EZ_DYNAMIC_PLUGIN_IMPLEMENTATION(EZ_SAMPLEGAMEPLUGIN_DLL, ezSampleGamePlugin);
