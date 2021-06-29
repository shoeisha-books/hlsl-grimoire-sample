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

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // モデルの頂点座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
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
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    return psIn;
}

/// <summary>
/// シャドウマップ描画用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return float4(psIn.pos.z, psIn.pos.z, psIn.pos.z, 1.0f);
}
