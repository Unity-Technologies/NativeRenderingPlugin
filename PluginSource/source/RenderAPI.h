#pragma once

#include "Unity/IUnityGraphics.h"

#include <stddef.h>

struct IUnityInterfaces;

// Super-simple "graphics abstraction". This is nothing like how a proper platform abstraction layer would look like;
// all this does is a base interface for whatever our plugin sample needs. Which is only "draw some triangles"
// and "modify a texture" at this point.
//
// There are implementations of this base class for D3D9, D3D11, OpenGL etc.; see individual RenderAPI_* files.
class RenderAPI
{
public:
	virtual ~RenderAPI() { }


	// Process general event like initialization, shutdown, device loss/reset etc.
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) = 0;

	// Is the API using "reversed" (1.0 at near plane, 0.0 at far plane) depth buffer?
	// Reversed Z is used on modern platforms, and improves depth buffer precision.
	virtual bool GetUsesReverseZ() = 0;

	// Draw some triangle geometry, using some simple rendering state.
	// Upon call into our plug-in the render state can be almost completely arbitrary depending
	// on what was rendered in Unity before. Here, we turn off culling, blending, depth writes etc.
	// and draw the triangles with a given world matrix. The triangle data is
	// float3 (position) and byte4 (color) per vertex.
	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4) = 0;


	// Begin modifying texture data. You need to pass texture width/height too, since some graphics APIs
	// (e.g. OpenGL ES) do not have a good way to query that from the texture itself...
	//
	// Returns pointer into the data buffer to write into (or NULL on failure), and pitch in bytes of a single texture row.
	virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch) = 0;
	// End modifying texture data.
	virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr) = 0;


	// Begin modifying vertex buffer data.
	// Returns pointer into the data buffer to write into (or NULL on failure), and buffer size.
	virtual void* BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize) = 0;
	// End modifying vertex buffer data.
	virtual void EndModifyVertexBuffer(void* bufferHandle) = 0;

	// --------------------------------------------------------------------------
	// DX12 plugin specific functions
	// --------------------------------------------------------------------------

	// Draws to a texture that is created by the plugin
	virtual void drawToPluginTexture() {}

	// Draws to a a unity RenderBuffer which can be set with
	// setRenderTextureResource(). When the texture resource is not
	// set with setRenderTextureResource() one is created by the plugin
	virtual void drawToRenderTexture() {}

	// Returns the native resource pointer to either unity render buffer or
	// to the resource created by the plugin (i.e ID3D12Resource* in case of DX12)
	virtual void* getRenderTexture() { return nullptr; }

	// Sets the underlying resource to unity RenderBuffer 
	// see https://docs.unity3d.com/ScriptReference/RenderBuffer.html
	// setting rb to nullptr is used to signal the plugin that the 
	// previously set RenderBuffer is no longer valid
	virtual void setRenderTextureResource(UnityRenderBuffer rb) {}

	// This should return true when the plugin is used with the unity player
	// and false when the editor is used. The editor uses multiple swap
	// chains (for each window) so instead of choosing one of them nullptr is returned.
	virtual bool isSwapChainAvailable() { return false; };

	// These require the swap chain to be available to be functional.
	// When the swap chain is not available these simply return 0

	virtual unsigned int getPresentFlags() { return 0; }
	virtual unsigned int getSyncInterval() { return 0; }
	virtual unsigned int getBackbufferWidth() { return 0;  }
	virtual unsigned int getBackbufferHeight() { return 0; }
};


// Create a graphics API implementation instance for the given API type.
RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType);

