/*!
 * @brief ブルーム
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

/*!
 * @brief 頂点シェーダー
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

Texture2D<float4> mainRenderTargetTexture : register(t0); // メインレンダリングターゲットのテクスチャ
sampler Sampler : register(s0);

/////////////////////////////////////////////////////////
// ブラー
/////////////////////////////////////////////////////////
struct PS_BlurInput
{
    float4 pos : SV_POSITION;
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
};

Texture2D<float4> sceneTexture : register(t0); // シーンテクスチャ

/*!
 * @brief ブラー用の定数バッファー
 */
cbuffer CBBlur : register(b1)
{
    float4 weight[2]; // 重み
}

/*!
 * @brief 横ブラー頂点シェーダー
 */
PS_BlurInput VSXBlur(VSInput In)
{
    // step-13 横ブラー用の頂点シェーダーを実装

    return Out;
}

/*!
 * @brief 縦ブラー頂点シェーダー
 */
PS_BlurInput VSYBlur(VSInput In)
{
    // step-14 縦ブラー用の頂点シェーダーを実装

    return Out;
}

/*!
 * @brief ブラーピクセルシェーダー
 */
float4 PSBlur(PS_BlurInput In) : SV_Target0
{
    // step-15 横、縦ブラー用のピクセルシェーダーを実装

}
