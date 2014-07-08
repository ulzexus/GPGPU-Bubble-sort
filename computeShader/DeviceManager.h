#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include <d3dx11.h>

/*
* @brife デバイスマネージャー
*/
class DeviceManager {
public:
	static DeviceManager* instance() {
		static DeviceManager mIns;
		return &mIns;
	}

public:
	// 初期化
	HRESULT CreateComputeDevice(
		ID3D11Device** ppD3DDevice, 
		ID3D11DeviceContext** ppD3DDeviceContext, 
		ID3D11ComputeShader** ppComputeShader );

	// コンピュートシェーダーへの入力時に使用するシェーダーリソースビューを作成する
	HRESULT CreateSRVForStructuredBuffer(
		ID3D11Device* pD3DDevice,
		UINT uElementSize,
		UINT uCount,
		VOID* pInitData,
		ID3D11Buffer** ppBuf,
		ID3D11ShaderResourceView** ppSRVOut );

	// コンピュートシェーダーからの出力時に使用するアンオーダードアクセスビューを作成する
	HRESULT CreateUAVForStructuredBuffer(
		ID3D11Device* pD3DDevice,
		UINT uElementSize,
		UINT uCount,
		VOID* pInitData,
		ID3D11Buffer** ppBuf,
		ID3D11UnorderedAccessView** ppUAVOut );

	// コンピュートシェーダーを実行する
	void RunComputeShader( 
		ID3D11DeviceContext* pD3DDeviceContext,
		ID3D11ComputeShader* pComputeShader,
		ID3D11ShaderResourceView* pBufSRV,        // 入力用,
		ID3D11UnorderedAccessView* pBufResultUAV,  // 出力用,
		UINT X, UINT Y, UINT Z );

	// アンオーダードアクセスビューのバッファの内容を CPU から読み込み可能なバッファへコピーする
	ID3D11Buffer* CreateAndCopyToDebugBuf( 
		ID3D11Device* pD3DDevice,
		ID3D11DeviceContext* pD3DDeviceContext,
		ID3D11Buffer* pBuffer );
};


#endif