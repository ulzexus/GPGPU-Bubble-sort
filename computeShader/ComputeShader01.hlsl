#define BLOCK 128

// 入力用の構造体定義
struct BufInType
{
   int i;
};

// 出力用の構造体定義
struct BufOutType
{
   int i;
};

// 読み取り専用の構造化バッファー
// アンオーダードアクセスバッファーとして作成されたバッファを使用する
StructuredBuffer<BufInType> BufferIn : register(t0);

// 読み書き可能な構造化バッファー
// アンオーダードアクセスバッファーとして作成されたバッファを使用する
RWStructuredBuffer<BufOutType> BufferOut : register(u0);

//共有
groupshared int shared_data[BLOCK];

[numthreads( 128, 1, 1 )]
void CS_Main( uint3 DTid : SV_DispatchThreadID, uint idx : SV_GroupIndex )
{
	//移動してみる
	shared_data[ idx ] = BufferIn[ DTid.x ].i;
	GroupMemoryBarrierWithGroupSync();

	int i, j, temp;

	//バブルソートしてみる
    for (i = 0; i < BLOCK - 1; i++) {
        for (j = BLOCK - 1; j > i; j--) {
        	bool swap = ( shared_data[ j - 1 ] > shared_data[ j ] ) ? 1 : 0;
            if ( swap ) {
                temp = shared_data[j];
                shared_data[j] = shared_data[j - 1];
                shared_data[j - 1]= temp;
            }
        }	
    }

	BufferOut[ DTid.x ].i = shared_data[ idx ];
}