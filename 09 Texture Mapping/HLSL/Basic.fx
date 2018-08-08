#include "LightHelper.hlsli"

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


cbuffer VSConstantBuffer : register(b0)
{
    row_major matrix gWorld; 
    row_major matrix gView;  
    row_major matrix gProj;  
    row_major matrix gWorldInvTranspose;
	row_major matrix gTexTransform;
}

cbuffer PSConstantBuffer : register(b1)
{
    DirectionalLight gDirLight[10];
    PointLight gPointLight[10];
    SpotLight gSpotLight[10];
    Material gMaterial;
	int gNumDirLight;
	int gNumPointLight;
	int gNumSpotLight;
    float3 gEyePosW;
}


struct VertexIn
{
	float3 PosL : POSITION;
    float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // �������е�λ��
    float3 NormalW : NORMAL;    // �������������еķ���
	float2 Tex : TEXCOORD;
};










