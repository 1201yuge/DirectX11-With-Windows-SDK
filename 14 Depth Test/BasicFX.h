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
	int isReflection;
	int isShadow;
	DirectX::XMINT2 pad;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};


struct CBChangesRarely
{
	DirectX::XMMATRIX reflection;
	DirectX::XMMATRIX shadow;
	DirectX::XMMATRIX refShadow;
	DirectionalLight dirLight[10];
	PointLight pointLight[10];
	SpotLight spotLight[10];
	int numDirLight;
	int numPointLight;
	int numSpotLight;
	float pad;		// �����֤16�ֽڶ���
};

class BasicFX
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// ��ʼ��Basix.fx������Դ����ʼ����Ⱦ״̬
	bool InitAll(ComPtr<ID3D11Device> device);
	// �Ƿ��Ѿ���ʼ��
	bool IsInit() const;

	template <class T>
	void UpdateConstantBuffer(const T& cbuffer);

	// Ĭ��״̬������
	void SetRenderDefault();
	// Alpha��ϻ���
	void SetRenderAlphaBlend();
	// �������綯������Ҫ����Ч���ر���Ȳ���
	void SetDrawBoltAnimNoDepthTest();
	// �������綯������Ҫ����Ч���ر����д��
	void SetDrawBoltAnimNoDepthWrite();

	// �޶��λ��
	void SetRenderNoDoubleBlend(UINT stencilRef);
	// ��д��ģ��ֵ
	void SetWriteStencilOnly(UINT stencilRef);
	// ��ָ��ģ��ֵ��������л��ƣ�����Ĭ��״̬
	void SetRenderDefaultWithStencil(UINT stencilRef);
	// ��ָ��ģ��ֵ��������л��ƣ�����Alpha���
	void SetRenderAlphaBlendWithStencil(UINT stencilRef);
	// �������綯������Ҫ����Ч���ر���Ȳ��ԣ���ָ��ģ��ֵ������л���
	void SetDrawBoltAnimNoDepthTestWithStencil(UINT stencilRef);
	// �������綯������Ҫ����Ч���ر����д�룬��ָ��ģ��ֵ������л���
	void SetDrawBoltAnimNoDepthWriteWithStencil(UINT stencilRef);


	// 2DĬ��״̬����
	void Set2DRenderDefault();
	// 2D��ϻ���
	void Set2DRenderAlphaBlend();


private:
	// objFileNameInOutΪ����õ���ɫ���������ļ�(.*so)������ָ��������Ѱ�Ҹ��ļ�����ȡ
	// hlslFileNameΪ��ɫ�����룬��δ�ҵ���ɫ���������ļ��������ɫ������
	// ����ɹ�����ָ����objFileNameInOut���򱣴����õ���ɫ����������Ϣ�����ļ�
	// ppBlobOut�����ɫ����������Ϣ
	HRESULT CreateShaderFromFile(const WCHAR* objFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

private:
	ComPtr<ID3D11VertexShader> mVertexShader3D;				// ����3D�Ķ�����ɫ��
	ComPtr<ID3D11PixelShader> mPixelShader3D;				// ����3D��������ɫ��
	ComPtr<ID3D11VertexShader> mVertexShader2D;				// ����2D�Ķ�����ɫ��
	ComPtr<ID3D11PixelShader> mPixelShader2D;				// ����2D��������ɫ��

	ComPtr<ID3D11InputLayout> mVertexLayout2D;				// ����2D�Ķ������벼��
	ComPtr<ID3D11InputLayout> mVertexLayout3D;				// ����3D�Ķ������벼��

	ComPtr<ID3D11DeviceContext> md3dImmediateContext;		// �豸������

	std::vector<ComPtr<ID3D11Buffer>> mConstantBuffers;		// ����������
};











#endif