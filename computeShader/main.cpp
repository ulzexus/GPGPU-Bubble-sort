#include <iostream>
#include <stdio.h>
#include <tchar.h>
using namespace std;

#include "DeviceManager.h"

#include "Vertex.h"

#include "def.h"

static const int NUM_ELEMENTS = 128;

int main() {
	HRESULT hr = E_FAIL;

	ID3D11Device* device = NULL;
	ID3D11DeviceContext* context = NULL;
	ID3D11ComputeShader* shader = NULL;

	ID3D11Buffer* buf = NULL;
	ID3D11Buffer* bufResult = NULL;

	ID3D11ShaderResourceView* bufSRV = NULL;
	ID3D11UnorderedAccessView* bufResultUAV = NULL;

	BUFIN_TYPE bufferIn[ NUM_ELEMENTS ];

	DeviceManager* m = DeviceManager::instance();

	hr = m->CreateComputeDevice( &device, &context, &shader );
	if ( FAILED( hr ) ) {
		cout << "CreateComputeDevice" << endl;
		return false;
	}

	for ( int i = 0; i < NUM_ELEMENTS; ++i ) {
		bufferIn[ i ].i = rand();
	}

	hr = m->CreateSRVForStructuredBuffer( device, sizeof( BUFIN_TYPE ), NUM_ELEMENTS, &bufferIn[ 0 ], &buf, &bufSRV );
	if ( FAILED( hr ) ) {
		cout << "CreateSRVForStructuredBuffer" << endl;
		return false;
	}

	hr = m->CreateUAVForStructuredBuffer( device, sizeof( BUFOUT_TYPE ), NUM_ELEMENTS, NULL, &bufResult, &bufResultUAV );
	if ( FAILED( hr ) ) {
		cout << "CreateUAVForStructuredBuffer" << endl;
		return false;
	}

	m->RunComputeShader( context, shader, bufSRV, bufResultUAV, NUM_ELEMENTS / 1, 1, 1 );

	ID3D11Buffer* debug = m->CreateAndCopyToDebugBuf( device, context, bufResult );
	D3D11_MAPPED_SUBRESOURCE map;
	context->Map( debug, 0, D3D11_MAP_READ, 0, &map );

	BUFOUT_TYPE* p = reinterpret_cast< BUFOUT_TYPE* >( map.pData );

	for ( int i = 0; i < NUM_ELEMENTS; ++i ) {
		printf( "%d = %d\n", i, p[ i ] );
	}

	context->Unmap( debug, 0 );

	SAFE_RELEASE( debug );

	hr = S_OK;

	return true;
	
}