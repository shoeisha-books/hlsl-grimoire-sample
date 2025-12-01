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

// ライトデータにアクセスするための定数バッファーを用意する
cbuffer DirectionLightCb : register(b1)
{
    // ディレクションライト用のデータ
    float3 dirDirection;    // ライトの方向
    float3 dirColor;        // ライトのカラー

    // step-6 定数バッファーにポイントライト用の変数を追加
    float3 ptPosition; // ポイントライトの位置
    float3 ptColor; // ポイントライトのカラー
    float3 ptRange; // ポイントライトの影響範囲
    
    float3 eyePos;          // 視点の位置
    float3 ambientLight;    // アンビエントライト
};

///////////////////////////////////////////
// 関数宣言
///////////////////////////////////////////
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal);

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
    // ディレクションライトによるLambert拡散反射光を計算する
    float3 diffDirection = CalcLambertDiffuse(dirDirection, dirColor, psIn.normal);
    // ディレクションライトによるPhong鏡面反射光を計算する
    float3 specDirection = CalcPhongSpecular(dirDirection, dirColor, psIn.worldPos, psIn.normal);

    // ポイントライトによるLambert拡散反射光とPhong鏡面反射光を計算する
    // step-7 サーフェイスに入射するポイントライトの光の向きを計算する
    float3 ligDir = psIn.worldPos - ptPosition;
    
    //正規化して大きさ１のベクトルにする
    ligDir = normalize(ligDir);

    // step-8 減衰なしのLambert拡散反射光を計算する
    float3 diffPoint = CalcLambertDiffuse(
    ligDir, // ライトの方向
    ptColor, // ライトのカラー
    psIn.normal // ピクセルの法線
    );

    // step-9 減衰なしのPhong鏡面反射光を計算する
    float3 specPoint = CalcPhongSpecular(
        ligDir, // ライトの方向
        ptColor, // ライトのカラー
        psIn.worldPos, // サーフェイスのワールド座標
        psIn.normal // ピクセルの法線
    );

    // step-10 距離による影響率を計算する
    //ポイントライトとの距離を計算する
    float distance = length(psIn.worldPos - ptPosition);
    
    //影響率は距離に比例して小さくなっていく
    float affect = 1.0f - 1.0f / ptRange * distance;
    
    //影響力がマイナスにならないように補正をかける
    if(affect < 0.0f)
    {
        affect = 0.0f;
    }
    
    //影響を指数関数的にする。今回は3乗している
    affect = pow(affect, 3.0f);
    // step-11 拡散反射光と鏡面反射光に影響率を乗算して影響を弱める
    diffPoint *= affect;
    specPoint *= affect;

    // step-12 2つの反射光を合算して最終的な反射光を求める
    float3 diffuseLig = diffDirection + diffPoint;
    float3 specularLig = specPoint + specDirection;

    // 拡散反射光と鏡面反射光を足し算して、最終的な光を求める
    float3 lig = diffuseLig + specularLig + ambientLight;
    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);

    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= lig;

    return finalColor;
}

/// <summary>
/// Lambert拡散反射光を計算する
/// </summary>
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal)
{
    // ピクセルの法線とライトの方向の内積を計算する
    float t = dot(normal, lightDirection) * -1.0f;

    // 内積の値を0以上の値にする
    t = max(0.0f, t);

    // 拡散反射光を計算する
    return lightColor * t;
}

/// <summary>
/// Phong鏡面反射光を計算する
/// </summary>
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal)
{
    // 反射ベクトルを求める
    float3 refVec = reflect(lightDirection, normal);

    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - worldPos;
    toEye = normalize(toEye);

    // 鏡面反射の強さを求める
    float t = dot(refVec, toEye);

    // 鏡面反射の強さを0以上の数値にする
    t = max(0.0f, t);

    // 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    // 鏡面反射光を求める
    return lightColor * t;
}
