#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>
#include "d3dApp.h"
#include "LightHelper.h"
#include "Geometry.h"

class GameApp : public D3DApp
{
public:
	struct VertexPosNormalColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 color;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
	};

	

	struct VSConstantBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMMATRIX worldInvTranspose;
		
	};

	struct PSConstantBuffer
	{
		DirectionalLight dirLight;
		PointLight pointLight;
		SpotLight spotLight;
		Material material;
		DirectX::XMFLOAT4 eyePos;
	};



public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	// ��.fx/.hlsl�ļ��б�����ɫ��
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

private:
	bool InitEffect();
	bool InitResource();
	bool ResetMesh(const Geometry::MeshData& meshData);


private:
	ComPtr<ID3D11InputLayout> mVertexLayout;	// �������벼��
	ComPtr<ID3D11Buffer> mVertexBuffer;			// ���㻺����
	ComPtr<ID3D11Buffer> mIndexBuffer;			// ����������
	ComPtr<ID3D11Buffer> mConstantBuffers[2];	// ����������
	int mIndexCount;							// ������������������С

	ComPtr<ID3D11VertexShader> mVertexShader;	// ������ɫ��
	ComPtr<ID3D11PixelShader> mPixelShader;		// ������ɫ��
	VSConstantBuffer mVSConstantBuffer;			// �����޸�����VS��GPU�����������ı���
	PSConstantBuffer mPSConstantBuffer;			// �����޸�����PS��GPU�����������ı���

	DirectionalLight mDirLight;					// Ĭ�ϻ�����
	PointLight mPointLight;						// Ĭ�ϵ��
	SpotLight mSpotLight;						// Ĭ�ϻ�۹�
	
};


#endif