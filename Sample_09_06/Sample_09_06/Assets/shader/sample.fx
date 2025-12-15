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
    float wipeSize;     // ワイプサイズ
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

    // step-1 チェッカーボードワイプを実装する
    //1.ピクセルのY座標を128で割り、小数部分を切り捨て、行番号を求める
    //float t = floor(In.pos.y / 128.0f);
    
    //斜めのチェッカーボードワイプになる
    float t = fmod(In.pos.y, 128.0f) / 128.0f;
    
    //2.行番号を2で割った余りを求める。偶数ならtは0、奇数ならtは1となる
    t = fmod(t, 2.0f);
    
    //3.奇数行ならX座標を64ずらして、縦じまのワイプ処理を行う
    t = (int) fmod(In.pos.x + 64.0f * t, 128.0f);
    //t = (int) fmod(In.pos.x + 128.0f * t, 128.0f);
    clip(t - wipeSize);

    return color;
}
