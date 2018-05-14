
cbuffer ConstantBuffer : register(b0)
{
    row_major matrix World; // matrix ������float4x4���������row_major������£�����Ĭ��Ϊ��������
    row_major matrix View;  // ��D3D���ݸ�HLSL�Ĺ����лᷢ��ת�á����˺󣬾ͱ����������
    row_major matrix Proj;  
}


struct VertexIn
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VertexOut VS(VertexIn pIn)
{
	VertexOut pOut;
    pOut.posH = mul(float4(pIn.pos, 1.0f), World);      // mul ���Ǿ���˷�, �����*Ҫ��
    pOut.posH = mul(pOut.posH, View);                   // ��������ȵ��������󣬽��Ϊ
    pOut.posH = mul(pOut.posH, Proj);                   // Cij = Aij * Bij
	pOut.color = pIn.color;	                            // ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.color;   
}
