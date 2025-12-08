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
//1.　引数は変換前の頂点情報
//2.　戻り値は変換後の頂点情報
VSOutput VSMain(VSInput In)
{
    VSOutput vsOut = (VSOutput) 0;
    
    //入力された頂点座標を変換せずに出力する
    vsOut.pos = In.pos;
    vsOut.uv = In.uv;
    return vsOut;
}

// step-10 ピクセルシェーダーを実装
//1.
//2.
float4 PSMain(VSOutput In) : SV_Target0
{
    return g_texture.Sample(g_sampler, In.uv);
}