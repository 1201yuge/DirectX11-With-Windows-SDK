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
	DirectX::XMMATRIX texTransform;
	Material material;
};

struct CBDrawingStates
{
	int isReflection;
	int isShadow;
	DirectX::XMINT2 pad;
};

struct CBChangesEveryFrame
{
	DirectX::XMMATRIX view;
	DirectX::XMVECTOR eyePos;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};


struct CBChangesRarely
{
	DirectX::XMMATRIX reflection;
	DirectX::XMMATRIX shadow;
	DirectX::XMMATRIX refShadow;
	DirectionalLight dirLight[BasicObjectFX::maxLights];
	PointLight pointLight[BasicObjectFX::maxLights];
	SpotLight spotLight[BasicObjectFX::maxLights];
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
	CBufferObject<1, CBDrawingStates>       cbStates;		// ÿ�λ���״̬����ĳ���������
	CBufferObject<2, CBChangesEveryFrame>   cbFrame;		// ÿ֡���Ƶĳ���������
	CBufferObject<3, CBChangesOnResize>     cbOnResize;		// ÿ�δ��ڴ�С����ĳ���������
	CBufferObject<4, CBChangesRarely>		cbRarely;		// �����������ĳ���������
	BOOL isDirty;											// �Ƿ���ֵ���
	std::vector<CBufferBase*> cBufferPtrs;					// ͳһ�����������еĳ���������


	ComPtr<ID3D11VertexShader> vertexShader3D;				// ����3D�Ķ�����ɫ��
	ComPtr<ID3D11PixelShader>  pixelShader3D;				// ����3D��������ɫ��
	ComPtr<ID3D11VertexShader> vertexShader2D;				// ����2D�Ķ�����ɫ��
	ComPtr<ID3D11PixelShader>  pixelShader2D;				// ����2D��������ɫ��

	ComPtr<ID3D11InputLayout>  vertexLayout2D;				// ����2D�Ķ������벼��
	ComPtr<ID3D11InputLayout>  vertexLayout3D;				// ����3D�Ķ������벼��

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

	// ����������ɫ��(2D)
	HR(pImpl->CreateShaderFromFile(L"HLSL\\BasicObject_VS_2D.vso", L"HLSL\\BasicObject_VS_2D.hlsl", "VS", "vs_5_0", blob.GetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->vertexShader2D.GetAddressOf()));
	// �������㲼��(2D)
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->vertexLayout2D.GetAddressOf()));

	// ����������ɫ��(2D)
	HR(pImpl->CreateShaderFromFile(L"HLSL\\BasicObject_PS_2D.pso", L"HLSL\\BasicObject_PS_2D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->pixelShader2D.GetAddressOf()));

	// ����������ɫ��(3D)
	HR(pImpl->CreateShaderFromFile(L"HLSL\\BasicObject_VS_3D.vso", L"HLSL\\BasicObject_VS_3D.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->vertexShader3D.GetAddressOf()));
	// �������㲼��(3D)
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->vertexLayout3D.GetAddressOf()));

	// ����������ɫ��(3D)
	HR(pImpl->CreateShaderFromFile(L"HLSL\\BasicObject_PS_3D.pso", L"HLSL\\BasicObject_PS_3D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->pixelShader3D.GetAddressOf()));


	// ��ʼ��
	RenderStates::InitAll(device);

	pImpl->cBufferPtrs.assign({
		&pImpl->cbDrawing, 
		&pImpl->cbFrame, 
		&pImpl->cbStates, 
		&pImpl->cbOnResize, 
		&pImpl->cbRarely});

	// ��������������
	for (auto& pBuffer : pImpl->cBufferPtrs)
	{
		pBuffer->CreateBuffer(device);
	}

	return true;
}

void BasicObjectFX::SetRenderDefault(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetRenderAlphaBlend(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetDrawBoltAnimNoDepthTest(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthTest.Get(), 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetDrawBoltAnimNoDepthWrite(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetRenderNoDoubleBlend(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetWriteStencilOnly(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetRenderDefaultWithStencil(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSCullClockWise.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetRenderAlphaBlendWithStencil(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetDrawBoltAnimNoDepthTestWithStencil(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthTestWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::SetDrawBoltAnimNoDepthWriteWithStencil(ComPtr<ID3D11DeviceContext> deviceContext, UINT stencilRef)
{
	deviceContext->IASetInputLayout(pImpl->vertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthWriteWithStencil.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSAdditive.Get(), nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::Set2DRenderDefault(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout2D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader2D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->pixelShader2D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicObjectFX::Set2DRenderAlphaBlend(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout2D.Get());
	deviceContext->VSSetShader(pImpl->vertexShader2D.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->pixelShader2D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
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
	auto& cBuffer = pImpl->cbFrame;
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
	pImpl->cbFrame.data.view = V;
	pImpl->cbOnResize.data.proj = P;

	auto& pCBuffers = pImpl->cBufferPtrs;
	pCBuffers[0]->isDirty = pCBuffers[1]->isDirty = pCBuffers[3]->isDirty = true;
	pImpl->isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetTexTransformMatrix(FXMMATRIX W)
{
	auto& cBuffer = pImpl->cbDrawing;
	cBuffer.data.texTransform = W;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetReflectionMatrix(FXMMATRIX R)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.reflection = R;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetShadowMatrix(FXMMATRIX S)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.shadow = S;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicObjectFX::SetRefShadowMatrix(DirectX::FXMMATRIX RefS)
{
	auto& cBuffer = pImpl->cbRarely;
	cBuffer.data.refShadow = RefS;
	pImpl->isDirty = cBuffer.isDirty = true;
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
	auto& cBuffer = pImpl->cbDrawing;
	cBuffer.data.material = material;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetTexture(ComPtr<ID3D11ShaderResourceView> texture)
{
	pImpl->texture = texture;
}

void XM_CALLCONV BasicObjectFX::SetEyePos(FXMVECTOR eyePos)
{
	auto& cBuffer = pImpl->cbFrame;
	cBuffer.data.eyePos = eyePos;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetReflectionState(bool isOn)
{
	auto& cBuffer = pImpl->cbStates;
	cBuffer.data.isReflection = isOn;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::SetShadowState(bool isOn)
{
	auto& cBuffer = pImpl->cbStates;
	cBuffer.data.isShadow = isOn;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicObjectFX::Apply(ComPtr<ID3D11DeviceContext> deviceContext)
{
	auto& pCBuffers = pImpl->cBufferPtrs;
	// ���������󶨵���Ⱦ������
	pCBuffers[0]->BindVS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);
	pCBuffers[3]->BindVS(deviceContext);
	pCBuffers[4]->BindVS(deviceContext);

	pCBuffers[0]->BindPS(deviceContext);
	pCBuffers[1]->BindPS(deviceContext);
	pCBuffers[2]->BindPS(deviceContext);
	pCBuffers[4]->BindPS(deviceContext);

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


