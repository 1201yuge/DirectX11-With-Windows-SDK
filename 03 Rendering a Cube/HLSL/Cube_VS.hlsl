#include "Cube.fx"

VertexOut VS(VertexIn pIn)
{
    VertexOut pOut;
    pOut.posH = mul(float4(pIn.posL, 1.0f), gWorld);  // mul ���Ǿ���˷�, �����*Ҫ���������Ϊ
    pOut.posH = mul(pOut.posH, gView);               // ��������ȵ��������󣬽��Ϊ
    pOut.posH = mul(pOut.posH, gProj);               // Cij = Aij * Bij
    pOut.color = pIn.color;                         // ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}