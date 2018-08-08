#include "Basic.fx"

// ������ɫ��(3D)
VertexOut VS_3D(VertexIn pIn)
{
    VertexOut pOut;
    
    float4 posW = mul(float4(pIn.PosL, 1.0f), gWorld);
    // ����ǰ�ڻ��Ʒ������壬�Ƚ��з������
    [flatten]
    if (gIsReflection)
    {
        posW = mul(posW, gReflection);
    }
    pOut.PosH = mul(mul(posW, gView), gProj);
    pOut.PosW = mul(float4(pIn.PosL, 1.0f), gWorld).xyz;
    pOut.NormalW = mul(pIn.NormalL, (float3x3) gWorldInvTranspose);
    pOut.Tex = mul(float4(pIn.Tex, 0.0f, 1.0f), gTexTransform).xy;
    return pOut;
}
