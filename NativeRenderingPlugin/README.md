# Native code (C++) rendering plugin example for Unity

This sample demonstrates how to render and do other graphics related things from a C++ plugin, via a
[native plugin interface](http://docs.unity3d.com/Manual/NativePluginInterface.html).

Unity version required: **5.4** _(might work in earlier versions, not tested)_.

The plugin itself does very few things:

* **Demonstrates basic plumbing**. How to initialize the graphics API, and how calls from Unity into plugin are made.
* **Draws a triangle**. A single colored rotating triangle in the middle of the screen. For each backend API, this shows bare basics of how to setup vertex data, setup
  some shaders or render states, and do a draw call.
* **Changes Unity texture data**. Unity side passes a texture into the plugin, and the code changes the pixels of it each frame, with an animated "plasma" pattern. This
  demonstrates how to work with [Texture.GetNativeTexturePtr](http://docs.unity3d.com/ScriptReference/Texture.GetNativeTexturePtr.html).


Native code rendering is implemented for several platforms and graphics APIs:

* Windows (D3D9, D3D11, D3D12, OpenGL Core)
* Mac OS X (Metal, OpenGL Core, OpenGL Legacy)
* Linux (OpenGL Core, OpenGL Legacy)
* Windows Store, UWP (Win10) and WSA (Win8.1) variants (D3D11, D3D12)
* ...more platforms (iOS, Android) coming soon, we just did not get around to adding project files yet.

Code is organized as follows:

* `PluginSource` is source code & IDE project files for the C++ plugin.
 	* `source`: The source code itself. `RenderingPlugin.cpp` is the main logic, `RenderAPI*.*` files contain rendering implementations for different APIs.
	* `projects/VisualStudio2015`: Visual Studio 2015 project files for regular Windows plugin
	* `projects/UWPVisualStudio2015`: Visual Studio 2015 project files for Windows Store (UWP - Win10) plugin
	* `projects/WSAVisualStudio2013`: Visual Studio 2013 project files for Windows Store (WSA - Win8.1) plugin
	* `projects/Xcode`: Apple Xcode project file for Mac OS X plugin, Xcode 7.3 on OS X 10.11 was tested
	* `projects/GNUMake`: Makefile for Linux
* `UnityProject` is the Unity (5.4b19 was tested) project.
	* Single `scene` that contains the plugin sample scene.
