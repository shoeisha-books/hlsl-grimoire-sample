/*!
 * @brief チェッカーボードワイプ
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

cbuffer WipeCB : register(b1)
{
    float monochromeRate;       // 白黒率
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

    // step-1 画像を徐々にモノクロに変化させていく
    //ピクセルの明るさを計算する
    float Y = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    
    //明るさの値をRGBに設定して、モノクロカラーを求める
    float3 monochromeColor = float3(Y, Y, Y);
    
    //モノクロ率を使って徐々に白黒にしていく
    color.xyz = lerp(color, monochromeColor, monochromeRate);

    return color;
}
