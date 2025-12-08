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
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;     // 法線
    float2 uv : TEXCOORD0;      // uv座標
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// サンプラーステート
sampler g_sampler : register(s0);

/////////////////////////////////////////////////////////
// 頂点シェーダー
/////////////////////////////////////////////////////////
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, vsIn.normal));
    psIn.uv = vsIn.uv;

    return psIn;
}

/////////////////////////////////////////////////////////
// ピクセルシェーダー
/////////////////////////////////////////////////////////
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float3 lig = 0;

    // ディレクションライト4本分の拡散反射光を計算する
    for(int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        ///////////////////////////////////////////////////////////
        // Lambert拡散反射モデルを使って拡散反射光を計算する
        ///////////////////////////////////////////////////////////
        // ライトの方向とサーフェイスの法線を使って、拡散反射の強さを計算する
        float NdotL = saturate(dot(psIn.normal, -directionalLight[ligNo].direction));

        // 拡散反射光を計算する
        float3 diffuse = directionalLight[ligNo].color * NdotL;

        // step-1 拡散反射光を正規化する
        diffuse /= 3.1415926f;

        // 拡散反射光を反射光に足し算する
        lig += diffuse;
    }

    // アルベドカラー
    float4 albedoColor = g_texture.Sample(g_sampler, psIn.uv);
    float4 finalColor = 1.0f;
    finalColor.xyz = albedoColor.xyz * lig;
    return finalColor;
}
