///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct DirectionLight
{
    float3 direction;   //ライトの方向
    float3 color;       //ライトのカラー
};

struct PointLight
{
    float3 position;    //ポイントライトの位置
    float3 color;       //ポイントライトのカラー
    float range;        //ポイントライトの影響範囲
};

///////////////////////////////////////////
// 定数バッファー
///////////////////////////////////////////
// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ディレクションライト用のデータを受け取るための定数バッファーを用意する
cbuffer DirectionLightCb : register(b1)
{
    DirectionLight directionLight;  //ディレクションライト
    PointLight pointLight;          //ポイントライト
    float3 eyePos;                  //視点の位置
    float3 ambientLight;            //アンビエントライト
};

///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

///////////////////////////////////////////
// サンプラーステート
///////////////////////////////////////////
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos);   //モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);    //ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    //カメラ座標系からスクリーン座標系に変換

    //頂点法線をピクセルシェーダーに渡す
    psIn.normal = mul(mWorld, vsIn.normal); //法線を回転させる
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float3 toCamera = normalize(eyePos - psIn.worldPos);
    float t = max(0.1f, pow(1.0f - abs(dot(toCamera, psIn.normal)), 0.7f));
    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);
    finalColor.xyz *= t;
    return finalColor;
}
