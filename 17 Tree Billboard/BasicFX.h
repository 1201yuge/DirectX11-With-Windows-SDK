#ifndef BASICFX_H
#define BASICFX_H

#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>
#include "LightHelper.h"
#include "RenderStates.h"
#include "Vertex.h"

// ���ڳ����������Ĵ�����Ҫ��16�ֽڵı������ú������Է��غ��ʵ��ֽڴ�С
inline UINT Align16Bytes(UINT size)
{
	return (size + 15) & (UINT)(-16);
}

struct CBChangesEveryDrawing
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX worldInvTranspose;
	DirectX::XMMATRIX texTransform;
	Material material;
};

struct CBChangesEveryFrame
{
	DirectX::XMMATRIX view;
	DirectX::XMFLOAT4 eyePos;
};

struct CBDrawingStates
{
	DirectX::XMFLOAT4 fogColor;
	int fogEnabled;
	float fogStart;
	float fogRange;
	float pad;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};

struct CBNeverChange
{
	DirectionalLight dirLight[4];
};

class BasicFX
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// ��ʼ��Basix.fx������Դ����ʼ����դ��״̬
	bool InitAll(ComPtr<ID3D11Device> device);
	// �Ƿ��Ѿ���ʼ��
	bool IsInit() const;

	template <class T>
	void UpdateConstantBuffer(const T& cbuffer);

	// Ĭ��״̬����
	void SetRenderDefault();

	// ��������
	void SetRenderBillboard(bool enableAlphaToCoverage);

private:
	// objFileNameInOutΪ����õ���ɫ���������ļ�(.*so)������ָ��������Ѱ�Ҹ��ļ�����ȡ
	// hlslFileNameΪ��ɫ�����룬��δ�ҵ���ɫ���������ļ��������ɫ������
	// ����ɹ�����ָ����objFileNameInOut���򱣴����õ���ɫ����������Ϣ�����ļ�
	// ppBlobOut�����ɫ����������Ϣ
	HRESULT CreateShaderFromFile(const WCHAR* objFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

private:
	ComPtr<ID3D11VertexShader> mBasicVS;
	ComPtr<ID3D11PixelShader> mBasicPS;

	ComPtr<ID3D11VertexShader> mBillboardVS;
	ComPtr<ID3D11GeometryShader> mBillboardGS;
	ComPtr<ID3D11PixelShader> mBillboardPS;


	ComPtr<ID3D11InputLayout> mVertexPosSizeLayout;			// �㾫�����벼��
	ComPtr<ID3D11InputLayout> mVertexPosNormalTexLayout;	// 3D�������벼��

	ComPtr<ID3D11DeviceContext> md3dImmediateContext;		// �豸������

	std::vector<ComPtr<ID3D11Buffer>> mConstantBuffers;		// ����������
};











#endif