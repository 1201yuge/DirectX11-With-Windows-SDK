#ifndef D3DAPP_H
#define D3DAPP_H

#include <wrl/client.h>
#include <assert.h>
#include <d3d11_1.h>
#include <string>
#include "GameTimer.h"
#include "dxerr.h"

// ��ֲ�����Ĵ����飬����Ŀ������ʹ��Unicode�ַ���
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
{                                                          \
	HRESULT hr = (x);                                      \
	if(FAILED(hr))                                         \
	{                                                      \
		DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
	}                                                      \
}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);    // �ڹ��캯���ĳ�ʼ���б�Ӧ�����úó�ʼ����
	virtual ~D3DApp();

	HINSTANCE AppInst()const;       // ��ȡӦ��ʵ���ľ��
	HWND      MainWnd()const;       // ��ȡ�����ھ��
	float     AspectRatio()const;   // ��ȡ��Ļ��߱�

	int Run();                      // ���г���ִ����Ϣ�¼���ѭ��

	// ��ܷ������ͻ���������Ҫ������Щ������ʵ���ض���Ӧ������
	virtual bool Init();            // �ø��෽����Ҫ��ʼ�����ں�Direct3D����
	virtual void OnResize();        // �ø��෽����Ҫ�ڴ��ڴ�С�䶯��ʱ�����
	virtual void UpdateScene(float dt) = 0;   // ������Ҫʵ�ָ÷��������ÿһ֡�ĸ���
	virtual void DrawScene() = 0;             // ������Ҫʵ�ָ÷��������ÿһ֡�Ļ���
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// ���ڵ���Ϣ�ص�����
protected:
	bool InitMainWindow();      // ���ڳ�ʼ��
	bool InitDirect3D();        // Direct3D��ʼ��

	void CalculateFrameStats(); // ����ÿ��֡�����ڴ�����ʾ

protected:

	HINSTANCE mhAppInst;        // Ӧ��ʵ�����
	HWND      mhMainWnd;        // �����ھ��
	bool      mAppPaused;       // Ӧ���Ƿ���ͣ
	bool      mMinimized;       // Ӧ���Ƿ���С��
	bool      mMaximized;       // Ӧ���Ƿ����
	bool      mResizing;        // ���ڴ�С�Ƿ�仯
	UINT      m4xMsaaQuality;   // MSAA֧�ֵ������ȼ�

	GameTimer mTimer;           // ��ʱ��

	// DX11
	Microsoft::WRL::ComPtr<ID3D11Device> md3dDevice;                    // D3D11�豸
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> md3dImmediateContext;   // D3D11�豸������
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;                  // D3D11������
	// DX11.1
	Microsoft::WRL::ComPtr<ID3D11Device1> md3dDevice1;                  // D3D11.1�豸
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> md3dImmediateContext1; // D3D11.1�豸������
	Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain1;                // D3D11.1������
	// ������Դ
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBuffer;        // ���ģ�建����
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;   // ��ȾĿ����ͼ
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;   // ���ģ����ͼ
	D3D11_VIEWPORT mScreenViewport;                                     // �ӿ�

	// ������Ӧ���ڹ��캯�����ú���Щ�Զ���ĳ�ʼ����
	std::wstring mMainWndCaption;                                       // �����ڱ���
	int mClientWidth;                                                   // �ӿڿ��
	int mClientHeight;                                                  // �ӿڸ߶�
};

#endif // D3DAPP_H