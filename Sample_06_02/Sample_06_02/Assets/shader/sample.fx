/////////////////////////////////////////////////////////////////////////
// 構造体
/////////////////////////////////////////////////////////////////////////
// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ライト用の定数バッファー
cbuffer LightCb : register(b1)
{
    float3 dirLigDirection; // ライトの方向
    float4 dirLigColor;     // ライトの色
    float3 eyePos;          // カメラの視点
    float specPow;          // スペキュラの絞り
    float3 ambientLight;    // 環境光
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // モデルの頂点座標
    float3 normal : NORMAL; // 法線
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;  // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
    float3 tangent : TANGENT;       // 接ベクトル
    float3 biNormal : BINORMAL;     // 従ベクトル

    float2 uv : TEXCOORD0;          // uv座標
    float3 worldPos : TEXCOORD1;    // ワールド空間でのピクセルの座標
};

/////////////////////////////////////////////////////////////////////////
// 関数宣言
/////////////////////////////////////////////////////////////////////////
float3 CalcNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv);
float3 CalcLambertDiffuse(float3 normal);
float3 CalcPhongSpecular(float3 normal, float3 worldPos);

/////////////////////////////////////////////////////////////////////////
// グローバル変数
/////////////////////////////////////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// 法線マップにアクセスするための変数
Texture2D<float4> g_normalMap : register(t1);

// step-1 スペキュラマップにアクセスするための変数を追加
Texture2D <float4> g_specularMap : register(t2);

// サンプラーステート
sampler g_sampler : register(s0);

/////////////////////////////////////////////////////////////////////////
// 頂点シェーダー
/////////////////////////////////////////////////////////////////////////
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, vsIn.normal));

    // 接ベクトルと従ベクトルをワールド空間に変換する
    psIn.tangent = normalize(mul(mWorld, vsIn.tangent));
    psIn.biNormal = normalize(mul(mWorld, vsIn.biNormal));

    psIn.uv = vsIn.uv;

    return psIn;
}

/////////////////////////////////////////////////////////////////////////
// ピクセルシェーダー
/////////////////////////////////////////////////////////////////////////
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // ディフューズマップをサンプリング
    float4 diffuseMap = g_texture.Sample(g_sampler, psIn.uv);

    // 法線を計算
    float3 normal =  CalcNormal(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);

    // Lambert拡散反射光を計算する
    float3 diffuseLig = CalcLambertDiffuse(normal);

    // Phong鏡面反射を計算
    // このサンプルでは鏡面反射の効果を分かりやすくするために10倍にしている
    float3 specLig = CalcPhongSpecular(normal, psIn.worldPos) * 10.0f;

    // step-2 スペキュラマップからスペキュラ反射の強さをサンプリング
    float specPower = g_specularMap.Sample(g_sampler, psIn.uv).r;

    // step-3 鏡面反射の強さを鏡面反射光に乗算する
    specLig *= specPower * 10.0f;

    // 拡散反射、鏡面反射、環境光を合算して最終的な反射光を計算する
    float3 lig = diffuseLig + specLig + ambientLight;

    float4 finalColor = diffuseMap;
    finalColor.xyz *= lig;

    return finalColor;
}

/////////////////////////////////////////////////////////////////////////
// 法線を計算
/////////////////////////////////////////////////////////////////////////
float3 CalcNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normalMap.SampleLevel (g_sampler, uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

    float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

    return newNormal;
}

/////////////////////////////////////////////////////////////////////////
//  Lambert拡散反射を計算
/////////////////////////////////////////////////////////////////////////
float3 CalcLambertDiffuse(float3 normal)
{
    return max(0.0f, dot(normal, -dirLigDirection)) * dirLigColor;
}

/////////////////////////////////////////////////////////////////////////
//  Phong鏡面反射を計算
/////////////////////////////////////////////////////////////////////////
float3 CalcPhongSpecular(float3 normal, float3 worldPos)
{
    // 反射ベクトルを求める
    float3 refVec = reflect(dirLigDirection, normal);

    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - worldPos;
    toEye = normalize(toEye);

    // 鏡面反射の強さを求める
    float t = saturate(dot(refVec, toEye) );

    // 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    //  鏡面反射光を求める
    float3 specularLig = dirLigColor * t;
    return specularLig;
}
