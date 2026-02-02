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

Texture2D<float4> sceneTexture : register(t0);  // シーンテクスチャ
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
    float4 color = sceneTexture.Sample(Sampler, In.uv);

    // step-7 ピクセルカラーをモノクロ化する
    //float Y = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    //color.r = Y;
    //color.g = Y;
    //color.b = Y;
    
    //この処理をしても結果は上の処理と変わらない
    float Y = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    color.rgb = float3(Y, Y, Y);

    return color;
}
