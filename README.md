# Native code (C++) rendering plugin example for Unity

This sample demonstrates how to render and do other graphics related things from a C++ plugin, via a
[native plugin interface](http://docs.unity3d.com/Manual/NativePluginInterface.html).

Unity versions:

* **2023.1+** use tip of default branch.

The plugin itself does very few things:

* **Demonstrates basic plumbing**. How to initialize the graphics API, and how calls from Unity into plugin are made.
* **Draws a triangle**. A single colored rotating triangle in the middle of the screen. For each backend API, this shows bare basics of how to setup vertex data, setup
  some shaders or render states, and do a draw call.
* **Changes Unity texture data**. Unity side passes a texture into the plugin, and the code changes the pixels of it each frame, with an animated "plasma" pattern. This
  demonstrates how to work with [Texture.GetNativeTexturePtr](https://docs.unity3d.com/ScriptReference/Texture.GetNativeTexturePtr.html).
* **Changes Unity mesh vertex data**. Unity side passes a vertex buffer into the plugin, and the code changes the vertices each frame, with an animated "heightfield" pattern. This
  demonstrates how to work with [Mesh.GetNativeVertexBufferPtr](https://docs.unity3d.com/ScriptReference/Mesh.GetNativeVertexBufferPtr.html).


Native code rendering is implemented for several platforms and graphics APIs:

* Windows (D3D11, D3D12, OpenGL, Vulkan)
	* Note that Vulkan and DX12 are not compiled in by default
	* Vulkan requires Vulkan SDK; enable it by editing `#define SUPPORT_VULKAN 0`
	to `1` under `UNITY_WIN` clause in `PlatformBase.h`
	* DX12 requires additional header files (see on [github](https://github.com/microsoft/DirectX-Headers) or [nuget package](https://www.nuget.org/packages/Microsoft.Direct3D.D3D12/1.4.10)); enable it by editing `#define SUPPORT_D3D12 0` to `1` under `UNITY_WIN` clause in `PlatformBase.h`
* macOS (Metal, OpenGL)
* Linux (OpenGL, Vulkan)
* Windows Store aka UWP (D3D11, D3D12)
* WebGL (OpenGL ES)
* Android (OpenGL ES, Vulkan)
* iOS/tvOS (Metal; Simulator is supported if you use unity 2020+ and XCode 11+)
* EmbeddedLinux (OpenGL, Open GLES)
* ...more platforms might be coming soon, we just did not get around to adding project files yet.

Code is organized as follows:

* `PluginSource` is source code & IDE project files for the C++ plugin.
 	* `source`: The source code itself. `RenderingPlugin.cpp` is the main logic, `RenderAPI*.*` files contain rendering implementations for different APIs.
	* `projects/VisualStudio2022`: Visual Studio 2022 project files for regular Windows plugin
	* `projects/UWPVisualStudio2022`: Visual Studio 2022 project files for Windows Store (UWP) plugin
	* `projects/Xcode`: Apple Xcode project file for Mac OS X plugin, Xcode 10.3 on macOS 10.14 was tested
	* `projects/GNUMake`: Makefile for Linux
	* `projects/EmbeddedLinux`: Windows .bat files to build plugins for different architectures
* `UnityProject` is the Unity (2023.1.15f1 was tested) project.
	* Single `scene` that contains the plugin sample scene.


### What license are the graphics samples shipped under?

Just like with most other samples, the license is MIT/X11.
