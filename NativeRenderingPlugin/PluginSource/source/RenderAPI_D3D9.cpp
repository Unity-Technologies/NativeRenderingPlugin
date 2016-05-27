#include "RenderAPI.h"
#include "PlatformBase.h"

// Direct3D 9 implementation of RenderAPI.

#if SUPPORT_D3D9

#include <assert.h>
#include <d3d9.h>
#include "Unity/IUnityGraphicsD3D9.h"


class RenderAPI_D3D9 : public RenderAPI
{
public:
	RenderAPI_D3D9();
	virtual ~RenderAPI_D3D9() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);

	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4);

	virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch);
	virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr);

private:
	IDirect3DDevice9* m_Device;
	// A dynamic vertex buffer just to demonstrate how to handle D3D9 device resets.
	IDirect3DVertexBuffer9* m_DynamicVB;
};


RenderAPI* CreateRenderAPI_D3D9()
{
	return new RenderAPI_D3D9();
}


RenderAPI_D3D9::RenderAPI_D3D9()
	: m_Device(NULL)
	, m_DynamicVB(NULL)
{
}


void RenderAPI_D3D9::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		IUnityGraphicsD3D9* d3d = interfaces->Get<IUnityGraphicsD3D9>();
		m_Device = d3d->GetDevice();
	}
	// fall-through!
	case kUnityGfxDeviceEventAfterReset:
		// After device is initialized or was just reset, create the VB.
		if (m_DynamicVB == NULL)
			m_Device->CreateVertexBuffer(1024, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_DynamicVB, NULL);
		break;
	case kUnityGfxDeviceEventBeforeReset:
	case kUnityGfxDeviceEventShutdown:
		// Before device is reset or being shut down, release the VB.
		SAFE_RELEASE(m_DynamicVB);
		break;
	}
}


void RenderAPI_D3D9::DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4)
{
	// Note: for simplicity, we'll use D3D9 fixed function pipeline to draw the geometry

	// Set basic render state
	m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// Transformation matrices
	m_Device->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)worldMatrix);
	float identityMatrix[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};
	m_Device->SetTransform(D3DTS_VIEW, (const D3DMATRIX*)identityMatrix);
	m_Device->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)identityMatrix);

	// Vertex layout
	const int kVertexSize = 12 + 4; // position + color
	m_Device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	// Texture stage states to output vertex color
	m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
	m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	m_Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_Device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Copy vertex data into our small dynamic vertex buffer. We could have used
	// DrawPrimitiveUP just fine as well.
	void* vbPtr;
	m_DynamicVB->Lock(0, 0, &vbPtr, D3DLOCK_DISCARD);
	memcpy(vbPtr, verticesFloat3Byte4, triangleCount * 3 * kVertexSize);
	m_DynamicVB->Unlock();
	m_Device->SetStreamSource(0, m_DynamicVB, 0, kVertexSize);

	// Draw
	m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, triangleCount);
}


void* RenderAPI_D3D9::BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch)
{
	IDirect3DTexture9* d3dtex = (IDirect3DTexture9*)textureHandle;
	assert(d3dtex);

	// Lock the texture and return pointer
	D3DLOCKED_RECT lr;
	HRESULT hr = d3dtex->LockRect(0, &lr, NULL, 0);
	if (FAILED(hr))
		return NULL;

	*outRowPitch = lr.Pitch;
	return lr.pBits;
}


void RenderAPI_D3D9::EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr)
{
	IDirect3DTexture9* d3dtex = (IDirect3DTexture9*)textureHandle;
	assert(d3dtex);

	// Unlock the texture after modification
	d3dtex->UnlockRect(0);
}


#endif // #if SUPPORT_D3D9
