#include "LightHelper.hlsli"

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
	row_major matrix gWorld;
	row_major matrix gWorldInvTranspose;
	row_major matrix gTexTransform;
	Material gMaterial;
}

cbuffer CBDrawingState : register(b1)
{
    int gIsReflection;
}

cbuffer CBChangesEveryFrame : register(b2)
{
	row_major matrix gView;
	float3 gEyePosW;
}

cbuffer CBChangesOnResize : register(b3)
{
	row_major matrix gProj;
}

cbuffer CBNeverChange : register(b4)
{
    row_major matrix gReflection;
	DirectionalLight gDirLight[10];
	PointLight gPointLight[10];
	SpotLight gSpotLight[10];
	int gNumDirLight;
	int gNumPointLight;
	int gNumSpotLight;
	float gPad;
}



struct VertexIn
{
	float3 Pos : POSITION;
    float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // �������е�λ��
    float3 NormalW : NORMAL;    // �������������еķ���
	float2 Tex : TEXCOORD;
};




// ������ɫ��(3D)
VertexOut VS_3D(VertexIn pIn)
{
	VertexOut pOut;
    
    float4 posH = mul(float4(pIn.Pos, 1.0f), gWorld);
    // ����ǰ�ڻ��Ʒ������壬�Ƚ��з������
    [flatten]
    if (gIsReflection)
    {
        posH = mul(posH, gReflection);
    }
    pOut.PosH = mul(mul(posH, gView), gProj);
    pOut.PosW = mul(float4(pIn.Pos, 1.0f), gWorld).xyz;
    pOut.NormalW = mul(pIn.Normal, (float3x3)gWorldInvTranspose);
	pOut.Tex = mul(float4(pIn.Tex, 0.0f, 1.0f), gTexTransform).xy;
    return pOut;
}


// ������ɫ��(3D)
float4 PS_3D(VertexOut pIn) : SV_Target
{
	// ��ǰ���вü����Բ�����Ҫ������ؿ��Ա����������
	float4 texColor = tex.Sample(samLinear, pIn.Tex);
	clip(texColor.a - 0.1f);

    // ��׼��������
    pIn.NormalW = normalize(pIn.NormalW);

    // ����ָ���۾�������
    float3 toEyeW = normalize(gEyePosW - pIn.PosW);

    // ��ʼ��Ϊ0 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
	int i;


	// ǿ��չ��ѭ���Լ���ָ����
	[unroll]
	for (i = 0; i < gNumDirLight; ++i)
	{
		ComputeDirectionalLight(gMaterial, gDirLight[i], pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
    
	[unroll]
	for (i = 0; i < gNumPointLight; ++i)
	{
        PointLight pointLight = gPointLight[i];
        // ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
        [flatten]
        if (gIsReflection)
        {
            pointLight.Position = (float3) mul(float4(pointLight.Position, 1.0f), gReflection);
        }

        ComputePointLight(gMaterial, pointLight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
    
	[unroll]
	for (i = 0; i < gNumSpotLight; ++i)
	{
        SpotLight spotLight = gSpotLight[i];
        // ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
        [flatten]
        if (gIsReflection)
        {
            spotLight.Position = (float3) mul(float4(spotLight.Position, 1.0f), gReflection);
        }

        ComputeSpotLight(gMaterial, spotLight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
    

	
	float4 litColor = texColor * (ambient + diffuse) + spec;
	litColor.a = texColor.a * gMaterial.Diffuse.a;
    return litColor;
}

// ������ɫ��(2D)
VertexOut VS_2D(VertexIn pIn)
{
	VertexOut pOut;
	pOut.PosH = float4(pIn.Pos, 1.0f);
	pOut.PosW = float3(0.0f, 0.0f, 0.0f);
	pOut.NormalW = pIn.Normal;
	pOut.Tex = mul(float4(pIn.Tex, 0.0f, 1.0f), gTexTransform).xy;
	return pOut;
}

// ������ɫ��(2D)
float4 PS_2D(VertexOut pIn) : SV_Target
{
	float4 color = tex.Sample(samLinear, pIn.Tex);
	clip(color.a - 0.1f);
	return color;
}