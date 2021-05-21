
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
    float3 color;            // カラー
};

Texture2D<float4> g_albedoTexture : register(t1);           // アルベドマップ
StructuredBuffer<SVertex> g_vertexBuffers : register(t6);   // 頂点バッファー
StructuredBuffer<int> g_indexBuffers : register(t7);        // インデックスバッファー

SamplerState  g_samplerState : register(s0);                // サンプラーステート

/////////////////////////////////////////////////////////////////
// レイと最も近い三角形ポリゴンと衝突したときに呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    // step-1 衝突したポリゴンの番号からポリゴンを構成する頂点番号を取得する

    // step-2 頂点番号から各頂点のUV座標を取得する

    // step-3 各頂点のUV座標と重心座標を使って、衝突点のUV座標を求める

    // step-4 求めたUV座標を使ってテクスチャカラーをサンプリングする

}

/////////////////////////////////////////////////////////////////
// レイが三角形ポリゴンと衝突しなかった時に呼ばれるシェーダー
/////////////////////////////////////////////////////////////////
[shader("miss")]
void miss(inout RayPayload payload)
{
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
    TraceRay(g_raytracingWorld, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);

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
