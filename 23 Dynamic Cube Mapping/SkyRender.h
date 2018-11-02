#ifndef SKYRENDER_H
#define SKYRENDER_H

#include <vector>
#include <string>
#include "GameObject.h"
#include "Camera.h"

class SkyRender
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


	// ��Ҫ�ṩ��������պ���ͼ ���� �Ѿ������õ���պ�����.dds�ļ�
	SkyRender(ComPtr<ID3D11Device> device, 
		ComPtr<ID3D11DeviceContext> deviceContext, 
		const std::wstring& cubemapFilename, 
		float skySphereRadius,
		bool generateMips = false);


	// ��Ҫ�ṩ��պе�������������ͼ
	SkyRender(ComPtr<ID3D11Device> device, 
		ComPtr<ID3D11DeviceContext> deviceContext, 
		const std::vector<std::wstring>& cubemapFilenames, 
		float skySphereRadius,
		bool generateMips = false);


	ComPtr<ID3D11ShaderResourceView> GetTextureCube();

	virtual void Draw(ComPtr<ID3D11DeviceContext> deviceContext, SkyEffect& skyEffect, const Camera& camera);

protected:
	void InitResource(ComPtr<ID3D11Device> device, float skySphereRadius);

protected:
	ComPtr<ID3D11Buffer> mVertexBuffer;
	ComPtr<ID3D11Buffer> mIndexBuffer;

	UINT mIndexCount;

	ComPtr<ID3D11ShaderResourceView> mTextureCubeSRV;
};

class DynamicSkyRender : public SkyRender
{
public:
	DynamicSkyRender(ComPtr<ID3D11Device> device,
		ComPtr<ID3D11DeviceContext> deviceContext,
		const std::wstring& cubemapFilename,
		float skySphereRadius,
		int dynamicCubeSize,	// �������ⳤ
		bool generateMips = false);

	DynamicSkyRender(ComPtr<ID3D11Device> device,
		ComPtr<ID3D11DeviceContext> deviceContext,
		const std::vector<std::wstring>& cubemapFilenames,
		float skySphereRadius,
		int dynamicCubeSize,	// �������ⳤ
		bool generateMips = false);


	// ���浱ǰ��ȾĿ����ͼ
	void Cache(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect& effect);

	// ָ����պ�ĳһ�濪ʼ���ƣ���Ҫ�ȵ���Cache����
	void BeginCapture(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect& effect, D3D11_TEXTURECUBE_FACE face,
		const DirectX::XMFLOAT3& pos, float nearZ = 1e-3f, float farZ = 1e3f);

	// �ָ���ȾĿ����ͼ������������󶨵�ǰ��̬��պ�
	void Restore(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect& effect, const Camera& camera);

	// ��ȡ��̬��պ�
	// ע�⣺�÷���ֻ����
	ComPtr<ID3D11ShaderResourceView> GetDynamicTextureCube();

	// ��ȡ��ǰ���ڲ������պ�
	const Camera& GetCamera() const;

private:
	void InitResource(ComPtr<ID3D11Device> device, int dynamicCubeSize);

private:
	ComPtr<ID3D11RenderTargetView>		mCacheRTV;		// ��ʱ����ĺ󱸻�����
	ComPtr<ID3D11DepthStencilView>		mCacheDSV;		// ��ʱ��������/ģ�建����
	
	FirstPersonCamera					mCamera;
	ComPtr<ID3D11DepthStencilView>		mDynamicCubeMapDSV;
	ComPtr<ID3D11ShaderResourceView>	mDynamicCubeMapSRV;
	ComPtr<ID3D11RenderTargetView>		mDynamicCubeMapRTVs[6];
	
};

#endif