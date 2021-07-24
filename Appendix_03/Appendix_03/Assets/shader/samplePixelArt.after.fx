/*!
 * @brief   ドット絵シェーダー
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

/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// ピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn)  : SV_Target0
{
    //テクスチャカラーをサンプリングする。
    float4 baseColor = g_albedo.Sample( g_sampler, psIn.uv);

    // step-9 輪郭付近のカラーを暗くする。
    // 近傍8テクセルへのUVオフセット
    float2 uvOffset[8] = {
        float2(           0.0f,  1.0f / 100.0f), //上
        float2(           0.0f, -1.0f / 100.0f), //下
        float2( 1.0f / 100.0f,           0.0f), //右
        float2(-1.0f / 100.0f,           0.0f), //左
        float2( 1.0f / 100.0f,  1.0f / 100.0f), //右上
        float2(-1.0f / 100.0f,  1.0f / 100.0f), //左上
        float2( 1.0f / 100.0f, -1.0f / 100.0f), //右下
        float2(-1.0f / 100.0f, -1.0f / 100.0f)  //左下
    };

    // このピクセルのα値を取得
    float alpha = g_albedo.Sample( g_sampler, psIn.uv).a;
    // 近傍8テクセルのα値の平均値を計算する
    float alpha2 = 0.0f;
    for( int i = 0; i < 8; i++)
    {
        alpha2 += g_albedo.Sample(g_sampler, psIn.uv + uvOffset[i]).a;
    }
    alpha2 /= 8.0f;

    // 自身の深度値と近傍8テクセルの深度値の差を調べる
    if(abs(alpha2 - alpha) > 0.2f)
    {
        // α値値が結構違うので輪郭付近なので、カラーを暗くする。
        // ->これがエッジカラーとなる
        baseColor.xyz *= 0.3f;
        
    }

    return baseColor;
}
