#include "RenderAPI.h"
#include "PlatformBase.h"

#include <cmath>

// Direct3D 12 implementation of RenderAPI.

#if SUPPORT_D3D12

#include <assert.h>
#include <dxgi1_6.h>
#include <initguid.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "Unity/IUnityGraphicsD3D12.h"
#include <atomic>
#include <unordered_map>
#include <utility>
#include <map>
#include <iostream>

#define ReturnOnFail(x, hr, OnFailureMsg, onFailureReturnValue) hr = x; if(FAILED(hr)){OutputDebugStringA(OnFailureMsg); return onFailureReturnValue;}

#define D3D12_DEFAULT_HEAP_TRIANGLE_BUFFER_NAME L"Native Plugin Default Heap Triangle Vertex Buffer"
#define D3D12_UPLOAD_HEAP_TRIANGLE_BUFFER_NAME L"Native Plugin Upload Heap Triangle Vertex Buffer"
#define D3D12_UPLOAD_HEAP_TEXTURE_BUFFER_NAME L"Native Plugin Upload Heap Texture"
#define D3D12_UPLOAD_HEAP_VERTEX_BUFFER_NAME L"Native Plugin Upload Heap Vertex Buffer"

// Compiled from:
/*
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
 */

 // using:
 /*
     dxc -T vs_6_0 -E VSMain -Fo vertex_shader .\shaders.hlsl -Qstrip_reflect -Qstrip_debug
     dxc -T ps_6_0 -E PSMain -Fo pixel_shader .\shaders.hlsl -Qstrip_reflect -Qstrip_debug
 */

const BYTE vertex_shader[] = 
{
    68,88,66,67,128,57,232,34,242,147,249,9,179,113,162,105,42,26,31,
    250,1,0,0,0,109,9,0,0,6,0,0,0,56,0,0,0,72,0,0,
    0,167,0,0,0,9,1,0,0,237,1,0,0,9,2,0,0,83,70,73,
    48,8,0,0,0,0,0,0,0,0,0,0,0,73,83,71,49,87,0,0,
    0,2,0,0,0,8,0,0,0,0,0,0,0,72,0,0,0,0,0,0,
    0,0,0,0,0,3,0,0,0,0,0,0,0,7,7,0,0,0,0,0,
    0,0,0,0,0,81,0,0,0,0,0,0,0,0,0,0,0,3,0,0,
    0,1,0,0,0,15,15,0,0,0,0,0,0,80,79,83,73,84,73,79,
    78,0,67,79,76,79,82,0,79,83,71,49,90,0,0,0,2,0,0,0,
    8,0,0,0,0,0,0,0,72,0,0,0,0,0,0,0,0,0,0,0,
    3,0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,
    78,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,1,0,0,0,
    15,0,0,0,0,0,0,0,67,79,76,79,82,0,83,86,95,80,111,115,
    105,116,105,111,110,0,80,83,86,48,220,0,0,0,48,0,0,0,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
    255,255,1,0,0,0,2,2,0,2,2,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,0,0,0,24,0,0,0,2,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,13,0,0,0,0,0,0,0,24,0,
    0,0,0,80,79,83,73,84,73,79,78,0,67,79,76,79,82,0,67,79,
    76,79,82,0,0,0,1,0,0,0,0,0,0,0,16,0,0,0,1,0,
    0,0,0,0,0,0,1,0,67,0,3,0,0,0,10,0,0,0,0,0,
    0,0,1,1,68,0,3,0,0,0,16,0,0,0,0,0,0,0,1,0,
    68,0,3,2,0,0,0,0,0,0,0,0,0,0,1,1,68,3,3,4,
    0,0,240,0,0,0,240,0,0,0,240,0,0,0,0,0,0,0,1,0,
    0,0,2,0,0,0,4,0,0,0,8,0,0,0,72,65,83,72,20,0,
    0,0,0,0,0,0,104,176,13,123,21,10,21,217,124,92,148,133,249,234,
    46,114,68,88,73,76,92,7,0,0,96,0,1,0,215,1,0,0,68,88,
    73,76,0,1,0,0,16,0,0,0,68,7,0,0,66,67,192,222,33,12,
    0,0,206,1,0,0,11,130,32,0,2,0,0,0,19,0,0,0,7,129,
    35,145,65,200,4,73,6,16,50,57,146,1,132,12,37,5,8,25,30,4,
    139,98,128,20,69,2,66,146,11,66,164,16,50,20,56,8,24,75,10,50,
    82,136,72,144,20,32,67,70,136,165,0,25,50,66,228,72,14,144,145,34,
    196,80,65,81,129,140,225,131,229,138,4,41,70,6,81,24,0,0,8,0,
    0,0,27,140,224,255,255,255,255,7,64,2,168,13,132,240,255,255,255,255,
    3,32,109,48,134,255,255,255,255,31,0,9,168,0,73,24,0,0,3,0,
    0,0,19,130,96,66,32,76,8,6,0,0,0,0,137,32,0,0,35,0,
    0,0,50,34,72,9,32,100,133,4,147,34,164,132,4,147,34,227,132,161,
    144,20,18,76,138,140,11,132,164,76,16,104,35,0,37,0,20,102,0,230,
    8,192,96,142,0,41,198,32,132,20,66,166,24,128,16,82,6,161,163,134,
    203,159,176,135,144,124,110,163,138,149,152,252,226,182,17,49,198,24,84,238,
    25,46,127,194,30,66,242,67,160,25,22,2,5,171,16,138,48,66,173,20,
    131,140,49,232,205,17,4,197,96,164,16,18,73,14,4,12,35,16,67,18,
    212,123,14,71,154,22,0,115,168,201,55,49,110,67,129,165,155,10,4,0,
    0,0,19,20,114,192,135,116,96,135,54,104,135,121,104,3,114,192,135,13,
    175,80,14,109,208,14,122,80,14,109,0,15,122,48,7,114,160,7,115,32,
    7,109,144,14,113,160,7,115,32,7,109,144,14,120,160,7,115,32,7,109,
    144,14,113,96,7,122,48,7,114,208,6,233,48,7,114,160,7,115,32,7,
    109,144,14,118,64,7,122,96,7,116,208,6,230,16,7,118,160,7,115,32,
    7,109,96,14,115,32,7,122,48,7,114,208,6,230,96,7,116,160,7,118,
    64,7,109,224,14,120,160,7,113,96,7,122,48,7,114,160,7,118,64,7,
    67,158,0,0,0,0,0,0,0,0,0,0,0,134,60,6,16,0,1,0,
    0,0,0,0,0,0,12,121,16,32,0,4,0,0,0,0,0,0,0,24,
    242,52,64,0,12,0,0,0,0,0,0,0,48,228,121,128,0,8,0,0,
    0,0,0,0,0,96,200,35,1,1,48,0,0,0,0,0,0,0,64,22,
    8,0,14,0,0,0,50,30,152,20,25,17,76,144,140,9,38,71,198,4,
    67,34,37,48,2,80,12,5,24,80,6,229,80,30,84,74,98,4,160,12,
    10,161,8,8,207,0,80,30,75,65,16,248,128,15,248,0,2,129,64,0,
    0,0,121,24,0,0,87,0,0,0,26,3,76,144,70,2,19,196,49,32,
    195,27,67,129,147,75,179,11,163,43,75,1,137,113,185,113,129,113,153,193,
    201,177,1,65,129,145,137,49,195,49,155,41,139,73,217,16,4,19,4,194,
    152,32,16,199,6,97,32,38,8,4,178,65,24,12,10,118,115,27,6,196,
    32,38,8,152,68,96,130,64,36,27,16,66,89,8,98,96,128,13,65,179,
    129,0,0,7,152,32,100,211,134,0,154,32,8,0,5,171,41,34,80,79,
    83,73,84,73,79,78,19,132,162,153,32,20,206,134,128,152,32,20,207,4,
    129,80,38,8,196,178,65,200,180,13,11,65,85,214,101,13,24,97,109,44,
    134,158,152,158,164,38,8,5,52,65,32,152,13,66,246,109,88,134,174,178,
    46,107,240,6,11,12,54,8,92,24,76,16,138,104,195,66,116,149,117,141,
    193,224,17,22,24,112,153,178,250,130,122,155,75,163,75,123,115,219,176,12,
    101,80,97,151,55,120,131,5,6,27,4,50,48,131,13,131,24,156,1,176,
    161,144,38,52,120,128,42,108,108,118,109,46,105,100,101,110,116,83,130,160,
    10,25,158,139,93,153,220,92,218,155,219,148,128,104,66,134,231,98,23,198,
    102,87,38,55,37,48,234,144,225,185,204,161,133,145,149,201,53,189,145,149,
    177,77,9,144,50,100,120,46,114,101,115,111,117,114,99,101,115,83,2,167,
    14,25,158,139,93,90,217,93,18,217,20,93,24,93,217,148,0,170,67,134,
    231,82,230,70,39,151,7,245,150,230,70,55,55,37,64,3,0,0,121,24,
    0,0,76,0,0,0,51,8,128,28,196,225,28,102,20,1,61,136,67,56,
    132,195,140,66,128,7,121,120,7,115,152,113,12,230,0,15,237,16,14,244,
    128,14,51,12,66,30,194,193,29,206,161,28,102,48,5,61,136,67,56,132,
    131,27,204,3,61,200,67,61,140,3,61,204,120,140,116,112,7,123,8,7,
    121,72,135,112,112,7,122,112,3,118,120,135,112,32,135,25,204,17,14,236,
    144,14,225,48,15,110,48,15,227,240,14,240,80,14,51,16,196,29,222,33,
    28,216,33,29,194,97,30,102,48,137,59,188,131,59,208,67,57,180,3,60,
    188,131,60,132,3,59,204,240,20,118,96,7,123,104,7,55,104,135,114,104,
    7,55,128,135,112,144,135,112,96,7,118,40,7,118,248,5,118,120,135,119,
    128,135,95,8,135,113,24,135,114,152,135,121,152,129,44,238,240,14,238,224,
    14,245,192,14,236,48,3,98,200,161,28,228,161,28,204,161,28,228,161,28,
    220,97,28,202,33,28,196,129,29,202,97,6,214,144,67,57,200,67,57,152,
    67,57,200,67,57,184,195,56,148,67,56,136,3,59,148,195,47,188,131,60,
    252,130,59,212,3,59,176,195,12,196,33,7,124,112,3,122,40,135,118,128,
    135,25,209,67,14,248,224,6,228,32,14,231,224,6,246,16,14,242,192,14,
    225,144,15,239,80,15,244,0,0,0,113,32,0,0,24,0,0,0,6,32,
    188,172,13,108,195,229,59,143,47,4,84,81,16,81,233,0,67,73,24,128,
    128,249,197,109,91,129,52,92,190,243,248,66,68,0,19,17,2,205,176,16,
    22,48,13,151,239,60,254,226,0,131,216,60,212,228,23,183,109,2,213,112,
    249,206,227,75,147,19,17,40,53,61,212,228,23,183,109,4,210,112,249,206,
    227,79,68,52,33,64,132,249,197,109,3,0,0,0,97,32,0,0,122,0,
    0,0,19,4,65,44,16,0,0,0,5,0,0,0,68,138,171,20,10,97,
    6,160,236,74,174,8,168,148,0,197,17,0,0,0,35,6,9,0,130,96,
    32,97,4,99,89,193,136,65,2,128,32,24,24,29,130,93,208,49,98,144,
    0,32,8,6,134,151,100,24,129,140,24,36,0,8,130,129,241,41,90,246,
    36,35,6,9,0,130,96,96,128,193,178,105,146,50,98,144,0,32,8,6,
    70,24,48,219,70,45,35,6,9,0,130,96,96,136,65,195,113,8,51,98,
    144,0,32,8,6,198,24,56,93,55,53,35,6,7,0,130,96,208,136,65,
    131,120,163,9,1,48,154,32,4,163,9,131,48,154,64,12,35,6,7,0,
    130,96,208,156,129,212,144,193,104,66,0,140,38,8,193,104,194,32,140,38,
    16,195,136,193,1,128,32,24,52,108,112,73,213,104,66,0,140,38,8,193,
    104,194,32,140,38,16,195,136,193,1,128,32,24,52,113,192,93,106,48,154,
    16,0,163,9,66,48,154,48,8,163,9,196,96,211,37,159,17,3,4,0,
    65,48,120,236,192,12,158,43,24,49,64,0,16,4,131,231,14,206,96,185,
    2,11,14,232,152,181,201,103,196,0,1,64,16,12,30,61,80,3,105,11,
    70,12,16,0,4,193,224,217,131,53,112,182,192,2,5,58,150,125,242,25,
    49,64,0,16,4,131,199,15,220,160,250,130,17,3,4,0,65,48,120,254,
    224,13,162,47,176,160,129,142,113,99,32,159,17,3,4,0,65,48,120,68,
    65,14,176,49,8,70,12,16,0,4,193,224,25,133,57,160,198,32,176,0,
    130,206,136,65,2,128,32,24,32,167,64,7,163,32,10,123,208,140,24,36,
    0,8,130,1,114,10,116,48,10,162,224,6,201,136,65,2,128,32,24,32,
    167,64,7,163,32,10,120,80,140,24,36,0,8,130,1,114,10,116,48,10,
    162,160,7,193,136,65,2,128,32,24,32,167,64,7,162,32,10,123,176,6,
    35,6,9,0,130,96,128,156,2,29,136,130,40,184,129,26,140,24,36,0,
    8,130,1,114,10,116,32,10,162,128,7,105,48,98,144,0,32,8,6,200,
    41,208,129,40,136,130,30,160,1,2,0,0,0,0
};

const BYTE pixel_shader[] = 
{
    68,88,66,67,155,38,61,235,152,175,36,0,5,163,174,225,114,25,46,
    165,1,0,0,0,48,6,0,0,6,0,0,0,56,0,0,0,72,0,0,
    0,126,0,0,0,184,0,0,0,64,1,0,0,92,1,0,0,83,70,73,
    48,8,0,0,0,0,0,0,0,0,0,0,0,73,83,71,49,46,0,0,
    0,1,0,0,0,8,0,0,0,0,0,0,0,40,0,0,0,0,0,0,
    0,0,0,0,0,3,0,0,0,0,0,0,0,15,15,0,0,0,0,0,
    0,67,79,76,79,82,0,79,83,71,49,50,0,0,0,1,0,0,0,8,
    0,0,0,0,0,0,0,40,0,0,0,0,0,0,0,64,0,0,0,3,
    0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,83,86,95,84,97,
    114,103,101,116,0,80,83,86,48,128,0,0,0,48,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
    255,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,8,0,0,0,0,67,79,76,79,82,0,
    0,1,0,0,0,0,0,0,0,16,0,0,0,1,0,0,0,0,0,0,
    0,1,0,68,0,3,2,0,0,0,0,0,0,0,0,0,0,1,0,68,
    16,3,0,0,0,1,0,0,0,2,0,0,0,4,0,0,0,8,0,0,
    0,72,65,83,72,20,0,0,0,0,0,0,0,12,2,178,107,86,235,87,
    236,39,12,117,190,13,71,209,140,68,88,73,76,204,4,0,0,96,0,0,
    0,51,1,0,0,68,88,73,76,0,1,0,0,16,0,0,0,180,4,0,
    0,66,67,192,222,33,12,0,0,42,1,0,0,11,130,32,0,2,0,0,
    0,19,0,0,0,7,129,35,145,65,200,4,73,6,16,50,57,146,1,132,
    12,37,5,8,25,30,4,139,98,128,16,69,2,66,146,11,66,132,16,50,
    20,56,8,24,75,10,50,66,136,72,144,20,32,67,70,136,165,0,25,50,
    66,228,72,14,144,17,34,196,80,65,81,129,140,225,131,229,138,4,33,70,
    6,81,24,0,0,6,0,0,0,27,140,224,255,255,255,255,7,64,2,168,
    13,132,240,255,255,255,255,3,32,1,0,0,0,73,24,0,0,2,0,0,
    0,19,130,96,66,32,0,0,0,137,32,0,0,15,0,0,0,50,34,8,
    9,32,100,133,4,19,34,164,132,4,19,34,227,132,161,144,20,18,76,136,
    140,11,132,132,76,16,48,35,0,37,0,138,25,128,57,2,48,152,35,64,
    138,49,68,84,68,86,12,32,162,26,194,129,128,52,32,0,0,19,20,114,
    192,135,116,96,135,54,104,135,121,104,3,114,192,135,13,175,80,14,109,208,
    14,122,80,14,109,0,15,122,48,7,114,160,7,115,32,7,109,144,14,113,
    160,7,115,32,7,109,144,14,120,160,7,115,32,7,109,144,14,113,96,7,
    122,48,7,114,208,6,233,48,7,114,160,7,115,32,7,109,144,14,118,64,
    7,122,96,7,116,208,6,230,16,7,118,160,7,115,32,7,109,96,14,115,
    32,7,122,48,7,114,208,6,230,96,7,116,160,7,118,64,7,109,224,14,
    120,160,7,113,96,7,122,48,7,114,160,7,118,64,7,67,158,0,0,0,
    0,0,0,0,0,0,0,0,134,60,6,16,0,1,0,0,0,0,0,0,
    0,12,121,16,32,0,4,0,0,0,0,0,0,0,200,2,1,11,0,0,
    0,50,30,152,16,25,17,76,144,140,9,38,71,198,4,67,162,18,24,1,
    40,134,50,40,15,170,146,24,1,40,130,66,40,16,218,177,12,130,8,4,
    2,1,0,0,0,121,24,0,0,65,0,0,0,26,3,76,144,70,2,19,
    196,49,32,195,27,67,129,147,75,179,11,163,43,75,1,137,113,185,113,129,
    113,153,193,201,177,1,65,129,145,137,49,195,49,155,41,139,73,217,16,4,
    19,4,98,152,32,16,196,6,97,32,38,8,68,177,65,24,12,10,112,115,
    27,6,196,32,38,8,75,179,33,80,38,8,2,64,1,106,138,197,208,19,
    211,147,212,4,161,64,38,8,69,178,33,32,38,8,133,50,65,40,150,9,
    2,97,76,16,136,99,131,64,85,27,22,194,121,160,72,26,38,2,178,54,
    4,23,147,41,171,47,170,48,185,179,50,186,9,66,193,108,88,136,236,209,
    34,104,152,8,200,218,16,108,27,6,140,3,54,20,76,211,1,64,21,54,
    54,187,54,151,52,178,50,55,186,41,65,80,133,12,207,197,174,76,110,46,
    237,205,109,74,64,52,33,195,115,177,11,99,179,43,147,155,18,24,117,200,
    240,92,230,208,194,200,202,228,154,222,200,202,216,166,4,72,29,50,60,23,
    187,180,178,187,36,178,41,186,48,186,178,41,129,82,135,12,207,165,204,141,
    78,46,15,234,45,205,141,110,110,74,208,1,0,121,24,0,0,76,0,0,
    0,51,8,128,28,196,225,28,102,20,1,61,136,67,56,132,195,140,66,128,
    7,121,120,7,115,152,113,12,230,0,15,237,16,14,244,128,14,51,12,66,
    30,194,193,29,206,161,28,102,48,5,61,136,67,56,132,131,27,204,3,61,
    200,67,61,140,3,61,204,120,140,116,112,7,123,8,7,121,72,135,112,112,
    7,122,112,3,118,120,135,112,32,135,25,204,17,14,236,144,14,225,48,15,
    110,48,15,227,240,14,240,80,14,51,16,196,29,222,33,28,216,33,29,194,
    97,30,102,48,137,59,188,131,59,208,67,57,180,3,60,188,131,60,132,3,
    59,204,240,20,118,96,7,123,104,7,55,104,135,114,104,7,55,128,135,112,
    144,135,112,96,7,118,40,7,118,248,5,118,120,135,119,128,135,95,8,135,
    113,24,135,114,152,135,121,152,129,44,238,240,14,238,224,14,245,192,14,236,
    48,3,98,200,161,28,228,161,28,204,161,28,228,161,28,220,97,28,202,33,
    28,196,129,29,202,97,6,214,144,67,57,200,67,57,152,67,57,200,67,57,
    184,195,56,148,67,56,136,3,59,148,195,47,188,131,60,252,130,59,212,3,
    59,176,195,12,196,33,7,124,112,3,122,40,135,118,128,135,25,209,67,14,
    248,224,6,228,32,14,231,224,6,246,16,14,242,192,14,225,144,15,239,80,
    15,244,0,0,0,113,32,0,0,10,0,0,0,6,32,164,172,5,76,195,
    229,59,143,191,56,192,32,54,15,53,249,197,109,155,64,53,92,190,243,248,
    210,228,68,4,74,77,15,53,249,197,109,3,0,97,32,0,0,30,0,0,
    0,19,4,65,44,16,0,0,0,3,0,0,0,68,133,48,3,80,10,84,
    37,80,6,0,0,35,6,9,0,130,96,96,72,197,243,40,196,136,65,2,
    128,32,24,24,147,1,65,67,49,98,144,0,32,8,6,6,117,68,209,98,
    140,24,36,0,8,130,129,81,33,146,68,28,35,6,9,0,130,96,128,84,
    199,52,57,196,136,65,2,128,32,24,32,213,49,77,198,48,98,144,0,32,
    8,6,72,117,76,83,35,140,24,36,0,8,130,1,82,29,211,84,4,8,
    0,0,0,0,0
};

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

struct Vertex
{
    Vec3 position;
    Vec4 color;
};

static void handle_hr(HRESULT hr, const char* error = "")
{
    if (FAILED(hr))
    {
        OutputDebugStringA(error);
        std::cerr << error << "\n";
        abort();
    }
}

struct D3D12MemoryObject
{
    ID3D12Resource* resource;
    void* mapped;
    D3D12_HEAP_TYPE heapType;
    D3D12_RESOURCE_FLAGS resourceFlags;
    UINT64 deviceMemorySize;
};

struct D3D12DefaultBufferMemoryObject
{
    D3D12MemoryObject uploadResource;
    D3D12MemoryObject defaultResource;
};

class RenderAPI_D3D12 : public RenderAPI
{
public:
    RenderAPI_D3D12();
    virtual ~RenderAPI_D3D12() override { }

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override;
    virtual bool GetUsesReverseZ() override { return true; }

    //-----------------------------------------------------------
    // These functions will be called from unity render thread,
    // see kUnityD3D12GraphicsQueueAccess_DontCare

    // Demonstrates how to use the CommandRecordingState
    virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4) override;

    // These demonstrate how to submit work via ExecuteCommandList
    virtual void* BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch) override;
    virtual void EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr) override;
    virtual void* BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize) override;
    virtual void EndModifyVertexBuffer(void* bufferHandle) override;
    virtual void drawToRenderTexture() override;
    //-----------------------------------------------------------

    //-----------------------------------------------------------
    // These functions will be called from unity submission thread,
    // see kUnityD3D12GraphicsQueueAccess_Allow

    // Demonstrates how to submit work to unity command queue
    // directly via GetCommandQueue
    virtual void drawToPluginTexture() override;
    //-----------------------------------------------------------

    virtual void* getRenderTexture() override;
    virtual void setRenderTextureResource(UnityRenderBuffer rb) override;

    virtual bool isSwapChainAvailable() override;
    virtual unsigned int getPresentFlags() override;
    virtual unsigned int getSyncInterval() override;
    virtual unsigned int getBackbufferWidth() override;
    virtual unsigned int getBackbufferHeight() override;

private:
    UINT64 align_pow2(UINT64 value);

    // Aligns texture to power of 2 boundary
    UINT64 get_aligned_size(int width, int height, int pixelSize, int rowPitch);

    // Creates a new buffer on D3D12_HEAP_TYPE_UPLOAD when resource points to a nullptr.
    // When resource points to a existing ID3D12Resource* we only check if it's big enough
    // to hold the size, if not return false
    bool get_upload_resource(ID3D12Resource** resource, UINT64 size, LPCWSTR name);

    // Creates dx12 buffer, if heapType allows CPU-access the resource will be also mapped
    bool create_D3D12_buffer(size_t sizeInBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_RESOURCE_STATES initialResourceState, LPCWSTR resourceName, D3D12MemoryObject* buffer);

    // Creates two buffers; one on D3D12_HEAP_TYPE_UPLOAD and one on D3D12_HEAP_TYPE_DEFAULT, 
    // copies contents from initData to -> upload buffer -> default buffer
    bool create_D3D12_default_buffer(ID3D12GraphicsCommandList* cmdLst, const void* initData, unsigned int bufferSizeInBytes, LPCWSTR uploadHeapResourceName, LPCWSTR defaultHeapResourceName, D3D12DefaultBufferMemoryObject& outMemoryObj);

    ID3D12PipelineState* create_triangle_pipeline();
    void create_triangle_input_layout();
    bool create_triangle_root_signature();

    // Push buffer to deletion queue
    void safe_destroy(unsigned long long frameNumber, const D3D12MemoryObject& buffer);

    // Processes deletion queue
    void garbage_collect(bool force = false);

    // Destroy buffer without deletion queue
    void immediate_destroy_d3d12_buffer(D3D12MemoryObject& buffer);

    // Creates and initializes all resources that are used across multiple frames
    void initialize_and_create_resources();
    void release_resources();

    // Records commands that draw a single triangle to target_texture
    void record_draw_cmd_list(ID3D12CommandAllocator* cmd_alloc, ID3D12GraphicsCommandList* cmd, ID3D12RootSignature* rootsig, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle, D3D12_VERTEX_BUFFER_VIEW* vbview, const float* world_matrix, D3D12_VIEWPORT* viewport, ID3D12PipelineState* pso, ID3D12Resource* target_texture, D3D12_RESOURCE_STATES target_state);

    // Demonstrates how to submit work to unity worker thread via ExecuteCommandList API
    UINT64 submit_cmd_to_unity_worker(ID3D12GraphicsCommandList* cmd, UnityGraphicsD3D12ResourceState* resource_states, int state_count);

    // Creates a texture that can be used as render target
    ID3D12Resource* create_render_texture();
    void create_render_texture_rtv(ID3D12DescriptorHeap* desc_heap, ID3D12Resource* target, UINT offset);

    void transition_barrier(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

    // When unity frame fence changes we can be sure that previously submitted command lists have finished executing
    void wait_for_unity_frame_fence(UINT64 fence_value);

    // Wait on any user provided fence
    void wait_on_fence(UINT64 fence_value, ID3D12Fence* fence, HANDLE fence_event);

    DXGI_FORMAT typeless_fmt_to_typed(DXGI_FORMAT format);

    typedef std::vector<D3D12MemoryObject>             D3D12Buffers;
    typedef std::map<unsigned long long, D3D12Buffers> DeleteQueue;
    typedef std::unordered_map<void*, void*>           MappedVertexBuffers;

    IUnityGraphicsD3D12v7*         s_d3d12;

    ID3D12Resource*                s_upload_texture;
    ID3D12Resource*                s_upload_buffer;
    ID3D12PipelineState*           m_triangle_pso;
    D3D12_INPUT_ELEMENT_DESC       m_triangle_layout[2];
    ID3D12RootSignature*           m_triangle_rootsig;
    D3D12DefaultBufferMemoryObject m_triangle_vertex_buffer;
    ID3D12DescriptorHeap*          m_triangle_rtv_desc_heap;
    ID3D12DescriptorHeap*          m_triangle_dsv_desc_heap;
    DeleteQueue                    m_DeleteQueue;
    MappedVertexBuffers            m_mapped_triangle_vertex_buffers;

    ID3D12DescriptorHeap*          m_texture_rtv_desc_heap;
    UINT                           m_texture_rtv_desc_size;
    ID3D12Resource*                m_render_texture_vertex_buffer;
    D3D12_VERTEX_BUFFER_VIEW       m_texture_vertex_buffer_view;
    std::atomic<ID3D12Resource*>   m_render_texture;
    std::atomic<ID3D12Resource*>   m_plugin_texture;
    UINT                           m_texture_width  = 256;
    UINT                           m_texture_height = 256;
    ID3D12CommandAllocator*        m_render_texture_cmd_allocator;
    ID3D12GraphicsCommandList*     m_render_texture_cmd_list;
    ID3D12CommandAllocator*        m_plugin_texture_cmd_allocator;
    ID3D12GraphicsCommandList*     m_plugin_texture_cmd_list;
    ID3DBlob*                      m_render_texture_root_blob;
    ID3D12RootSignature*           m_texture_root_sig;
    ID3D12PipelineState*           m_texture_pso;
    std::atomic<bool>              m_are_resources_initialized = false;

    UINT64                         m_plugin_texture_fence_value = 0;
    ID3D12Fence*                   m_plugin_texture_fence;
    HANDLE                         m_plugin_texture_fence_event;

    ID3D12CommandAllocator*        m_vertex_copy_cmd_allocator;
    ID3D12GraphicsCommandList*     m_vertex_copy_cmd_list;

    ID3D12CommandAllocator*        m_texture_copy_cmd_allocator;
    ID3D12GraphicsCommandList*     m_texture_copy_cmd_list;

    UINT64                         m_vertex_copy_fence = 0;
    UINT64                         m_texture_copy_fence = 0;
    UINT64                         m_render_texture_draw_fence = 0;

    HANDLE                         m_fence_event;

    bool                           m_is_render_texture_created_by_unity = false;
};

RenderAPI* CreateRenderAPI_D3D12()
{
    return new RenderAPI_D3D12();
}

const UINT kNodeMask = 0;

RenderAPI_D3D12::RenderAPI_D3D12()
    : s_d3d12(NULL)
    , s_upload_texture(NULL)
    , s_upload_buffer(NULL)
    , m_triangle_pso(NULL)
    , m_triangle_rootsig(NULL)
    , m_triangle_rtv_desc_heap(NULL)
    , m_triangle_dsv_desc_heap(NULL)
    , m_texture_rtv_desc_heap(NULL)
    , m_texture_rtv_desc_size(NULL)
    , m_render_texture_vertex_buffer(NULL)
    , m_render_texture_cmd_allocator(NULL)
    , m_render_texture_cmd_list(NULL)
{
}

UINT64 CalcByteAlignedValue(unsigned int byteSize, unsigned int byteAlignment)
{
    UINT byteAlignmentMinusOne = byteAlignment - 2;
    return (byteSize + byteAlignmentMinusOne) & ~byteAlignmentMinusOne;
}

UINT64 RenderAPI_D3D12::align_pow2(UINT64 value)
{
    UINT64 aligned = static_cast<UINT64>(pow(2, (int)log2(value)));
    return aligned >= value ? aligned : aligned * 2;
}

UINT64 RenderAPI_D3D12::get_aligned_size(int width, int height, int pixelSize, int rowPitch)
{
    UINT64 size = width * height * pixelSize;

    size = align_pow2(size);

    if (size < D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
    {
        return D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
    }
    else if (width * pixelSize < rowPitch)
    {
        return rowPitch * height;
    }
    else
    {
        return size;
    }
}

bool RenderAPI_D3D12::create_D3D12_default_buffer(ID3D12GraphicsCommandList* cmdLst, const void* initData, unsigned int bufferSizeInBytes, LPCWSTR uploadHeapResourceName, LPCWSTR defaultHeapResourceName, D3D12DefaultBufferMemoryObject& outMemoryObj)
{
    if (!cmdLst)
        return false;

    if (!create_D3D12_buffer(bufferSizeInBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapResourceName, &outMemoryObj.uploadResource))
        return false;

    if (!create_D3D12_buffer(bufferSizeInBytes, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, defaultHeapResourceName, &outMemoryObj.defaultResource))
        return false;

    ID3D12Resource* uploadResource = outMemoryObj.uploadResource.resource;
    ID3D12Resource* defaultResource = outMemoryObj.defaultResource.resource;

    D3D12_SUBRESOURCE_DATA subResourceDataDesc;
    subResourceDataDesc.pData = initData;
    subResourceDataDesc.RowPitch = outMemoryObj.uploadResource.deviceMemorySize;
    subResourceDataDesc.SlicePitch = subResourceDataDesc.RowPitch;

    transition_barrier(cmdLst, defaultResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    UpdateSubresources<1>(cmdLst, defaultResource, uploadResource, 0, 0, 1, &subResourceDataDesc);
    transition_barrier(cmdLst, defaultResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

    return true;
}

bool RenderAPI_D3D12::create_D3D12_buffer(size_t sizeInBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_RESOURCE_STATES initialResourceState, LPCWSTR resourceName, D3D12MemoryObject* buffer)
{
    ID3D12Device* device = s_d3d12->GetDevice();

    UINT64 alignedBufferSizeInBytes = CalcByteAlignedValue(static_cast<unsigned int>(sizeInBytes), 256);

    HRESULT hr = S_OK;

    CD3DX12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(heapType);
    CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(alignedBufferSizeInBytes, resourceFlags);

    ReturnOnFail(
        device->CreateCommittedResource(
            &heap_props,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            initialResourceState,
            NULL,
            IID_PPV_ARGS(&buffer->resource)),
        hr,
        "CreateD3D12Buffer Failed\n",
        false
    );

    if (heapType == D3D12_HEAP_TYPE_UPLOAD)
    {
        hr = buffer->resource->Map(0, NULL, &buffer->mapped);
        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to map buffer.\n");
            buffer->resource->Release();
            buffer->resource = NULL;
            return false;
        }
    }
    else
        buffer->mapped = NULL;

    buffer->heapType = heapType;
    buffer->resourceFlags = resourceFlags;
    buffer->deviceMemorySize = alignedBufferSizeInBytes;
    buffer->resourceFlags = resourceFlags;

    if (resourceName != NULL)
        buffer->resource->SetName(resourceName);

    return true;
}

void RenderAPI_D3D12::create_triangle_input_layout()
{
    m_triangle_layout[0] = D3D12_INPUT_ELEMENT_DESC(
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    );

    m_triangle_layout[1] = D3D12_INPUT_ELEMENT_DESC(
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    );
}

bool RenderAPI_D3D12::create_triangle_root_signature()
{
    HRESULT hr = S_OK;
    CD3DX12_ROOT_PARAMETER slotRootParameter[1];

    slotRootParameter[0].InitAsConstants(64, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc(1, slotRootParameter, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ID3DBlob* serializedRootSigBlob = NULL;
    ID3DBlob* errorBlob = NULL;

    ReturnOnFail(
        D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSigBlob, &errorBlob),
        hr,
        "D3D12SerializeRootSignature Failed\n",
        false
    );

    ID3D12Device* device = s_d3d12->GetDevice();
    ReturnOnFail(
        device->CreateRootSignature(0, serializedRootSigBlob->GetBufferPointer(), serializedRootSigBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_triangle_rootsig)); ,
        hr,
        "CreateRootSignature Failed\n",
        false
    );

    return true;
}

ID3D12PipelineState* RenderAPI_D3D12::create_triangle_pipeline()
{
    HRESULT hr = S_OK;

    if (!create_triangle_root_signature())
        return NULL;

    create_triangle_input_layout();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
    ZeroMemory(&desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    desc.InputLayout = { m_triangle_layout, 2 };
    desc.pRootSignature = m_triangle_rootsig;
    desc.VS = {
        vertex_shader,
        sizeof(vertex_shader)
    };

    desc.PS = {
        pixel_shader,
        sizeof(pixel_shader)
    };

    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.RasterizerState.MultisampleEnable = true;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.DepthStencilState.DepthEnable = true;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  // Unity uses reverse z depth
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

    desc.SampleMask = UINT_MAX;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    ID3D12Device* device = s_d3d12->GetDevice();
    ReturnOnFail(
        device->CreateGraphicsPipelineState(&desc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_triangle_pso)),
        hr,
        "CreateGraphicsPipelineState Failed\n",
        NULL
    );

    return m_triangle_pso;
}

bool RenderAPI_D3D12::get_upload_resource(ID3D12Resource** outResource, UINT64 size, LPCWSTR name)
{
    ID3D12Resource*& resource = *outResource;
    if (resource)
    {
        D3D12_RESOURCE_DESC desc = resource->GetDesc();
        if (desc.Width == size)
            return true;
        else
        {
            resource->Release();
            return false;
        }
    }

    // Texture upload buffer
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = kNodeMask;
    heapProps.VisibleNodeMask = kNodeMask;

    D3D12_RESOURCE_DESC heapDesc = {};
    heapDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    heapDesc.Alignment = 0;
    heapDesc.Width = size;
    heapDesc.Height = 1;
    heapDesc.DepthOrArraySize = 1;
    heapDesc.MipLevels = 1;
    heapDesc.Format = DXGI_FORMAT_UNKNOWN;
    heapDesc.SampleDesc.Count = 1;
    heapDesc.SampleDesc.Quality = 0;
    heapDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    heapDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = S_OK;
    ID3D12Device* device = s_d3d12->GetDevice();
    ReturnOnFail(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &heapDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&resource)),
        hr,
        "CreateCommittedResource Failed\n",
        false
    );

    if (name != NULL)
        resource->SetName(name);

    return true;
}

void RenderAPI_D3D12::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
    switch (type)
    {
    case kUnityGfxDeviceEventInitialize:
        s_d3d12 = interfaces->Get<IUnityGraphicsD3D12v7>();

        UnityD3D12PluginEventConfig config_1;
        config_1.graphicsQueueAccess = kUnityD3D12GraphicsQueueAccess_DontCare;
        config_1.flags = kUnityD3D12EventConfigFlag_SyncWorkerThreads | kUnityD3D12EventConfigFlag_ModifiesCommandBuffersState | kUnityD3D12EventConfigFlag_EnsurePreviousFrameSubmission;
        config_1.ensureActiveRenderTextureIsBound = true;
        s_d3d12->ConfigureEvent(1, &config_1);

        UnityD3D12PluginEventConfig config_2;
        config_2.graphicsQueueAccess = kUnityD3D12GraphicsQueueAccess_Allow;
        config_2.flags = kUnityD3D12EventConfigFlag_SyncWorkerThreads | kUnityD3D12EventConfigFlag_ModifiesCommandBuffersState | kUnityD3D12EventConfigFlag_EnsurePreviousFrameSubmission;
        config_2.ensureActiveRenderTextureIsBound = false;
        s_d3d12->ConfigureEvent(2, &config_2);

        initialize_and_create_resources();
        break;
    case kUnityGfxDeviceEventShutdown:
        release_resources();
        break;
    }
}

void RenderAPI_D3D12::DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4)
{
    UnityGraphicsD3D12RecordingState recordingState;
    if (!s_d3d12->CommandRecordingState(&recordingState))
        return;

    if (m_triangle_pso == NULL)
    {
        m_triangle_pso = create_triangle_pipeline();
    }

    if (m_triangle_pso != NULL)
    {
        ID3D12GraphicsCommandList* cmdLst = recordingState.commandList;

        garbage_collect();

        const int kVertexSize = 12 + 4; // 12 bytes for position and 4 bytes for color
        const UINT vertexBufferSizeInBytes = kVertexSize * 3 * triangleCount;
        if (!create_D3D12_default_buffer(cmdLst, verticesFloat3Byte4, vertexBufferSizeInBytes, D3D12_UPLOAD_HEAP_TRIANGLE_BUFFER_NAME, D3D12_DEFAULT_HEAP_TRIANGLE_BUFFER_NAME, m_triangle_vertex_buffer))
            return;

        cmdLst->SetPipelineState(m_triangle_pso);
        cmdLst->SetGraphicsRootSignature(m_triangle_rootsig);
        cmdLst->SetGraphicsRoot32BitConstants(0, 64, worldMatrix, 0);

        D3D12_VERTEX_BUFFER_VIEW vbView;
        vbView.BufferLocation = m_triangle_vertex_buffer.defaultResource.resource->GetGPUVirtualAddress();
        vbView.SizeInBytes = vertexBufferSizeInBytes;
        vbView.StrideInBytes = kVertexSize;

        cmdLst->IASetVertexBuffers(0, 1, &vbView);
        cmdLst->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdLst->DrawInstanced(triangleCount * 3, 1, 0, 0);

        UINT64 nextval = s_d3d12->GetNextFrameFenceValue();

        ID3D12Fence* fence = s_d3d12->GetFrameFence();
        UINT64 lastCompletedFenceValue = fence->GetCompletedValue();
        lastCompletedFenceValue += 3;
        safe_destroy(lastCompletedFenceValue, m_triangle_vertex_buffer.uploadResource);
        safe_destroy(lastCompletedFenceValue, m_triangle_vertex_buffer.defaultResource);
    }
}

void RenderAPI_D3D12::safe_destroy(unsigned long long frameNumber, const D3D12MemoryObject& buffer)
{
    m_DeleteQueue[frameNumber].push_back(buffer);
}

void RenderAPI_D3D12::garbage_collect(bool force /*= false*/)
{
    ID3D12Fence* fence = s_d3d12->GetFrameFence();
    UINT64 lastCompletedFenceValue = fence->GetCompletedValue();

    DeleteQueue::iterator it = m_DeleteQueue.begin();
    while (it != m_DeleteQueue.end())
    {
        if (force || it->first <= lastCompletedFenceValue)
        {
            for (size_t i = 0; i < it->second.size(); ++i)
                immediate_destroy_d3d12_buffer(it->second[i]);
            m_DeleteQueue.erase(it++);
        }
        else
            ++it;
    }
}

void RenderAPI_D3D12::immediate_destroy_d3d12_buffer(D3D12MemoryObject& buffer)
{
    if (buffer.resource == NULL)
        return;

    if (buffer.mapped)
    {
        buffer.resource->Unmap(0, NULL);
        buffer.mapped = 0;
    }

    SAFE_RELEASE(buffer.resource);
}

void RenderAPI_D3D12::initialize_and_create_resources()
{
    if (m_are_resources_initialized)
        return;

    ID3D12Device* device = s_d3d12->GetDevice();
    assert(device != nullptr);

    UnityGraphicsD3D12PhysicalVideoMemoryControlValues control_values;
    control_values.reservation = 64000000;
    control_values.systemMemoryThreshold = 64000000;
    control_values.residencyHysteresisThreshold = 128000000;
    control_values.nonEvictableRelativeThreshold = 0.25;
    s_d3d12->SetPhysicalVideoMemoryControlValues(&control_values);

    constexpr Vertex vertices[] {
        //      position                   color
        //  <--------------->      <-------------------->
        {{  0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        {{  1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        {{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

     D3D12_INPUT_ELEMENT_DESC position = {};
     position.SemanticName = "POSITION";
     position.SemanticIndex = 0;
     position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
     position.AlignedByteOffset = 0;
     position.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
     position.InstanceDataStepRate = 0;

     D3D12_INPUT_ELEMENT_DESC color = {};
     color.SemanticName = "COLOR";
     color.SemanticIndex = 0;
     color.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
     color.AlignedByteOffset = 12;
     color.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
     color.InstanceDataStepRate = 0;

    D3D12_INPUT_ELEMENT_DESC input_element_desc[] = {position, color};

    D3D12_RESOURCE_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertex_buffer_desc.Alignment = 0;
    vertex_buffer_desc.Width = sizeof(vertices);
    vertex_buffer_desc.Height = 1;
    vertex_buffer_desc.DepthOrArraySize = 1;
    vertex_buffer_desc.MipLevels = 1;
    vertex_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
    vertex_buffer_desc.SampleDesc.Count = 1;
    vertex_buffer_desc.SampleDesc.Quality = 0;
    vertex_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertex_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES vertex_buffer_heap_props = {};
    vertex_buffer_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
    vertex_buffer_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    vertex_buffer_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    vertex_buffer_heap_props.CreationNodeMask = 1;
    vertex_buffer_heap_props.VisibleNodeMask = 1;

    handle_hr(device->CreateCommittedResource(&vertex_buffer_heap_props, D3D12_HEAP_FLAG_NONE, &vertex_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_render_texture_vertex_buffer)),
              "Failed to create vertex buffer for render texture\n");

    char* data;
    D3D12_RANGE read_range{ 0, 0 };
    handle_hr(m_render_texture_vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&data)));
    memcpy(data, vertices, sizeof(vertices));
    m_render_texture_vertex_buffer->Unmap(0, nullptr);

    m_texture_vertex_buffer_view = {};
    m_texture_vertex_buffer_view.BufferLocation = m_render_texture_vertex_buffer->GetGPUVirtualAddress();
    m_texture_vertex_buffer_view.SizeInBytes = sizeof(vertices);
    m_texture_vertex_buffer_view.StrideInBytes = sizeof(Vertex);

    // 1st descriptor is for m_render_texture and 2nd one is for m_plugin_texture
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.NumDescriptors = 2;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtv_heap_desc.NodeMask = 0;

    handle_hr(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_texture_rtv_desc_heap)),
              "Failed to create descriptor heap for render texture RTVs\n");

    m_texture_rtv_desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // When Unity does create it's own RenderBuffer and passes it to the plugin
    // using setRenderTexture() we don't have to create one here.
    if (!m_is_render_texture_created_by_unity)
        m_render_texture = create_render_texture();

    m_plugin_texture = create_render_texture();

    create_render_texture_rtv(m_texture_rtv_desc_heap, m_render_texture, 0);
    create_render_texture_rtv(m_texture_rtv_desc_heap, m_plugin_texture, 1);

    m_render_texture.load()->SetName(L"plugin render texture\n");

    // we store the world matrix in the root signature
    D3D12_ROOT_CONSTANTS root_constants = {};
    root_constants.ShaderRegister = 0;
    root_constants.RegisterSpace = 0;
    root_constants.Num32BitValues = 64; // maximum size for root sig

    D3D12_ROOT_PARAMETER root_param = {};
    root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    root_param.Constants = root_constants;
    root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.NumParameters = 1;
    root_signature_desc.pParameters = &root_param;
    root_signature_desc.NumStaticSamplers = 0;
    root_signature_desc.pStaticSamplers = nullptr;
    root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    handle_hr(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &m_render_texture_root_blob, nullptr),
              "Failed to serialize root signature for render texture\n");
    handle_hr(device->CreateRootSignature(0, m_render_texture_root_blob->GetBufferPointer(), m_render_texture_root_blob->GetBufferSize(), IID_PPV_ARGS(&m_texture_root_sig)),
              "Failed to create root signature for render texture\n");

    D3D12_RASTERIZER_DESC rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer_desc.FrontCounterClockwise = FALSE;
    rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer_desc.SlopeScaledDepthBias = 0;
    rasterizer_desc.MultisampleEnable = FALSE;
    rasterizer_desc.MultisampleEnable = FALSE;
    rasterizer_desc.AntialiasedLineEnable = FALSE;
    rasterizer_desc.ForcedSampleCount = 0;
    rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc = {};
    render_target_blend_desc.BlendEnable = FALSE;
    render_target_blend_desc.LogicOpEnable = FALSE;
    render_target_blend_desc.SrcBlend = D3D12_BLEND_ONE;
    render_target_blend_desc.DestBlend = D3D12_BLEND_ZERO;
    render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
    render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
    render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
    render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    render_target_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
    render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_BLEND_DESC blend_desc = {};
    blend_desc.AlphaToCoverageEnable = FALSE;
    blend_desc.IndependentBlendEnable = FALSE;
    blend_desc.RenderTarget[0] = render_target_blend_desc;

    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = FALSE;
    depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
    depth_stencil_desc.StencilEnable = FALSE;

    D3D12_SHADER_BYTECODE vertex_bytecode = {};
    vertex_bytecode.pShaderBytecode = (void*)(vertex_shader);
    vertex_bytecode.BytecodeLength = sizeof(vertex_shader);

    D3D12_SHADER_BYTECODE pixel_bytecode = {};
    pixel_bytecode.pShaderBytecode = (void*)(pixel_shader);
    pixel_bytecode.BytecodeLength = sizeof(pixel_shader);

    D3D12_INPUT_LAYOUT_DESC input_layout_desc = {};
    input_layout_desc.pInputElementDescs = input_element_desc;
    input_layout_desc.NumElements = _countof(input_element_desc);

    DXGI_SAMPLE_DESC dxgi_sample_desc = {};
    dxgi_sample_desc.Count = 1;
    dxgi_sample_desc.Quality = 0;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.pRootSignature = m_texture_root_sig;
    pso_desc.VS = vertex_bytecode;
    pso_desc.PS = pixel_bytecode;
    pso_desc.DS = { nullptr, 0 };
    pso_desc.HS = { nullptr, 0 };
    pso_desc.GS = { nullptr, 0 };
    pso_desc.StreamOutput = {};
    pso_desc.BlendState = blend_desc;
    pso_desc.SampleMask = 1;
    pso_desc.RasterizerState = rasterizer_desc;
    pso_desc.DepthStencilState = depth_stencil_desc;
    pso_desc.InputLayout = input_layout_desc;
    pso_desc.IBStripCutValue = {};
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = {DXGI_FORMAT_R8G8B8A8_UNORM};
    pso_desc.DSVFormat = {};
    pso_desc.SampleDesc = dxgi_sample_desc;
    pso_desc.NodeMask = 0;
    pso_desc.CachedPSO = { nullptr, 0 };
    pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    handle_hr(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_texture_pso)),
              "Failed to create PSO for render texture\n");
    handle_hr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_render_texture_cmd_allocator)),
              "Failed to create cmd allocator for render texture\n");
    handle_hr(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_render_texture_cmd_allocator, nullptr, IID_PPV_ARGS(&m_render_texture_cmd_list)),
              "Failed to create cmd list for render texture\n");

    handle_hr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_plugin_texture_cmd_allocator)),
            "Failed to create cmd allocator for plugin texture\n");

    handle_hr(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_plugin_texture_cmd_allocator, nullptr, IID_PPV_ARGS(&m_plugin_texture_cmd_list)),
            "Failed to create cmd list for render texture2\n");

    handle_hr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_texture_copy_cmd_allocator)),
              "Failed to create cmd allocator for texture copy\n");

    handle_hr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_vertex_copy_cmd_allocator)),
              "Failed to create cmd allocator for vertex copy\n");

    handle_hr(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_texture_copy_cmd_allocator, nullptr, IID_PPV_ARGS(&m_texture_copy_cmd_list)),
             "Failed to create texture copy cmd list\n");

    handle_hr(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_vertex_copy_cmd_allocator, nullptr, IID_PPV_ARGS(&m_vertex_copy_cmd_list)),
              "Failed to create vertex copy cmd list\n");

    handle_hr(device->CreateFence(m_plugin_texture_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_plugin_texture_fence)),
             "Failed to create fence for plugin texture");

    m_texture_copy_cmd_allocator->SetName(L"texture copy cmd allocator");
    m_vertex_copy_cmd_allocator->SetName(L"vertex copy cmd allocator");
    m_render_texture_cmd_allocator->SetName(L"render texture cmd allocator");

    m_vertex_copy_cmd_list->SetName(L"vertex copy cmd list");
    m_texture_copy_cmd_list->SetName(L"texture copy cmd list");
    m_render_texture_cmd_list->SetName(L"render texture cmd list");

    handle_hr(m_vertex_copy_cmd_list->Close(), "Failed to close cmd list for vertex copy\n");
    handle_hr(m_texture_copy_cmd_list->Close(), "Failed to close cmd list for texture copy\n");
    handle_hr(m_render_texture_cmd_list->Close(), "Failed to close cmd list for render texture\n");
    handle_hr(m_plugin_texture_cmd_list->Close(), "Failed to close cmd list for plugin texture\n");

    m_fence_event = CreateEvent(NULL, false, false, nullptr);
    m_plugin_texture_fence_event = CreateEvent(NULL, false, false, nullptr);

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = 1;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDesc.NodeMask = 0;

    handle_hr(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_triangle_rtv_desc_heap)),
              "CreateDescriptorHeap Failed\n");

    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvDesc.NodeMask = 0;

    handle_hr(device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_triangle_dsv_desc_heap)),
              "CreateDescriptorHeap Failed\n");

    m_are_resources_initialized = true;
}

void RenderAPI_D3D12::release_resources()
{
    SAFE_RELEASE(m_render_texture_cmd_list);
    SAFE_RELEASE(m_render_texture_cmd_allocator);
    SAFE_RELEASE(m_plugin_texture_cmd_list);
    SAFE_RELEASE(m_plugin_texture_cmd_allocator);
    SAFE_RELEASE(m_texture_pso);
    SAFE_RELEASE(m_texture_root_sig);
    SAFE_RELEASE(m_render_texture_root_blob);
    SAFE_RELEASE(m_texture_rtv_desc_heap);
    if (!m_is_render_texture_created_by_unity)
    {
        ID3D12Resource* render_texture = m_render_texture.load();
        if (render_texture)
        {
            render_texture->Release();
            m_render_texture = nullptr;
        }
    }
    SAFE_RELEASE(m_render_texture_vertex_buffer);
    SAFE_RELEASE(m_triangle_pso);
    SAFE_RELEASE(m_triangle_rootsig);
    SAFE_RELEASE(m_triangle_rtv_desc_heap);
    SAFE_RELEASE(m_triangle_dsv_desc_heap);
    SAFE_RELEASE(s_upload_texture);
    SAFE_RELEASE(s_upload_buffer);
    SAFE_RELEASE(m_vertex_copy_cmd_list);
    SAFE_RELEASE(m_texture_copy_cmd_list);
    SAFE_RELEASE(m_vertex_copy_cmd_allocator);
    SAFE_RELEASE(m_texture_copy_cmd_allocator);

    if (ID3D12Resource* plugin_texture = m_plugin_texture.load())
    {
        plugin_texture->Release();
        m_plugin_texture = nullptr;
    }

    CloseHandle(m_fence_event);
    CloseHandle(m_plugin_texture_fence_event);

    garbage_collect(true);
    m_mapped_triangle_vertex_buffers.clear();
}

void RenderAPI_D3D12::record_draw_cmd_list(ID3D12CommandAllocator* cmd_alloc, ID3D12GraphicsCommandList* cmd, ID3D12RootSignature* rootsig, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle, D3D12_VERTEX_BUFFER_VIEW* vbview, const float* world_matrix, D3D12_VIEWPORT* viewport, ID3D12PipelineState* pso, ID3D12Resource* target, D3D12_RESOURCE_STATES target_state)
{
    constexpr FLOAT clear_color[4] = { 0.5f, 0.5f, 0.5f, 1.f };

    handle_hr(cmd_alloc->Reset(), "Failed to reset cmd allocator\n");
    handle_hr(cmd->Reset(cmd_alloc, pso), "Failed to reset cmd list\n");

    cmd->SetGraphicsRootSignature(rootsig);
    cmd->SetGraphicsRoot32BitConstants(0, 64, world_matrix, 0);

    if (target_state != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
       transition_barrier(cmd, target, target_state, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    cmd->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
    cmd->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);

    D3D12_RECT scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(viewport->Width);
    scissor.bottom = static_cast<LONG>(viewport->Height);

    cmd->RSSetViewports(1, viewport);
    cmd->RSSetScissorRects(1, &scissor);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, vbview);

    const UINT vertices = vbview->SizeInBytes / vbview->StrideInBytes;

    cmd->DrawInstanced(vertices, 1, 0, 0);

    if (target_state != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        transition_barrier(cmd, target, D3D12_RESOURCE_STATE_RENDER_TARGET, target_state);
    }

    handle_hr(cmd->Close(), "Failed to close draw cmd list\n");
}

UINT64 RenderAPI_D3D12::submit_cmd_to_unity_worker(ID3D12GraphicsCommandList* cmd, UnityGraphicsD3D12ResourceState* resource_states, int state_count)
{
    return s_d3d12->ExecuteCommandList(cmd, state_count, resource_states);
}

ID3D12Resource* RenderAPI_D3D12::create_render_texture()
{
    ID3D12Device* device = s_d3d12->GetDevice();

    DXGI_SAMPLE_DESC dxgi_sample_desc = {};
    dxgi_sample_desc.Count = 1;
    dxgi_sample_desc.Quality = 0;

    D3D12_RESOURCE_DESC render_texture_desc = {};
    render_texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    render_texture_desc.Width = m_texture_width;
    render_texture_desc.Height = m_texture_height;
    render_texture_desc.DepthOrArraySize = 1;
    render_texture_desc.MipLevels = 1;
    render_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    render_texture_desc.SampleDesc = dxgi_sample_desc;
    render_texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    render_texture_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES render_texture_heap_props = {};
    render_texture_heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
    render_texture_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    render_texture_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    render_texture_heap_props.CreationNodeMask = 1;
    render_texture_heap_props.VisibleNodeMask = 1;

    ID3D12Resource* pRendertex;
    handle_hr(device->CreateCommittedResource(&render_texture_heap_props, D3D12_HEAP_FLAG_NONE, &render_texture_desc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&pRendertex)),
              "Failed to create a render texture\n");

    return pRendertex;
}

void RenderAPI_D3D12::create_render_texture_rtv(ID3D12DescriptorHeap* heap, ID3D12Resource* target, UINT offset)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{ heap->GetCPUDescriptorHandleForHeapStart() };
    rtv_handle.Offset(offset, m_texture_rtv_desc_size);

    D3D12_TEX2D_RTV rtv;
    rtv.MipSlice = 0;
    rtv.PlaneSlice = 0;

    D3D12_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = typeless_fmt_to_typed(target->GetDesc().Format);
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D = rtv;

    s_d3d12->GetDevice()->CreateRenderTargetView(target, &desc, rtv_handle);
}

void RenderAPI_D3D12::transition_barrier(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_TRANSITION_BARRIER transition = {};
    transition.pResource = resource;
    transition.Subresource = 0;
    transition.StateBefore = before;
    transition.StateAfter = after;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition = transition;

    cmd->ResourceBarrier(1, &barrier);
}

void RenderAPI_D3D12::wait_for_unity_frame_fence(UINT64 fence_value)
{
    ID3D12Fence* unity_fence = s_d3d12->GetFrameFence();
    UINT64 current_fence_value = unity_fence->GetCompletedValue();

    if (current_fence_value < fence_value)
    {
        handle_hr(unity_fence->SetEventOnCompletion(fence_value, m_fence_event), "Failed to set fence event on completion\n");
        WaitForSingleObject(m_fence_event, INFINITE);
    }
}

void RenderAPI_D3D12::wait_on_fence(UINT64 fence_value, ID3D12Fence* fence, HANDLE fence_event)
{
    UINT64 current_fence_value = fence->GetCompletedValue();

    if (current_fence_value < fence_value)
    {
        handle_hr(fence->SetEventOnCompletion(fence_value, fence_event));
        WaitForSingleObject(fence_event, INFINITE);
    }
}

DXGI_FORMAT RenderAPI_D3D12::typeless_fmt_to_typed(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
            return DXGI_FORMAT_R32G32B32_UINT;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            return DXGI_FORMAT_R16G16B16A16_UNORM;

        case DXGI_FORMAT_R32G32_TYPELESS:
            return DXGI_FORMAT_R32G32_UINT;

        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case DXGI_FORMAT_R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_UNORM;

        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_UINT;

        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

        case DXGI_FORMAT_R8G8_TYPELESS:
            return DXGI_FORMAT_R8G8_UNORM;

        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_UNORM;

        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_UNORM;

        case DXGI_FORMAT_BC1_TYPELESS:
            return DXGI_FORMAT_BC1_UNORM;

        case DXGI_FORMAT_BC2_TYPELESS:
            return DXGI_FORMAT_BC2_UNORM;

        case DXGI_FORMAT_BC3_TYPELESS:
            return DXGI_FORMAT_BC3_UNORM;
        
        case DXGI_FORMAT_BC4_TYPELESS:
            return DXGI_FORMAT_BC4_UNORM;

        case DXGI_FORMAT_BC5_TYPELESS:
            return DXGI_FORMAT_BC5_UNORM;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

        case DXGI_FORMAT_BC6H_TYPELESS:
            return DXGI_FORMAT_BC6H_UF16;

        case DXGI_FORMAT_BC7_TYPELESS:
            return DXGI_FORMAT_BC7_UNORM;

        default:
            return format;
     }
}

void* RenderAPI_D3D12::BeginModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int* outRowPitch)
{
    wait_for_unity_frame_fence(m_texture_copy_fence);

    // Fill data
    // Clamp to minimum rowPitch of RGBA32
    *outRowPitch = static_cast<int>(max(align_pow2(textureWidth * 4), 256));
    const UINT64 kDataSize = get_aligned_size(textureWidth, textureHeight, 4, *outRowPitch);
    if (!get_upload_resource(&s_upload_texture, kDataSize, D3D12_UPLOAD_HEAP_TEXTURE_BUFFER_NAME))
        return NULL;

    void* mapped = NULL;
    s_upload_texture->Map(0, NULL, &mapped);
    return mapped;
}

void RenderAPI_D3D12::EndModifyTexture(void* textureHandle, int textureWidth, int textureHeight, int rowPitch, void* dataPtr)
{
    ID3D12Device* device = s_d3d12->GetDevice();

    const UINT64 kDataSize = get_aligned_size(textureWidth, textureHeight, 4, rowPitch);
    if (!get_upload_resource(&s_upload_texture, kDataSize, D3D12_UPLOAD_HEAP_TEXTURE_BUFFER_NAME))
        return;

    s_upload_texture->Unmap(0, 0);

    ID3D12Resource* resource = (ID3D12Resource*)textureHandle;
    D3D12_RESOURCE_DESC desc = resource->GetDesc();
    assert(desc.Width == textureWidth);
    assert(desc.Height == textureHeight);

    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource = s_upload_texture;
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    device->GetCopyableFootprints(&desc, 0, 1, 0, &srcLoc.PlacedFootprint, nullptr, nullptr, nullptr);

    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource = resource;
    dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = 0;

    m_texture_copy_cmd_allocator->Reset();
    m_texture_copy_cmd_list->Reset(m_texture_copy_cmd_allocator, nullptr);
    transition_barrier(m_texture_copy_cmd_list, resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    m_texture_copy_cmd_list->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    transition_barrier(m_texture_copy_cmd_list, resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
    m_texture_copy_cmd_list->Close();

    UnityGraphicsD3D12ResourceState resource_states = {};
    resource_states.resource = resource;
    resource_states.expected = D3D12_RESOURCE_STATE_COMMON;
    resource_states.current = D3D12_RESOURCE_STATE_COMMON;

    m_texture_copy_fence = submit_cmd_to_unity_worker(m_texture_copy_cmd_list, &resource_states, 1);
}

void* RenderAPI_D3D12::BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize)
{
    wait_for_unity_frame_fence(m_vertex_copy_fence);

    if (bufferHandle == NULL)
        return NULL;

    ID3D12Resource* unity_vertex_buffer = reinterpret_cast<ID3D12Resource*>(bufferHandle);
    D3D12_RESOURCE_DESC desc = unity_vertex_buffer->GetDesc();
    *outBufferSize = static_cast<size_t>(desc.Width);

    if (!get_upload_resource(&s_upload_buffer, desc.Width, D3D12_UPLOAD_HEAP_VERTEX_BUFFER_NAME))
        return NULL;

    MappedVertexBuffers::iterator it = m_mapped_triangle_vertex_buffers.find(bufferHandle);
    if (it == m_mapped_triangle_vertex_buffers.end())
        m_mapped_triangle_vertex_buffers.insert(std::make_pair(bufferHandle, reinterpret_cast<void*>(new int())));

    HRESULT hr = s_upload_buffer->Map(0, 0, &m_mapped_triangle_vertex_buffers[bufferHandle]);
    return m_mapped_triangle_vertex_buffers[bufferHandle];
}

void RenderAPI_D3D12::EndModifyVertexBuffer(void* bufferHandle)
{
    m_vertex_copy_cmd_allocator->Reset();
    m_vertex_copy_cmd_list->Reset(m_vertex_copy_cmd_allocator, nullptr);

    if (bufferHandle == NULL)
        return;

    UnityGraphicsD3D12RecordingState recordingState;
    if (!s_d3d12->CommandRecordingState(&recordingState))
        return;

    ID3D12Resource* unity_vertex_buffer = reinterpret_cast<ID3D12Resource*>(bufferHandle);
    D3D12_RESOURCE_DESC desc = unity_vertex_buffer->GetDesc();
    if (!get_upload_resource(&s_upload_buffer, desc.Width, D3D12_UPLOAD_HEAP_VERTEX_BUFFER_NAME))
        return;

    assert(desc.Height == 1);

    s_upload_buffer->Unmap(0, 0);

    D3D12_HEAP_PROPERTIES heap_props;
    handle_hr(unity_vertex_buffer->GetHeapProperties(&heap_props, nullptr), "Failed to get heap properties for unitys vertex buffer");

    // On DX12 Mesh.MarkDynamic() doesn't guarantee the underlying resource to be CPU mappable. In the case when it's not we need to 
    // place correct transition barriers before and after the copy command.

    if (heap_props.Type == D3D12_HEAP_TYPE_DEFAULT)
        transition_barrier(m_vertex_copy_cmd_list, unity_vertex_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

    m_vertex_copy_cmd_list->CopyBufferRegion(unity_vertex_buffer, 0, s_upload_buffer, 0, desc.Width);

    if (heap_props.Type == D3D12_HEAP_TYPE_DEFAULT)
        transition_barrier(m_vertex_copy_cmd_list, unity_vertex_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

    handle_hr(m_vertex_copy_cmd_list->Close(), "Failed to close vertex copy cmd list");

    UnityGraphicsD3D12ResourceState resource_states = {};
    if (heap_props.Type == D3D12_HEAP_TYPE_DEFAULT)
    {
        resource_states.resource = unity_vertex_buffer;
        resource_states.expected = D3D12_RESOURCE_STATE_COMMON;
        resource_states.current = D3D12_RESOURCE_STATE_COMMON;
    }

    m_vertex_copy_fence = submit_cmd_to_unity_worker(m_vertex_copy_cmd_list, &resource_states, 1);
}

void RenderAPI_D3D12::drawToPluginTexture()
{
    if (!m_plugin_texture)
        return;

    wait_on_fence(m_plugin_texture_fence_value, m_plugin_texture_fence, m_plugin_texture_fence_event);

    // Draw the triangle upside down
    constexpr float world_matrix[16] = { 1, 0, 0, 0,
                                         0,-1, 0, 0,
                                         0, 0, 1, 0,
                                         0, 0, 0, 1 };

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(m_texture_width);
    viewport.Height = static_cast<FLOAT>(m_texture_height);
    viewport.MinDepth = D3D12_MIN_DEPTH;
    viewport.MaxDepth = D3D12_MAX_DEPTH;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle { m_texture_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart() };
    rtv_handle.Offset(1, m_texture_rtv_desc_size);

    // We can reuse some of the resources for both plugin_texture and render_texture
    record_draw_cmd_list(m_plugin_texture_cmd_allocator,
                         m_plugin_texture_cmd_list,
                         m_texture_root_sig,
                         rtv_handle,
                         &m_texture_vertex_buffer_view,
                         world_matrix, &viewport,
                         m_texture_pso,
                         m_plugin_texture,
                         D3D12_RESOURCE_STATE_RENDER_TARGET);

    ID3D12CommandQueue* unity_command_queue = s_d3d12->GetCommandQueue();
    unity_command_queue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_plugin_texture_cmd_list);
    unity_command_queue->Signal(m_plugin_texture_fence, ++m_plugin_texture_fence_value);
}

void RenderAPI_D3D12::drawToRenderTexture()
{
    wait_for_unity_frame_fence(m_render_texture_draw_fence);

    if (!m_render_texture)
        return;

    // Draw the triangle upright
    constexpr float world_matrix[16] = { 1, 0, 0, 0,
                                         0, 1, 0, 0,
                                         0, 0, 1, 0,
                                         0, 0, 0, 1 };

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(m_texture_width);
    viewport.Height = static_cast<FLOAT>(m_texture_height);
    viewport.MinDepth = D3D12_MIN_DEPTH;
    viewport.MaxDepth = D3D12_MAX_DEPTH;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle { m_texture_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart() };
    rtv_handle.Offset(0, m_texture_rtv_desc_size);

    D3D12_RESOURCE_STATES target_state;
    if (m_is_render_texture_created_by_unity)
    {
        target_state = D3D12_RESOURCE_STATE_COMMON;
    }
    else
    {
        target_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    record_draw_cmd_list(m_render_texture_cmd_allocator, 
                         m_render_texture_cmd_list, 
                         m_texture_root_sig,  
                         rtv_handle, 
                         &m_texture_vertex_buffer_view, 
                         world_matrix, &viewport, 
                         m_texture_pso,
                         m_render_texture,
                         target_state);

    UnityGraphicsD3D12ResourceState resource_states;
    resource_states.resource = m_render_texture;
    resource_states.expected = D3D12_RESOURCE_STATE_COMMON;
    resource_states.current = D3D12_RESOURCE_STATE_COMMON;

    m_render_texture_draw_fence = submit_cmd_to_unity_worker(m_render_texture_cmd_list, &resource_states, 1);
}

void* RenderAPI_D3D12::getRenderTexture()
{
    return reinterpret_cast<void*>(m_render_texture.load());
}

void RenderAPI_D3D12::setRenderTextureResource(UnityRenderBuffer rb)
{
    // Release existing resource if there is already one that wasn't created by Unity C# script
    if (!m_is_render_texture_created_by_unity && m_render_texture)
    {
        m_render_texture.load()->Release();
        m_render_texture = nullptr;
    }

    // rb == nullptr is used to signal that the previously used UnityRenderBuffer
    // is not available anymore (i.e when it gets destroyed)
    if (!rb)
    {
        m_render_texture = nullptr;
        m_is_render_texture_created_by_unity = false;
        return;
    }

    // TextureFromRenderBuffer might not be immediately available when this function is called.
    ID3D12Resource* rb_resource = nullptr;
    while (!rb_resource)
    {
        rb_resource = s_d3d12->TextureFromRenderBuffer(rb);
    }

    D3D12_RESOURCE_DESC desc = rb_resource->GetDesc();
    m_texture_width = desc.Width;
    m_texture_height = desc.Height;
    assert(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
    assert(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    m_render_texture = rb_resource;

    // we need to update the rtv if we won't be calling initialize_render_texture_resources() after
    // (when it's already initialized)
    if (m_are_resources_initialized)
        create_render_texture_rtv(m_texture_rtv_desc_heap, m_render_texture, 0);

    m_is_render_texture_created_by_unity = true;
}

bool RenderAPI_D3D12::isSwapChainAvailable()
{
    return s_d3d12->GetSwapChain();
}

unsigned int RenderAPI_D3D12::getPresentFlags()
{
    return s_d3d12->GetPresentFlags();
};

unsigned int RenderAPI_D3D12::getSyncInterval()
{
    return s_d3d12->GetSyncInterval();
}

unsigned int RenderAPI_D3D12::getBackbufferWidth()
{
    IDXGISwapChain* swap_chain = s_d3d12->GetSwapChain();
    if (!swap_chain)
        return 0;

    DXGI_SWAP_CHAIN_DESC desc;
    handle_hr(swap_chain->GetDesc(&desc), "Failed to get DXGI swap chain desc\n");

    return desc.BufferDesc.Width;
}

unsigned int RenderAPI_D3D12::getBackbufferHeight()
{
    IDXGISwapChain* swap_chain = s_d3d12->GetSwapChain();
    if (!swap_chain)
        return 0;

    DXGI_SWAP_CHAIN_DESC desc;
    handle_hr(swap_chain->GetDesc(&desc), "Failed to get DXGI swap chain desc\n");

    return desc.BufferDesc.Height;
}

#undef ReturnOnFail

#endif // #if SUPPORT_D3D12
