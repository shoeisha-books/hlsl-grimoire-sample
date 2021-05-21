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
// step-11 影用のパラメータにアクセスする定数バッファーを定義
cbuffer ShadowParamCb : register(b1)
{
    float4x4 mLVP;      // ライトビュープロジェクション行列
    float3 lightPos;    // ライトの座標
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;      // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;     // 法線
    float2 uv : TEXCOORD0;      // uv座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
    float2 uv : TEXCOORD0;          // uv座標

    // ライトビュースクリーン空間での座標を追加
    float4 posInLVP : TEXCOORD1;    // ライトビュースクリーン空間でのピクセルの座標
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

    // ライトビュースクリーン空間の座標を計算する
    psIn.posInLVP = mul(mLVP, worldPos);

    // step-12 頂点のライトから見た深度値を計算する
    psIn.posInLVP.z = length(worldPos.xyz - lightPos) / 1000.0f;
    return psIn;
}

/// <summary>
// / 影が落とされる3Dモデル用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float4 color = g_albedo.Sample(g_sampler, psIn.uv);

    // ライトビュースクリーン空間からUV空間に座標変換
    float2 shadowMapUV = psIn.posInLVP.xy / psIn.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;

    // ライトビュースクリーン空間でのZ値を計算する
    float zInLVP = psIn.posInLVP.z;

    if(shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        // step-13 シャドウレシーバーに影を落とす
        // シャドウマップから値をサンプリング
        float2 shadowValue = g_shadowMap.Sample(g_sampler, shadowMapUV).xy;

        // まずこのピクセルが遮蔽されているか調べる。これは通常のデプスシャドウと同じ
        if(zInLVP > shadowValue.r && zInLVP <= 1.0f)
        {
            // 遮蔽されているなら、チェビシェフの不等式を利用して光が当たる確率を求める
            float depth_sq = shadowValue.x * shadowValue.x;

            // このグループの分散具合を求める
            // 分散が大きいほど、varianceの数値は大きくなる
            float variance = min(max(shadowValue.y - depth_sq, 0.0001f), 1.0f);

            // このピクセルのライトから見た深度値とシャドウマップの平均の深度値の差を求める
            float md = zInLVP - shadowValue.x;

            // 光が届く確率を求める
            float lit_factor = variance / (variance + md * md);

            // シャドウカラーを求める
            float3 shadowColor = color.xyz * 0.5f;

            // 光が当たる確率を使って通常カラーとシャドウカラーを線形補完
            color.xyz = lerp(shadowColor, color.xyz, lit_factor);
        }
    }

    return color;
}
