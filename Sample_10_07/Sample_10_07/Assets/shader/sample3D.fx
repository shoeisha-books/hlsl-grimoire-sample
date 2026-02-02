/*!
 * @brief   ディズニーベースの物理ベースシェーダー
 */

///////////////////////////////////////////////////
// 構造体
///////////////////////////////////////////////////

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
    float4 pos : POSITION; // モデルの頂点座標
    float3 normal : NORMAL; // 法線
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0; // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION; // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL; // 法線
    float3 tangent : TANGENT; // 接ベクトル
    float3 biNormal : BINORMAL; // 従法線ベクトル
    float2 uv : TEXCOORD0; // UV座標
    float3 worldPos : TEXCOORD1; // ワールド空間でのピクセルの座標

    // step-7 カメラ空間でのZ値を記録する変数を追加
    float3 depthInView : TEXCOORD2; // カメラ空間でのZ値
};

// step-8 ピクセルシェーダーからの出力構造体を定義する。
struct SPSOut
{
    float4 color : SV_Target0; // レンダリングターゲット0に描きこむ
    float depth : SV_Target1; // レンダリングターゲット1に描きこむ
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////
// サンプラーステート
sampler g_sampler : register(s0);

// 物理ベースの処理をインクルード
#include "preset/pbr.h"

/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);

    // step-9 頂点シェーダーでカメラ空間でのZ値を設定する
    psIn.depthInView = psIn.pos.z;

    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, vsIn.normal));
    psIn.tangent = normalize(mul(mWorld, vsIn.tangent));
    psIn.biNormal = normalize(mul(mWorld, vsIn.biNormal));
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// ピクセルシェーダー
/// </summary>
SPSOut PSMain(SPSIn psIn)
{
    // step-10 ピクセルシェーダーからカラーとZ値を出力する
    SPSOut psOut;

    // カラーを計算。
    psOut.color = CalcPBR(psIn);

    // カメラ空間での深度値を設定
    psOut.depth = psIn.depthInView;
    return psOut;
}
