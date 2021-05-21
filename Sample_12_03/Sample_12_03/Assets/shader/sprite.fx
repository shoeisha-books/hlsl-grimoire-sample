
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

cbuffer DirectionLight : register(b1)
{
    float3 ligColor;        // ライトのカラー
    float3 ligDirection;    // ライトの方向

    // step-9 定数バッファーに視点の位置を追加
    float3 eyePos;          // 視点
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

Texture2D<float4> albedoTexture : register(t0);     // アルベド
Texture2D<float4> normalTexture : register(t1);     // 法線

// step-10 ワールド座標テクスチャにアクセスするための変数を追加
Texture2D<float4> worldPosTexture : register(t2);   // ワールド座標

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
    // G-Bufferの内容を使ってライティング
    float4 albedo = albedoTexture.Sample(Sampler, In.uv);
    float3 normal = normalTexture.Sample(Sampler, In.uv).xyz;

    normal = (normal * 2.0f)-1.0f;

    // 拡散反射光を計算
    float3 lig = 0.0f;
    float t = max(0.0f, dot(normal, ligDirection) * -1.0f);
    lig = ligColor * t;

    // step-11 スペキュラ反射を計算
    float3 worldPos = worldPosTexture.Sample(Sampler, In.uv).xyz;
    float3 toEye = normalize(eyePos - worldPos);
    float3 r = reflect(ligDirection, normal);
    t = max(0.0f, dot(toEye, r));
    t = pow(t, 5.0f);
    lig += ligColor * t;

    float4 finalColor = albedo;
    finalColor.xyz *= lig;
    return finalColor;
}
