#include "BasicObject.fx"

// ������ɫ��
VertexPosHWNormalTex VS(VertexPosNormalTex pIn)
{
    VertexPosHWNormalTex pOut;
    
    row_major matrix viewProj = mul(gView, gProj);

    pOut.PosW = mul(float4(pIn.PosL, 1.0f), gWorld).xyz;
    pOut.PosH = mul(float4(pOut.PosW, 1.0f), viewProj);
    pOut.NormalW = mul(pIn.NormalL, (float3x3) gWorldInvTranspose);
    pOut.Tex = mul(float4(pIn.Tex, 0.0f, 1.0f), gTexTransform).xy;
    return pOut;
}