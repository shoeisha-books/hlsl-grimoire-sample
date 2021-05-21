/*!
 * @brief プリミティブ用のシェーダー
 */

static const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの本数

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ディレクションライト
struct DirectionalLight
{
    float3 direction;   // ライトの方向
    float4 color;       // ライトの色
};

// ライト用の定数バッファー
cbuffer LightCb : register(b1)
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];
    float3 eyePos;          // カメラの視点
    float specPow;          // スペキュラの絞り
    float3 ambientLight;    // 環境光
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // モデルの頂点座標
    float3 normal : NORMAL; // 法線
    float2 uv : TEXCOORD0;  // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
    float2 uv : TEXCOORD0;          // uv座標
    float3 worldPos : TEXCOORD1;    // ワールド空間でのピクセルの座標
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);
Texture2D<float4> g_normalMap : register(t1);
Texture2D<float4> g_specularMap : register(t2);

// サンプラーステート
sampler g_sampler : register(s0);

static const float PI = 3.14159265358979323846;

float sqr(float x) { return x*x; }

float Beckmann(float m, float t)
{
    float M = m*m;
    float T = t*t;
    return exp((T-1)/(M*T)) / (M*T*T);
}

float spcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1-f0) * pow(1-u, 5);
}

float BRDF(float3 L, float3 V, float3 N)
{
    float microfacet = 1.0f;
    float f0 = 0.5;
    bool include_F = 0;
    bool include_G = 0;

    // compute the half float3
    float3 H = normalize(L + V);

    float NdotH = dot(N, H);
    float VdotH = dot(V, H);
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);

    float D = Beckmann(microfacet, NdotH);
    float F = spcFresnel(f0, VdotH);

    float t = 2.0 * NdotH / VdotH;
    float G = max(0.0f, min(1.0, min(t * NdotV, t * NdotL)));
    float m = 3.14159265 * NdotV * NdotL;

    /*
    NdotH = NdotH + NdotH;
    float G = (NdotV < NdotL) ?
        ((NdotV*NdotH < VdotH) ?
         NdotH / VdotH :
         oneOverNdotV)
        :
        ((NdotL*NdotH < VdotH) ?
         NdotH*NdotL / (VdotH*NdotV) :
         oneOverNdotV);
    */
    // if (include_G) G = oneOverNdotV;
    return max(F * D * G / m, 0.0);
}

float SchlickFresnel(float u, float f0, float f90)
{
  return f0 + (f90-f0)*pow(1.0f-u,5.0f);
}

float3 NormalizedDisneyDiffuse(float3 N, float3 L, float3 V, float roughness)
{
  // 光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
  float3 H = normalize(L+V);

  float energyBias = lerp(0.0f, 0.5f, roughness);
  float energyFactor = lerp(1.0f, 1.0f/1.51f, roughness);

  // 光源に向かうベクトルとハーフベクトルがどれだけ似ているかを内積で求める
  float dotLH = saturate(dot(L,H));

  // 法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
  float dotNL = saturate(dot(N,L));

  // 法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
  float dotNV = saturate(dot(N,V));

  float Fd90 = energyBias + 2.0 * dotLH * dotLH * roughness;

  float FL = SchlickFresnel(1.0f, Fd90, dotNL);
  float FV = SchlickFresnel(1.0f, Fd90, dotNV);
  return (FL*FV)/PI;
}

// 頂点シェーダー
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, vsIn.normal));

    psIn.uv = vsIn.uv;

    return psIn;
}

// テクスチャなしプリミティブ描画用のピクセルシェーダー
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // 法線を計算
    float3 normal = psIn.normal;

    // Lambert拡散反射
    float3 lig = 0;

    float3 toEye = normalize(eyePos - psIn.worldPos);

    // メタリックは固定
    float metaric = g_specularMap.Sample(g_sampler, psIn.uv).a;

    for(int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // Disney
        float NdotL = saturate(dot(normal, -directionalLight[ligNo].direction));

        float3 diffuse = NormalizedDisneyDiffuse(normal, -directionalLight[ligNo].direction, toEye, 1.0f-metaric) * directionalLight[ligNo].color *(1.0f-metaric) * NdotL;

        // スペキュラ反射
        float3 spec = BRDF(-directionalLight[ligNo].direction, toEye, normal) * directionalLight[ligNo].color * metaric;
        lig += (diffuse + spec);
    }

    // シンプルな環境光
    lig += ambientLight;

    // アルベドカラー
    float4 albedoColor = g_texture.Sample(g_sampler, psIn.uv);
    float4 finalColor = 1.0f;
    finalColor.xyz = albedoColor.xyz * lig;
    return finalColor;
}
