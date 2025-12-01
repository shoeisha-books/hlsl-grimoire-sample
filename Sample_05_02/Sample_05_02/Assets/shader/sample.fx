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

    // 定数バッファーにポイントライト用の変数を追加
    float3 ptPosition;      // ポイントライトの位置
    float3 ptColor;         // ポイントライトのカラー
    float ptRange;          // ポイントライトの影響範囲

    // step-5 スポットライトのデータにアクセスするための変数を追加する
    float3 spPosition; // スポットライトの位置
    float3 spColor; // スポットライトのカラー
    float spRange; // スポットライトの影響範囲
    float3 spDirection; // スポットライトの方向
    float spAngle; // スポットライトの射出角度

    float3 eyePos;          // 視点の位置
    float3 ambientLight;    // アンビエントライト
};

///////////////////////////////////////////
// 関数宣言
///////////////////////////////////////////
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal);
float3 CalcLigFromPointLight(SPSIn psIn);
float3 CalcLigFromDirectionLight(SPSIn psIn);

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
    // ディレクションライトによるライティングを計算する
    float3 directionLig = CalcLigFromDirectionLight(psIn);

    // ポイントライトによるライティングを計算する
    float3 pointLig = CalcLigFromPointLight(psIn);

    // スポットライトによるライティングを計算する
    // ほとんどポイントライトと同じ

    // step-6 サーフェイスに入射するスポットライトの光の向きを計算する
    //ピクセル座標～スポットライトの座標を計算
    float3 ligDir = psIn.worldPos - spPosition;
    
    //正規化して大きさ１のベクトルにする
    ligDir = normalize(ligDir);

    // step-7 減衰なしのLambert拡散反射光を計算する    
    float3 diffSpotLight = CalcLambertDiffuse(
        ligDir,     // ライトの方向
        spColor,    // ライトのカラー
        psIn.normal // サーフェイスの法線
    );

    // step-8 減衰なしのPhong鏡面反射光を計算する
    float3 specSpotLight = CalcPhongSpecular(
        ligDir,         // ライトの方向
        spColor,        // ライトのカラー
        psIn.worldPos,  // サーフェイズのワールド座標
        psIn.normal     // サーフェイズの法線
    );

    // step-9 距離による影響率を計算する
    //スポットライトとの距離を計算する
    float3 distance = length(psIn.worldPos - spPosition);
    
    //影響力は距離に比例して小さくなっていく
    float affect = 1.0f - 1.0f / spRange * distance;
    
    //影響力がマイナスにならないように補正をかける
    affect = max(0.0f, affect);
    
    //影響の仕方を指数関数的にする。今回のサンプルでは3乗している
    affect = pow(affect, 3.0f);

    // step-10 影響率を乗算して反射光を弱める
    diffSpotLight *= affect;
    specSpotLight *= affect;

    // step-11 入射光と射出方向の角度を求める
    //dot()を利用して内積を求める
    float angle = dot(ligDir, spDirection);
    
    //dot()で求めた値をacos()に渡して角度を求める
    angle = abs(acos(angle));

    // step-12 角度による影響率を求める
    //角度に比例して小さくなっていく影響率を計算する
    affect = 1.0f - 1.0f / spAngle * angle;
    
    //影響力がマイナスにならないように補正をかける
    affect = max(0.0f, affect);
    
    //影響の仕方を指数関数的にする。今回のサンプルでは0.5乗している
    affect = pow(affect, 0.5f);

    // step-13 角度による影響率を反射光に乗算して、影響を弱める
    diffSpotLight *= affect;
    specSpotLight *= affect;

    // ディレクションライト+ポイントライト+環境光を求める
    float3 finalLig = directionLig
                    + pointLig
                    + ambientLight;

    // step-14 スポットライトの反射光を最終的な反射光に足し算する
    finalLig += diffSpotLight + specSpotLight;

    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);

    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= finalLig;

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

/// <summary>
/// ポイントライトによる反射光を計算
/// </summary>
/// <param name="psIn">ピクセルシェーダーに渡されている引数</param>
float3 CalcLigFromPointLight(SPSIn psIn)
{
    // このサーフェイスに入射しているポイントライトの光の向きを計算する
    float3 ligDir = psIn.worldPos - ptPosition;

    // 正規化して大きさ1のベクトルにする
    ligDir = normalize(ligDir);

    // 減衰なしのLambert拡散反射光を計算する
    float3 diffPoint = CalcLambertDiffuse(
        ligDir,     // ライトの方向
        ptColor,    // ライトのカラー
        psIn.normal // サーフェイスの法線
    );

    // 減衰なしのPhong鏡面反射光を計算する
    float3 specPoint = CalcPhongSpecular(
        ligDir,         // ライトの方向
        ptColor,        // ライトのカラー
        psIn.worldPos,  // サーフェイズのワールド座標
        psIn.normal     // サーフェイズの法線
    );

    // 距離による影響率を計算する
    // ポイントライトとの距離を計算する
    float3 distance = length(psIn.worldPos - ptPosition);

    // 影響率は距離に比例して小さくなっていく
    float affect = 1.0f - 1.0f / ptRange * distance;

    // 影響力がマイナスにならないように補正をかける
    if(affect < 0.0f)
    {
        affect = 0.0f;
    }

    // 影響の仕方を指数関数的にする。今回のサンプルでは3乗している
    affect = pow(affect, 3.0f);

    // 拡散反射光と鏡面反射光に減衰率を乗算して影響を弱める
    diffPoint *= affect;
    specPoint *= affect;

    return diffPoint + specPoint;
}

/// <summary>
/// ディレクションライトによる反射光を計算
/// </summary
/// <param name="psIn">ピクセルシェーダーからの入力。</param>
float3 CalcLigFromDirectionLight(SPSIn psIn)
{
    // ディレクションライトによるLambert拡散反射光を計算する
    float3 diffDirection = CalcLambertDiffuse(dirDirection, dirColor, psIn.normal);

    // ディレクションライトによるPhong鏡面反射光を計算する
    float3 specDirection = CalcPhongSpecular(
            dirDirection, dirColor, psIn.worldPos, psIn.normal);
    return diffDirection + specDirection;
}
