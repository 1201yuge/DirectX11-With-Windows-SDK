#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <d3d11_1.h>			// �Ѱ���Windows.h
#include <DirectXCollision.h>	// �Ѱ���DirectXMath.h
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <filesystem>
#include <vector>
#include <string>
#include "dxerr.h"
#include "DDSTextureLoader.h"	
#include "WICTextureLoader.h"

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

//
// ��ɫ��������غ���
//

// ------------------------------
// CreateShaderFromFile����
// ------------------------------
// [In]objFileNameInOut ����õ���ɫ���������ļ�(.cso)������ָ��������Ѱ�Ҹ��ļ�����ȡ
// [In]hlslFileName     ��ɫ�����룬��δ�ҵ���ɫ���������ļ��������ɫ������
// [In]entryPoint       ��ڵ�(ָ����ʼ�ĺ���)
// [In]shaderModel      ��ɫ��ģ�ͣ���ʽΪ"*s_5_0"��*����Ϊc,d,g,h,p,v֮һ
// [Out]ppBlobOut       �����ɫ����������Ϣ
HRESULT CreateShaderFromFile(const WCHAR* objFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

//
// ����������غ���
//

// ���ݸ�����DDS�����ļ����ϣ�����2D��������
// Ҫ����������Ŀ�Ⱥ͸߶ȶ�һ��
// ��maxMipMapSizeΪ0��ʹ��Ĭ��mipmap�ȼ�
// ����mipmap�ȼ������ᳬ��maxMipMapSize
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateDDSTexture2DArrayFromFile(
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext,
	const std::vector<std::wstring>& filenames,
	UINT maxMipMapSize = 0);





#endif
