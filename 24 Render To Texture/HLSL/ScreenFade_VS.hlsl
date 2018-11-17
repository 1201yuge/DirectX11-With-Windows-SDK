#include "ScreenFade.hlsli"

// ������ɫ��
VertexPosHTex VS(VertexPosNormalTex vIn)
{
    VertexPosHTex vOut;
    
    vOut.PosH = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);
    vOut.Tex = vIn.Tex;
    return vOut;
}