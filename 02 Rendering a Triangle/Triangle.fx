
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

// ������ɫ��
VertexOut VS(VertexIn pIn)
{
	VertexOut pOut;
	pOut.posH = float4(pIn.pos, 1.0f);
	pOut.color = pIn.color;	// ����alphaͨ����ֵĬ��Ϊ1.0
    return pOut;
}


// ������ɫ��
float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.color;   
}
