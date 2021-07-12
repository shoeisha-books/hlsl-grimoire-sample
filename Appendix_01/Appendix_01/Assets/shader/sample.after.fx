///////////////////////////////////////////
// 簡易リムライト、ファーシェーダー。
///////////////////////////////////////////

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
    float4 pos          : SV_POSITION;
    float3 normal       : NORMAL;
    float2 uv           : TEXCOORD0;
    float3 worldPos     : TEXCOORD1;
    float3 normalInView : TEXCOORD2; // カメラ空間の法線
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
    float3 eyePos;          // 視点の位置
    float3 ambientLight;    // アンビエントライト
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

    // step-2 カメラ空間の法線を求める
    psIn.normalInView = mul(mView, psIn.normal); // カメラ空間の法線を求める

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // step-1 サーフェイスの法線を利用してリムライトの強さを求める。
    // カメラ空間でのZ値の値を使って、リムライトの強さを計算する。
    float limLight = 1.0f - max(0.0f, psIn.normalInView.z * -1.0f);

    // step-2 pow()を使用して、強さの変化を指数関数的にする
    // 今回は効果を分かりやすくするためにリムの強さを３倍にしている。
    limLight = pow(limLight, 1.5f) * 3.0f;

    // step-3 最終的な反射光にリムライトの反射光を合算する
    // 最終的な反射光にリムの反射光を合算する。
    // 今回のサンプルはリムの効果を分かりやすくするために、、
    // シンプルな環境光とリムライトの効果のみとする
    float3 finalLig = ambientLight + limLight;

    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める。
    // テクスチャカラーをサンプリング。
    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);
    finalColor.xyz *= finalLig;

    return finalColor;
}
