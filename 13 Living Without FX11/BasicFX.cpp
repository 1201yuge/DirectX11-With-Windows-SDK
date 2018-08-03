#include "BasicFX.h"
#include <filesystem>

using namespace DirectX;
using namespace std::experimental;


bool BasicFX::InitAll(ComPtr<ID3D11Device> device)
{
	if (!device)
		return false;

	ComPtr<ID3DBlob> blob;

	// �Ѿ�����õ���ɫ���ļ���
	std::wstring pso2DPath = L"HLSL\\Basic_PS_2D.cso", vso2DPath = L"HLSL\\Basic_VS_2D.cso";
	std::wstring pso3DPath = L"HLSL\\Basic_PS_3D.cso", vso3DPath = L"HLSL\\Basic_VS_3D.cso";
	// ******************************************************
	// Ѱ���Ƿ����Ѿ�����õĶ�����ɫ��(2D)�������������ڱ���
	if (filesystem::exists(vso2DPath))
	{
		HR(D3DReadFileToBlob(vso2DPath.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"HLSL\\Basic.fx", "VS_2D", "vs_5_0", blob.GetAddressOf()));
	}
	// ����������ɫ��(2D)
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader2D.GetAddressOf()));
	// �������㲼��(2D)
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout2D.GetAddressOf()));

	blob.Reset();

	// ******************************************************
	// Ѱ���Ƿ����Ѿ�����õĶ�����ɫ��(3D)�������������ڱ���
	if (filesystem::exists(vso3DPath))
	{
		HR(D3DReadFileToBlob(vso3DPath.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"HLSL\\Basic.fx", "VS_3D", "vs_5_0", blob.GetAddressOf()));
	}
	// ����������ɫ��(3D)
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader3D.GetAddressOf()));
	// �������㲼��(3D)
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout3D.GetAddressOf()));
	blob.Reset();

	// ******************************************************
	// Ѱ���Ƿ����Ѿ�����õ�������ɫ��(2D)�������������ڱ���
	if (filesystem::exists(pso2DPath))
	{
		HR(D3DReadFileToBlob(pso2DPath.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"HLSL\\Basic.fx", "PS_2D", "ps_5_0", blob.GetAddressOf()));
	}
	// ����������ɫ��(2D)
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader2D.GetAddressOf()));
	blob.Reset();

	// ******************************************************
	// Ѱ���Ƿ����Ѿ�����õ�������ɫ��(3D)�������������ڱ���
	if (filesystem::exists(pso3DPath))
	{
		HR(D3DReadFileToBlob(pso3DPath.c_str(), blob.GetAddressOf()));
	}
	else
	{
		HR(CompileShaderFromFile(L"HLSL\\Basic.fx", "PS_3D", "ps_5_0", blob.GetAddressOf()));
	}
	// ����������ɫ��(3D)
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader3D.GetAddressOf()));
	blob.Reset();


	RenderStates::InitAll(device);
	device->GetImmediateContext(md3dImmediateContext.GetAddressOf());

	// ******************
	// ���ó�������������
	mConstantBuffers.assign(5, nullptr);
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	
	cbd.ByteWidth = Align16Bytes(sizeof(CBChangesEveryDrawing));
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = Align16Bytes(sizeof(CBDrawingState));
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffers[1].GetAddressOf()));
	cbd.ByteWidth = Align16Bytes(sizeof(CBChangesEveryFrame));
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffers[2].GetAddressOf()));
	cbd.ByteWidth = Align16Bytes(sizeof(CBChangesOnResize));
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffers[3].GetAddressOf()));
	cbd.ByteWidth = Align16Bytes(sizeof(CBNeverChange));
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffers[4].GetAddressOf()));

	// Ԥ�Ȱ󶨸�������Ļ�����������ÿ֡���µĻ�������Ҫ�󶨵�������������
	md3dImmediateContext->VSSetConstantBuffers(0, 1, mConstantBuffers[0].GetAddressOf());
	md3dImmediateContext->VSSetConstantBuffers(1, 1, mConstantBuffers[1].GetAddressOf());
	md3dImmediateContext->VSSetConstantBuffers(2, 1, mConstantBuffers[2].GetAddressOf());
	md3dImmediateContext->VSSetConstantBuffers(3, 1, mConstantBuffers[3].GetAddressOf());
	md3dImmediateContext->VSSetConstantBuffers(4, 1, mConstantBuffers[4].GetAddressOf());

	md3dImmediateContext->PSSetConstantBuffers(0, 1, mConstantBuffers[0].GetAddressOf());
	md3dImmediateContext->PSSetConstantBuffers(2, 1, mConstantBuffers[2].GetAddressOf());
	md3dImmediateContext->PSSetConstantBuffers(4, 1, mConstantBuffers[4].GetAddressOf());
	// ����Ĭ��ͼԪ
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

bool BasicFX::IsInit() const
{
	return md3dImmediateContext != nullptr;
}

void BasicFX::SetRenderDefault()
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicFX::SetRenderAlphaBlend()
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicFX::SetRenderNoDoubleBlend(UINT stencilRef)
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	md3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicFX::SetWriteStencilOnly(UINT stencilRef)
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), stencilRef);
	md3dImmediateContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
}

void BasicFX::SetRenderDefaultWithStencil(UINT stencilRef)
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(RenderStates::RSCullClockWise.Get());
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	md3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicFX::SetRenderAlphaBlendWithStencil(UINT stencilRef)
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	md3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicFX::Set2DRenderDefault()
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout2D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader2D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->PSSetShader(mPixelShader2D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicFX::Set2DRenderAlphaBlend()
{
	md3dImmediateContext->IASetInputLayout(mVertexLayout2D.Get());
	md3dImmediateContext->VSSetShader(mVertexShader2D.Get(), nullptr, 0);
	md3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	md3dImmediateContext->PSSetShader(mPixelShader2D.Get(), nullptr, 0);
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}




HRESULT BasicFX::CompileShaderFromFile(const WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob ** ppBlobOut)
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

template<class T>
void BasicFX::UpdateConstantBuffer(const T& cbuffer)
{
}

template<>
void BasicFX::UpdateConstantBuffer<CBChangesEveryDrawing>(const CBChangesEveryDrawing& cbuffer)
{
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[0].Get(), 0, nullptr, &cbuffer, 0, 0);
}

template<>
void BasicFX::UpdateConstantBuffer<CBDrawingState>(const CBDrawingState& cbuffer)
{
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &cbuffer, 0, 0);
}

template<>
void BasicFX::UpdateConstantBuffer<CBChangesEveryFrame>(const CBChangesEveryFrame& cbuffer)
{
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[2].Get(), 0, nullptr, &cbuffer, 0, 0);
}

template<>
void BasicFX::UpdateConstantBuffer<CBChangesOnResize>(const CBChangesOnResize& cbuffer)
{
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[3].Get(), 0, nullptr, &cbuffer, 0, 0);
}

template<>
void BasicFX::UpdateConstantBuffer<CBNeverChange>(const CBNeverChange& cbuffer)
{
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[4].Get(), 0, nullptr, &cbuffer, 0, 0);
}
