#include "Effects.h"
#include "EffectHelper.h"
#include "Vertex.h"
#include <d3dcompiler.h>
#include <experimental/filesystem>
using namespace DirectX;
using namespace std::experimental;

//
// ��Щ�ṹ���ӦHLSL�Ľṹ�壬�������ļ�ʹ�á���Ҫ��16�ֽڶ���
//

struct CBChangesEveryDrawing
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX worldInvTranspose;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};


struct CBChangesRarely
{
	DirectionalLight dirLight[BasicObjectFX::maxLights];
	PointLight pointLight[BasicObjectFX::maxLights];
	SpotLight spotLight[BasicObjectFX::maxLights];
	Material material;
	DirectX::XMMATRIX view;
	DirectX::XMFLOAT3 eyePos;
	float cylinderHeight;
};


//
// BasicObjectFX::Impl ��Ҫ����BasicObjectFX�Ķ���
//

class BasicObjectFX::Impl : public AlignedType<BasicObjectFX::Impl>
{
public:
	// ������ʽָ��
	Impl() = default;
	~Impl() = default;

	// objFileNameInOutΪ����õ���ɫ���������ļ�(.*so)������ָ��������Ѱ�Ҹ��ļ�����ȡ
	// hlslFileNameΪ��ɫ�����룬��δ�ҵ���ɫ���������ļ��������ɫ������
	// ����ɹ�����ָ����objFileNameInOut���򱣴����õ���ɫ����������Ϣ�����ļ�
	// ppBlobOut�����ɫ����������Ϣ
	HRESULT CreateShaderFromFile(const WCHAR* objFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

public:
	// ��Ҫ16�ֽڶ�������ȷ���ǰ��
	CBufferObject<0, CBChangesEveryDrawing> cbDrawing;		// ÿ�ζ�����Ƶĳ���������
	CBufferObject<1, CBChangesOnResize>     cbOnResize;		// ÿ�δ��ڴ�С����ĳ���������
	CBufferObject<2, CBChangesRarely>		cbRarely;		// �����������ĳ���������
	BOOL isDirty;											// �Ƿ���ֵ���
	std::vector<CBufferBase*> cBufferPtrs;					// ͳһ�����������еĳ���������


	ComPtr<ID3D11VertexShader> triangleVS;
	ComPtr<ID3D11PixelShader> trianglePS;
	ComPtr<ID3D11GeometryShader> triangleGS;

	ComPtr<ID3D11VertexShader> cylinderVS;
	ComPtr<ID3D11PixelShader> cylinderPS;
	ComPtr<ID3D11GeometryShader> cylinderGS;

	ComPtr<ID3D11VertexShader> normalVS;
	ComPtr<ID3D11PixelShader> normalPS;
	ComPtr<ID3D11GeometryShader> normalGS;

	ComPtr<ID3D11InputLayout> vertexPosColorLayout;		// VertexPosColor���벼��
	ComPtr<ID3D11InputLayout> vertexPosNormalColorLayout;	// VertexPosNormalColor���벼��

	ComPtr<ID3D11ShaderResourceView> texture;				// ���ڻ��Ƶ�����

};

//
// BasicObjectFX
//

namespace
{
	// BasicObjectFX����
	static BasicObjectFX * pInstance = nullptr;
}

BasicObjectFX::BasicObjectFX()
{
	if (pInstance)
		throw std::exception("BasicObjectFX is a singleton!");
	pInstance = this;
	pImpl = std::make_unique<BasicObjectFX::Impl>();
}

BasicObjectFX::~BasicObjectFX()
{
}

BasicObjectFX::BasicObjectFX(BasicObjectFX && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
}

BasicObjectFX & BasicObjectFX::operator=(BasicObjectFX && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

BasicObjectFX & BasicObjectFX::Get()
{
	if (!pInstance)
		throw std::exception("BasicObjectFX needs an instance!");
	return *pInstance;
}


bool BasicObjectFX::InitAll(ComPtr<ID3D11Device> device)
{
	if (!device)
		return false;

	if (!pImpl->cBufferPtrs.empty())
		return true;


	ComPtr<ID3DBlob> blob;

	// ����������ɫ���Ͷ��㲼��
	HR(pImpl->CreateShaderFromFile(L"HLSL\\Triangle_VS.vso", L"HLSL\\Triangle_VS.hlsl", "VS", "vs_5_0", blob.GetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->triangleVS.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->vertexPosColorLayout.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Cylinder_VS.vso", L"HLSL\\Cylinder_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->cylinderVS.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->vertexPosNormalColorLayout.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Normal_VS.vso", L"HLSL\\Normal_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->normalVS.GetAddressOf()));

	// ����������ɫ��
	HR(pImpl->CreateShaderFromFile(L"HLSL\\Triangle_PS.pso", L"HLSL\\Triangle_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->trianglePS.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Cylinder_PS.pso", L"HLSL\\Cylinder_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->cylinderPS.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Normal_PS.pso", L"HLSL\\Normal_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->normalPS.GetAddressOf()));

	// ����������ɫ��
	HR(pImpl->CreateShaderFromFile(L"HLSL\\Triangle_GS.gso", L"HLSL\\Triangle_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->triangleGS.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Cylinder_GS.gso", L"HLSL\\Cylinder_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->cylinderGS.GetAddressOf()));

	HR(pImpl->CreateShaderFromFile(L"HLSL\\Normal_GS.gso", L"HLSL\\Normal_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->normalGS.GetAddressOf()));


	// ��ʼ��
	RenderStates::InitAll(device);

	pImpl->cBufferPtrs.assign({
		&pImpl->cbDrawing, 
		&pImpl->cbOnResize, 
		&pImpl->cbRarely});

	// ��������������
	for (auto& pBuffer : pImpl->cBufferPtrs)
	{
		pBuffer->CreateBuffer(device);
	}

	return true;
}

void BasicObjectFX::SetRenderSplitedTriangle(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexPosColorLayout.Get());
	deviceContext->VSSetShader(pImpl->triangleVS.Get(), nullptr, 0);
	deviceContext->GSSetShader(pImpl->triangleGS.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->trianglePS.Get(), nullptr, 0);

}

void BasicObjectFX::SetRenderCylinderNoCap(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	deviceContext->IASetInputLayout(pImpl->vertexPosNormalColorLayout.Get());
	deviceContext->VSSetShader(pImpl->cylinderVS.Get(), nullptr, 0);
	deviceContext->GSSetShader(pImpl->cylinderGS.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->cylinderPS.Get(), nullptr, 0);

}

void BasicObjectFX::SetRenderNormal(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(pImpl->vertexPosNormalColorLayout.Get());
	deviceContext->VSSetShader(pImpl->normalVS.Get(), nullptr, 0);
	deviceContext->GSSetShader(pImpl->normalGS.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->normalPS.Get(), nullptr, 0);

}



void XM_CALLCONV BasicObjectFX::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	auto& cBuffer = pImpl->cbDrawing;
	cBuffer.data.world = W;
	cBuffer.data.worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, W));
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetViewMatrix(FXMMATRIX V)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.view = V;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetProjMatrix(FXMMATRIX P)
{
	auto& cBuffer = pImpl->cbOnResize;
	cBuffer.data.proj = P;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetWorldViewProjMatrix(FXMMATRIX W, CXMMATRIX V, CXMMATRIX P)
{
	pImpl->cbDrawing.data.world = W;
	pImpl->cbDrawing.data.worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, W));
	pImpl->cbRarely.data.view = V;
	pImpl->cbOnResize.data.proj = P;

	auto& pCBuffers = pImpl->cBufferPtrs;
	pCBuffers[0]->isDirty = pCBuffers[1]->isDirty = pCBuffers[2]->isDirty = true;
	pImpl->isDirty = true;
}

void BasicObjectFX::SetDirLight(size_t pos, const DirectionalLight & dirLight)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.dirLight[pos] = dirLight;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetPointLight(size_t pos, const PointLight & pointLight)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.pointLight[pos] = pointLight;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetSpotLight(size_t pos, const SpotLight & spotLight)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.spotLight[pos] = spotLight;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetMaterial(const Material & material)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.material = material;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetEyePos(FXMVECTOR eyePos)
{
	auto& cBuffer = pImpl->cbRarely;
	XMStoreFloat3(&cBuffer.data.eyePos, eyePos);
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetCylinderHeight(float height)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.cylinderHeight = height;
	pImpl->isDirty = cBuffer.isDirty = true;
}



void BasicObjectFX::Apply(ComPtr<ID3D11DeviceContext> deviceContext)
{
	auto& pCBuffers = pImpl->cBufferPtrs;
	// ���������󶨵���Ⱦ������
	pCBuffers[0]->BindVS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);

	pCBuffers[0]->BindGS(deviceContext);
	pCBuffers[1]->BindGS(deviceContext);
	pCBuffers[2]->BindGS(deviceContext);
	
	pCBuffers[2]->BindPS(deviceContext);

	// ��������
	deviceContext->PSSetShaderResources(0, 1, pImpl->texture.GetAddressOf());

	if (pImpl->isDirty)
	{
		pImpl->isDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}

//
// BasicObjectFX::Implʵ�ֲ���
//


HRESULT BasicObjectFX::Impl::CreateShaderFromFile(const WCHAR * objFileNameInOut, const WCHAR * hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob ** ppBlobOut)
{
	HRESULT hr = S_OK;

	// Ѱ���Ƿ����Ѿ�����õĶ�����ɫ��
	if (objFileNameInOut && filesystem::exists(objFileNameInOut))
	{
		HR(D3DReadFileToBlob(objFileNameInOut, ppBlobOut));
	}
	else
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// ���� D3DCOMPILE_DEBUG ��־���ڻ�ȡ��ɫ��������Ϣ���ñ�־���������������飬
		// ����Ȼ������ɫ�������Ż�����
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// ��Debug�����½����Ż��Ա������һЩ����������
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ComPtr<ID3DBlob> errorBlob = nullptr;
		hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
			dwShaderFlags, 0, ppBlobOut, errorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			if (errorBlob != nullptr)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			return hr;
		}

		// ��ָ��������ļ���������ɫ����������Ϣ���
		if (objFileNameInOut)
		{
			HR(D3DWriteBlobToFile(*ppBlobOut, objFileNameInOut, FALSE));
		}
	}

	return hr;
}


