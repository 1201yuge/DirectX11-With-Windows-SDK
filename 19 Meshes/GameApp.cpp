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
		mCBChangesOnReSize.proj = mCamera->GetProjXM();
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
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(mCamera);
	
	// �����˳�������Ĳ���

	// ��������ת
	cam3rd->RotateX(mouseState.y * dt * 1.25f);
	cam3rd->RotateY(mouseState.x * dt * 1.25f);
	cam3rd->Approach(-mouseState.scrollWheelValue / 120 * 1.0f);
	
	// ���¹۲����
	mCamera->UpdateViewMatrix();
	XMStoreFloat4(&mCBFrame.eyePos, mCamera->GetPositionXM());
	mCBFrame.view = mCamera->GetViewXM();

	// ���ù���ֵ
	mMouse->ResetScrollWheelValue();

	// �˳���������Ӧ�򴰿ڷ���������Ϣ
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	
	// ����ÿ֡�仯��ֵ
	mBasicFX.UpdateConstantBuffer(mCBFrame);
}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mBasicFX.SetRenderObjectDefault();
	
	mGround.Draw(md3dImmediateContext);
	mHouse.Draw(md3dImmediateContext);
	

	//
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	std::wstring text = L"��ǰ�����ģʽ: �����˳�  Esc�˳�\n"
		"����ƶ�������Ұ ���ֿ��Ƶ����˳ƹ۲����\n";
	md2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));
}



bool GameApp::InitResource()
{
	


	// ******************
	// ��ʼ����Ϸ����
	// ��ʼ���ذ�
	mObjReader.Read(L"Model\\ground.mbo", L"Model\\ground.obj");
	mGround.SetModel(md3dDevice, mObjReader);

	// ��ʼ������ģ��
	mObjReader.Read(L"Model\\house.mbo", L"Model\\house.obj");
	mHouse.SetModel(md3dDevice, mObjReader);
	BoundingBox mHouseBox;
	// ��ȡ���ݰ�Χ��
	mHouse.GetBoundingBox(mHouseBox, XMMatrixScaling(0.015f, 0.015f, 0.015f));
	// �÷��ݵײ���������
	mHouse.SetWorldMatrix(XMMatrixScaling(0.015f, 0.015f, 0.015f) *
		XMMatrixTranslation(0.0f, -(mHouseBox.Center.y - mHouseBox.Extents.y + 1.0f), 0.0f));
	// ******************
	// ��ʼ��������������ֵ
	// ��ʼ��ÿ֡���ܻ�仯��ֵ
	mCameraMode = CameraMode::ThirdPerson;
	auto camera = std::shared_ptr<ThirdPersonCamera>(new ThirdPersonCamera);
	mCamera = camera;
	
	camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
	camera->SetDistance(10.0f);
	camera->SetDistanceMinMax(6.0f, 100.0f);
	mCBFrame.view = mCamera->GetViewXM();
	XMStoreFloat4(&mCBFrame.eyePos, mCamera->GetPositionXM());

	// ��ʼ�����ڴ��ڴ�С�䶯ʱ�޸ĵ�ֵ
	mCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	mCBChangesOnReSize.proj = mCamera->GetProjXM();

	// ��ʼ������仯��ֵ
	// ������
	mCBRarely.dirLight.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBRarely.dirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCBRarely.dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBRarely.dirLight.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// �ƹ�
	mCBRarely.pointLight.Position = XMFLOAT3(0.0f, 20.0f, 0.0f);
	mCBRarely.pointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mCBRarely.pointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mCBRarely.pointLight.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCBRarely.pointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mCBRarely.pointLight.Range = 30.0f;	

	// ���²����ױ��޸ĵĳ�����������Դ
	mBasicFX.UpdateConstantBuffer(mCBChangesOnReSize);
	mBasicFX.UpdateConstantBuffer(mCBRarely);

	return true;
}


