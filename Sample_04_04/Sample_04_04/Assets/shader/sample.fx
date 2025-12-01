///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct DirectionLight
{
    float3 direction;   // ライトの方向
    float3 color;       // ライトのカラー
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
    DirectionLight directionLight;
    float3 eyePos; // 視点の位置
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

    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換

    // 頂点法線をピクセルシェーダーに渡す
    psIn.normal = mul(mWorld, vsIn.normal); // 法線を回転させる
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // ピクセルの法線とライトの方向の内積を計算する
    float t = dot(psIn.normal, directionLight.direction);
    t *= -1.0f;

    // 内積の結果が0以下なら0にする
    if(t < 0.0f)
    {
        t = 0.0f;
    }

    // 拡散反射光を求める
    float3 diffuseLig = directionLight.color * t;

    // 反射ベクトルを求める
    float3 refVec = reflect(directionLight.direction, psIn.normal);

    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - psIn.worldPos;
    toEye = normalize(toEye);

    // 鏡面反射の強さを求める
    t = dot(refVec, toEye);
    if(t < 0.0f)
    {
        t = 0.0f;
    }

    // 鏡面反射の強さを絞る
    t = pow(t, 1.0f);

    // 鏡面反射光を求める
    float3 specularLig = directionLight.color * t;

    // 拡散反射光と鏡面反射光を足し算して、最終的な光を求める
    float3 lig = diffuseLig + specularLig;

    // step-1 ライトの効果を一律で底上げする
    lig.x += 0.3f;
    lig.y += 0.3f;
    lig.z += 0.3f;

    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);

    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= lig;

    return finalColor;
}
