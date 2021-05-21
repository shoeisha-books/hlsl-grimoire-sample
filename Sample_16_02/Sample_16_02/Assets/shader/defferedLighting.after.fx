cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

// ディレクションライト
struct DirectionLight
{
    float3 color;       // ライトのカラー
    float3 direction;   // ライトの方向
};

// ポイントライト
struct PointLight
{
    float3 position;    // 座標
    float3 color;       // カラー
    float range;        // 範囲
};

static const int NUM_POINT_LIGHT = 1000;    // ポイントライトの数
static const int NUM_DIRECTION_LIGHT = 4;   // ディレクションライトの数

cbuffer Light : register(b1)
{
    DirectionLight directionLight[NUM_DIRECTION_LIGHT];
    PointLight pointLight[NUM_POINT_LIGHT];
    float4x4 mViewProjInv;  // ビュープロジェクション行列の逆行列
    float3 eyePos;          // 視点
    float specPow;          // スペキュラの絞り
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

Texture2D<float4> albedoTexture : register(t0); // アルベド
Texture2D<float4> normalTexture : register(t1); // 法線

// step-6 深度テクスチャの追加
Texture2D<float> depthTexture : register(t2);   // 射影空間に正規化された深度値

sampler Sampler : register(s0);

/*!
 *@brief        UV座標と深度値からワールド座標を計算する
 *@param[in]    uv：uv座標
 *@param[in]    zInProjectionSpace：射影空間の深度値
 *@param[in]    mViewProjInv：ビュープロジェクション行列の逆行列
 */
float3 CalcWorldPosFromUVZ(float2 uv, float zInProjectionSpace, float4x4 mViewProjInv)
{
    float3 screenPos;
    screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
    screenPos.z = zInProjectionSpace;

    float4 worldPos = mul(mViewProjInv, float4(screenPos, 1.0f));
    worldPos.xyz /= worldPos.w;
    return worldPos.xyz;
}

/*!
 *@brief    Lambert拡散反射を計算
 *@param    ligDir：サーフェイスに入射してくるライトの方向
 *@param    lightColor：ライトのカラー
 *@param    normal：サーフェイスの法線
 *@return   拡散反射の光
 */
float3 CalcLambertReflection(float3 ligDir, float3 ligColor, float3 normal)
{
    float t = max(0.0f, dot(normal, ligDir) * -1.0f);
    return ligColor * t;
}

/*!
 *@brief    スペキュラ反射を計算
 *@param    ligDir：ライトの方法
 *@param    ligColor：ライトのカラー
 *@param    normal：サーフェイスの法線
 *@param    toEye：サーフェイスから視点へのベクトル
 *@return   スペキュラ反射光
 */
float3 CalcSpecularReflection(float3 ligDir, float3 ligColor, float3 normal, float3 toEye)
{
    float3 r = reflect(ligDir, normal);
    float t = max(0.0f, dot(toEye, r));
    t = pow(t, specPow);
    return ligColor * t;
}

/*!
 *@brief 頂点シェーダーエントリーポイント
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

/*!
 *@brief ピクセルシェーダーエントリーポイント
 */
float4 PSMain(PSInput In) : SV_Target0
{
    // G-Bufferの内容を使ってライティング
    float4 albedo = albedoTexture.Sample(Sampler, In.uv);
    float3 normal = normalTexture.Sample(Sampler, In.uv).xyz;

    // step-7 射影空間の深度値からワールド座標を復元する
    float z = depthTexture.Sample(Sampler, In.uv);

    // 射影空間の深度値からワールド座標を復元する
    float3 worldPos = CalcWorldPosFromUVZ(In.uv, z, mViewProjInv);

    float3 lig = 0.0f;

    // 視点に向かって伸びるベクトルを計算
    float3 toEye = normalize(eyePos - worldPos);

    // ディレクションライトを計算
    for(int ligNo = 0; ligNo < NUM_DIRECTION_LIGHT; ligNo++)
    {
        // 拡散反射光を計算
        lig += CalcLambertReflection(
            directionLight[ligNo].direction,
            directionLight[ligNo].color,
            normal);

        // スペキュラ反射を計算
        lig += CalcSpecularReflection(
            directionLight[ligNo].direction,
            directionLight[ligNo].color,
            normal,
            toEye);
    }

    // step-8 ポイントライトを計算する
    for(int ligNo = 0; ligNo < NUM_POINT_LIGHT; ligNo++)
    {
        // 拡散反射を計算
        // 1. 光源からサーファイスに入射するベクトルを計算
        float3 ligDir = normalize(worldPos - pointLight[ligNo].position);
        // 2. 光源からサーフェイスまでの距離を計算
        float distance = length(worldPos - pointLight[ligNo].position);
        // 3. 影響率を計算する影響率は0.0～1.0の範囲で、
        //    指定した距離（pointsLights[i].range）を超えたら、影響率は0.0になる
        float affect = 1.0f - min(1.0f, distance / pointLight[ligNo].range);
        // 4. 拡散反射光を加算
        lig += CalcLambertReflection(
            ligDir,
            pointLight[ligNo].color,
            normal) * affect;

        // スペキュラ反射を加算
        lig += CalcSpecularReflection(
            ligDir,
            pointLight[ligNo].color,
            normal,
            toEye) * affect;
    }

    float4 finalColor = albedo;
    finalColor.xyz *= lig;
    return finalColor;
}
