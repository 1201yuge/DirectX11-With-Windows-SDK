#include "BasicObject.fx"

// ������ɫ��(3D)
Vertex3DOut VS_3D(Vertex3DIn pIn)
{
    Vertex3DOut pOut;
    
    float4 posW = mul(float4(pIn.PosL, 1.0f), gWorld);
    // ����ǰ�ڻ��Ʒ������壬�Ƚ��з������
    [flatten]
    if (gIsReflection)
    {
        posW = mul(posW, gReflection);
    }
    // ����ǰ�ڻ�����Ӱ���Ƚ���ͶӰ����
    [flatten]
    if (gIsShadow)
    {
        posW = (gIsReflection ? mul(posW, gRefShadow) : mul(posW, gShadow));
    }

    pOut.PosH = mul(mul(posW, gView), gProj);
    pOut.PosW = mul(float4(pIn.PosL, 1.0f), gWorld).xyz;
    pOut.NormalW = mul(pIn.NormalL, (float3x3) gWorldInvTranspose);
    pOut.Tex = pIn.Tex;
    return pOut;
}