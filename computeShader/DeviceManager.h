#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include <d3dx11.h>

/*
* @brife �f�o�C�X�}�l�[�W���[
*/
class DeviceManager {
public:
	static DeviceManager* instance() {
		static DeviceManager mIns;
		return &mIns;
	}

public:
	// ������
	HRESULT CreateComputeDevice(
		ID3D11Device** ppD3DDevice, 
		ID3D11DeviceContext** ppD3DDeviceContext, 
		ID3D11ComputeShader** ppComputeShader );

	// �R���s���[�g�V�F�[�_�[�ւ̓��͎��Ɏg�p����V�F�[�_�[���\�[�X�r���[���쐬����
	HRESULT CreateSRVForStructuredBuffer(
		ID3D11Device* pD3DDevice,
		UINT uElementSize,
		UINT uCount,
		VOID* pInitData,
		ID3D11Buffer** ppBuf,
		ID3D11ShaderResourceView** ppSRVOut );

	// �R���s���[�g�V�F�[�_�[����̏o�͎��Ɏg�p����A���I�[�_�[�h�A�N�Z�X�r���[���쐬����
	HRESULT CreateUAVForStructuredBuffer(
		ID3D11Device* pD3DDevice,
		UINT uElementSize,
		UINT uCount,
		VOID* pInitData,
		ID3D11Buffer** ppBuf,
		ID3D11UnorderedAccessView** ppUAVOut );

	// �R���s���[�g�V�F�[�_�[�����s����
	void RunComputeShader( 
		ID3D11DeviceContext* pD3DDeviceContext,
		ID3D11ComputeShader* pComputeShader,
		ID3D11ShaderResourceView* pBufSRV,        // ���͗p,
		ID3D11UnorderedAccessView* pBufResultUAV,  // �o�͗p,
		UINT X, UINT Y, UINT Z );

	// �A���I�[�_�[�h�A�N�Z�X�r���[�̃o�b�t�@�̓��e�� CPU ����ǂݍ��݉\�ȃo�b�t�@�փR�s�[����
	ID3D11Buffer* CreateAndCopyToDebugBuf( 
		ID3D11Device* pD3DDevice,
		ID3D11DeviceContext* pD3DDeviceContext,
		ID3D11Buffer* pBuffer );
};


#endif