#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <DirectXColors.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "d3dApp.h"
#include "BasicFX.h"
class GameApp : public D3DApp
{
public:
	enum class Mode { SplitedTriangle, SplitedSnow, SplitedSphere };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

	void ResetSplitedTriangle();
	void ResetSplitedSnow();
	void ResetSplitedSphere();


private:
	
	ComPtr<ID2D1SolidColorBrush> mColorBrush;				// ��ɫ��ˢ
	ComPtr<IDWriteFont> mFont;								// ����
	ComPtr<IDWriteTextFormat> mTextFormat;					// �ı���ʽ

	ComPtr<ID3D11Buffer> mVertexBuffers[7];					// ���㻺��������
	int mVertexCounts[7];									// ������Ŀ
	int mCurrIndex;											// ��ǰ����
	Mode mShowMode;											// ��ǰ��ʾģʽ
	bool mIsWireFrame;										// �Ƿ�Ϊ�߿�ģʽ
	bool mShowNormal;										// �Ƿ���ʾ������
	BasicFX mBasicFX;										// Basic��Ч������

	CBChangesEveryFrame mCBChangesEveryFrame;				// �û��������ÿ֡���µı���
	CBChangesOnResize mCBChangesOnReSize;							// �û�������Ž��ڴ��ڴ�С�仯ʱ���µı���
	CBChangesRarely mCBRarely;							// �û�������Ų����ٽ����޸ĵı���
};


#endif