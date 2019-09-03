
#include "RenderAPI.h"
#include "PlatformBase.h"


// Metal implementation of RenderAPI.


#if SUPPORT_METAL

#include "Unity/IUnityGraphicsMetal.h"
#import <Metal/Metal.h>


class RenderAPI_Metal : public RenderAPI
{
public:
	RenderAPI_Metal();
	virtual ~RenderAPI_Metal() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ() { return true; }

	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4);

	virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch);
	virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr);

	virtual void* BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize);
	virtual void EndModifyVertexBuffer(void* bufferHandle);

private:
	void CreateResources();

private:
	IUnityGraphicsMetal*	m_MetalGraphics;
	id<MTLBuffer>			m_VertexBuffer;
	id<MTLBuffer>			m_ConstantBuffer;

	id<MTLDepthStencilState> m_DepthStencil;
	id<MTLRenderPipelineState>	m_Pipeline;
};


RenderAPI* CreateRenderAPI_Metal()
{
	return new RenderAPI_Metal();
}


static Class MTLVertexDescriptorClass;
static Class MTLRenderPipelineDescriptorClass;
static Class MTLDepthStencilDescriptorClass;
const int kVertexSize = 12 + 4;

// Simple vertex & fragment shader source
static const char kShaderSource[] =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct AppData\n"
"{\n"
"    float4x4 worldMatrix;\n"
"};\n"
"struct Vertex\n"
"{\n"
"    float3 pos [[attribute(0)]];\n"
"    float4 color [[attribute(1)]];\n"
"};\n"
"struct VSOutput\n"
"{\n"
"    float4 pos [[position]];\n"
"    half4  color;\n"
"};\n"
"struct FSOutput\n"
"{\n"
"    half4 frag_data [[color(0)]];\n"
"};\n"
"vertex VSOutput vertexMain(Vertex input [[stage_in]], constant AppData& my_cb [[buffer(0)]])\n"
"{\n"
"    VSOutput out = { my_cb.worldMatrix * float4(input.pos.xyz, 1), (half4)input.color };\n"
"    return out;\n"
"}\n"
"fragment FSOutput fragmentMain(VSOutput input [[stage_in]])\n"
"{\n"
"    FSOutput out = { input.color };\n"
"    return out;\n"
"}\n";



void RenderAPI_Metal::CreateResources()
{
	id<MTLDevice> metalDevice = m_MetalGraphics->MetalDevice();
	NSError* error = nil;

	// Create shaders
	NSString* srcStr = [[NSString alloc] initWithBytes:kShaderSource length:sizeof(kShaderSource) encoding:NSASCIIStringEncoding];
	id<MTLLibrary> shaderLibrary = [metalDevice newLibraryWithSource:srcStr options:nil error:&error];
	if(error != nil)
	{
		NSString* desc		= [error localizedDescription];
		NSString* reason	= [error localizedFailureReason];
		::fprintf(stderr, "%s\n%s\n\n", desc ? [desc UTF8String] : "<unknown>", reason ? [reason UTF8String] : "");
	}

	id<MTLFunction> vertexFunction = [shaderLibrary newFunctionWithName:@"vertexMain"];
	id<MTLFunction> fragmentFunction = [shaderLibrary newFunctionWithName:@"fragmentMain"];


	// Vertex / Constant buffers

#	if UNITY_OSX
	MTLResourceOptions bufferOptions = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeManaged;
#	else
	MTLResourceOptions bufferOptions = MTLResourceOptionCPUCacheModeDefault;
#	endif

	m_VertexBuffer = [metalDevice newBufferWithLength:1024 options:bufferOptions];
	m_VertexBuffer.label = @"PluginVB";
	m_ConstantBuffer = [metalDevice newBufferWithLength:16*sizeof(float) options:bufferOptions];
	m_ConstantBuffer.label = @"PluginCB";

	// Vertex layout
	MTLVertexDescriptor* vertexDesc = [MTLVertexDescriptorClass vertexDescriptor];
	vertexDesc.attributes[0].format			= MTLVertexFormatFloat3;
	vertexDesc.attributes[0].offset			= 0;
	vertexDesc.attributes[0].bufferIndex	= 1;
	vertexDesc.attributes[1].format			= MTLVertexFormatUChar4Normalized;
	vertexDesc.attributes[1].offset			= 3*sizeof(float);
	vertexDesc.attributes[1].bufferIndex	= 1;
	vertexDesc.layouts[1].stride			= kVertexSize;
	vertexDesc.layouts[1].stepFunction		= MTLVertexStepFunctionPerVertex;
	vertexDesc.layouts[1].stepRate			= 1;

	// Pipeline

	MTLRenderPipelineDescriptor* pipeDesc = [[MTLRenderPipelineDescriptorClass alloc] init];
	// Let's assume we're rendering into BGRA8Unorm...
	pipeDesc.colorAttachments[0].pixelFormat= MTLPixelFormatBGRA8Unorm;

	// OSX/tvOS insist on more modern enums
#if (UNITY_IPHONE && TARGET_OS_TV) || UNITY_OSX
	pipeDesc.depthAttachmentPixelFormat		= MTLPixelFormatDepth32Float_Stencil8;
	pipeDesc.stencilAttachmentPixelFormat	= MTLPixelFormatDepth32Float_Stencil8;
#else
	pipeDesc.depthAttachmentPixelFormat		= MTLPixelFormatDepth32Float;
	pipeDesc.stencilAttachmentPixelFormat	= MTLPixelFormatStencil8;
#endif

	pipeDesc.sampleCount = 1;
	pipeDesc.colorAttachments[0].blendingEnabled = NO;

	pipeDesc.vertexFunction		= vertexFunction;
	pipeDesc.fragmentFunction	= fragmentFunction;
	pipeDesc.vertexDescriptor	= vertexDesc;

	m_Pipeline = [metalDevice newRenderPipelineStateWithDescriptor:pipeDesc error:&error];
	if (error != nil)
	{
		::fprintf(stderr, "Metal: Error creating pipeline state: %s\n%s\n", [[error localizedDescription] UTF8String], [[error localizedFailureReason] UTF8String]);
		error = nil;
	}

	// Depth/Stencil state
	MTLDepthStencilDescriptor* depthDesc = [[MTLDepthStencilDescriptorClass alloc] init];
	depthDesc.depthCompareFunction = GetUsesReverseZ() ? MTLCompareFunctionGreaterEqual : MTLCompareFunctionLessEqual;
	depthDesc.depthWriteEnabled = false;
	m_DepthStencil = [metalDevice newDepthStencilStateWithDescriptor:depthDesc];
}


RenderAPI_Metal::RenderAPI_Metal()
{
}


void RenderAPI_Metal::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	if (type == kUnityGfxDeviceEventInitialize)
	{
		m_MetalGraphics = interfaces->Get<IUnityGraphicsMetal>();
		NSBundle* metalBundle = m_MetalGraphics->MetalBundle();
		MTLVertexDescriptorClass			= [metalBundle classNamed:@"MTLVertexDescriptor"];
		MTLRenderPipelineDescriptorClass	= [metalBundle classNamed:@"MTLRenderPipelineDescriptor"];
		MTLDepthStencilDescriptorClass		= [metalBundle classNamed:@"MTLDepthStencilDescriptor"];

		CreateResources();
	}
	else if (type == kUnityGfxDeviceEventShutdown)
	{
		//@TODO: release resources
	}
}


void RenderAPI_Metal::DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4)
{
	// Update vertex and constant buffers
	//@TODO: we don't do any synchronization here :)

	const int vbSize = triangleCount * 3 * kVertexSize;
	const int cbSize = 16 * sizeof(float);

	::memcpy(m_VertexBuffer.contents, verticesFloat3Byte4, vbSize);
	::memcpy(m_ConstantBuffer.contents, worldMatrix, cbSize);

#if UNITY_OSX
	[m_VertexBuffer didModifyRange:NSMakeRange(0, vbSize)];
	[m_ConstantBuffer didModifyRange:NSMakeRange(0, cbSize)];
#endif

	id<MTLRenderCommandEncoder> cmd = (id<MTLRenderCommandEncoder>)m_MetalGraphics->CurrentCommandEncoder();

	// Setup rendering state
	[cmd setRenderPipelineState:m_Pipeline];
	[cmd setDepthStencilState:m_DepthStencil];
	[cmd setCullMode:MTLCullModeNone];

	// Bind buffers
	[cmd setVertexBuffer:m_VertexBuffer offset:0 atIndex:1];
	[cmd setVertexBuffer:m_ConstantBuffer offset:0 atIndex:0];

	// Draw
	[cmd drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:triangleCount*3];
}


void* RenderAPI_Metal::BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch)
{
	const int rowPitch = textureWidth * 4;
	// Just allocate a system memory buffer here for simplicity
	unsigned char* data = new unsigned char[rowPitch * textureHeight];
	*outRowPitch = rowPitch;
	return data;
}


void RenderAPI_Metal::EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr)
{
	id<MTLTexture> tex = (__bridge id<MTLTexture>)textureHandle;
	// Update texture data, and free the memory buffer
	[tex replaceRegion:MTLRegionMake3D(0,0,0, textureWidth,textureHeight,1) mipmapLevel:0 withBytes:dataPtr bytesPerRow:rowPitch];
	delete[](unsigned char*)dataPtr;
}


void* RenderAPI_Metal::BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize)
{
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)bufferHandle;
	*outBufferSize = [buf length];
	return [buf contents];
}


void RenderAPI_Metal::EndModifyVertexBuffer(void* bufferHandle)
{
#	if UNITY_OSX
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)bufferHandle;
	[buf didModifyRange:NSMakeRange(0, buf.length)];
#	endif // if UNITY_OSX
}


#endif // #if SUPPORT_METAL
