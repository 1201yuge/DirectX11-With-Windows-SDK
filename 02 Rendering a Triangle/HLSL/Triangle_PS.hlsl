#include "Triangle.fx"

// ������ɫ��
float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.color;
}

