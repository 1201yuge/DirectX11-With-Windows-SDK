#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <d3dcompiler.h>
#include <directxmath.h>
#include "d3dApp.h"
class GameApp : public D3DApp
{
public:
	struct VertexPosColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
	};

	struct ConstantBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
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



private:
	ComPtr<ID3D11InputLayout> mVertexLayout;	// �������벼��
	ComPtr<ID3D11Buffer> mVertexBuffer;			// ���㻺����
	ComPtr<ID3D11Buffer> mIndexBuffer;			// ����������
	ComPtr<ID3D11Buffer> mConstantBuffer;		// ����������

	ComPtr<ID3D11VertexShader> mVertexShader;	// ������ɫ��
	ComPtr<ID3D11PixelShader> mPixelShader;		// ������ɫ��
	ConstantBuffer mCBuffer;	// �����޸�GPU�����������ı���
};


#endif