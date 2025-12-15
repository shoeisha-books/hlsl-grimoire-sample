/*!
 * @brief リニアワイプ
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

// step3 ワイプパラメータにアクセスするための定数バッファを定義
cbuffer WipeCB : register(b1)
{
    float2 wipeDirection;//ワイプの方向
    float wipeSize; //ワイプサイズ
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

    //step-4 ワイプ方向とワイプサイズを利用して、ピクセルをクリップする
    //
    //float t = dot(wipeDirection, In.pos.xy);
    
    //シマシマ模様の円が広がるワイプ
    float t = (int) fmod(length(In.pos.xy - float2(640.0f, 360.0f)), 128.0f);
    clip(t - wipeSize);

    return color;
}
