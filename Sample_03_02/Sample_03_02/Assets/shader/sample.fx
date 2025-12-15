// 頂点シェーダーへの入力頂点構造体
struct VSInput
{
    float4 pos : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD; // 頂点からUV座標のデータを引っ張ってくる
};

// 頂点シェーダーの出力
struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

float4x4 g_worldMatrix : register(b0);
sampler g_sampler : register(s0);

// step-4 t0レジスタのテクスチャにアクセスする変数を追加する
Texture2D g_texture : register(t0);

// 頂点シェーダー
// 1. 引数は変換前の頂点情報
// 2. 戻り値は変換後の頂点情報
VSOutput VSMain(VSInput In)
{
    VSOutput vsOut = (VSOutput)0;
    vsOut.pos = mul(g_worldMatrix, In.pos);
    vsOut.uv = In.uv;
    vsOut.color = In.color; //
    return vsOut;
}

// ピクセルシェーダー
float4 PSMain(VSOutput vsOut) : SV_Target0
{
    // step-5 テクスチャカラーをサンプリングして返す
    
    float4 texColor = g_texture.Sample(g_sampler, vsOut.uv);
    return texColor;
    //return float4(vsOut.color, 1.0f);
}
