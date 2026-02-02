/*!
 * @brief 影が落とされるモデル用のシェーダー
 */

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// step-3 ライトビュープロジェクション行列の定数バッファーを定義
cbuffer ShadowCb : register(b1)
{
    float4x4 mLVP;
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL; // 法線
    float2 uv : TEXCOORD0;  // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;     // 法線
    float2 uv : TEXCOORD0;      // UV座標

    // step-4 ライトビュースクリーン空間での座標を追加
    float4 posInLVP : TEXCOORD1;
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////

Texture2D<float4> g_albedo : register(t0);      // アルベドマップ
Texture2D<float4> g_shadowMap : register(t10);  // シャドウマップ
sampler g_sampler : register(s0);               // サンプラーステート

/// <summary>
/// 影が落とされる3Dモデル用の頂点シェーダー
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
    // 通常の座標変換
    SPSIn psIn;
    float4 worldPos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, worldPos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.uv = vsIn.uv;
    psIn.normal = mul(mWorld, vsIn.normal);

    // step-5 ライトビュースクリーン空間の座標を計算する
    psIn.posInLVP = mul(mLVP, worldPos);

    return psIn;
}

/// <summary>
/// 影が落とされる3Dモデル用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float4 color = g_albedo.Sample(g_sampler, psIn.uv);

    // step-6 ライトビュースクリーン空間からUV空間に座標変換wwwwwww22
    //【注目】ライトビュースクリーン空間からUV座標空間に変換している
    float2 shadowMapUV = psIn.posInLVP.xy / psIn.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;
    
    // step-7 UV座標を使ってシャドウマップから影情報をサンプリング
    float3 shadowMap = 1.0f;
    if(shadowMapUV.x>0.0f&&shadowMapUV.x<1.0f
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        shadowMap = g_shadowMap.Sample(g_sampler, shadowMapUV);
    }

    // step-8 サンプリングした影情報をテクスチャカラーに乗算する
    //テクスチャカラーにシャドウマップからサンプリングした情報を掛け算する
    //影が書き込まれていたら0.5になるので、色味が落ちて影っぽくなる
    color.xyz *= shadowMap;

    return color;
}
