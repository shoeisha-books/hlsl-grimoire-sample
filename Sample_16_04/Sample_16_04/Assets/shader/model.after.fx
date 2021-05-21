/*!
 * @brief シンプルなモデル表示用のシェーダー
 */

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

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
    float4 worldPos : TEXCOORD1;
};

static const int NUM_DIRECTION_LIGHT = 4;   // ディレクションライトの数
static const int MAX_POINT_LIGHT = 1000;    // ポイントライトの最大数

// ディレクションライト
struct DirectionLight
{
    float3 color;       // ライトのカラー
    float3 direction;   // ライトの方向
};

// ポイントライト
struct PointLight
{
    float3 position;        // 座標
    float3 positionInView;  // カメラ空間での座標
    float3 color;           // カラー
    float range;            // 範囲
};

cbuffer Light : register(b1)
{
    DirectionLight directionLight[NUM_DIRECTION_LIGHT];
    PointLight pointLight[MAX_POINT_LIGHT];
    float4x4 mViewProjInv;  // ビュープロジェクション行列の逆行列
    float4 screenParam;     // スクリーン情報
    float3 eyePos;          // 視点
    float specPow;          // スペキュラの絞り
    int numPointLight;      // ポイントライトの数
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// タイルごとのポイントライトのインデックスのリスト
StructuredBuffer<uint> pointLightListInTile : register(t10);

// サンプラーステート
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.worldPos = mul(mWorld, vsIn.pos);  // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.worldPos);   // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);        // カメラ座標系からスクリーン座標系に変換
    psIn.normal = normalize(mul(mWorld, vsIn.normal));
    psIn.uv = vsIn.uv;
    return psIn;
}

/*!
 *@brief Lambert拡散反射を計算
 *@param ligDir：サーフェイスに入射してくるライトの方向
 *@param lightColor：ライトのカラー
 *@param normal：サーフェイスの法線
 *@return 拡散反射の光
 */
float3 CalcLambertReflection(float3 ligDir, float3 ligColor, float3 normal)
{
    float t = max(0.0f, dot(normal, ligDir) * -1.0f);
    return ligColor * t;
}

/*!
 *@brief スペキュラ反射を計算
 *@param ligDir：ライトの方法
 *@param ligColor：ライトのカラー
 *@param normal：サーフェイスの法線
 *@param toEye：サーフェイスから視点へのベクトル
 *@return スペキュラ反射光
 */
float3 CalcSpecularReflection(float3 ligDir, float3 ligColor, float3 normal, float3 toEye)
{
    float3 r = reflect(ligDir, normal);
    float t = max(0.0f, dot(toEye, r));
    t = pow(t, specPow);
    return ligColor * t;
}

/*!
 *@brief ディレクションライトを計算
 */
float3 CalcDirectionLight(SPSIn psIn)
{
    float3 lig = 0.0f;
    float3 toEye = normalize(eyePos - psIn.worldPos.xyz);
    for(int ligNo; ligNo < NUM_DIRECTION_LIGHT; ligNo++)
    {
        // 拡散反射光を計算
        lig += CalcLambertReflection(
            directionLight[ligNo].direction,
            directionLight[ligNo].color,
            psIn.normal);

        // スペキュラ反射を計算
        lig += CalcSpecularReflection(
            directionLight[ligNo].direction,
            directionLight[ligNo].color,
            psIn.normal,
            toEye
        );
    }
    return lig;
}

/*!
 *@brief ポイントライトの影響を計算する
 */
float3 CalcPointLight(SPSIn psIn)
{
    // step-5 作成したライトのリストを使って、ポイントライトを計算する
    // タイルの幅と高さ
    const int TILE_WIDTH = 16;
    const int TILE_HEIGHT = 16;

    // スクリーンの左上を(0,0)、右下を(1,1)とする座標系に変換する
    // ビューポート座標系に変換する
    float2 viewportPos = psIn.pos.xy;

    // スクリーンをタイルで分割したときのセルのX座標を求める
    uint numCellX = (screenParam.z + TILE_WIDTH - 1) / TILE_WIDTH;

    // タイルインデックスを計算する
    uint tileIndex = floor(viewportPos.x / TILE_WIDTH) + floor(viewportPos.y / TILE_WIDTH) * numCellX;

    // このピクセルが含まれるタイルのライトインデックスリストの開始位置を計算する
    uint lightStart = tileIndex * numPointLight;

    // このピクセルが含まれるタイルのライトインデックスリストの終了位置を計算する
    uint lightEnd = lightStart + numPointLight;

    float3 lig = 0.0f;
    float3 toEye = normalize(eyePos - psIn.worldPos.xyz);
    for (uint lightListIndex = lightStart;
        lightListIndex < lightEnd;
        lightListIndex++)
    {
        uint ligNo = pointLightListInTile[lightListIndex];
        if (ligNo == 0xffffffff)
        {
            // このタイルに含まれるポイントライトはもうない
            break;
        }
        // 拡散反射を計算
        // 1. 光源からサーファイスに入射するベクトルを計算
        float3 ligDir = normalize(psIn.worldPos - pointLight[ligNo].position);

        // 2. 光源からサーフェイスまでの距離を計算
        float distance = length(psIn.worldPos - pointLight[ligNo].position);

        // 3. 影響率を計算する。影響率は0.0～1.0の範囲で、
        //    指定した距離（pointsLights[i].range）を超えたら、影響率は0.0になる
        float affect = 1.0f - min(1.0f, distance / pointLight[ligNo].range);

        // 4. 拡散反射光を加算
        lig += CalcLambertReflection(
            ligDir,
            pointLight[ligNo].color,
            psIn.normal) * affect;

        // スペキュラ反射を加算
        lig += CalcSpecularReflection(
            ligDir,
            pointLight[ligNo].color,
            psIn.normal,
            toEye) * affect;
    }
    return lig;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float4 albedo = g_texture.Sample(g_sampler, psIn.uv);

    //ライティングを計算
    float3 lig  = 0.0f;

    // ディレクションライトの影響を計算する
    lig += CalcDirectionLight(psIn);

    // ポイントライトの影響を計算する
    lig += CalcPointLight(psIn);

    float4 finalColor;
    finalColor = albedo;
    finalColor.xyz *= lig;
    return finalColor;
}
