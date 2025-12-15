/*!
 * @brief チェッカーボードワイプ
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;           // MVP行列
    float4 mulColor;        // 乗算カラー
};

cbuffer NagaCB : register( b1 )
{
    float negaRate;         // ネガポジ反転率、ネガポジ反転の割合でもある
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
    
    //ピクセルの明るさを計算する
    //float Y = (color.b * 0.114478 + color.g * 0.586611 + color.r * 0.298912);

    // step-1 画像を徐々にネガポジ反転させていく
    //ネガポジ反転はRGBの値を反転させることで実現できる
    float3 negaColor;
    //モノクロ用の変数
    //float3 monochromeColor;
    negaColor.x = 1.0f - color.x;
    negaColor.y = 1.0f - color.y;
    negaColor.z = 1.0f - color.z;
    
    //モノクロ化するため、RGBに明るさをそのまま代入
    //monochromeColor.r = Y;
    //monochromeColor.g = Y;
    //monochromeColor.b = Y;
    
    //negaRateが0.5以上なら1.0、未満なら0.0にする
    //negaRateの数値を固定するようにして徐々に反転させるようにするのではなく切り替えるようにしている
    float switchValue = step(0.5f, negaRate);
    
    //ネガポジ率を使って徐々にネガポジ画像にしていく
    color.xyz = lerp(color.rgb, negaColor, switchValue);
    
    //徐々にグレースケールにする
    //color.xyz = lerp(color.rgb, monochromeColor, negaRate);

    return color;
}
