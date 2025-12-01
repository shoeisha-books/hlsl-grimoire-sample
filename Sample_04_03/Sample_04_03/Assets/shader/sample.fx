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

// ディレクションライト用のデータを受け取るための定数バッファーを用意する
cbuffer DirectionLightCb : register(b1)
{
    float3 ligDirection;    // ライトの方向
    float3 ligColor;        // ライトのカラー

    // step-3 視点のデータにアクセスするための変数を定数バッファーに追加する
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
    psIn.worldPos = vsIn.pos;
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
    float t = dot(psIn.normal, ligDirection);
    t *= -1.0f;

    // 内積の結果が0以下なら0にする
    if(t < 0.0f)
    {
        t = 0.0f;
    }

    // 拡散反射光を求める
    float3 diffuseLig = ligColor * t;

    // step-4 反射ベクトルを求める
    float3 refVec = reflect(ligDirection, psIn.normal);

    // step-5 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - psIn.worldPos;
    
    //正規化する
    toEye = normalize(toEye);

    // step-6 鏡面反射の強さを求める
    t = dot(refVec, toEye);
    
    //dot関数を利用してrefVecとtoEyeの内積を計算する
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    // step-7 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    // step-8 鏡面反射光を求める
    float3 specularLig = ligColor * t;

    // step-9 拡散反射光と鏡面反射光を足し算して、最終的な光を求める
    float3 lig = diffuseLig + specularLig;

    // テクスチャからカラーをフェッチする
    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);

    // step-10 テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= lig;
    //finalColor.rgb += specularLig * 3.0f; //鏡面反射を強調する

    return finalColor;
}
