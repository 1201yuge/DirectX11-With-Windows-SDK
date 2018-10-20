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

	if (!mBasicEffect.InitAll(md3dDevice))
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
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
		mTextFormat.GetAddressOf()));

	// ����������ʾ
	if (mCamera != nullptr)
	{
		mCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		mCamera->SetViewPort(0.0f, 0.0f, (float)mClientWidth, (float)mClientHeight);
		mBasicEffect.SetProjMatrix(mCamera->GetProjXM());
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

	// ******************
	// ��¼����������λ�ú���ת����
	//
	static float theta = 0.0f, phi = 0.0f;
	static XMMATRIX Left = XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
	static XMMATRIX Top = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	static XMMATRIX Right = XMMatrixTranslation(5.0f, 0.0f, 0.0f);
	static XMMATRIX Bottom = XMMatrixTranslation(0.0f, -4.0f, 0.0f);

	theta += dt * 0.5f;
	phi += dt * 0.3f;
	// ���������˶�
	mSphere.SetWorldMatrix(Left);
	mCube.SetWorldMatrix(XMMatrixRotationX(-phi) * XMMatrixRotationY(theta) * Top);
	mCylinder.SetWorldMatrix(XMMatrixRotationX(phi) * XMMatrixRotationY(theta) * Right);
	mHouse.SetWorldMatrix(XMMatrixScaling(0.005f, 0.005f, 0.005f) * XMMatrixRotationY(theta) * Bottom);
	mTriangle.SetWorldMatrix(XMMatrixRotationY(theta));

	// ******************
	// ʰȡ���
	//
	mPickedObjStr = L"��";
	Ray ray = Ray::ScreenToRay(*mCamera, (float)mouseState.x, (float)mouseState.y);
	
	// �����ζ���任
	static XMVECTOR V[3];
	for (int i = 0; i < 3; ++i)
	{
		V[i] = XMVector3TransformCoord(XMLoadFloat3(&mTriangleMesh.vertexVec[i].pos), 
			XMMatrixRotationY(theta));
	}

	if (ray.Hit(mBoundingSphere))
	{
		mPickedObjStr = L"����";
	}
	else if (ray.Hit(mCube.GetBoundingOrientedBox()))
	{
		mPickedObjStr = L"������";
	}
	else if (ray.Hit(mCylinder.GetBoundingOrientedBox()))
	{
		mPickedObjStr = L"Բ����";
	}
	else if (ray.Hit(mHouse.GetBoundingOrientedBox()))
	{
		mPickedObjStr = L"����";
	}
	else if (ray.Hit(V[0], V[1], V[2]))
	{
		mPickedObjStr = L"������";
	}

	// ���ù���ֵ
	mMouse->ResetScrollWheelValue();
}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	// ******************
	// ����Direct3D����
	//
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ���Ʋ���Ҫ�����ģ��
	mBasicEffect.SetTextureUsed(false);
	mSphere.Draw(md3dImmediateContext, mBasicEffect);
	mCube.Draw(md3dImmediateContext, mBasicEffect);
	mCylinder.Draw(md3dImmediateContext, mBasicEffect);
	mTriangle.Draw(md3dImmediateContext, mBasicEffect);

	// ������Ҫ�����ģ��
	mBasicEffect.SetTextureUsed(true);
	mHouse.Draw(md3dImmediateContext, mBasicEffect);

	// ******************
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	std::wstring text = L"��ǰʰȡ����: " + mPickedObjStr;

	md2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));
}



bool GameApp::InitResource()
{
	// Ĭ�Ͽ�����׶��ü���Ӳ��ʵ����
	mEnableInstancing = true;
	mEnableFrustumCulling = true;

	// ******************
	// ��ʼ����Ϸ����
	//
	
	// ����(Ԥ����ð�Χ��)
	mSphere.SetModel(Model(md3dDevice, Geometry::CreateSphere()));
	mBoundingSphere.Center = XMFLOAT3(-5.0f, 0.0f, 0.0f);
	mBoundingSphere.Radius = 1.0f;
	// ������
	mCube.SetModel(Model(md3dDevice, Geometry::CreateBox()));
	// Բ����
	mCylinder.SetModel(Model(md3dDevice, Geometry::CreateCylinder()));
	// ����
	mObjReader.Read(L"Model\\house.mbo", L"Model\\house.obj");
	mHouse.SetModel(Model(md3dDevice, mObjReader));
	// ������(������)
	mTriangleMesh.vertexVec.assign({
		{XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)},
		{XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
		{XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
		{XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)}
		});
	mTriangleMesh.indexVec.assign({ 0, 1, 2, 3, 4, 5 });
	mTriangle.SetModel(Model(md3dDevice, mTriangleMesh));

	// ******************
	// ��ʼ�������
	//
	mCameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	mCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)mClientWidth, (float)mClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(
		XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	// ��ʼ�������¹۲����ͶӰ����(����������̶�)
	camera->UpdateViewMatrix();
	mBasicEffect.SetViewMatrix(camera->GetViewXM());
	mBasicEffect.SetProjMatrix(camera->GetProjXM());
	

	// ******************
	// ��ʼ������仯��ֵ
	//

	// �����
	DirectionalLight dirLight;
	dirLight.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
	dirLight.Direction = XMFLOAT3(-0.707f, -0.707f, 0.707f);
	mBasicEffect.SetDirLight(0, dirLight);

	// Ĭ��ֻ���������
	mBasicEffect.SetRenderDefault(md3dImmediateContext, BasicEffect::RenderObject);

	return true;
}

