#include "RenderAPI.h"
#include "PlatformBase.h"

// OpenGL 2 (legacy, deprecated) implementation of RenderAPI.


#if SUPPORT_OPENGL_LEGACY

#include "GLEW/glew.h"


class RenderAPI_OpenGL2 : public RenderAPI
{
public:
	RenderAPI_OpenGL2();
	virtual ~RenderAPI_OpenGL2() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);

	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4);

	virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch);
	virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr);
};


RenderAPI* CreateRenderAPI_OpenGL2()
{
	return new RenderAPI_OpenGL2();
}


RenderAPI_OpenGL2::RenderAPI_OpenGL2()
{
}


void RenderAPI_OpenGL2::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	// not much to do :)
}


void RenderAPI_OpenGL2::DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4)
{
	// Note: for simplicity, we'll use fixed function pipeline to draw the geometry

	// Set basic render state
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Transformation matrices
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(worldMatrix);
	glMatrixMode(GL_PROJECTION);
	// Tweak the projection matrix a bit to make it match what identity
	// projection would do in D3D case.
	float projectionMatrix[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,2,0,
		0,0,-1,1,
	};
	glLoadMatrixf(projectionMatrix);

	// Vertex layout
	const int kVertexSize = 12 + 4;
	glVertexPointer(3, GL_FLOAT, kVertexSize, verticesFloat3Byte4);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, kVertexSize, (const char*)verticesFloat3Byte4 + 12);
	glEnableClientState(GL_COLOR_ARRAY);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
}


void* RenderAPI_OpenGL2::BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch)
{
	const int rowPitch = textureWidth * 4;
	// Just allocate a system memory buffer here for simplicity
	unsigned char* data = new unsigned char[rowPitch * textureHeight];
	*outRowPitch = rowPitch;
	return data;
}


void RenderAPI_OpenGL2::EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr)
{
	GLuint gltex = (GLuint)(size_t)(textureHandle);
	// Update texture data, and free the memory buffer
	glBindTexture(GL_TEXTURE_2D, gltex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, dataPtr);
	delete[](unsigned char*)dataPtr;
}


#endif // #if SUPPORT_OPENGL_LEGACY
