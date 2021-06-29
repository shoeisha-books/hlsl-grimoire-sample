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

    // step-8 垂直方向のUVオフセットを計算

    // step-9 垂直方向にカラーをサンプリングして平均する

    // step-10 対角線方向のUVオフセットを計算

    // step-11 対角線方向にカラーをサンプリングして平均化する

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

    // step-13 左斜め下方向にカラーをサンプリングする

    // step-14 右斜め下方向へのUVオフセットを計算する

    // step-15 右斜め下方向にカラーをサンプリングする

    // step-16 平均化

    return color;
}
