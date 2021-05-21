
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

cbuffer ExpandCbParam : register(b1)
{
    Matrix viewProjInvMatrix;   // ビュープロジェクション行列の逆行列。
    float3 ligColor;            // ライトのカラー
    float3 ligDirection;        // ライトの方向
    float3 eyePos;              // 視点
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
Texture2D<float4> depthTexture : register(t2);      // 深度

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
    // アルベドカラーをサンプリング。
    float4 albedo = albedoTexture.Sample(Sampler, In.uv);
    // 法線をサンプリング。
    float3 normal = normalTexture.Sample(Sampler, In.uv).xyz;
    // 深度情報からワールド座標を計算する。
    float4 worldPos;

    // まず正規化スクリーン座標系での座標を計算する。
    // z座標は深度テクスチャから引っ張ってくる。
    worldPos.z = depthTexture.Sample( Sampler, In.uv).r;
    // xy座標はUV座標から計算する。
    worldPos.xy = In.uv * float2( 2.0f, -2.0f ) - 1.0f;
    worldPos.w = 1.0f;
    // ビュープロジェクション行列の逆行列を乗算して、ワールド座標に戻す。
    worldPos = mul( viewProjInvMatrix, worldPos );
    worldPos.xyz /= worldPos.w;
    
    normal = (normal * 2.0f)-1.0f;
    
    // 拡散反射光を計算
    float3 lig = 0.0f;
    float t = max(0.0f, dot(normal, ligDirection) * -1.0f);
    lig = ligColor * t;

    // スペキュラ反射を計算
    float3 toEye = normalize(eyePos - worldPos.xyz);
    float3 r = reflect(ligDirection, normal);
    t = max(0.0f, dot(toEye, r));
    t = pow(t, 5.0f);
    // このサンプルでは、スペキュラの効果を分かりやすくするために、50倍している。
    float3 specColor = ligColor * t * 50.0f;

    // step-3 スペキュラ強度を法線テクスチャのw要素からサンプリングする。
    float specPower = normalTexture.Sample(Sampler, In.uv).w;

    // step-4 スペキュラ強度をスペキュラライトに乗算する。
    specColor *= specPower;

    //反射光にスペキュラカラーを足し算する。
    lig += specColor;
    //環境光。このサンプルでは一律で0.7底上げをする。
    lig += 0.7f;
    
    float4 finalColor = albedo;
    finalColor.xyz *= lig;
    return finalColor;
}
