#include "GameApp.h"
#include <filesystem>
using namespace DirectX;
using namespace std::experimental;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{

}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	static float blue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ����������
	md3dImmediateContext->Draw(3, 0);
	HR(mSwapChain->Present(0, 0));
}

HRESULT GameApp::CompileShaderFromFile(const WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob ** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// ���� D3DCOMPILE_DEBUG ��־���ڻ�ȡ��ɫ��������Ϣ���ñ�־���������������飬
	// ����Ȼ������ɫ�������Ż�����
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// ��Debug�����½����Ż��Ա������һЩ����������
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		return hr;
	}


	return S_OK;
}



bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// �Ѿ�����õ���ɫ���ļ���
	filesystem::path psoPath = "Triangle_PS.cso", vsoPath = "Triangle_VS.cso";
	std::wstring wstr;
	// Ѱ���Ƿ����Ѿ�����õĶ�����ɫ���������������ڱ���
	if (filesystem::exists(vsoPath))
	{
		wstr = vsoPath.generic_wstring();
		HR(D3DReadFileToBlob(wstr.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"Triangle.fx", "VS", "vs_4_0", blob.GetAddressOf()));
	}
	// ����������ɫ��
	HR(md3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));
	// �������󶨶��㲼��
	HR(md3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout.GetAddressOf()));
	md3dImmediateContext->IASetInputLayout(mVertexLayout.Get());

	blob.Reset();


	// Ѱ���Ƿ����Ѿ�����õ�������ɫ���������������ڱ���
	if (filesystem::exists(psoPath))
	{
		wstr = psoPath.generic_wstring();
		HR(D3DReadFileToBlob(wstr.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"Triangle.fx", "PS", "ps_4_0", blob.GetAddressOf()));
	}
	// ����������ɫ��
	HR(md3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));

	blob.Reset();

	// ����ɫ���󶨵���Ⱦ����
	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	return true;
}

bool GameApp::InitResource()
{
	// ���������ζ���
	VertexPosColor vertices[] =
	{
		{ XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
	};
	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof vertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mVertexBuffer.GetAddressOf()));

	// ����װ��׶εĶ��㻺��������
	UINT stride = sizeof(VertexPosColor);	// ��Խ�ֽ���
	UINT offset = 0;						// ��ʼƫ����

	md3dImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);


	// ����ͼԪ����
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	return true;
}
