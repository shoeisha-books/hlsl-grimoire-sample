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
    float3 worldPos : TEXCOORD1; // ワールド座標
};

// ピクセルシェーダーからの出力
struct SPSOut
{
    float4 albedo : SV_Target0;     // アルベド
    float3 normal : SV_Target1;     // 法線
    float3 worldPos : SV_Target2;   // ワールド座標
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

    psIn.pos = mul(mWorld, vsIn.pos); // モデルの頂点をワールド座標系に変換

    // 頂点シェーダーからワールド座標を出力
    psIn.worldPos = psIn.pos;

    psIn.pos = mul(mView, psIn.pos); // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos); // カメラ座標系からスクリーン座標系に変換
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
    psOut.normal = psIn.normal;

    psOut.worldPos = psIn.worldPos;
    return psOut;
}

// step-3 ライトの情報を受け取るための定数バッファーを追加
cbuffer DirectionLight : register(b1)
{
    float3 ligColor;        // ライトのカラー
    float3 ligDirection;    // ライトの方向
    float3 eyePos;          // 視点
};

// step-4 半透明オブジェクト用のピクセルシェーダーを実装
float4 PSMainTrans(SPSIn psIn) : SV_Target0
{
    // フォワードレンダリング
    // 普通にライティングをする
    // 拡散反射光を計算
    float3 lig = 0.0f;
    float3 normal = psIn.normal;
    float t = max(0.0f, dot(psIn.normal, ligDirection) * -1.0f);
    lig = ligColor * t;

    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);
    finalColor.xyz *= lig;
    return finalColor;
}
