#include "Light.fx"

// ������ɫ��
VertexOut VS(VertexIn pIn)
{
    VertexOut pOut;
    row_major matrix worldViewProj = mul(mul(gWorld, gView), gProj);
    pOut.PosH = mul(float4(pIn.Pos, 1.0f), worldViewProj);
    pOut.PosW = mul(float4(pIn.Pos, 1.0f), gWorld).xyz;
    pOut.NormalW = mul(pIn.Normal, (float3x3) gWorldInvTranspose);
    pOut.Color = pIn.Color; // ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}