// 頂点シェーダーへの入力
struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD;
};

// 頂点シェーダーの出力
struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

sampler g_sampler : register(s0);
Texture2D g_texture : register(t0);

// step-9 頂点シェーダーを実装

// step-10 ピクセルシェーダーを実装
