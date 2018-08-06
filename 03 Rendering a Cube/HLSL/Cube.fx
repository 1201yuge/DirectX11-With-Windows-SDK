
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
