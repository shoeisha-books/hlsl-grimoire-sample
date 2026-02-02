/*!
 * @brief シャドウマップ描画用のシェーダー
 */

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// step-8 影用のパラメータにアクセスする定数バッファーを定義
cbuffer ShadowParamCB : register(b1)
{
    float4x4 mLVP;//ライトビュープロジェクション行列
    float3 lightPos;//ライトの座標
}

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // モデルの頂点座標
    float3 normal : NORMAL; // 法線
    float2 uv : TEXCOORD0;  // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;     // 法線
    float2 uv : TEXCOORD0;      // uv座標
    float2 depth : TEXCOORD1;   // ライト空間での座標
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////

/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMain(SVSIn vsIn)
{
    // シャドウマップ描画用の頂点シェーダーを実装
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    float3 worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    // step-9 頂点のライトから見た深度値と、ライトから見た深度値の2乗を計算する
    psIn.depth.x = length(worldPos - lightPos) / 1000.0f;
    psIn.depth.y = psIn.depth.x * psIn.depth.x;

    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// シャドウマップ描画用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // step-10 ライトから見た深度値と、ライトから見た深度値の2乗を出力する
    return float4(psIn.depth.x, psIn.depth.y, 0.0f, 1.0f);
}
