#include "Cube.fx"



// ������ɫ��
float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.color;
}
