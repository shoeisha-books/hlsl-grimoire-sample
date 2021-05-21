
/////////////////////////////////////////////////////////////////
// 構造体定義
/////////////////////////////////////////////////////////////////

// 頂点構造体
// フォーマットはTkmFile.hのSVertexと同じになっている必要がある
struct SVertex
{
    float3 pos;
    float3 normal;
    float3 tangent;
    float3 binormal;
    float2 uv;
    int4 indices;
    float4 skinweigths;
};

// closesthitシェーダー、missシェーダーに渡される引数構造体
struct RayPayload
{
    float3 color;   // カラー
    int hit;        // 衝突フラグ
    int reflection; // リフレクションフラグ
};

/////////////////////////////////////////////////////////////////
// リソース
/////////////////////////////////////////////////////////////////
RaytracingAccelerationStructure g_raytracingWorld : register(t0);   // レイトレワールド
Texture2D<float4> g_albedoTexture : register(t1);                   // アルベドマップ
StructuredBuffer<SVertex> g_vertexBuffers : register(t6);           // 頂点バッファー
StructuredBuffer<int> g_indexBuffers : register(t7);                // インデックスバッファー

SamplerState  g_samplerState : register(s0);                        // サンプラーステート

/////////////////////////////////////////////////////////////////
// 関数宣言
/////////////////////////////////////////////////////////////////
float2 GetUV(BuiltInTriangleIntersectionAttributes attribs);
float3 GetNormal(BuiltInTriangleIntersectionAttributes attribs);
void TraceShadowRay(inout RayPayload payload);

/////////////////////////////////////////////////////////////////
// シャドウレイと最も近い三角形ポリゴンと衝突したときに呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("closesthit")]
void shadowChs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = 1;
}
/////////////////////////////////////////////////////////////////
// シャドウレイが三角形ポリゴンと衝突しなかった時に呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("miss")]
void shadowMiss(inout RayPayload payload)
{
    payload.hit = 0;
}

/////////////////////////////////////////////////////////////////
// レイと最も近い三角形ポリゴンと衝突したときに呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    // 衝突点のUV座標を求める
    float2 uv = GetUV(attribs);

    // 求めたUV座標を使ってテクスチャカラーをサンプリングする
    payload.color = g_albedoTexture.SampleLevel(g_samplerState,uv,0.0f);

    // step-1 カメラレイがポリゴンと衝突した位置を計算する
    // 1. ぶつかったレイの方向ベクトルを取得
    float3 rayDirW = WorldRayDirection();

    // 2. レイを飛ばした座標を取得。今回であればカメラの視点
    float3 rayOriginW = WorldRayOrigin();

    // 3. レイを飛ばした場所から、衝突した点までの距離を取得
    float hitT = RayTCurrent();

    // 1.、2.、3.の情報から衝突した座標を求める
    float3 hitPos = rayOriginW + rayDirW * hitT;

    // step-2 反射ベクトルを計算する
    // 法線を取得
    float3 normal = GetNormal(attribs);

    // reflect関数を利用して反射ベクトルを計算
    float3 refDir = reflect(rayDirW, normal);

    // step-3 レイを作る
    RayDesc ray;

    // レイの射出位置はカメラレイとポリゴンの衝突点
    ray.Origin = hitPos;

    // レイの方向は反射ベクトル
    ray.Direction = refDir;
    ray.TMin = 0.01f;
    ray.TMax = 10000;

    // step-4 レイを飛ばす
    if(payload.reflection == 0)
    {
        // このレイがリフレクションレイでないなら
        RayPayload reflectionPayload;
        reflectionPayload.color = 0.0f;

        // このレイはリフレクションレイなのでフラグを立てる
        reflectionPayload.reflection = 1;
        TraceRay(
            g_raytracingWorld,
            0,
            0xFF,
            0,          //ヒットグループのオフセット番号が0
                        //つまり、ポリゴンと衝突するとchs関数が呼ばれる！
            0,
            1,
            ray,
            reflectionPayload
        );

        // step-5 反射カラーの合成
        payload.color = payload.color * 0.7f + reflectionPayload.color * 0.3f;
    }
}

/////////////////////////////////////////////////////////////////
// レイが三角形ポリゴンと衝突しなかった時に呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("miss")]
void miss(inout RayPayload payload)
{
    // レイがポリゴンが衝突しなかった時のカラーを計算する
    payload.color = float3(1.0f, 0.0f, 0.0f);
}

// カメラ構造体
// 定数バッファーなので16バイトアライメントに気を付けること
struct Camera
{
    float4x4 mCameraRot;    // カメラの回転行列
    float3 pos;             // カメラ座標
    float aspect;           // アスペクト比
    float far;              // 遠平面
    float near;             // 近平面
};

cbuffer rayGenCB :register(b0)
{
    Camera g_camera; // カメラ
};

RWTexture2D<float4> gOutput : register(u0); // カラー出力先

[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    RayDesc ray;
    ray.Origin = g_camera.pos;
    ray.Direction = normalize(float3(d.x * g_camera.aspect, -d.y, -1));
    ray.Direction = mul(g_camera.mCameraRot, ray.Direction);

    ray.TMin = 0;
    ray.TMax = 10000;

    RayPayload payload;
    payload.reflection = 0;
    TraceRay(g_raytracingWorld, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);

    float3 col = payload.color;

    gOutput[launchIndex.xy] = float4(col, 1);
}

// UV座標を取得
float2 GetUV(BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    // プリミティブIDを取得
    uint primID = PrimitiveIndex();

    // プリミティブIDから頂点番号を取得する
    uint v0_id = g_indexBuffers[primID * 3];
    uint v1_id = g_indexBuffers[primID * 3 + 1];
    uint v2_id = g_indexBuffers[primID * 3 + 2];
    float2 uv0 = g_vertexBuffers[v0_id].uv;
    float2 uv1 = g_vertexBuffers[v1_id].uv;
    float2 uv2 = g_vertexBuffers[v2_id].uv;

    float2 uv = barycentrics.x * uv0 + barycentrics.y * uv1 + barycentrics.z * uv2;
    return uv;
}

// シャドウレイを飛ばす
void TraceShadowRay(inout RayPayload payload)
{
    // 1. ぶつかったレイの方向ベクトルを取得
    float3 rayDirW = WorldRayDirection();

    // 2. レイを飛ばした座標を取得。今回であればカメラの視点
    float3 rayOriginW = WorldRayOrigin();

    // 2. レイを飛ばした場所から、衝突した点までの距離を取得
    float hitT = RayTCurrent();

    // 1.、2.、3.の情報から衝突した座標を求める
    float3 hitPos = rayOriginW + rayDirW * hitT;

    // シャドウレイを作る
    RayDesc ray;

    // シャドウレイを飛ばす場所
    ray.Origin = hitPos;

    // ライトまでのベクトル。今回のハンズオンでは固定
    ray.Direction = float3(0.5f, 0.5f, 0.2f);

    // 正規化しておく
    ray.Direction = normalize(ray.Direction);

    // レイの最小距離と最大距離
    ray.TMin = 0.01f;
    ray.TMax = 100;

    // シャドウレイを飛ばす
    payload.hit = 0;
    TraceRay(
        g_raytracingWorld,  // レイトレワールド
        0,                  //
        0xFF,
        1,                  // ヒットグループのオフセット番号
        0,
        1,                  // ミスシェーダーの番号
        ray,                // レイ
        payload
    );
}

// 法線を取得
float3 GetNormal(BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    // プリミティブIDを取得
    uint primID = PrimitiveIndex();

    // プリミティブIDから頂点番号を取得する
    uint v0_id = g_indexBuffers[primID * 3];
    uint v1_id = g_indexBuffers[primID * 3 + 1];
    uint v2_id = g_indexBuffers[primID * 3 + 2];

    float3 normal0 = g_vertexBuffers[v0_id].normal;
    float3 normal1 = g_vertexBuffers[v1_id].normal;
    float3 normal2 = g_vertexBuffers[v2_id].normal;
    float3 normal = barycentrics.x * normal0 + barycentrics.y * normal1 + barycentrics.z * normal2;
    normal = normalize(normal);

    // ワールド空間に変換するのが面倒なので、超適当に……さーせん
    float cs = cos(-1.57f);
    float sn = sin(-1.57f);
    float4x4 m;
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = cs;
    m[1][2] = sn;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = -sn;
    m[2][2] = cs;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;

    m = transpose(m);
    normal = mul(m, normal);

    return normal;
}
