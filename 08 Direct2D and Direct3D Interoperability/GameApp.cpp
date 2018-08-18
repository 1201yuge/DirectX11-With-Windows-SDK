#include "GameApp.h"
#include <filesystem>
using namespace DirectX;
using namespace std::experimental;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosNormalColor::inputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

	// ��ʼ����꣬���̲���Ҫ
	mMouse->SetWindow(mhMainWnd);
	mMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	return true;
}

void GameApp::OnResize()
{
	assert(md2dFactory);
	assert(mdwriteFactory);
	// �ͷ�D2D�������Դ
	mColorBrush.Reset();
	md2dRenderTarget.Reset();

	D3DApp::OnResize();

	// ΪD2D����DXGI������ȾĿ��
	ComPtr<IDXGISurface> surface;
	HR(mSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HR(md2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, md2dRenderTarget.GetAddressOf()));

	surface.Reset();
	// �����̶���ɫˢ���ı���ʽ
	HR(md2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		mColorBrush.GetAddressOf()));
	HR(mdwriteFactory->CreateTextFormat(L"����", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"zh-cn",
		mTextFormat.GetAddressOf()));
	
}

void GameApp::UpdateScene(float dt)
{
	// ���³�������������������ת����
	static float phi = 0.0f, theta = 0.0f;
	phi += 0.00003f, theta += 0.00005f;
	mVSConstantBuffer.world = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	mVSConstantBuffer.worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, mVSConstantBuffer.world));
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[0].Get(), 0, nullptr, &mVSConstantBuffer, 0, 0);

	// �����л��ƹ�����
	Keyboard::State state = mKeyboard->GetState();
	mKeyboardTracker.Update(state);	
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		mPSConstantBuffer.dirLight = mDirLight;
		mPSConstantBuffer.pointLight = PointLight();
		mPSConstantBuffer.spotLight = SpotLight();
		md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &mPSConstantBuffer, 0, 0);
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		mPSConstantBuffer.dirLight = DirectionalLight();
		mPSConstantBuffer.pointLight = mPointLight;
		mPSConstantBuffer.spotLight = SpotLight();
		md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &mPSConstantBuffer, 0, 0);
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		mPSConstantBuffer.dirLight = DirectionalLight();
		mPSConstantBuffer.pointLight = PointLight();
		mPSConstantBuffer.spotLight = mSpotLight;
		md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &mPSConstantBuffer, 0, 0);
	}

	// �����л�ģ������
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		Geometry::MeshData meshData = Geometry::CreateBox();
		ResetMesh(meshData);
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::W))
	{
		Geometry::MeshData meshData = Geometry::CreateSphere();
		ResetMesh(meshData);
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::E))
	{
		Geometry::MeshData meshData = Geometry::CreateCylinder();
		ResetMesh(meshData);
	}
}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// ���Ƽ���ģ��
	md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);

	//
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	static const WCHAR* textStr = L"�л��ƹ�����: 1-ƽ�й� 2-��� 3-�۹��\n"
		 "�л�ģ��: Q-������ W-���� E-Բ����";
	md2dRenderTarget->DrawTextW(textStr, (UINT32)wcslen(textStr), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));
}

HRESULT GameApp::CreateShaderFromFile(const WCHAR * objFileNameInOut, const WCHAR * hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob ** ppBlobOut)
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




bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// ����������ɫ��
	HR(CreateShaderFromFile(L"HLSL\\Light_VS.vso", L"HLSL\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));
	// �������󶨶��㲼��
	HR(md3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout.GetAddressOf()));

	// ����������ɫ��
	HR(CreateShaderFromFile(L"HLSL\\Light_PS.vso", L"HLSL\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// ��ʼ������ģ��
	Geometry::MeshData meshData = Geometry::CreateBox();
	ResetMesh(meshData);


	// ******************
	// ���ó�������������
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	// �½�����VS��PS�ĳ���������
	HR(md3dDevice->CreateBuffer(&cbd, nullptr, mConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(md3dDevice->CreateBuffer(&cbd, nullptr, mConstantBuffers[1].GetAddressOf()));

	// ��ʼ��Ĭ�Ϲ���
	// �����
	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	// ���
	mPointLight.Position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mPointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range = 25.0f;
	// �۹��
	mSpotLight.Position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	mSpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 12.0f;
	mSpotLight.Range = 10000.0f;
	// ��ʼ������VS�ĳ�����������ֵ
	mVSConstantBuffer.world = XMMatrixIdentity();			
	mVSConstantBuffer.view = XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
	mVSConstantBuffer.proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f);
	mVSConstantBuffer.worldInvTranspose = XMMatrixIdentity();
	
	// ��ʼ������PS�ĳ�����������ֵ
	mPSConstantBuffer.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mPSConstantBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPSConstantBuffer.material.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	// ʹ��Ĭ��ƽ�й�
	mPSConstantBuffer.dirLight = mDirLight;
	// ע�ⲻҪ�������ô˴��Ĺ۲�λ�ã�����������ֻ�������
	mPSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);


	// ����PS������������Դ
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &mPSConstantBuffer, 0, 0);


	// ******************
	// ����Ⱦ���߸����׶ΰ󶨺�������Դ

	// ����ͼԪ���ͣ��趨���벼��
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetInputLayout(mVertexLayout.Get());
	// ����ɫ���󶨵���Ⱦ����
	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	// VS������������ӦHLSL�Ĵ���b0�ĳ���������
	md3dImmediateContext->VSSetConstantBuffers(0, 1, mConstantBuffers[0].GetAddressOf());
	// PS������������ӦHLSL�Ĵ���b1�ĳ���������
	md3dImmediateContext->PSSetConstantBuffers(1, 1, mConstantBuffers[1].GetAddressOf());
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData & meshData)
{
	// �ͷž���Դ
	mVertexBuffer.Reset();
	mIndexBuffer.Reset();

	size_t vertexSize = meshData.posVec.size();
	std::vector<VertexPosNormalColor> vertices(vertexSize);
	for (size_t i = 0; i < vertexSize; ++i)
	{
		vertices[i].pos = meshData.posVec[i];
		vertices[i].normal = meshData.normalVec[i];
		vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)vertices.size() * sizeof(VertexPosNormalColor);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mVertexBuffer.GetAddressOf()));

	// ����װ��׶εĶ��㻺��������
	UINT stride = sizeof(VertexPosNormalColor);	// ��Խ�ֽ���
	UINT offset = 0;							// ��ʼƫ����

	md3dImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);



	// ������������������
	mIndexCount = (int)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(WORD) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// �½�����������
	InitData.pSysMem = meshData.indexVec.data();
	HR(md3dDevice->CreateBuffer(&ibd, &InitData, mIndexBuffer.GetAddressOf()));
	// ����װ��׶ε���������������
	md3dImmediateContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	return true;
}
