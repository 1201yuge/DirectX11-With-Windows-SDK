#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <DirectXCollision.h>
#include "BasicFX.h"
#include "ObjReader.h"
#include "Geometry.h"

struct ModelPart
{
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	Material material;
	ComPtr<ID3D11ShaderResourceView> texA;
	ComPtr<ID3D11ShaderResourceView> texD;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT vertexCount;
	UINT indexCount;
	DXGI_FORMAT indexFormat;
};


class GameObject
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	GameObject();

	// ��ȡλ��
	DirectX::XMFLOAT3 GetPosition() const;
	// ��ȡ��ģ��
	const ModelPart& GetModelPart(size_t pos) const;
	size_t GetModelPartSize() const;
	// ��ȡ��Χ��
	void GetBoundingBox(DirectX::BoundingBox& box) const;
	void GetBoundingBox(DirectX::BoundingBox& box, DirectX::FXMMATRIX worldMatrix) const;

	//
	// ����ģ��
	//
	
	void SetModel(ComPtr<ID3D11Device> device, const ObjReader& model);
	

	//
	// ��������
	//

	void SetMesh(ComPtr<ID3D11Device> device, const Geometry::MeshData& meshData);
	void SetMesh(ComPtr<ID3D11Device> device, const std::vector<VertexPosNormalTex>& vertices, const std::vector<WORD> indices);
	void SetMesh(ComPtr<ID3D11Device> device, const std::vector<VertexPosNormalTex>& vertices, const std::vector<DWORD> indices);
	
	//
	// ��������
	//

	void SetTexture(ComPtr<ID3D11ShaderResourceView> texture);
	void SetTexture(ComPtr<ID3D11ShaderResourceView> texA, ComPtr<ID3D11ShaderResourceView> texD);
	void SetTexture(size_t partIndex, ComPtr<ID3D11ShaderResourceView> texture);
	void SetTexture(size_t partIndex, ComPtr<ID3D11ShaderResourceView> texA, ComPtr<ID3D11ShaderResourceView> texD);
	
	//
	// ���ò���
	//

	void SetMaterial(const Material& material);
	void SetMaterial(size_t partIndex, const Material& material);
	
	//
	// ���þ���
	//

	void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
	void SetWorldMatrix(DirectX::FXMMATRIX world);
	void SetTexTransformMatrix(const DirectX::XMFLOAT4X4& texTransform);
	void SetTexTransformMatrix(DirectX::FXMMATRIX texTransform);


	// ���ƶ���
	void Draw(ComPtr<ID3D11DeviceContext> deviceContext);

private:
	void SetMesh(ComPtr<ID3D11Device> device, const VertexPosNormalTex* vertices, UINT vertexCount,
		const void * indices, UINT indexCount, DXGI_FORMAT indexFormat);

private:
	DirectX::XMFLOAT4X4 mWorldMatrix;							// �������
	DirectX::XMFLOAT4X4 mTexTransform;							// ����任����
	std::vector<ModelPart> mParts;								// ģ�͵ĸ�������
	DirectX::BoundingBox mBoundingBox;							// ģ�͵�AABB��
};


#endif


