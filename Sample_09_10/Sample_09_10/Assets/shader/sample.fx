/*!
 * @brief ノイズシェーダー
 */

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

Texture2D<float4> colorTexture : register(t0); // カラーテクスチャ
sampler Sampler : register(s0);

// ハッシュ関数
float hash( float n )
{
    return frac(sin(n)*43758.5453);
}

// 3次元ベクトルからシンプレックスノイズを生成する関数
float SimplexNoise(float3 x)
{
    // The noise function returns a value in the range -1.0f -> 1.0f
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
                     lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
                lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
                     lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    // step-1 シンプレックスノイズを利用してノイズ加工を行う
    //シンプレックスノイズを使用して、０～１の乱数を取得
    float t = SimplexNoise(In.pos.xyz);
    
    //ノイズの値の範囲を０～１からー１～１に変換
    t = (t - 0.5f) * 2.0f;
    
    //UV座標にノイズを加える。0.01fはノイズの強さ
    //この数値を大きくするとノイズが大きくなる
    float2 uv = In.uv + t * 0.01f;
    
    //ずらしたUV座標を利用して、カラーをサンプリングする
    float4 color = colorTexture.Sample(Sampler, uv);

    return color;
}
