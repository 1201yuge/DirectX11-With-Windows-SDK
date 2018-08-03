#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "BasicFX.h"
#include "Geometry.h"

// һ��������С����Ϸ������
class GameObject
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// ��ȡλ��
	DirectX::XMFLOAT3 GetPosition() const;

	// ���û�����
	void SetBuffer(ComPtr<ID3D11Device> device, const Geometry::MeshData& meshData);
	// ��������
	void SetTexture(ComPtr<ID3D11ShaderResourceView> texture);
	// ���ò���
	void SetMaterial(const Material& material);
	// ���þ���
	void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
	void SetWorldMatrix(DirectX::FXMMATRIX world);
	void SetTexTransformMatrix(const DirectX::XMFLOAT4X4& texTransform);
	void SetTexTransformMatrix(DirectX::FXMMATRIX texTransform);
	// ����
	void Draw(ComPtr<ID3D11DeviceContext> deviceContext);
private:
	DirectX::XMFLOAT4X4 mWorldMatrix;				// �������
	DirectX::XMFLOAT4X4 mTexTransform;				// ����任����
	Material mMaterial;								// �������
	ComPtr<ID3D11ShaderResourceView> mTexture;		// ����
	ComPtr<ID3D11Buffer> mVertexBuffer;				// ���㻺����
	ComPtr<ID3D11Buffer> mIndexBuffer;				// ����������
	int mIndexCount;								// ������Ŀ	
};


#endif
