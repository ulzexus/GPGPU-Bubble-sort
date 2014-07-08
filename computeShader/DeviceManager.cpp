#include "DeviceManager.h"
#include <tchar.h>
#include "def.h"
#include "Vertex.h"
#include <iostream>
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dx11.lib" )

HRESULT DeviceManager::CreateComputeDevice(
	ID3D11Device** ppD3DDevice,
	ID3D11DeviceContext** ppD3DDeviceContext,
	ID3D11ComputeShader** ppComputeShader ) {
	HRESULT hr = E_FAIL;

	// �R���s���[�g�V�F�[�_�[�� SM 4 �ł����삷��炵�����T���v���ł͑Ή����Ȃ�
	D3D_FEATURE_LEVEL FeatureLevel[] = {
		D3D_FEATURE_LEVEL_11_0,  // Direct3D 11.0 SM 5
	};
	UINT FeatureLevelLength = _countof( FeatureLevel );

#if defined(DEBUG) || defined(_DEBUG)
	// �f�o�b�O�R���p�C���̏ꍇ�A�f�o�b�O���C���[��L���ɂ���B
	UINT createDeviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlag = 0;
#endif

	// �f�o�C�X���쐬����B
	hr = D3D11CreateDevice(
		NULL,                       // �g�p����A�_�v�^�[��ݒ�BNULL�̏ꍇ�̓f�t�H���g�̃A�_�v�^�[�B
		D3D_DRIVER_TYPE_HARDWARE,   // D3D_DRIVER_TYPE�̂����ꂩ�B�h���C�o�[�̎�ށB
		NULL,                       // �\�t�g�E�F�A���X�^���C�U����������DLL�ւ̃n���h���B
		createDeviceFlag,           // D3D11_CREATE_DEVICE_FLAG�̑g�ݍ��킹�B�f�o�C�X���쐬���Ɏg�p�����p�����[�^�B
		FeatureLevel,               // D3D_FEATURE_LEVEL�̃|�C���^
		FeatureLevelLength,         // D3D_FEATURE_LEVEL�z��̗v�f���B
		D3D11_SDK_VERSION,          // DirectX SDK�̃o�[�W�����B���̒l�͌Œ�B
		ppD3DDevice,                // ���������ꂽ�f�o�C�X
		NULL,                       // �̗p���ꂽ�t�B�[�`���[���x���B�g�p���Ȃ��̂� NULL
		ppD3DDeviceContext          // ���������ꂽ�f�o�C�X�R���e�L�X�g
		);
	if ( FAILED( hr ) )
		goto EXIT;

	ID3D10Blob* pBlob = NULL;
	ID3DBlob* errors = NULL;

	UINT Flag1 = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	// �t�@�C�������ɃG�t�F�N�g���R���p�C������
	hr = D3DX11CompileFromFile( "ComputeShader01.hlsl", NULL, NULL, "CS_Main", "cs_5_0", Flag1, 0, NULL, &pBlob, &errors, NULL );
	if ( FAILED( hr ) ) {
		std::cout << ( char* )errors->GetBufferPointer();
		goto EXIT;
	}

	// �R���p�C���ς݃V�F�[�_�[����A�R���s���[�g�V�F�[�_�[ �I�u�W�F�N�g���쐬����
	hr = ( *ppD3DDevice )->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppComputeShader );
	if ( FAILED( hr ) )
		goto EXIT;

	hr = S_OK;

EXIT:
	SAFE_RELEASE( pBlob );
	return hr;
}

HRESULT DeviceManager::CreateSRVForStructuredBuffer(
ID3D11Device* pD3DDevice,
UINT uElementSize,
UINT uCount,
VOID* pInitData,
ID3D11Buffer** ppBuf,
ID3D11ShaderResourceView** ppSRVOut ) {
	HRESULT hr = E_FAIL;

	*ppBuf = nullptr;
	*ppSRVOut = nullptr;

	//�\�����o�b�t�@���쐬����
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	bufferDesc.ByteWidth = uElementSize * uCount;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = uElementSize;
	bufferDesc.BindFlags =
		D3D11_BIND_UNORDERED_ACCESS |                               // �A���I�[�_�[�h �A�N�Z�X ���\�[�X���o�C���h����
		D3D11_BIND_SHADER_RESOURCE;

	if ( pInitData ) {
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		hr = pD3DDevice->CreateBuffer( &bufferDesc, &InitData, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	} else {
		hr = pD3DDevice->CreateBuffer( &bufferDesc, NULL, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	}

	//�\�����o�b�t�@�[����V�F�[�_�[���\�[�X�r���[���쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory( &srv, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srv.BufferEx.FirstElement = 0;
	srv.Format = DXGI_FORMAT_UNKNOWN;
	srv.BufferEx.NumElements = uCount;

	hr = pD3DDevice->CreateShaderResourceView( *ppBuf, &srv, ppSRVOut );
	if ( FAILED( hr ) ) {
		return E_FAIL;
	}

	return S_OK;

}

HRESULT DeviceManager::CreateUAVForStructuredBuffer(
ID3D11Device* pD3DDevice,
UINT uElementSize,
UINT uCount,
VOID* pInitData,
ID3D11Buffer** ppBuf,
ID3D11UnorderedAccessView** ppUAVOut ){
	HRESULT hr = E_FAIL;

	*ppBuf = NULL;
	*ppUAVOut = NULL;

	// �\�����o�b�t�@�[���쐬����

	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory( &BufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	BufferDesc.BindFlags =
		D3D11_BIND_UNORDERED_ACCESS |                               // �A���I�[�_�[�h �A�N�Z�X ���\�[�X���o�C���h����
		D3D11_BIND_SHADER_RESOURCE;                                 // �o�b�t�@�[���V�F�[�_�[ �X�e�[�W�Ƀo�C���h����
	BufferDesc.ByteWidth = uElementSize * uCount;                  // �o�b�t�@�T�C�Y
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;  // �\�����o�b�t�@�[�Ƃ��ă��\�[�X���쐬����
	BufferDesc.StructureByteStride = uElementSize;                 // �\�����o�b�t�@�[�̃T�C�Y (�o�C�g�P��)

	// �����l��ݒ�
	if ( pInitData ) {
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		hr = pD3DDevice->CreateBuffer( &BufferDesc, &InitData, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	}
	// �����l�Ȃ��ŗ̈�̂݊m�ۂ���
	else {
		hr = pD3DDevice->CreateBuffer( &BufferDesc, NULL, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	}

	// �\�����o�b�t�@�[����A���I�[�_�[�h�A�N�Z�X�r���[���쐬����

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory( &UAVDesc, sizeof( D3D11_UNORDERED_ACCESS_VIEW_DESC ) );
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;   // �o�b�t�@�[�ł��邱�Ƃ��w�肷��
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.Buffer.NumElements = uCount;                  // ���\�[�X���̗v�f�̐�

	// �\�����o�b�t�@�[�����ƂɃA���I�[�_�[�h �A�N�Z�X �r���[���쐬����
	hr = pD3DDevice->CreateUnorderedAccessView( *ppBuf, &UAVDesc, ppUAVOut );
	if ( FAILED( hr ) ) {
		return E_FAIL;
	}

	return S_OK;
}

void DeviceManager::RunComputeShader(
ID3D11DeviceContext* pD3DDeviceContext,
ID3D11ComputeShader* pComputeShader,
ID3D11ShaderResourceView* pBufSRV,        // ���͗p,
ID3D11UnorderedAccessView* pBufResultUAV,  // �o�͗p,
UINT X, UINT Y, UINT Z ) {
	pD3DDeviceContext->CSSetShader( pComputeShader, NULL, 0 );

	pD3DDeviceContext->CSSetShaderResources( 0, 1, &pBufSRV );

	pD3DDeviceContext->CSSetUnorderedAccessViews( 0, 1, &pBufResultUAV, NULL );

	pD3DDeviceContext->Dispatch( X, Y, Z );	
}

ID3D11Buffer* DeviceManager::CreateAndCopyToDebugBuf(
ID3D11Device* pD3DDevice,
ID3D11DeviceContext* pD3DDeviceContext,
ID3D11Buffer* pBuffer ) {
	ID3D11Buffer* buf = NULL;

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	pBuffer->GetDesc( &bufferDesc );
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.MiscFlags = 0;
	if ( FAILED( pD3DDevice->CreateBuffer( &bufferDesc, NULL, &buf ) ) ) {
		return nullptr;
	}

	pD3DDeviceContext->CopyResource( buf, pBuffer );

	return buf;
}
