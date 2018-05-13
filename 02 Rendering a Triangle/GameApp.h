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
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mVertexLayout;	// �������벼��
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;			// ���㻺����
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;	// ������ɫ��
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;		// ������ɫ��
};


#endif