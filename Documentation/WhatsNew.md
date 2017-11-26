What's New {#WhatsNew}
==========

Milestone 8
-----------

[//]: # (SVN Revision 1192 to 3287)

General features:
  * Added PhysX support
  * Added 'Surface' assets / resources, which describe the physical properties (e.g. friction) and interactions with objects. E.g. which effects to spawn when shooting a surface or which sounds to play when walking on them.
  * Added Fmod support for sound and music.
  * Added a particle effect system and editor.
  * Added a Visual Shader editor. Can be used to easily create custom material shaders.
  * Added UWP port
  * Added support for Windows Mixed Reality and the HoloLens
  * Added support for 'dear imgui' for displaying UIs in game
  * Added Visual Scripting functionality
  * Added Recast integration to generate navmeshes from the level geometry and compute paths for AI characters
  * Added decal functionality for rendering signs, splatter, dirt, etc.
  * Added "property animations" and an editor to edit to edit the curves both through curve editor as well as through recording changes to a scene

Editor:
  * Shortcuts can now be configured with a hotkey editor
  * All UI actions can now be translated
  * Objects can now be hidden for easier editing
  * Added editor prefabs, objects that can be edited like scenes and added to other scenes. Modifications to the original will be synchronized to all instances.
  * Scenes can now be exported to a binary format and run through ezPlayer
  * Various improvements to orthographic views, eg most gizmos work
  * Scenes can now be simulated or played directly from the editor, simulation can be done at different speeds
  * Added "shape icons" for objects that are otherwise invisible (e.g. sound sources)
  * Added a "Duplicate Special" dialog for quickly duplicating objects in certain patterns
  * Added drag&drop for material assets to assign materials to objects in the 3D viewport
  * Added a dialog to configure which plugins to load into the editor and the runtime
  * The input bindings (which keys trigger which action) can now be configured from the editor
  * Added manipulators for visualizing and modifying property values. For instance light source cones can now be modified in the 3D viewport.
  * Editor can now spawn multiple 'container windows' to display documents on multiple monitors
  * Added a dialog to configure the game's window mode and resolution. Allows for different settings when run from the editor.
  * Before exporting a scene, the editor can now modify (bake / optimize) the world
  * The editor can now connect to an instance of the engine process, running on a remote device, for previewing purposes
  * Added a panel to display and modify CVars. This allows to enable debug functionality inside the editor.
  * Added marquee selection via SPACE+Drag
  * Added a "Move Parent Only" mode for adjusting node transforms without changing the child transforms
  * Added a "Delta Transform" dialog to precisely modify object transforms
  * Added "Import Assets" functionality that allows to quickly import multiple source assets and auto-generate the necessary documents
  * Assets in a folder named "XYZ_data" are now hidden in the asset browser, unless one enables showing them or navigates directly to that folder
  * Added support for importing Source Engine BSP files
  * Added a greyboxing tool to easily generate prototyping geometry such as boxes, columns, ramps and stairs

Engine Runtime:
  * Added ezWorldReader and ezWorldWriter for importing and exporting ezWorld's in binary format
  * Added ezPrefabResource and ezSpawnComponent which are used for instantiating prefabs at runtime
  * Added ezTimedDeathComponent which deletes an object after a timeout
  * Added ezInputComponent for handling input events at specific nodes
  * Added ezProjectileComponent for implementing weapons
  * ezGameObject now has a 'GlobalKey' property to identify very important objects
  * Added 'Collection' assets/resources which are used to preload assets at startup and to access resources through a custom name
  * Added an ingame console for modifying CVars. Can be opened in ezPlayer with F1.
  * Added ezCameraComponent for managing runtime cameras
  * Editor can now ignore unknown component types. Shows warnings when opening and saving such a scene, but does not crash.
  * Negative mesh scaling (mirroring) is now supported

Rendering:
  * Support for point and line rendering
  * Added ambient, point, spot and directional light sources
  * Added dynamic shadows for all light types
  * Added ezDebugRenderer, which allows to easily display shapes for debugging purposes
  * Added functionality to enumerate all available screens on Windows
  * Added a Bloom post processor.
  * Added a tonemapping post processor.
  * Added ezSkyboxComponent for rendering a skybox.
  * Added MSAA support
  * Added support for JPG and PNG as texture files
  * Added two different SSAO post processors with different speed/quality trade offs
  * Added emissive and occlusion to the material model
  * Embedded a font in code for debug text output
  * Implemented proper culling and various other optimizations
  * Added stereoscopic rendering support
  * Added depth and height fog rendering through the ezFogComponent

Infrastructure:
  * Added ezDeferredFileWriter for only writing files to disk after everything succeeded
  * Added a random number generator
  * Added readers and writers for OpenDLL files
  * Added ezGameApplication and ezGameState which decouple the application framework and custom game code, which allows to load game code via a plugin into the editor
  * Added ezDGMLGraph, ezDGMLGraphWriter and ezDGMLGraphCreator for generating DGML diagrams
  * Added ezProcessingStream for efficient, data oriented processing. Used by the particle system.
  * Added color gradient and curve resources and respective editors. Used by the property animation system and the particle effects.
  * Added ezModelImporter for loading and processing mesh data
  * Added a typesafe printf replacement to ezStringBuffer
  * Added SIMD versions of the vector and matrix classes

Tools / Samples:
  * Added ezPlayer, a stand-alone application to run exported scenes
  * Added ezTexConv for converting image files to optimized formats
  * Added ezShaderCompiler for compiling shaders in a separate process. Used by the Visual Shader editor for verification purposes.
  * Added the ComputeShaderHistogram sample
  * Added ezFileServe, an application to stream game data to a remote device


Milestone 7
-----------
  * Added solid color texture resources. E.g. use a texture resource with name "CornflowerBlue.color" or "#FF00FF.color" or "FF00FF00.color" and a 4x4 color texture will be generated from memory, instead of read from disk.
  * Added RendererFoundation low-level rendering infrastructure
  * Added RendererCore mid-level rendering infrastructure, see ezRenderContext, ezRenderPipeline, ezRenderPipelinePass, ezView
  * Added DX11 implementation of RendererFoundation
  * Added tools infrastructure
  * Added ezEditor, a general purpose document based editor with undo/redo and out-of-process rendering
  * Added an asset management system
  * Added file redirection through ezDataDirectory::FolderType for asset management
  * Added ezDirectoryWatcher
  * Enhanced the reflection system to handle enums and bitfields
  * Reflection system now supports custom attributes for properties
  * Split up ezStringView and added ezStringIterator
  * Added ezTagRegistry, ezTag and ezTagSet to handle tags
  * Added support for more types to ezVariant
  * Named colors can now be converted back and forth using ezConversionUtils
  * Added some basic infrastructure for node graphs
  * Added GameFoundation library
  * Added some basic components for rotating and translating objects
  * Added rendering resources: ezConstantBufferResource, ezMaterialResource, ezMeshResource, ezShaderResource, ezTextureResource
  * Added (runtime) shader compilation and shader permutation generation
  * Added ezGPUResourcePool for sharing gpu resources


Milestone 6
-----------

  * Added ezOBJLoader to load OBJ/MTL files.
  * Renamed ezStringIterator to ezStringView.
  * Added ezGeometry, a class with functions to generate basic geometric shapes.
  * Added ezChunkStreamReader / ezChunkStreamWriter to work with 'chunked' file formats.
  * Added ezArchiveReader / ezArchiveWriter which implement a chunked format that also allows to store and restore reflected types and references between objects.
  * Added ezArrayMap container, which is an associative container that is typically more efficient than ezMap.
  * Added ezResourceManager, which allows to manage resources and stream data. This is still work in progress though.
  * Refactored ezColor a bit. The interface is more streamlined. sRGB features have been moved into separate classes ezColorLinearUB and ezColorGammaUB.
  * ezColorRgba8UNorm has been renamed to ezColorLinearUB (UB == unsigned byte).
  * Removed ezColorBgra8Unorm
  * Added the 140 predefined colors of the CSS specification to ezColor.
  * Added ezDirectoryWatcher. This class allows to watch a directory for file changes. It is currently only implemented on Windows.
  * Added a few utilty functions to ezStringBuilder: Set, SetSubString_FromTo, SetSubString_ElementCount, SetSubString_CharacterCount, RemoveFileExtension
  * Added ezUuid, a class that allows to generate uuid/guid values. Implemented on Windows and Posix systems.
  * Added a CMake file "FindezEngine.cmake" (in "Code\BuildSystem\CMake") which you can use for easier integration of ezEngine into your own CMake based project. For more details see this: \ref HowToUseCMake
  * The CMake builds now allow to configure some settings, that previously needed to be done through the UserConfig.h in Foundation. Use 'EZ_IGNORE_USERCONFIG_HEADER' for this. However, this will only work, when you mirror the same defines in your project setup.

Milestone 5
-----------

  * Added ezJSONParser which is a low level parser that reads JSON incrementally.
  * Added ezJSONReader which generates a high level representation of a JSON document, that can be used to lookup data easily.
  * Added ezExtendedJSONReader which generates a high level representation of a JSON document, but also uses the extended JSON syntax for more precise type information and values.
  * Added ezImage to the CoreUtils library, which allows to easily read / convert / write images of different file types.
    * Supported image file types: BMP, DDS, TGA
  * Added namespace ezIntersectionUtils with functions to do intersection tests.
  * Added ezTransform, a 4x3 matrix that can be used for object transformations that don't require a full 4x4 matrix.
  * Added a function to create a simple embedded ASCII font from code, so no file loading is required to get some simple text on screen.
  * Added ezConsole, a Quake-style ingame console for inspecting the log, modifying cvars and calling exposed functions.
  * Removed ezSharedString. Use ezString or ezHashedString instead.
  * Removed official support for Visual Studio 2010. Might still compile, but we won't ensure that anymore.
  * More extensive usage of C++ 11:
    * ez now uses nullptr directly, instead of a redefined NULL, which solves issues on platforms that already redefined NULL to some compiler specific intrinsic
    * Some container and string classes now support move semantics for improved performance when they are copied around. Note that it is still more efficient to NOT copy such objects around, at all, though.
  * Added ezDataTransfer, which allows ezInspector to pull arbitrary (custom) data from the connected application. This can be used to make screenshots, etc.
  * Improved the profiling system: When GPA is not used, it can now store the profiling history in a ringbuffer in memory and dump it on request. We are currently supporting a JSON file format that can be displayed by Google Chrome using chrome://tracing
  * Added ezTokenizer, a class that allows to easily tokenize a C language like piece of text.
  * The natvis Visual Studio Visualizer now also supports ezStringView, ezHashedString and ezHashTable.
  * GameObject improvements:
    * Parenting works now
    * Transformation propagation to child objects has been implemented
    * Message routing has been improved
    * The different update phases have been fixed
  * Changed some of the macros of the reflection system to be easier to use.
  * Fixed some issues compiling ez on Windows 8 and Windows 8.1
  * Added ezPreprocessor, a class that allows to run a C preprocessor on text and get the result, it supports all the standard features
  * ezVariant now supports more basic types and is more efficient by using move semantics when possible.


Milestone 4
-----------

  * Added Linux support, which is on par with OS X port. Tested with Ubuntu 12.4 and GCC 4.8.
  * Removed the InputWindows plugin and merged the functionality into the ezWindow code (System library).
  * Added an SFML port for Window creation and Input handling to ezWindow, which works on non-Windows platforms.
  * Added code and a small tool that fixes problems with static linking.
  * Added ezDynamicOctree and ezDynamicQuadtree to the CoreUtils library, which are data structures to optimize range queries.
  * Added Lua to the ThirdParty library.
  * Added ezHashedString and ezTempHashedString for storing strings that rarely change but where comparisons should be really fast.
  * Added ezFrustum for object culling.
  * Added ezLuaWrapper to CoreUtils for easier working with Lua scripts.
  * Added ezGraphicsUtils with functions for mapping points to and from world-space and screen-space.
  * Added ezCamera, a lightweight class to do standard camera actions (moving, rotating, etc.)
  * In debug builds all math classes do NaN checks, to detect the usage of uninitialized data.
  * Added ezGameUtils library which contains some general purpose functions that might be useful for different types of games.
  * Added a reflection system for runtime type information (see ezRTTI etc.).
  * ezInspector can show some information about reflected types.
  * ezInspector will now stay in the foreground while active.
  * Allocators are now implemented in such a way that ezStaticAllocatorWrapper etc. are not needed anymore. Global variables can now be created without any workarounds.
  * Added a new sample that implements some of the basics for an RTS-style game. Work in progress.
  * Moved ezSystemTime::Now() into ezTime.
  * The ezTaskSystem now computes the worker thread utilization, which can be queried manually and is also transmitted to ezInspector.
  * ezInspector can now display the history (up to 10 minutes) of every stat variable, as long as it contains a number.

Milestone 3
-----------

  * Added ezTimestamp, a platform independent timestamp for file-time and system-time stored in microseconds since Unix epoch.
  * Added ezDateTime, a class that converts an ezTimestamp into human-readable form.
  * Added an input abstraction system (ezInputManager, ezInputDevice, etc.).
     * Implements mouse/keyboard and XBox 360 controller on Windows.
     * Contains ezVirtualThumbStick, which can be used on touch devices for 'controller input'.
     * Can be extended through additional ezInputDevice's to handle more platforms and device types.
  * Added the basics of a game object system (ezWorld, ezGameObject, ezComponent). The Asteroids game sample already shows how to do a simple game with it. The system is not yet complete but good enough for now.
  * Added a sample game, similar to Asteroids, to show how to use the Game Object system and the Input System.
  * Added ezCompressedStreamReader and ezCompressedStreamWriter, which allow to (un-)compress data while writing/reading a file or memory stream.
  * Asserts can now be intercepted with an Assert Handler
  * Many subsystems (ezOSFile, ezInputManager, ezCVar) now broadcast more info about the status via ezEvent so that introspection becomes possible.
  * ezEvent can now be used with a locking policy, if thread-safety is required.
  * Added ezTelemetry, a system that allows to broadcast information about the application to interested other applications via network. Once connected, communication can go both ways to modify the application behavior.
  * Added ezStats, an easy system to store information about the current application state (player health, ammo, AI status, ...). This can be transmitted via ezTelemetry (or other tools) and displayed in a convenient way for better debugging.
  * Added ezInspector, a tool that can display information about ezStats, ezCVar, ezLog messages, ezSubSystem, ezPlugin, ezGlobalEvent, ezOSFile, memory allocations and asserts. It can be run in parallel to a game, either on the same PC or a different one and connect via TCP to the application that will then transmit that information.
  * Added ezInspectorPlugin, the engine-side plugin that enables the introspection features required to communicate with ezInspector.
  * Added a Qt GUI for the testframework.
  * Added ezApplication, a basic application abstraction.
  * Added ezWindow, a basic window abstraction.
  * Changed ezMemoryStreamStorage, ezMemoryStreamReader and ezMemoryStreamWriter to allow to use them in more complicated code and without allocations in many situations.
  * Added ezRectTemplate and ezSizeTemplate for rect and size types.
  * Added ezMemoryUtils::DefaultConstruct which will always default construct objects, even POD types such as int, float, etc.
  * Added ezMemoryUtils::ByteCompare to do a real byte compare even on class types.
  * Added 'ThirdParty' library.
    * Included zlib for zip compression.
    * Included enet for network communication.
  * Added 'System' library.
    * Currently only includes ezWindow.
  * Added ezDelegate for callbacks that work with static functions and member functions.
  * Fixed some code analysis warnings with Visual Studio 2012.
  * Fixed several bugs.
  * New ezVariant type that is much more powerful and useful than the previous implementation.
  * Added ezConversionUtils class for converting strings to int/float/bool.
  * Added optional *.natvis support for Visual Studio 2012. This enables much more comfortable inspection of basic types in the debugger.
  * The logging system has been improved. You can now log locally into another system (useful for tools) or you can replace the default implementation entirely.
  * Added ezClock for managing game time updates.
  * Added ezStopwatch for measuring time.
  * Added the ezAngle class to better deal with angles in degree, radians (easier to use, typesafe and more efficient!)
  * Added ezJSONWriter interface and two derived implementations for writing standard JSON and 'extended' JSON.
  * Added ezColor, a class to convert color values back and forth between different color spaces.
  * Added ezColor8UNorm to store 8-Bit RGBA color values.
  * Added ezColor16f to store 16-Bit RGBA color values.
  * Added ezFloat16 to convert float values to 16-Bit half floats and back.
  * Added ezTimestamp which stores a timestamp in microseconds since unix epoch.
  * Added ezDateTime which converts an ezTimestamp into human readable form and back.

Milestone 2
-----------

  * Added CVars (global configuration variables)
  * Added Plugin loading / unloading / reloading
  * Added GPA integration for profiling
  * Added Reloadable global variables for plugins
  * Added Bitfield Container
  * Added generic Id class and Id mapping table
  * Added Stack Tracing for better memory leak detection
  * Added Mac Port (mostly)
  * Added Application Entry Point abstraction (main / WinMain)
  * Added Task Pool System
  * Added BMP Writer
  * Added Fixed Point type
  * All math classes are now templated, allowing double as internal type
  * Added ezVec2d, ezVec3d, ezVec4d, ezPlaned, ezQuatd, ezMat3d, ezMat4d, ezBoundingBoxd, ezBoundingSphered
  * New Libraries: Core, CoreUtils
  * Added ezLargeBlockAllocator
  * Implemented a proper hashing function for strings
  * Added ezSystemInformation to report CPU core count, Available RAM, etc.
  * Added features to the build system to work with Qt5
  * Added ezThreadSignal for inter-thread synchronization
  * Fixed several bugs.

