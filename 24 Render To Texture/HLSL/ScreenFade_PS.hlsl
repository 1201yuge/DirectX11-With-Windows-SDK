#include "ScreenFade.hlsli"

// ������ɫ��
float4 PS(VertexPosHTex pIn) : SV_Target
{
    return tex.Sample(sam, pIn.Tex) * gFadeAmount;
}
