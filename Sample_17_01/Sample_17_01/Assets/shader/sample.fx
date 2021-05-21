// closesthitシェーダー、missシェーダーに渡される引数構造体
struct RayPayload
{
    float3 color;            // カラー
};

/////////////////////////////////////////////////////////////////
// レイと最も近い三角形ポリゴンと衝突したときに呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    // step-4 レイと衝突した点の色を計算する
}

/////////////////////////////////////////////////////////////////
// レイが三角形ポリゴンと衝突しなかった時に呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("miss")]
void miss(inout RayPayload payload)
{
    // step-5 レイがポリゴンが衝突しなかった時のカラーを計算する
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

RaytracingAccelerationStructure g_raytracingWorld : register(t0);   // レイトレワールド
RWTexture2D<float4> gOutput : register(u0);                         // カラー出力先

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
    TraceRay(g_raytracingWorld,
        0 /*rayFlags*/,
        0xFF,
        0 /* ray index*/,
        0, 0, ray, payload);

    float3 col = payload.color;

    gOutput[launchIndex.xy] = float4(col, 1);
}

[shader("closesthit")]
void shadowChs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
}

[shader("miss")]
void shadowMiss(inout RayPayload payload)
{
}
