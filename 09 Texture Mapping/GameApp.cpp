#include "GameApp.h"
#include <filesystem>
using namespace DirectX;
using namespace std::experimental;



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

	Keyboard::State state = mKeyboard->GetState();
	mKeyboardTracker.Update(state);	

	// �����л�ģʽ
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		// ����ľ�䶯��
		mCurrMode = ShowMode::WoodCrate;
		md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
		Geometry::MeshData meshData = Geometry::CreateBox();
		ResetMesh(meshData);
		md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
		md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
		md3dImmediateContext->PSSetShaderResources(0, 1, mWoodCrate.GetAddressOf());
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		mCurrMode = ShowMode::FireAnim;
		mCurrFrame = 0;
		md3dImmediateContext->IASetInputLayout(mVertexLayout2D.Get());
		Geometry::MeshData meshData = Geometry::Create2DShow();
		ResetMesh(meshData);
		md3dImmediateContext->VSSetShader(mVertexShader2D.Get(), nullptr, 0);
		md3dImmediateContext->PSSetShader(mPixelShader2D.Get(), nullptr, 0);
		md3dImmediateContext->PSSetShaderResources(0, 1, mFireAnim[0].GetAddressOf());
	}

	if (mCurrMode == ShowMode::WoodCrate)
	{
		// ���³�������������������ת����
		static float phi = 0.0f, theta = 0.0f;
		phi += 0.00003f, theta += 0.00005f;
		XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
		mVSConstantBuffer.world = XMMatrixTranspose(W);
		mVSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, W);	// ����ת�õ���
		md3dImmediateContext->UpdateSubresource(mConstantBuffers[0].Get(), 0, nullptr, &mVSConstantBuffer, 0, 0);
	}
	else if (mCurrMode == ShowMode::FireAnim)
	{
		// ����������1��60֡
		static float totDeltaTime = 0;

		totDeltaTime += dt;
		if (totDeltaTime > 1.0f / 60)
		{
			totDeltaTime -= 1.0f / 60;
			mCurrFrame = (mCurrFrame + 1) % 120;
			md3dImmediateContext->PSSetShaderResources(0, 1, mFireAnim[mCurrFrame].GetAddressOf());
		}		
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
	static const WCHAR* textStr = L"�л���ʾ: 1-ľ��(3D) 2-����(2D)\n";
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

	// ����������ɫ��(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_2D.vso", L"HLSL\\Basic_VS_2D.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader2D.GetAddressOf()));
	// �������㲼��(2D)
	HR(md3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout2D.GetAddressOf()));

	// ����������ɫ��(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_2D.pso", L"HLSL\\Basic_PS_2D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader2D.GetAddressOf()));

	// ����������ɫ��(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.vso", L"HLSL\\Basic_VS_3D.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mVertexShader3D.GetAddressOf()));
	// �������㲼��(3D)
	HR(md3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), mVertexLayout3D.GetAddressOf()));

	// ����������ɫ��(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.pso", L"HLSL\\Basic_PS_3D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(md3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, mPixelShader3D.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// ��ʼ������ģ�Ͳ����õ�����װ��׶�
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

	// ******************
	// ��ʼ������Ͳ�����״̬
	
	// ��ʼ��ľ������
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, mWoodCrate.GetAddressOf()));
	// ��ʼ����������
	WCHAR strFile[40];
	mFireAnim.resize(120);
	for (int i = 1; i <= 120; ++i)
	{
		wsprintf(strFile, L"Texture\\FireAnim\\Fire%03d.bmp", i);
		HR(CreateWICTextureFromFile(md3dDevice.Get(), strFile, nullptr, mFireAnim[i - 1].GetAddressOf()));
	}
		
	// ��ʼ��������״̬
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(md3dDevice->CreateSamplerState(&sampDesc, mSamplerState.GetAddressOf()));

	
	// ******************
	// ��ʼ��������������ֵ

	// ��ʼ������VS�ĳ�����������ֵ
	mVSConstantBuffer.world = XMMatrixIdentity();			
	mVSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	mVSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	mVSConstantBuffer.worldInvTranspose = XMMatrixIdentity();
	
	// ��ʼ������PS�ĳ�����������ֵ
	// ����ֻʹ��һյ�������ʾ
	mPSConstantBuffer.pointLight[0].Position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	mPSConstantBuffer.pointLight[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPSConstantBuffer.pointLight[0].Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPSConstantBuffer.pointLight[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mPSConstantBuffer.pointLight[0].Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPSConstantBuffer.pointLight[0].Range = 25.0f;
	mPSConstantBuffer.numDirLight = 0;
	mPSConstantBuffer.numPointLight = 1;
	mPSConstantBuffer.numSpotLight = 0;
	mPSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);	// ����������©���Ѳ���
	// ��ʼ������
	mPSConstantBuffer.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mPSConstantBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPSConstantBuffer.material.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 5.0f);
	// ע�ⲻҪ�������ô˴��Ĺ۲�λ�ã�����������ֻ�������
	mPSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

	// ����PS������������Դ
	md3dImmediateContext->UpdateSubresource(mConstantBuffers[1].Get(), 0, nullptr, &mPSConstantBuffer, 0, 0);

	// ******************
	// ����Ⱦ���߸����׶ΰ󶨺�������Դ
	// ����ͼԪ���ͣ��趨���벼��
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetInputLayout(mVertexLayout3D.Get());
	// Ĭ�ϰ�3D��ɫ��
	md3dImmediateContext->VSSetShader(mVertexShader3D.Get(), nullptr, 0);
	// VS������������ӦHLSL�Ĵ���b0�ĳ���������
	md3dImmediateContext->VSSetConstantBuffers(0, 1, mConstantBuffers[0].GetAddressOf());
	// PS������������ӦHLSL�Ĵ���b1�ĳ���������
	md3dImmediateContext->PSSetConstantBuffers(1, 1, mConstantBuffers[1].GetAddressOf());
	// ������ɫ�׶����úò�����
	md3dImmediateContext->PSSetSamplers(0, 1, mSamplerState.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(0, 1, mWoodCrate.GetAddressOf());
	md3dImmediateContext->PSSetShader(mPixelShader3D.Get(), nullptr, 0);
	
	
	// ������ɫ�׶�Ĭ������ľ������
	mCurrMode = ShowMode::WoodCrate;

	return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData & meshData)
{
	// �ͷž���Դ
	mVertexBuffer.Reset();
	mIndexBuffer.Reset();



	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalTex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mVertexBuffer.GetAddressOf()));

	// ����װ��׶εĶ��㻺��������
	UINT stride = sizeof(VertexPosNormalTex);	// ��Խ�ֽ���
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
