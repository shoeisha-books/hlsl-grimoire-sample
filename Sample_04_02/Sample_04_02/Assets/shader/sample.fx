/*!
 * @brief シンプルなモデル表示用のシェーダー
 */
///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

///////////////////////////////////////////
// 定数バッファー
///////////////////////////////////////////
// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// step-5 ディレクションライト用のデータを受け取る定数バッファーを用意する
cbuffer diDirectionLightCb : register(b1)
{
    float3 ligDirection; // ライトの方向
    float3 ligColor; // ライトの色
}

///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

///////////////////////////////////////////
// サンプラーステート
///////////////////////////////////////////
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

    // step-6 頂点法線をピクセルシェーダーに渡す
    psIn.normal = mul(mWorld, vsIn.normal);

    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // step-7 ピクセルの法線とライトの方向の内積を計算する
    float t = dot(psIn.normal, ligDirection);
    
    //内積の結果に-1を乗算する
    t *= -1.0f;

    // step-8 内積の結果が0以下なら0にする
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // step-9 ピクセルが受けているライトの光を求める
    float3 diffuseLig = ligColor * t;

    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);

    // step-10 最終出力カラーに光を乗算する
    finalColor.xyz *= diffuseLig;

    return finalColor;
}
