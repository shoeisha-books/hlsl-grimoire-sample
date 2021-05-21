///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;
    float2 uv       : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos          : SV_POSITION;
    float2 uv           : TEXCOORD0;
    float distToEye     : TEXCOORD1;    // 視点との距離
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

///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

///////////////////////////////////////////
// サンプラーステート
///////////////////////////////////////////
sampler g_sampler : register(s0);

// step-1 ディザパターンを定義する
static const int pattern[4][4] = {
    { 0, 32,  8, 40},
    { 48, 16, 56, 24},
    { 12, 44,  4, 36},
    { 60, 28, 52, 20},
};

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    // オブジェクトの座標をワールド行列の平行移動成分から引っ張ってくる
    float4 objectPos = mWorld[3];
    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系から正規化スクリーン座標系に変換
    psIn.uv = vsIn.uv;
    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // step-2 ディザパターンを利用してディザリングを実装する
    // このピクセルのスクリーン座標系でのX座標、Y座標を4で割った余りを求める
    int x = (int)fmod(psIn.pos.x, 4.0f);
    int y = (int)fmod(psIn.pos.y, 4.0f);

    // 上で求めた、xとyを利用して、このピクセルのディザリング閾値を取得する
    int dither = pattern[y][x];

    // ディザリング閾値が50以下のピクセルはピクセルキルする
    clip(dither - 50);

    float4 tex = g_texture.Sample(g_sampler, psIn.uv);
    return tex;
}
