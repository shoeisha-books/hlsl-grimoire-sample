/*!
 * @brief ZPrepass
 */

// step-4 ZPrepassシェーダーを実装する

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
    float4 pos : POSITION;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;
};

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    return psIn;
}

/// <summary>
/// ピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return float4(psIn.pos.z, psIn.pos.z, psIn.pos.z, 1.0f);
}
