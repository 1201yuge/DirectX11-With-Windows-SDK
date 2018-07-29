#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <Windows.h>
#include <vector>
#include "Vertex.h"

class Geometry
{
public:
	struct MeshData
	{
		std::vector<VertexPosNormalTex> vertexVec;	// ��������
		std::vector<WORD> indexVec;					// ��������
	};

	// ��������ģ�����ݣ�levels��slicesԽ�󣬾���Խ�ߡ�
	static MeshData CreateSphere(float radius = 1.0f, int levels = 20, int slices = 20);

	// ����������ģ������
	static MeshData CreateBox(float width = 2.0f, float height = 2.0f, float depth = 2.0f);

	// ����Բ����ģ�����ݣ�slicesԽ�󣬾���Խ�ߡ�
	static MeshData CreateCylinder(float radius = 1.0f, float height = 2.0f, int slices = 20);

	// ����һ������NDC��Ļ����
	static MeshData Create2DShow(const DirectX::XMFLOAT2& center = { 0.0f, 0.0f }, const DirectX::XMFLOAT2& scale = { 1.0f, 1.0f });
	static MeshData Create2DShow(float centerX = 0.0f, float centerY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f);

	// ����һ��ƽ��
	static MeshData CreatePlane(const DirectX::XMFLOAT3& center = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT2& planeSize = { 10.0f, 10.0f }, const DirectX::XMFLOAT2& maxTexCoord = { 1.0f, 1.0f });
	static MeshData CreatePlane(float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float width = 10.0f, float depth = 10.0f, float texU = 1.0f, float texV = 1.0f);

};



#endif