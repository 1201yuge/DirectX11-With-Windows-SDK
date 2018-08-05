#include "GameApp.h"
#include <filesystem>
#include <algorithm>

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
	
	// ����������ʾ
	if (mBasicFX.IsInit())
	{
		mCamera->SetFrustum(XM_PIDIV2, AspectRatio(), 0.5f, 1000.0f);
		mCBOnReSize.proj = mCamera->GetProj();
		mBasicFX.UpdateConstantBuffer(mCBOnReSize);
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
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(mCamera);
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(mCamera);
	
	if (mCameraMode == CameraMode::FirstPerson || mCameraMode == CameraMode::Free)
	{
		// ��һ�˳�/����������Ĳ���

		// �����ƶ�
		if (keyState.IsKeyDown(Keyboard::W))
		{
			if (mCameraMode == CameraMode::FirstPerson)
				cam1st->Walk(dt * 3.0f);
			else
				cam1st->MoveForward(dt * 3.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S))
		{
			if (mCameraMode == CameraMode::FirstPerson)
				cam1st->Walk(dt * -3.0f);
			else
				cam1st->MoveForward(dt * -3.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
			cam1st->Strafe(dt * -3.0f);
		if (keyState.IsKeyDown(Keyboard::D))
			cam1st->Strafe(dt * 3.0f);

		// ��λ��������[-8.9f, 8.9f]��������
		// ��������
		XMFLOAT3 adjustedPos;
		XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		cam1st->SetPosition(adjustedPos);

		// ���ڵ�һ�˳�ģʽ�ƶ�����
		if (mCameraMode == CameraMode::FirstPerson)
			mWoodCrate.SetWorldMatrix(XMMatrixTranslation(adjustedPos.x, adjustedPos.y, adjustedPos.z));
		// ��Ұ��ת����ֹ��ʼ�Ĳ�ֵ�����µ�ͻȻ��ת
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
	}
	else if (mCameraMode == CameraMode::ThirdPerson)
	{
		// �����˳�������Ĳ���

		cam3rd->SetTarget(mWoodCrate.GetPosition());

		// ��������ת
		cam3rd->RotateX(mouseState.y * dt * 1.25f);
		cam3rd->RotateY(mouseState.x * dt * 1.25f);
		cam3rd->Approach(-mouseState.scrollWheelValue / 120 * 1.0f);
	}

	// ���¹۲����
	mCamera->UpdateViewMatrix();
	XMStoreFloat4(&mCBFrame.eyePos, mCamera->GetPositionXM());
	mCBFrame.view = mCamera->GetView();

	// ���ù���ֵ
	mMouse->ResetScrollWheelValue();

	// �����ģʽ�л�
	if (keyState.IsKeyDown(Keyboard::D1) && mCameraMode != CameraMode::FirstPerson)
	{
		if (!cam1st)
		{
			cam1st.reset(new FirstPersonCamera);
			cam1st->SetFrustum(XM_PIDIV2, AspectRatio(), 0.5f, 1000.0f);
			mCamera = cam1st;
		}
		XMFLOAT3 pos = mWoodCrate.GetPosition();
		XMFLOAT3 target = (!pos.x && !pos.z ? XMFLOAT3{ 0.0f, 0.0f, 1.0f } : XMFLOAT3{});
		cam1st->LookAt(pos, target, XMFLOAT3(0.0f, 1.0f, 0.0f));

		mCameraMode = CameraMode::FirstPerson;
	}
	else if (keyState.IsKeyDown(Keyboard::D2) && mCameraMode != CameraMode::ThirdPerson)
	{
		if (!cam3rd)
		{
			cam3rd.reset(new ThirdPersonCamera);
			cam3rd->SetFrustum(XM_PIDIV2, AspectRatio(), 0.5f, 1000.0f);
			mCamera = cam3rd;
		}
		XMFLOAT3 target = mWoodCrate.GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistanceMinMax(3.0f, 20.0f);
		// ��ʼ��ʱ������󷽿�
		// cam3rd->RotateY(-XM_PIDIV2);

		mCameraMode = CameraMode::ThirdPerson;
	}
	else if (keyState.IsKeyDown(Keyboard::D3) && mCameraMode != CameraMode::Free)
	{
		if (!cam1st)
		{
			cam1st.reset(new FirstPersonCamera);
			cam1st->SetFrustum(XM_PIDIV2, AspectRatio(), 0.5f, 1000.0f);
			mCamera = cam1st;
		}
		// �������Ϸ���ʼ
		XMFLOAT3 pos = mWoodCrate.GetPosition();
		XMFLOAT3 look{ 0.0f, 0.0f, 1.0f };
		XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
		pos.y += 3;
		cam1st->LookTo(pos, look, up);

		mCameraMode = CameraMode::Free;
	}
	
	// �˳���������Ӧ�򴰿ڷ���������Ϣ
	if (keyState.IsKeyDown(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	
	mBasicFX.UpdateConstantBuffer(mCBFrame);
}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	
	// *********************
	// 1. �����淴������д��ֵ1��ģ�建����
	// 

	mBasicFX.SetWriteStencilOnly(1);
	mMirror.Draw(md3dImmediateContext);

	// ***********************
	// 2. ���Ʋ�͸���ķ�������
	//

	// �����������
	mDrawingState.isReflection = 1;		// ���俪��
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	mBasicFX.SetRenderDefaultWithStencil(1);

	mWalls[2].Draw(md3dImmediateContext);
	mWalls[3].Draw(md3dImmediateContext);
	mWalls[4].Draw(md3dImmediateContext);
	mFloor.Draw(md3dImmediateContext);
	mWoodCrate.Draw(md3dImmediateContext);

	// ������Ӱ
	mWoodCrate.SetMaterial(mShadowMat);
	mDrawingState.isShadow = 1;			// ���俪������Ӱ����
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	mBasicFX.SetRenderNoDoubleBlend(1);

	mWoodCrate.Draw(md3dImmediateContext);

	// �ָ���ԭ����״̬
	mDrawingState.isShadow = 0;
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	mWoodCrate.SetMaterial(mWoodCrateMat);
	

	// ***********************
	// 3. ����͸������
	//

	mBasicFX.SetRenderAlphaBlendWithStencil(1);

	mMirror.Draw(md3dImmediateContext);
	
	// �رշ������
	mDrawingState.isReflection = 0;
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	

	// ************************
	// 4. ���Ʋ�͸������������
	//
	mBasicFX.SetRenderDefault();

	for (auto& wall : mWalls)
		wall.Draw(md3dImmediateContext);
	mFloor.Draw(md3dImmediateContext);
	mWoodCrate.Draw(md3dImmediateContext);

	// ������Ӱ
	mWoodCrate.SetMaterial(mShadowMat);
	mDrawingState.isShadow = 1;			// ����رգ���Ӱ����
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	mBasicFX.SetRenderNoDoubleBlend(0);

	mWoodCrate.Draw(md3dImmediateContext);

	mDrawingState.isShadow = 0;			// ��Ӱ�ر�
	mBasicFX.UpdateConstantBuffer(mDrawingState);
	mWoodCrate.SetMaterial(mWoodCrateMat);


	//
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	std::wstring text = L"�л������ģʽ: 1-��һ�˳� 2-�����˳� 3-�����ӽ�\n"
		"W/S/A/D ǰ��/����/��ƽ��/��ƽ�� (�����˳���Ч)  Esc�˳�\n"
		"����ƶ�������Ұ ���ֿ��Ƶ����˳ƹ۲����\n"
		"��ǰģʽ: ";
	if (mCameraMode == CameraMode::FirstPerson)
		text += L"��һ�˳�(���������ƶ�)";
	else if (mCameraMode == CameraMode::ThirdPerson)
		text += L"�����˳�";
	else
		text += L"�����ӽ�";
	md2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 500.0f, 60.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));
}



bool GameApp::InitResource()
{
	
	// ******************
	// ��ʼ����Ϸ����
	ComPtr<ID3D11ShaderResourceView> texture;
	Material material;
	material.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	mWoodCrateMat = material;
	mShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	// ��ʼ��ľ��
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	mWoodCrate.SetBuffer(md3dDevice, Geometry::CreateBox());
	// ̧��߶ȱ�����Ȼ�������Դ����
	mWoodCrate.SetWorldMatrix(XMMatrixTranslation(0.0f, 0.01f, 5.0f));
	mWoodCrate.SetTexTransformMatrix(XMMatrixIdentity());
	mWoodCrate.SetTexture(texture);
	mWoodCrate.SetMaterial(material);
	
	

	// ��ʼ���ذ�
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	mFloor.SetBuffer(md3dDevice, 
		Geometry::CreatePlane(XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	mFloor.SetWorldMatrix(XMMatrixIdentity());
	mFloor.SetTexTransformMatrix(XMMatrixIdentity());
	mFloor.SetTexture(texture);
	mFloor.SetMaterial(material);

	// ��ʼ��ǽ��
	mWalls.resize(5);
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// �������ǽ�����������ɣ�0��1���м�λ�����ڷ��þ���
	//     ____     ____
	//    /| 0 |   | 1 |\
	//   /4|___|___|___|2\
	//  /_/_ _ _ _ _ _ _\_\
	// | /       3       \ |
	// |/_________________\|
	//
	for (int i = 0; i < 5; ++i)
	{
		mWalls[i].SetMaterial(material);
		mWalls[i].SetTexTransformMatrix(XMMatrixIdentity());
		mWalls[i].SetTexture(texture);
	}
	mWalls[0].SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	mWalls[1].SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	mWalls[2].SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	mWalls[3].SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	mWalls[4].SetBuffer(md3dDevice, Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	
	mWalls[0].SetWorldMatrix(XMMatrixRotationX(-XM_PIDIV2) * XMMatrixTranslation(-7.0f, 3.0f, 10.0f));
	mWalls[1].SetWorldMatrix(XMMatrixRotationX(-XM_PIDIV2) * XMMatrixTranslation(7.0f, 3.0f, 10.0f));
	mWalls[2].SetWorldMatrix(XMMatrixRotationY(-XM_PIDIV2) * XMMatrixRotationZ(XM_PIDIV2) * XMMatrixTranslation(10.0f, 3.0f, 0.0f));
	mWalls[3].SetWorldMatrix(XMMatrixRotationX(XM_PIDIV2) * XMMatrixTranslation(0.0f, 3.0f, -10.0f));
	mWalls[4].SetWorldMatrix(XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixTranslation(-10.0f, 3.0f, 0.0f));

	// ��ʼ������
	material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Texture\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	mMirror.SetBuffer(md3dDevice,
		Geometry::CreatePlane(XMFLOAT3(), XMFLOAT2(8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f)));
	mMirror.SetWorldMatrix(XMMatrixRotationX(-XM_PIDIV2) * XMMatrixTranslation(0.0f, 3.0f, 10.0f));
	mMirror.SetTexTransformMatrix(XMMatrixIdentity());
	mMirror.SetTexture(texture);
	mMirror.SetMaterial(material);

	// ******************
	// ��ʼ��������������ֵ
	// ��ʼ��ÿ֡���ܻ�仯��ֵ
	mCameraMode = CameraMode::ThirdPerson;
	auto camera = std::shared_ptr<ThirdPersonCamera>(new ThirdPersonCamera);
	mCamera = camera;
	
	camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	camera->SetDistance(5.0f);
	camera->SetDistanceMinMax(2.0f, 14.0f);
	mCBFrame.view = mCamera->GetView();
	XMStoreFloat4(&mCBFrame.eyePos, mCamera->GetPositionXM());

	// ��ʼ�����ڴ��ڴ�С�䶯ʱ�޸ĵ�ֵ
	mCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	mCBOnReSize.proj = mCamera->GetProj();

	// ��ʼ������仯��ֵ
	mCBNeverChange.reflection = XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f));
	// ��΢��һ��λ������ʾ��Ӱ
	mCBNeverChange.shadow = XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f));
	mCBNeverChange.refShadow = XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f));
	// ������
	mCBNeverChange.dirLight[0].Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBNeverChange.dirLight[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCBNeverChange.dirLight[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBNeverChange.dirLight[0].Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// �ƹ�
	mCBNeverChange.pointLight[0].Position = XMFLOAT3(0.0f, 10.0f, -10.0f);
	mCBNeverChange.pointLight[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mCBNeverChange.pointLight[0].Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mCBNeverChange.pointLight[0].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCBNeverChange.pointLight[0].Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mCBNeverChange.pointLight[0].Range = 25.0f;
	mCBNeverChange.numDirLight = 1;
	mCBNeverChange.numPointLight = 1;
	mCBNeverChange.numSpotLight = 0;
	


	// ���²����ױ��޸ĵĳ�����������Դ
	mBasicFX.UpdateConstantBuffer(mCBOnReSize);
	mBasicFX.UpdateConstantBuffer(mCBNeverChange);


	return true;
}



