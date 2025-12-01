// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ライト用の定数バッファー
cbuffer LightCb : register(b1)
{
    float3 dirLigDirection; // ライトの方向
    float4 dirLigColor;     // ライトの色
    float3 eyePos;          // カメラの視点
    float specPow;          // スペキュラの絞り
    float3 ambientLight;    // 環境光
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION; // モデルの頂点座標
    float3 normal : NORMAL; // 法線

    // step-1 頂点シェーダーの入力に接ベクトルと従ベクトルを追加
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;

    float2 uv : TEXCOORD0;  // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線

    // step-2 ピクセルシェーダーの入力に接ベクトルと従ベクトルを追加
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;

    float2 uv : TEXCOORD0;          // UV座標
    float3 worldPos : TEXCOORD1;    // ワールド空間でのピクセルの座標
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// step-3 法線マップにアクセスするための変数を追加
Texture2D<float4> g_normalMap : register(t1);

// サンプラーステート
sampler g_sampler : register(s0);

// 頂点シェーダー
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, vsIn.normal));

    // step-4 接ベクトルと従ベクトルをワールド空間に変換する
    psIn.tangent = normalize(mul(mWorld, vsIn.tangent));
    psIn.biNormal = normalize(mul(mWorld, vsIn. biNormal));

    psIn.uv = vsIn.uv;
    return psIn;
}

// テクスチャなしプリミティブ描画用のピクセルシェーダー
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // ディフューズマップをサンプリング
    float4 diffuseMap = g_texture.Sample(g_sampler, psIn.uv);

    float3 normal =  psIn.normal;

    // step-5 法線マップからタンジェントスペースの法線をサンプリングする
    float3 localNormal = g_normalMap.Sample(g_sampler, psIn.uv).xyz;
    //タンジェントスペースの法線を０～１の範囲からー１～１の範囲に復元する
    localNormal = (localNormal - 0.5f) * 2.0f;

    // step-6 タンジェントスペースの法線をワールドスペースに変換する
    normal = psIn.tangent * localNormal.x + psIn.biNormal * localNormal.y +
             psIn.normal * localNormal.z;

    // 拡散反射光を計算する
    float3 lig = 0.0f;
    lig += max(0.0f, dot(normal, -dirLigDirection)) * dirLigColor;
    lig += ambientLight;

    float4 finalColor = diffuseMap;
    finalColor.xyz *= lig;

    return finalColor;
}
