# Native code (C++) rendering plugin example for Unity

This sample demonstrates how to render and do other graphics related things from a C++ plugin, via a
[native plugin interface](http://docs.unity3d.com/Manual/NativePluginInterface.html).

The plugin itself does only two things:

* Draws a colored rotating triangle in the middle of the screen.
* Changes texture data of a Unity texture each frame, with an animated "plasma" pattern.

Native code rendering is implemented for several platforms and graphics APIs:
* Windows (D3D9, D3D11, D3D12, OpenGL Core)
* Mac OS X (Metal, OpenGL Core, OpenGL Legacy)
* Windows Store, UWP (Windows 10) variant (D3D11, D3D12)
* ...more platforms should be possible, we just did not get around to add project files & test them in this project yet.