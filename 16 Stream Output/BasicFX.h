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

struct CBChangesEveryFrame
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX worldInvTranspose;
};

struct CBChangesOnResize
{
	DirectX::XMMATRIX proj;
};

struct CBNeverChange
{
	DirectionalLight dirLight;
	Material material;
	DirectX::XMMATRIX view;
	DirectX::XMFLOAT3 sphereCenter;
	float sphereRadius;
	DirectX::XMFLOAT3 eyePos;
	float pad;
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

	// ���������η���
	void SetRenderSplitedTriangle();
	// ����ѩ��
	void SetRenderSplitedSnow();
	// ��������
	void SetRenderSplitedSphere();
	// ͨ��������׶λ�ȡ�����η��ѵ���һ�׷���
	void SetStreamOutputSplitedTriangle(ComPtr<ID3D11Buffer> vertexBufferIn, ComPtr<ID3D11Buffer> vertexBufferOut);
	// ͨ��������׶λ�ȡѩ������һ�׷���
	void SetStreamOutputSplitedSnow(ComPtr<ID3D11Buffer> vertexBufferIn, ComPtr<ID3D11Buffer> vertexBufferOut);
	// ͨ��������׶λ�ȡ�����һ�׷���
	void SetStreamOutputSplitedSphere(ComPtr<ID3D11Buffer> vertexBufferIn, ComPtr<ID3D11Buffer> vertexBufferOut);

	// �������ж���ķ�����
	void SetRenderNormal();

private:
	// objFileNameInOutΪ����õ���ɫ���������ļ�(.*so)������ָ��������Ѱ�Ҹ��ļ�����ȡ
	// hlslFileNameΪ��ɫ�����룬��δ�ҵ���ɫ���������ļ��������ɫ������
	// ����ɹ�����ָ����objFileNameInOut���򱣴����õ���ɫ����������Ϣ�����ļ�
	// ppBlobOut�����ɫ����������Ϣ
	HRESULT CreateShaderFromFile(const WCHAR* objFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

private:
	ComPtr<ID3D11VertexShader> mTriangleSOVS;
	ComPtr<ID3D11GeometryShader> mTriangleSOGS;

	ComPtr<ID3D11VertexShader> mTriangleVS;
	ComPtr<ID3D11PixelShader> mTrianglePS;

	ComPtr<ID3D11VertexShader> mSphereSOVS;
	ComPtr<ID3D11GeometryShader> mSphereSOGS;

	ComPtr<ID3D11VertexShader> mSphereVS;
	ComPtr<ID3D11PixelShader> mSpherePS;
	
	ComPtr<ID3D11VertexShader> mSnowSOVS;
	ComPtr<ID3D11GeometryShader> mSnowSOGS;

	ComPtr<ID3D11VertexShader> mSnowVS;
	ComPtr<ID3D11PixelShader> mSnowPS;

	ComPtr<ID3D11VertexShader> mNormalVS;
	ComPtr<ID3D11GeometryShader> mNormalGS;
	ComPtr<ID3D11PixelShader> mNormalPS;

	ComPtr<ID3D11InputLayout> mVertexPosColorLayout;		// VertexPosColor���벼��
	ComPtr<ID3D11InputLayout> mVertexPosNormalColorLayout;	// VertexPosNormalColor���벼��

	ComPtr<ID3D11DeviceContext> md3dImmediateContext;		// �豸������

	std::vector<ComPtr<ID3D11Buffer>> mConstantBuffers;		// ����������
};











#endif