/*!
 * @brief   シンプルなインスタンス描画用のシェーダー。
 */

///////////////////////////////////////////////////
// 構造体
///////////////////////////////////////////////////

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;      // モデルの頂点座標
    float2 uv : TEXCOORD0;      // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標    
    float2 uv : TEXCOORD0;          // uv座標
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////
// サンプラーステート
sampler g_sampler : register(s0);
Texture2D<float4> g_albedo : register(t0);      // アルベドマップ

// step-7 ワールド行列の配列にアクセスするための変数を追加。
StructuredBuffer<float4x4> g_worldMatrixArray : register( t10 );

/// <summary>
/// 頂点シェーダー
/// <summary>

SPSIn VSMain(
    SVSIn vsIn, 
    // step-8 頂点シェーダーの引数にインスタンスIDを追加。
    uint instanceId : SV_InstanceID
)
{
    SPSIn psIn;
    psIn.uv = vsIn.uv;

    // step-9 ワールド行列の配列とインスタンスIDを利用して座標変換。
    float4x4 m = g_worldMatrixArray[instanceId];
    psIn.pos = mul(m, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    return psIn;
}

/// <summary>
/// ピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn)  : SV_Target0
{
    return g_albedo.Sample( g_sampler, psIn.uv);
}
