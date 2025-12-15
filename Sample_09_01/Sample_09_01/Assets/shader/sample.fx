/*!
 * @brief リニアワイプ
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

// step-4 ワイプサイズにアクセスするための定数バッファーを定義する
cbuffer WipeCB : register(b1)
{
    float wipeSize; // ワイプサイズ
}

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

    // step-5 ワイプサイズの値に応じてピクセルをクリップする
    //pos.xのxの部分をy部すると縦にワイプする
    clip(In.pos.x - wipeSize);//左から右へワイプ
    //clip(In.uv.x - wipeSize);

    return color;
}
