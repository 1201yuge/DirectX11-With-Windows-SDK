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
	enum class Mode { SplitedTriangle, CylinderNoCap, CylinderNoCapWithNormal };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

	void ResetTriangle();
	void ResetRoundWire();



private:
	
	ComPtr<ID2D1SolidColorBrush> mColorBrush;				// ��ɫ��ˢ
	ComPtr<IDWriteFont> mFont;								// ����
	ComPtr<IDWriteTextFormat> mTextFormat;					// �ı���ʽ

	ComPtr<ID3D11Buffer> mVertexBuffer;						// ���㼯��
	int mVertexCount;										// ������Ŀ
	Mode mShowMode;											// ��ǰ��ʾģʽ

	BasicFX mBasicFX;										// Basic��Ч������

	CBChangesEveryFrame mCBChangesEveryFrame;				// �û��������ÿ֡���µı���
	CBChangesOnResize mCBChangesOnReSize;					// �û�������Ž��ڴ��ڴ�С�仯ʱ���µı���
	CBNeverChange mCBNeverChange;							// �û�������Ų����ٽ����޸ĵı���
};


#endif