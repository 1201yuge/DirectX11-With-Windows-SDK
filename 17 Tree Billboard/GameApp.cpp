#include "GameApp.h"
#include <filesystem>
#include <algorithm>
using namespace DirectX;
using namespace std::experimental;
using namespace Microsoft::WRL;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	// ����4�����ز���
	mEnable4xMsaa = true;
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!mBasicFX.InitAll(md3dDevice))
		return false;

	if (!InitResource())
		return false;

	// ��ʼ����꣬���̲���Ҫ
	mMouse->SetWindow(mhMainWnd);
	mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

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
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
		mTextFormat.GetAddressOf()));

	// ֻ�г�������������ʼ�����ִ�и��²���
	if (mBasicFX.IsInit())
	{
		mCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		mCBChangesOnReSize.proj = mCamera->GetViewXM();
		mBasicFX.UpdateConstantBuffer(mCBChangesOnReSize);
	}

}

void GameApp::UpdateScene(float dt)
{

	// ��������¼�����ȡ���ƫ����
	Mouse::State mouseState = mMouse->GetState();
	Mouse::State lastMouseState = mMouseTracker.GetLastState();
	mMouseTracker.Update(mouseState);

	Keyboard::State keyState = mKeyboard->GetState();
	mKeyboardTracker.Update(keyState);

	// ��ȡ����
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(mCamera);

	if (mCameraMode == CameraMode::Free)
	{
		// ����������Ĳ���

		// �����ƶ�
		if (keyState.IsKeyDown(Keyboard::W))
		{
			cam1st->MoveForward(dt * 3.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S))
		{
			cam1st->MoveForward(dt * -3.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
			cam1st->Strafe(dt * -3.0f);
		if (keyState.IsKeyDown(Keyboard::D))
			cam1st->Strafe(dt * 3.0f);

		// ��Ұ��ת����ֹ��ʼ�Ĳ�ֵ�����µ�ͻȻ��ת
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
	}

	// ��λ��������[-49.9f, 49.9f]��������
	// ��������
	XMFLOAT3 adjustedPos;
	XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), 
		XMVectorSet(-49.9f, 0.0f, -49.9f, 0.0f), XMVectorSet(49.9f, 99.9f, 49.9f, 0.0f)));
	cam1st->SetPosition(adjustedPos);

	// ���¹۲����
	mCamera->UpdateViewMatrix();
	XMStoreFloat4(&mCBChangesEveryFrame.eyePos, mCamera->GetPositionXM());
	mCBChangesEveryFrame.view = mCamera->GetViewXM();

	bool isDrawingStateChanged = false;
	// ������Ч
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		mCBDrawingStates.fogEnabled = !mCBDrawingStates.fogEnabled;
		isDrawingStateChanged = true;
	}
	// ����/��ҹ�任
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		mIsNight = !mIsNight;
		if (mIsNight)
		{
			// ��ҹģʽ�±�Ϊ�𽥺ڰ�
			mCBDrawingStates.fogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			mCBDrawingStates.fogStart = 5.0f;
		}
		else
		{
			// ����ģʽ���Ӧ��Ч
			mCBDrawingStates.fogColor = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
			mCBDrawingStates.fogStart = 15.0f;
		}
		isDrawingStateChanged = true;
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		mEnableAlphaToCoverage = !mEnableAlphaToCoverage;
	}
	// ������ķ�Χ
	if (mouseState.scrollWheelValue != 0)
	{
		// һ�ι��ֹ�������С��λΪ120
		mCBDrawingStates.fogRange += mouseState.scrollWheelValue / 120;
		if (mCBDrawingStates.fogRange < 15.0f)
			mCBDrawingStates.fogRange = 15.0f;
		else if (mCBDrawingStates.fogRange > 175.0f)
			mCBDrawingStates.fogRange = 175.0f;
		isDrawingStateChanged = true;
	}
	

	// ���ù���ֵ
	mMouse->ResetScrollWheelValue();

	

	// �˳���������Ӧ�򴰿ڷ���������Ϣ
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);

	if (isDrawingStateChanged)
	{
		mBasicFX.UpdateConstantBuffer(mCBDrawingStates);
	}
	mBasicFX.UpdateConstantBuffer(mCBChangesEveryFrame);



}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	// ����/��ҹ����
	if (mIsNight)
	{
		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	}
	else
	{
		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Silver));
	}
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ���Ƶ���
	mBasicFX.SetRenderDefault();
	mGround.Draw(md3dImmediateContext);

	// ������
	mBasicFX.SetRenderBillboard(mEnableAlphaToCoverage);
	UINT stride = sizeof(VertexPosSize);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, mPointSpritesBuffer.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->Draw(16, 0);

	//
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	std::wstring text = L"1-��Ч���� 2-����/��ҹ��Ч�л� 3-AlphaToCoverage���� Esc-�˳�\n"
		"����-������Ч��Χ\n"
		"��֧�������ӽ������\n";
	text += std::wstring(L"AlphaToCoverage״̬: ") + (mEnableAlphaToCoverage ? L"����\n" : L"�ر�\n");
	text += std::wstring(L"��Ч״̬: ") + (mCBDrawingStates.fogEnabled ? L"����\n" : L"�ر�\n");
	if (mCBDrawingStates.fogEnabled)
	{
		text += std::wstring(L"�������: ") + (mIsNight ? L"��ҹ\n" : L"����\n");
		text += L"��Ч��Χ: " + std::to_wstring(mIsNight ? 5 : 15) + L"-" + 
			std::to_wstring((mIsNight ? 5 : 15) + (int)mCBDrawingStates.fogRange);
	}


	md2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));

}



bool GameApp::InitResource()
{
	// Ĭ�ϰ��죬����AlphaToCoverage
	mIsNight = false;
	mEnableAlphaToCoverage = true;
	// ******************
	// ��ʼ����������

	// ��ʼ����������Դ
	mTreeTexArray = CreateDDSTexture2DArrayShaderResourceView(
		md3dDevice,
		md3dImmediateContext,
		std::vector<std::wstring>{
		L"Texture\\tree0.dds",
			L"Texture\\tree1.dds",
			L"Texture\\tree2.dds",
			L"Texture\\tree3.dds"});
	
	// ��ʼ���㾫�黺����
	InitPointSpritesBuffer();

	// ��ʼ�����Ĳ���
	mTreeMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mTreeMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	ComPtr<ID3D11ShaderResourceView> texture;
	// ��ʼ���ذ�
	mGround.SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(0.0f, -5.0f, 0.0f), XMFLOAT2(100.0f, 100.0f), XMFLOAT2(10.0f, 10.0f)));
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\Grass.dds", nullptr, texture.GetAddressOf()));
	mGround.SetTexture(texture);
	Material material;
	material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
	mGround.SetMaterial(material);

	// ******************
	// ��ʼ��������������ֵ

	mCBChangesEveryDrawing.material = mTreeMat;
	mCBChangesEveryDrawing.world = mCBChangesEveryDrawing.worldInvTranspose = XMMatrixIdentity();
	mCBChangesEveryDrawing.texTransform = XMMatrixIdentity();


	// �����
	mCBRarely.dirLight[0].Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mCBRarely.dirLight[0].Diffuse = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mCBRarely.dirLight[0].Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mCBRarely.dirLight[0].Direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	mCBRarely.dirLight[1] = mCBRarely.dirLight[0];
	mCBRarely.dirLight[1].Direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
	mCBRarely.dirLight[2] = mCBRarely.dirLight[0];
	mCBRarely.dirLight[2].Direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
	mCBRarely.dirLight[3] = mCBRarely.dirLight[0];
	mCBRarely.dirLight[3].Direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);

	// ��������
	mCameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	mCamera = camera;
	camera->SetPosition(XMFLOAT3());
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	camera->UpdateViewMatrix();


	mCBChangesEveryFrame.view = camera->GetViewXM();
	XMStoreFloat4(&mCBChangesEveryFrame.eyePos, camera->GetPositionXM());

	mCBChangesOnReSize.proj = camera->GetProjXM();
	
	// ��״̬Ĭ�Ͽ���
	mCBDrawingStates.fogEnabled = 1;
	mCBDrawingStates.fogColor = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);	// ��ɫ
	mCBDrawingStates.fogRange = 75.0f;
	mCBDrawingStates.fogStart = 15.0f;
	// ���³�����������Դ
	mBasicFX.UpdateConstantBuffer(mCBChangesEveryDrawing);
	mBasicFX.UpdateConstantBuffer(mCBChangesEveryFrame);
	mBasicFX.UpdateConstantBuffer(mCBChangesOnReSize);
	mBasicFX.UpdateConstantBuffer(mCBDrawingStates);
	mBasicFX.UpdateConstantBuffer(mCBRarely);

	// ֱ�Ӱ���������
	md3dImmediateContext->PSSetShaderResources(1, 1, mTreeTexArray.GetAddressOf());
	
	return true;
}

void GameApp::InitPointSpritesBuffer()
{
	srand((unsigned)time(nullptr));
	VertexPosSize vertexes[16];
	float theta = 0.0f;
	for (int i = 0; i < 16; ++i)
	{
		// ȡ20-50�İ뾶�����������
		float radius = (float)(rand() % 31 + 20);
		float randomRad = rand() % 256 / 256.0f * XM_2PI / 16;
		vertexes[i].pos = XMFLOAT3(radius * cosf(theta + randomRad), 8.0f, radius * sinf(theta + randomRad));
		vertexes[i].size = XMFLOAT2(30.0f, 30.0f);
		theta += XM_2PI / 16;
	}

	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// ���ݲ����޸�
	vbd.ByteWidth = sizeof (vertexes);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertexes;
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mPointSpritesBuffer.GetAddressOf()));
}


ComPtr<ID3D11ShaderResourceView> GameApp::CreateDDSTexture2DArrayShaderResourceView(
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11DeviceContext> deviceContext,
	const std::vector<std::wstring>& filenames,
	int maxMipMapSize)
{
	//
	// 1. ��ȡ��������
	//
	size_t size = filenames.size();
	std::vector<ComPtr<ID3D11Texture2D>> srcTex(size);
	UINT mipLevel = maxMipMapSize;
	UINT width, height;
	DXGI_FORMAT format;
	for (size_t i = 0; i < size; ++i)
	{
		// ������Щ�������ᱻGPUʹ�ã�����ʹ��D3D11_USAGE_STAGINGö��ֵ
		// ʹ��CPU���Զ�ȡ��Դ
		HR(CreateDDSTextureFromFileEx(device.Get(),
			deviceContext.Get(),
			filenames[i].c_str(),
			maxMipMapSize,
			D3D11_USAGE_STAGING,							// Usage
			0,												// BindFlags
			D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ,	// CpuAccessFlags
			0,												// MiscFlags
			false,
			(ID3D11Resource**)srcTex[i].GetAddressOf(),
			nullptr));

		// ��ȡ�����õ�����Mipmap�ȼ�, ��Ⱥ͸߶�
		D3D11_TEXTURE2D_DESC texDesc;
		srcTex[i]->GetDesc(&texDesc);
		if (i == 0)
		{
			mipLevel = texDesc.MipLevels;
			width = texDesc.Width;
			height = texDesc.Height;
			format = texDesc.Format;
		}
		// ����������������MipMap�ȼ�����Ⱥ͸߶�Ӧ��һ��
		assert(mipLevel == texDesc.MipLevels);
		assert(texDesc.Width == width && texDesc.Height == height);
		// ����Ҫ�������ṩ��ͼƬ���ݸ�ʽӦ����һ�µģ������ڲ�һ�µ��������
		// ʹ��dxtex.exe(DirectX Texture Tool)�����е�ͼƬת��һ�µ����ݸ�ʽ
		assert(texDesc.Format == format);
		
	}

	//
	// 2.������������
	//
	D3D11_TEXTURE2D_DESC texDesc, texArrayDesc;
	srcTex[0]->GetDesc(&texDesc);
	texArrayDesc.Width = texDesc.Width;
	texArrayDesc.Height = texDesc.Height;
	texArrayDesc.MipLevels = texDesc.MipLevels;
	texArrayDesc.ArraySize = size;
	texArrayDesc.Format = texDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> texArray;
	HR(device->CreateTexture2D(&texArrayDesc, nullptr, texArray.GetAddressOf()));

	//
	// 3.�����е���������Դ��ֵ������������
	//

	// ÿ������Ԫ��
	for (size_t i = 0; i < size; ++i)
	{
		// �����е�ÿ��mipmap�ȼ�
		for (UINT j = 0; j < mipLevel; ++j)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			// ����ӳ������i�����У�����j��mipmap�ȼ���2D����
			HR(deviceContext->Map(srcTex[i].Get(),
				j, D3D11_MAP_READ, 0, &mappedTex2D));
			deviceContext->UpdateSubresource(
				texArray.Get(),
				D3D11CalcSubresource(j, i, mipLevel),	// i * mipLevel + j
				nullptr,
				mappedTex2D.pData,
				mappedTex2D.RowPitch,
				mappedTex2D.DepthPitch);
			// ֹͣӳ��
			deviceContext->Unmap(srcTex[i].Get(), j);
		}
	}

	//
	// 4.�������������SRV
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ComPtr<ID3D11ShaderResourceView> texArraySRV;
	HR(device->CreateShaderResourceView(texArray.Get(), &viewDesc, texArraySRV.GetAddressOf()));


	// �Ѿ�ȷ��������Դ��ComPtr���������ֶ��ͷ�

	return texArraySRV;
}


