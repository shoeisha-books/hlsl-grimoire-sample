/*!
 * @brief シンプルなモデル表示用のシェーダー
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
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// ピクセルシェーダーからの出力
struct SPSOut
{
    float4 albedo : SV_Target0; // アルベド
    float4 normal : SV_Target1; // 法線
    float depth : SV_Target2;   // 深度値
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// サンプラーステート
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    psIn.normal = normalize(mul(mWorld, vsIn.normal));
    psIn.uv = vsIn.uv;
    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
SPSOut PSMain(SPSIn psIn)
{
    // G-Bufferに出力
    SPSOut psOut;

    // アルベドカラーを出力
    psOut.albedo = g_texture.Sample(g_sampler, psIn.uv);

    // 法線を出力
    psOut.normal.xyz = psIn.normal;

    psOut.normal.w = 1.0f;

    // 深度値を出力
    psOut.depth = psIn.pos.z;
    return psOut;
}
