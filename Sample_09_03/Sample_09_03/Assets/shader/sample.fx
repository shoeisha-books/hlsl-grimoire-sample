/*!
 * @brief 円形ワイプ
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

cbuffer WipeCB : register(b1)
{
    float wipeSize;        //ワイプサイズ
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

Texture2D<float4> colorTexture : register(t0); // カラーテクスチャ
sampler Sampler : register(s0);

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    float4 color = colorTexture.Sample(Sampler, In.uv);

    // step-1 画面の中央からこのピクセルに向かって伸びるベクトルを計算する
    float2 posFromCenter = In.pos.xy - float2(640.0f, 360.0f); // 画面中央が(640,360)の場合

    // step-2 画面の中央からの距離とwipeSizeを利用してピクセルキル
    clip(length(posFromCenter) - wipeSize);

    return color;
}
