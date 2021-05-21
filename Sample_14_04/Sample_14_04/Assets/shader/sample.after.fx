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
    float distToEye     : TEXCOORD1;    //視点との距離
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
    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    psIn.uv = vsIn.uv;

    // step-1 オブジェクトとカメラとの距離を求める
    // オブジェクトの座標をワールド行列の平行移動成分から取得する
    float4 objectPos = mWorld[3];

    // オブジェクトの座標をカメラ座標系に変換する
    float4 objectPosInCamera = mul(mView, objectPos);

    // カメラからの距離を計算する
    psIn.distToEye = length(objectPosInCamera );

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // このピクセルのX座標、Y座標を4で割った余りを求める
    int x = (int)fmod(psIn.pos.x, 4.0f);
    int y = (int)fmod(psIn.pos.y, 4.0f);

    // 上で求めた、xとyを利用して、このピクセルのディザリング閾値を取得する
    int dither = pattern[y][x];

    // step-2 完全にクリップされる範囲を定義する
    // カメラがオブジェクトのクリップ範囲内に入ると、
    // 完全にオブジェクトがクリップされる
    // （この数値を変更すると、オブジェクトが完全に消える範囲が変わる）
    float clipRange = 50.0f;

    // step-3 視点とクリップ範囲までの距離を計算する
    // オブジェクトとカメラの距離が50以下になると、
    // psIn.distToEye - clipRangeの結果がマイナスになるので、tに0が代入される
    // psIn.distToEyeが50以上なら、tには視点からクリップ範囲までの距離が
    // 計算される
    float eyeToClipRange = max(0.0f, psIn.distToEye - clipRange);

    // step-4 クリップ率を求める
    // clipRateは0～1の値を取り、clipRateが1になると完全にクリップされる
    // 下記のコードは、視点とクリップ範囲の距離が100以下になると、
    // 線形にclipRateの1に近づいていき、eyeToClipRangeが0になると、
    // clipRateが1になる計算になっている
    float clipRate = 1.0f - min(1.0f, eyeToClipRange / 100.0f);

    // step-5 clipRateを利用してピクセルキルを行う
    // tの値は0～1の値をとる。tが0ならどのピクセルもクリップされない
    // tの値が1になると、64以下のピクセルがクリップされるため、
    // すべてのピクセルがキリップされる（ditherの値の最大値は62なので）
    clip(dither - 64 * clipRate);

    float4 tex = g_texture.Sample(g_sampler, psIn.uv);
    return tex;
}
