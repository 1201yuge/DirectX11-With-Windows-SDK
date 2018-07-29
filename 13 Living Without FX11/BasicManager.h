#ifndef BASICMANAGER_H
#define BASICMANAGER_H

#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>
#include "LightHelper.h"
#include "RenderStates.h"
#include "Vertex.h"

// ���ڳ����������Ĵ�����Ҫ��16�ֽڵı������ú������Է��غ��ʵ��ֽڴ�С
inline size_t Align16Bytes(size_t size)
{
	return (size + 15) & (size_t)(-16);
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

struct CBDrawingState
{
	int isReflection;
	int isShadow;
	DirectX::XMINT2 pad;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};


struct CBNeverChange
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

class BasicManager
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

	// Ĭ��״̬������
	void SetRenderDefault();
	// Alpha��ϻ���
	void SetRenderAlphaBlend();		
	// �޶��λ��
	void SetRenderNoDoubleBlend(UINT stencilRef);
	// ��д��ģ��ֵ
	void SetWriteStencilOnly(UINT stencilRef);		
	// ��ָ��ģ��ֵ��������л��ƣ�����Ĭ��״̬
	void SetRenderDefaultWithStencil(UINT stencilRef);		
	// ��ָ��ģ��ֵ��������л��ƣ�����Alpha���
	void SetRenderAlphaBlendWithStencil(UINT stencilRef);		
	// 2DĬ��״̬����
	void Set2DRenderDefault();
	// 2D��ϻ���
	void Set2DRenderAlphaBlend();


private:
	// ��.fx/.hlsl�ļ��б�����ɫ��
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

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