#include "Basic.hlsli"

// ������ɫ��(2D)
float4 PS_2D(VertexPosHTex pIn) : SV_Target
{
    return tex.Sample(samLinear, pIn.Tex);
}