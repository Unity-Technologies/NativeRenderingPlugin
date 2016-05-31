#include "RenderAPI.h"
#include "PlatformBase.h"

// Direct3D 11 implementation of RenderAPI.

#if SUPPORT_D3D11

#include <assert.h>
#include <d3d11.h>
#include "Unity/IUnityGraphicsD3D11.h"


class RenderAPI_D3D11 : public RenderAPI
{
public:
	RenderAPI_D3D11();
	virtual ~RenderAPI_D3D11() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ() { return (int)m_Device->GetFeatureLevel() >= (int)D3D_FEATURE_LEVEL_10_0; }

	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4);

	virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch);
	virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr);

	virtual void* BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize);
	virtual void EndModifyVertexBuffer(void* bufferHandle);

private:
	void CreateResources();
	void ReleaseResources();

private:
	ID3D11Device* m_Device;
	ID3D11Buffer* m_VB; // vertex buffer
	ID3D11Buffer* m_CB; // constant buffer
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_InputLayout;
	ID3D11RasterizerState* m_RasterState;
	ID3D11BlendState* m_BlendState;
	ID3D11DepthStencilState* m_DepthState;
};


RenderAPI* CreateRenderAPI_D3D11()
{
	return new RenderAPI_D3D11();
}


// Simple compiled shader bytecode.
//
// Shader source that was used:
#if 0
cbuffer MyCB : register(b0)
{
	float4x4 worldMatrix;
}
void VS(float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position)
{
	opos = mul(worldMatrix, float4(pos, 1));
	ocolor = color;
}
float4 PS(float4 color : COLOR) : SV_TARGET
{
	return color;
}
#endif // #if 0
//
// Which then was compiled with:
// fxc /Tvs_4_0_level_9_3 /EVS source.hlsl /Fh outVS.h /Qstrip_reflect /Qstrip_debug /Qstrip_priv
// fxc /Tps_4_0_level_9_3 /EPS source.hlsl /Fh outPS.h /Qstrip_reflect /Qstrip_debug /Qstrip_priv
// and results pasted & formatted to take less lines here
const BYTE kVertexShaderCode[] =
{
	68,88,66,67,86,189,21,50,166,106,171,1,10,62,115,48,224,137,163,129,1,0,0,0,168,2,0,0,4,0,0,0,48,0,0,0,0,1,0,0,4,2,0,0,84,2,0,0,
	65,111,110,57,200,0,0,0,200,0,0,0,0,2,254,255,148,0,0,0,52,0,0,0,1,0,36,0,0,0,48,0,0,0,48,0,0,0,36,0,1,0,48,0,0,0,0,0,
	4,0,1,0,0,0,0,0,0,0,0,0,1,2,254,255,31,0,0,2,5,0,0,128,0,0,15,144,31,0,0,2,5,0,1,128,1,0,15,144,5,0,0,3,0,0,15,128,
	0,0,85,144,2,0,228,160,4,0,0,4,0,0,15,128,1,0,228,160,0,0,0,144,0,0,228,128,4,0,0,4,0,0,15,128,3,0,228,160,0,0,170,144,0,0,228,128,
	2,0,0,3,0,0,15,128,0,0,228,128,4,0,228,160,4,0,0,4,0,0,3,192,0,0,255,128,0,0,228,160,0,0,228,128,1,0,0,2,0,0,12,192,0,0,228,128,
	1,0,0,2,0,0,15,224,1,0,228,144,255,255,0,0,83,72,68,82,252,0,0,0,64,0,1,0,63,0,0,0,89,0,0,4,70,142,32,0,0,0,0,0,4,0,0,0,
	95,0,0,3,114,16,16,0,0,0,0,0,95,0,0,3,242,16,16,0,1,0,0,0,101,0,0,3,242,32,16,0,0,0,0,0,103,0,0,4,242,32,16,0,1,0,0,0,
	1,0,0,0,104,0,0,2,1,0,0,0,54,0,0,5,242,32,16,0,0,0,0,0,70,30,16,0,1,0,0,0,56,0,0,8,242,0,16,0,0,0,0,0,86,21,16,0,
	0,0,0,0,70,142,32,0,0,0,0,0,1,0,0,0,50,0,0,10,242,0,16,0,0,0,0,0,70,142,32,0,0,0,0,0,0,0,0,0,6,16,16,0,0,0,0,0,
	70,14,16,0,0,0,0,0,50,0,0,10,242,0,16,0,0,0,0,0,70,142,32,0,0,0,0,0,2,0,0,0,166,26,16,0,0,0,0,0,70,14,16,0,0,0,0,0,
	0,0,0,8,242,32,16,0,1,0,0,0,70,14,16,0,0,0,0,0,70,142,32,0,0,0,0,0,3,0,0,0,62,0,0,1,73,83,71,78,72,0,0,0,2,0,0,0,
	8,0,0,0,56,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,7,7,0,0,65,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,
	15,15,0,0,80,79,83,73,84,73,79,78,0,67,79,76,79,82,0,171,79,83,71,78,76,0,0,0,2,0,0,0,8,0,0,0,56,0,0,0,0,0,0,0,0,0,0,0,
	3,0,0,0,0,0,0,0,15,0,0,0,62,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,1,0,0,0,15,0,0,0,67,79,76,79,82,0,83,86,95,80,111,115,
	105,116,105,111,110,0,171,171
};
const BYTE kPixelShaderCode[]=
{
	68,88,66,67,196,65,213,199,14,78,29,150,87,236,231,156,203,125,244,112,1,0,0,0,32,1,0,0,4,0,0,0,48,0,0,0,124,0,0,0,188,0,0,0,236,0,0,0,
	65,111,110,57,68,0,0,0,68,0,0,0,0,2,255,255,32,0,0,0,36,0,0,0,0,0,36,0,0,0,36,0,0,0,36,0,0,0,36,0,0,0,36,0,1,2,255,255,
	31,0,0,2,0,0,0,128,0,0,15,176,1,0,0,2,0,8,15,128,0,0,228,176,255,255,0,0,83,72,68,82,56,0,0,0,64,0,0,0,14,0,0,0,98,16,0,3,
	242,16,16,0,0,0,0,0,101,0,0,3,242,32,16,0,0,0,0,0,54,0,0,5,242,32,16,0,0,0,0,0,70,30,16,0,0,0,0,0,62,0,0,1,73,83,71,78,
	40,0,0,0,1,0,0,0,8,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,15,15,0,0,67,79,76,79,82,0,171,171,79,83,71,78,
	44,0,0,0,1,0,0,0,8,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,15,0,0,0,83,86,95,84,65,82,71,69,84,0,171,171
};


RenderAPI_D3D11::RenderAPI_D3D11()
	: m_Device(NULL)
	, m_VB(NULL)
	, m_CB(NULL)
	, m_VertexShader(NULL)
	, m_PixelShader(NULL)
	, m_InputLayout(NULL)
	, m_RasterState(NULL)
	, m_BlendState(NULL)
	, m_DepthState(NULL)
{
}


void RenderAPI_D3D11::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
		m_Device = d3d->GetDevice();
		CreateResources();
		break;
	}
	case kUnityGfxDeviceEventShutdown:
		ReleaseResources();
		break;
	}
}


void RenderAPI_D3D11::CreateResources()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	// vertex buffer
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 1024;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_Device->CreateBuffer(&desc, NULL, &m_VB);

	// constant buffer
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 64; // hold 1 matrix
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_Device->CreateBuffer(&desc, NULL, &m_CB);

	// shaders
	HRESULT hr;
	hr = m_Device->CreateVertexShader(kVertexShaderCode, sizeof(kVertexShaderCode), nullptr, &m_VertexShader);
	if (FAILED(hr))
		OutputDebugStringA("Failed to create vertex shader.\n");
	hr = m_Device->CreatePixelShader(kPixelShaderCode, sizeof(kPixelShaderCode), nullptr, &m_PixelShader);
	if (FAILED(hr))
		OutputDebugStringA("Failed to create pixel shader.\n");

	// input layout
	if (m_VertexShader)
	{
		D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		m_Device->CreateInputLayout(s_DX11InputElementDesc, 2, kVertexShaderCode, sizeof(kVertexShaderCode), &m_InputLayout);
	}

	// render states
	D3D11_RASTERIZER_DESC rsdesc;
	memset(&rsdesc, 0, sizeof(rsdesc));
	rsdesc.FillMode = D3D11_FILL_SOLID;
	rsdesc.CullMode = D3D11_CULL_NONE;
	rsdesc.DepthClipEnable = TRUE;
	m_Device->CreateRasterizerState(&rsdesc, &m_RasterState);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	memset(&dsdesc, 0, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = GetUsesReverseZ() ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL;
	m_Device->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_BLEND_DESC bdesc;
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.RenderTarget[0].BlendEnable = FALSE;
	bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
	m_Device->CreateBlendState(&bdesc, &m_BlendState);
}


void RenderAPI_D3D11::ReleaseResources()
{
	SAFE_RELEASE(m_VB);
	SAFE_RELEASE(m_CB);
	SAFE_RELEASE(m_VertexShader);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_RasterState);
	SAFE_RELEASE(m_BlendState);
	SAFE_RELEASE(m_DepthState);
}


void RenderAPI_D3D11::DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4)
{
	ID3D11DeviceContext* ctx = NULL;
	m_Device->GetImmediateContext(&ctx);

	// Set basic render state
	ctx->OMSetDepthStencilState(m_DepthState, 0);
	ctx->RSSetState(m_RasterState);
	ctx->OMSetBlendState(m_BlendState, NULL, 0xFFFFFFFF);

	// Update constant buffer - just the world matrix in our case
	ctx->UpdateSubresource(m_CB, 0, NULL, worldMatrix, 64, 0);

	// Set shaders
	ctx->VSSetConstantBuffers(0, 1, &m_CB);
	ctx->VSSetShader(m_VertexShader, NULL, 0);
	ctx->PSSetShader(m_PixelShader, NULL, 0);

	// Update vertex buffer
	const int kVertexSize = 12 + 4;
	ctx->UpdateSubresource(m_VB, 0, NULL, verticesFloat3Byte4, triangleCount * 3 * kVertexSize, 0);

	// set input assembler data and draw
	ctx->IASetInputLayout(m_InputLayout);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = kVertexSize;
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
	ctx->Draw(triangleCount * 3, 0);

	ctx->Release();
}


void* RenderAPI_D3D11::BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch)
{
	const int rowPitch = textureWidth * 4;
	// Just allocate a system memory buffer here for simplicity
	unsigned char* data = new unsigned char[rowPitch * textureHeight];
	*outRowPitch = rowPitch;
	return data;
}


void RenderAPI_D3D11::EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr)
{
	ID3D11Texture2D* d3dtex = (ID3D11Texture2D*)textureHandle;
	assert(d3dtex);

	ID3D11DeviceContext* ctx = NULL;
	m_Device->GetImmediateContext(&ctx);
	// Update texture data, and free the memory buffer
	ctx->UpdateSubresource(d3dtex, 0, NULL, dataPtr, rowPitch, 0);
	delete[] (unsigned char*)dataPtr;
	ctx->Release();
}


void* RenderAPI_D3D11::BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize)
{
	ID3D11Buffer* d3dbuf = (ID3D11Buffer*)bufferHandle;
	assert(d3dbuf);
	D3D11_BUFFER_DESC desc;
	d3dbuf->GetDesc(&desc);
	*outBufferSize = desc.ByteWidth;

	ID3D11DeviceContext* ctx = NULL;
	m_Device->GetImmediateContext(&ctx);
	D3D11_MAPPED_SUBRESOURCE mapped;
	ctx->Map(d3dbuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	ctx->Release();

	return mapped.pData;
}


void RenderAPI_D3D11::EndModifyVertexBuffer(void* bufferHandle)
{
	ID3D11Buffer* d3dbuf = (ID3D11Buffer*)bufferHandle;
	assert(d3dbuf);

	ID3D11DeviceContext* ctx = NULL;
	m_Device->GetImmediateContext(&ctx);
	ctx->Unmap(d3dbuf, 0);
	ctx->Release();
}

#endif // #if SUPPORT_D3D11
