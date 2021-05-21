/*!
 *@brief 六角形ブラー
 */

// ブラーをかけるテクスチャの幅
static const float BLUR_TEX_W = 1280.0f;

// ブラーをかけるテクスチャの高さ
static const float BLUR_TEX_H = 720.0f;

// ブラー半径。この数値を大きくすると六角形ボケが大きくなる
static const float BLUR_RADIUS = 8.0f;

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

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

// step-6  垂直、対角線ブラーの出力構造体を定義
struct PSOutput
{
    float4 color_0 : SV_Target0;    // 垂直ブラーの出力先
    float4 color_1 : SV_Target1;    // 斜めブラーの出力先
};

/*!
 *@brief 頂点シェーダー
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

Texture2D<float4> srcTexture : register(t0); // ブラーをかける前のオリジナルテクスチャ
// サンプラーステート
sampler g_sampler : register(s0);

/*!
 *@brief 垂直、斜めブラーのピクセルシェーダー
 */
PSOutput PSVerticalDiagonalBlur(PSInput pIn)
{
    PSOutput psOut = (PSOutput)0;

    // ブラーをかけるテクスチャのカラーを取得
    float4 srcColor = srcTexture.Sample(
        g_sampler, pIn.uv );

    // step-7 ブラー半径（BLUR_RADIUS）からブラーステップの長さを求める
    float blurStepLen = BLUR_RADIUS / 4.0f;

    // step-8 垂直方向のUVオフセットを計算
    float2 uvOffset = float2(0.0f, 1.0f / BLUR_TEX_H);
    uvOffset *= blurStepLen;

    // step-9 垂直方向にカラーをサンプリングして平均する
    // 1ステップ進める
    psOut.color_0 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset);

    // 2ステップ進める
    psOut.color_0 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 2);

    // 3ステップ進める
    psOut.color_0 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 3);

    // 4ステップ進める
    psOut.color_0 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 4);

    // 平均化
    psOut.color_0 /= 4.0f;

    // step-10 対角線方向のuvオフセットを計算
    uvOffset.x = 0.86602f / BLUR_TEX_W;
    uvOffset.y = -0.5f / BLUR_TEX_H;
    uvOffset *= blurStepLen;

    // step-11 対角線方向にカラーをサンプリングして平均化する
    psOut.color_1 = srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset);

    psOut.color_1 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 2);

    psOut.color_1 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 3);

    psOut.color_1 += srcTexture.Sample(
        g_sampler, pIn.uv + uvOffset * 4);

    psOut.color_1 += srcColor;
    psOut.color_1 /= 5.0f;

    // 垂直方向に平均化
    psOut.color_1 += psOut.color_0;
    psOut.color_1 /= 2.0f;

    return psOut;
}

Texture2D<float4> blurTexture_0 : register(t0); // ブラーテクスチャ_0。1パス目で作成されたテクスチャ
Texture2D<float4> blurTexture_1 : register(t1); // ブラーテクスチャ_1。1パス目で作成されたテクスチャ

/*!
 *@brief 六角形作成ブラー
 */
float4 PSRhomboidBlur(PSInput pIn) : SV_Target0
{
    // ブラーステップの長さを求める
    float blurStepLen = BLUR_RADIUS / 4.0f;

    // step-12 左斜め下方向へのUVオフセットを計算する
    float2 uvOffset;
    uvOffset.x = 0.86602f / BLUR_TEX_W;
    uvOffset.y = -0.5f / BLUR_TEX_H;
    uvOffset *= blurStepLen;

    // step-13 左斜め下方向にカラーをサンプリングする
    float4 color = blurTexture_0.Sample(
        g_sampler, pIn.uv + uvOffset);

    color += blurTexture_0.Sample(
        g_sampler, pIn.uv + uvOffset * 2);

    color += blurTexture_0.Sample(
        g_sampler, pIn.uv + uvOffset * 3);

    color += blurTexture_0.Sample(
        g_sampler, pIn.uv + uvOffset * 4);

    // step-14 右斜め下方向へのUVオフセットを計算する
    uvOffset.x = -0.86602f / BLUR_TEX_W * blurStepLen;

    // step-15 右斜め下方向にカラーをサンプリングする
    color += blurTexture_1.Sample(
        g_sampler, pIn.uv);

    color += blurTexture_1.Sample(
        g_sampler, pIn.uv + uvOffset);

    color += blurTexture_1.Sample(
        g_sampler, pIn.uv + uvOffset * 2);

    color += blurTexture_1.Sample(
        g_sampler, pIn.uv + uvOffset * 3);

    color += blurTexture_1.Sample(
        g_sampler, pIn.uv + uvOffset * 4);

    // step-16 平均化
    color /= 9.0f;

    return color;
}
