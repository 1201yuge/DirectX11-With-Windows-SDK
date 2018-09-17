#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <DirectXColors.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "d3dApp.h"
#include "Camera.h"
#include "GameObject.h"
class GameApp : public D3DApp
{
public:
	// �����ģʽ
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();
	void InitPointSpritesBuffer();

	// ���ݸ�����DDS�����ļ����ϣ�����2D��������
	// Ҫ����������Ŀ�Ⱥ͸߶ȶ�һ��
	// ��maxMipMapSizeΪ0��ʹ��Ĭ��mipmap�ȼ�
	// ����mipmap�ȼ������ᳬ��maxMipMapSize
	ComPtr<ID3D11ShaderResourceView> CreateDDSTexture2DArrayShaderResourceView(
		ComPtr<ID3D11Device> device,
		ComPtr<ID3D11DeviceContext> deviceContext,
		const std::vector<std::wstring>& filenames,
		int maxMipMapSize = 0);


private:
	
	ComPtr<ID2D1SolidColorBrush> mColorBrush;				// ��ɫ��ˢ
	ComPtr<IDWriteFont> mFont;								// ����
	ComPtr<IDWriteTextFormat> mTextFormat;					// �ı���ʽ

	ComPtr<ID3D11Buffer> mPointSpritesBuffer;				// �㾫�鶥�㻺����
	ComPtr<ID3D11ShaderResourceView> mTreeTexArray;			// ������������
	Material mTreeMat;										// ���Ĳ���

	GameObject mGround;										// ����
	
	BasicFX mBasicFX;										// Basic��Ч������

	CameraMode mCameraMode;									// �����ģʽ
	std::shared_ptr<Camera> mCamera;						// �����

	bool mIsNight;											// �Ƿ��ҹ
	bool mEnableAlphaToCoverage;							// �Ƿ���Alpha-To-Coverage

	CBChangesEveryDrawing mCBChangesEveryDrawing;			// �û��������ÿ�λ��Ƹ��µı���
	CBChangesEveryFrame mCBChangesEveryFrame;				// �û��������ÿ֡���µı���
	CBDrawingStates mCBDrawingStates;						// �û�������Ż���״̬
	CBChangesOnResize mCBChangesOnReSize;					// �û�������Ž��ڴ��ڴ�С�仯ʱ���µı���
	CBChangesRarely mCBRarely;							// �û�������Ų����ٽ����޸ĵı���
};


#endif