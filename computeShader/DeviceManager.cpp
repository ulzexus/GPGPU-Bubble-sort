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

	// コンピュートシェーダーは SM 4 でも動作するらしいがサンプルでは対応しない
	D3D_FEATURE_LEVEL FeatureLevel[] = {
		D3D_FEATURE_LEVEL_11_0,  // Direct3D 11.0 SM 5
	};
	UINT FeatureLevelLength = _countof( FeatureLevel );

#if defined(DEBUG) || defined(_DEBUG)
	// デバッグコンパイルの場合、デバッグレイヤーを有効にする。
	UINT createDeviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlag = 0;
#endif

	// デバイスを作成する。
	hr = D3D11CreateDevice(
		NULL,                       // 使用するアダプターを設定。NULLの場合はデフォルトのアダプター。
		D3D_DRIVER_TYPE_HARDWARE,   // D3D_DRIVER_TYPEのいずれか。ドライバーの種類。
		NULL,                       // ソフトウェアラスタライザを実装するDLLへのハンドル。
		createDeviceFlag,           // D3D11_CREATE_DEVICE_FLAGの組み合わせ。デバイスを作成時に使用されるパラメータ。
		FeatureLevel,               // D3D_FEATURE_LEVELのポインタ
		FeatureLevelLength,         // D3D_FEATURE_LEVEL配列の要素数。
		D3D11_SDK_VERSION,          // DirectX SDKのバージョン。この値は固定。
		ppD3DDevice,                // 初期化されたデバイス
		NULL,                       // 採用されたフィーチャーレベル。使用しないので NULL
		ppD3DDeviceContext          // 初期化されたデバイスコンテキスト
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

	// ファイルを元にエフェクトをコンパイルする
	hr = D3DX11CompileFromFile( "ComputeShader01.hlsl", NULL, NULL, "CS_Main", "cs_5_0", Flag1, 0, NULL, &pBlob, &errors, NULL );
	if ( FAILED( hr ) ) {
		std::cout << ( char* )errors->GetBufferPointer();
		goto EXIT;
	}

	// コンパイル済みシェーダーから、コンピュートシェーダー オブジェクトを作成する
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

	//構造化バッファを作成する
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	bufferDesc.ByteWidth = uElementSize * uCount;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = uElementSize;
	bufferDesc.BindFlags =
		D3D11_BIND_UNORDERED_ACCESS |                               // アンオーダード アクセス リソースをバインドする
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

	//構造化バッファーからシェーダーリソースビューを作成
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

	// 構造化バッファーを作成する

	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory( &BufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	BufferDesc.BindFlags =
		D3D11_BIND_UNORDERED_ACCESS |                               // アンオーダード アクセス リソースをバインドする
		D3D11_BIND_SHADER_RESOURCE;                                 // バッファーをシェーダー ステージにバインドする
	BufferDesc.ByteWidth = uElementSize * uCount;                  // バッファサイズ
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;  // 構造化バッファーとしてリソースを作成する
	BufferDesc.StructureByteStride = uElementSize;                 // 構造化バッファーのサイズ (バイト単位)

	// 初期値を設定
	if ( pInitData ) {
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		hr = pD3DDevice->CreateBuffer( &BufferDesc, &InitData, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	}
	// 初期値なしで領域のみ確保する
	else {
		hr = pD3DDevice->CreateBuffer( &BufferDesc, NULL, ppBuf );
		if ( FAILED( hr ) ) {
			return E_FAIL;
		}
	}

	// 構造化バッファーからアンオーダードアクセスビューを作成する

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory( &UAVDesc, sizeof( D3D11_UNORDERED_ACCESS_VIEW_DESC ) );
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;   // バッファーであることを指定する
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.Buffer.NumElements = uCount;                  // リソース内の要素の数

	// 構造化バッファーをもとにアンオーダード アクセス ビューを作成する
	hr = pD3DDevice->CreateUnorderedAccessView( *ppBuf, &UAVDesc, ppUAVOut );
	if ( FAILED( hr ) ) {
		return E_FAIL;
	}

	return S_OK;
}

void DeviceManager::RunComputeShader(
ID3D11DeviceContext* pD3DDeviceContext,
ID3D11ComputeShader* pComputeShader,
ID3D11ShaderResourceView* pBufSRV,        // 入力用,
ID3D11UnorderedAccessView* pBufResultUAV,  // 出力用,
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
