#include "Cube.fx"

VertexOut VS(VertexIn pIn)
{
    VertexOut pOut;
    pOut.posH = mul(float4(pIn.posL, 1.0f), World);  // mul ���Ǿ���˷�, �����*Ҫ���������Ϊ
    pOut.posH = mul(pOut.posH, View);               // ��������ȵ��������󣬽��Ϊ
    pOut.posH = mul(pOut.posH, Proj);               // Cij = Aij * Bij
    pOut.color = pIn.color;                         // ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}