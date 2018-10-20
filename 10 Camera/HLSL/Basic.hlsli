#include "LightHelper.hlsli"

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
	matrix gWorld;
	matrix gWorldInvTranspose;
}

cbuffer CBChangesEveryFrame : register(b1)
{
	matrix gView;
	float3 gEyePosW;
}

cbuffer CBChangesOnResize : register(b2)
{
	matrix gProj;
}

cbuffer CBChangesRarely : register(b3)
{
	DirectionalLight gDirLight[10];
	PointLight gPointLight[10];
	SpotLight gSpotLight[10];
	Material gMaterial;
	int gNumDirLight;
	int gNumPointLight;
	int gNumSpotLight;
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











