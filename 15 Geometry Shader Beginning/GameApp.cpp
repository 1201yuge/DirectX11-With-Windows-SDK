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
	
	// ֻ�г�������������ʼ�����ִ�и��²���
	if (mBasicFX.IsInit())
	{
		mCBOnReSize.proj = XMMatrixPerspectiveFovLH(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
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

	// ����ÿ֡�仯��ֵ
	if (mShowMode == Mode::SplitedTriangle)
	{
		mCBChangeEveryFrame.worldInvTranspose = mCBChangeEveryFrame.world = XMMatrixIdentity();
	}
	else
	{
		static float phi = 0.0f, theta = 0.0f;
		phi += 0.2f * dt, theta += 0.3f * dt;
		mCBChangeEveryFrame.world = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
		mCBChangeEveryFrame.worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, mCBChangeEveryFrame.world));
	}
	mBasicFX.UpdateConstantBuffer(mCBChangeEveryFrame);


	// �л���ʾģʽ
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		mShowMode = Mode::SplitedTriangle;
		ResetTriangle();
		// ����װ��׶εĶ��㻺��������
		UINT stride = sizeof(VertexPosColor);		// ��Խ�ֽ���
		UINT offset = 0;							// ��ʼƫ����
		md3dImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		mBasicFX.SetRenderSplitedTriangle();
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		mShowMode = Mode::CylinderNoCap;
		ResetRoundWire();
		// ����װ��׶εĶ��㻺��������
		UINT stride = sizeof(VertexPosNormalColor);		// ��Խ�ֽ���
		UINT offset = 0;								// ��ʼƫ����
		md3dImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		mBasicFX.SetRenderCylinderNoCap();
	}

	// ��ʾ������
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		if (mShowMode == Mode::CylinderNoCap)
			mShowMode = Mode::CylinderNoCapWithNormal;
		else if (mShowMode == Mode::CylinderNoCapWithNormal)
			mShowMode = Mode::CylinderNoCap;
	}

}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->Draw(mVertexCount, 0);
	// ���Ʒ��������������ǵù�λ
	if (mShowMode == Mode::CylinderNoCapWithNormal)
	{
		mBasicFX.SetRenderNormal();
		md3dImmediateContext->Draw(mVertexCount, 0);
		mBasicFX.SetRenderCylinderNoCap();
	}


	//
	// ����Direct2D����
	//
	md2dRenderTarget->BeginDraw();
	std::wstring text = L"�л����ͣ�1-���ѵ������� 2-Բ�߹�������\n"
		"��ǰģʽ: ";
	if (mShowMode == Mode::SplitedTriangle)
		text += L"���ѵ�������";
	else if (mShowMode == Mode::CylinderNoCap)
		text += L"Բ�߹�������(Q-��ʾԲ�ߵķ�����)";
	else
		text += L"Բ�߹�������(Q-����Բ�ߵķ�����)";
	md2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), mTextFormat.Get(),
		D2D1_RECT_F{ 0.0f, 0.0f, 500.0f, 60.0f }, mColorBrush.Get());
	HR(md2dRenderTarget->EndDraw());

	HR(mSwapChain->Present(0, 0));
}



bool GameApp::InitResource()
{
	
	// Ĭ�ϻ���������
	mShowMode = Mode::SplitedTriangle;
	ResetTriangle();
	
	// ******************
	// ��ʼ��������������ֵ
	// �����
	mCBNeverChange.dirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCBNeverChange.dirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCBNeverChange.dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBNeverChange.dirLight.Direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	// ����
	mCBNeverChange.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCBNeverChange.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCBNeverChange.material.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	// �����λ��
	mCBNeverChange.eyePos = XMFLOAT3(0.0f, 0.0f, -5.0f);
	// ����
	mCBChangeEveryFrame.world = XMMatrixIdentity();
	mCBChangeEveryFrame.worldInvTranspose = XMMatrixIdentity();
	mCBNeverChange.view = XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f), 
		XMVectorZero(), 
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	mCBNeverChange.cylinderHeight = 2.0f;
	mCBOnReSize.proj = XMMatrixPerspectiveFovLH(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

	// ���³�����������Դ
	mBasicFX.UpdateConstantBuffer(mCBOnReSize);
	mBasicFX.UpdateConstantBuffer(mCBNeverChange);

	// ******************
	// ����Ⱦ���߸����׶ΰ󶨺�������Դ

	// ����װ��׶εĶ��㻺��������
	UINT stride = sizeof(VertexPosColor);		// ��Խ�ֽ���
	UINT offset = 0;							// ��ʼƫ����
	md3dImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
	// ����Ĭ����Ⱦ״̬
	mBasicFX.SetRenderSplitedTriangle();


	return true;
}


void GameApp::ResetTriangle()
{
	// ******************
	// ��ʼ��������
	// ���������ζ���
	VertexPosColor vertices[] =
	{
		{ XMFLOAT3(-1.0f * 3, -0.866f * 3, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f * 3, 0.866f * 3, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f * 3, -0.866f * 3, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
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
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mVertexBuffer.ReleaseAndGetAddressOf()));
	// �����ζ�����
	mVertexCount = 3;
}

void GameApp::ResetRoundWire()
{
	// ******************
	// ��ʼ��Բ��
	// ����Բ���ϸ�����
	// ����Ҫ��˳ʱ������
	// ����Ҫ�γɱջ�����ʼ����Ҫʹ��2��
	//  ______
	// /      \
	// \______/
	//
	VertexPosNormalColor vertices[41];
	for (int i = 0; i < 40; ++i)
	{
		vertices[i].pos = XMFLOAT3(cosf(XM_PI / 20 * i), -1.0f, -sinf(XM_PI / 20 * i));
		vertices[i].normal = XMFLOAT3(cosf(XM_PI / 20 * i), 0.0f, -sinf(XM_PI / 20 * i));
		vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	vertices[40] = vertices[0];

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
	HR(md3dDevice->CreateBuffer(&vbd, &InitData, mVertexBuffer.ReleaseAndGetAddressOf()));
	// �߿򶥵���
	mVertexCount = 41;
}



