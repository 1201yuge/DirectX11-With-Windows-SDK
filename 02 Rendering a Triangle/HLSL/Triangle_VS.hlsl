#include "Triangle.fx"

// ������ɫ��
VertexOut VS(VertexIn pIn)
{
    VertexOut pOut;
    pOut.posH = float4(pIn.pos, 1.0f);
    pOut.color = pIn.color; // ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}