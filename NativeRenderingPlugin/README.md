# Native code (C++) rendering plugin example for Unity

This sample demonstrates how to render and do other graphics related things from a C++ plugin, via a
[native plugin interface](http://docs.unity3d.com/Manual/NativePluginInterface.html).

Unity version required: **5.4** _(might work in earlier versions, not tested)_.

The plugin itself does only two things:

* Draws a colored rotating triangle in the middle of the screen.
* Changes texture data of a Unity texture each frame, with an animated "plasma" pattern.

Native code rendering is implemented for several platforms and graphics APIs:

* Windows (D3D9, D3D11, D3D12, OpenGL Core)
* Mac OS X (Metal, OpenGL Core, OpenGL Legacy)
* Windows Store, UWP (Windows 10) variant (D3D11, D3D12)
* ...more platforms should be possible, we just did not get around to add project files & test them in this project yet.

Code is organized as follows:

* `PluginSource` is source code & IDE project files for the C++ plugin.
 	* `source`: The source code itself. `RenderingPlugin.cpp` is the main logic, `RenderAPI*.*` files contain rendering implementations for different APIs.
	* `projects/VisualStudio2015`: Visual Studio 2015 project files for regular Windows plugin
	* `projects/UWPVisualStudio2015`: Visual Studio 2015 project files for Windows Store (UWP) plugin
	* `projects/Xcode`: Apple Xcode project file for Mac OS X plugin, Xcode 7.3 on OS X 10.11 was tested
* `UnityProject` is the Unity (5.4b19 was tested) project.
	* Single `scene` that contains the plugin sample scene.
