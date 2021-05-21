#include "stdafx.h"
#include "system/system.h"
#include <random>
#include "util/stopwatch.h"

/////////////////////////////////////////////////////////////////
// 定数
/////////////////////////////////////////////////////////////////
const int TILE_WIDTH = 16;      // タイルの幅
const int TILE_HEIGHT = 16;     // タイルの高さ
const int NUM_TILE = (FRAME_BUFFER_W / TILE_WIDTH) * (FRAME_BUFFER_H / TILE_HEIGHT); // タイルの数

/////////////////////////////////////////////////////////////////
// 構造体
/////////////////////////////////////////////////////////////////
// ディレクションライト構造体
struct alignas(16) DirectionalLight
{
    Vector3  color;     // ライトのカラー
    float pad0;
    Vector3  direction; // ライトの方向
};

// ポイントライト構造体
struct alignas(16) PointLight
{
    Vector3 position;       // 座標
    float pad0;
    Vector3 positionInView; // カメラ空間での座標
    float pad1;
    Vector3 color;          // ライトのカラー
    float range;            // ライトの影響を与える範囲
};

// ライトカリングで使用するカメラ情報
struct alignas(16) LightCullingCameraData
{
    Matrix mProj;           // プロジェクション行列
    Matrix mProjInv;        // プロジェクション行列の逆行列
    Matrix mCameraRot;      // カメラの回転行列
    Vector4 screenParam;    // スクリーン情報
};

const int MAX_POINT_LIGHT = 1000;   // ポイントライトの最大数
const int NUM_DIRECTION_LIGHT = 4;  // ディレクションライトの数

// ライト構造体
struct Light
{
    DirectionalLight directionLights[ NUM_DIRECTION_LIGHT]; // ディレクションライト
    PointLight pointLights[MAX_POINT_LIGHT];                // ポイントライト
    Matrix mViewProjInv;                                    // ビュープロジェクション行列の逆行列
    Vector3 eyePos;                                         // 視点
    float specPow;                                          // スペキュラの絞り
    int numPointLight;                                      // ポイントライトの数
};

/////////////////////////////////////////////////////////////////
//関数宣言
/////////////////////////////////////////////////////////////////
void InitRootSignature(RootSignature& rs);
void InitStandardIOConsole();
void InitLight(Light& light);
void InitModel(Model& teapotModel, Model& bgModel);
void InitDefferedLightingSprite(Sprite& defferedSprite, RenderTarget* gbuffers[], int numGbuffer, Light& light, IShaderResource& srv);
void InitGBuffers(RenderTarget& albedoRT, RenderTarget& normalRT, RenderTarget& depthRT);
void RenderGBuffer(RenderContext& renderContext, RenderTarget* gbuffers[], int numGbuffer, Model& teapotModel, Model& bgModel);
void InitPipelineState(RootSignature& rs, PipelineState& pipelineState, Shader& cs);
void DefferedLighting(RenderContext& renderContext, Sprite& defferedSprite);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
    // 標準入出力コンソールの初期化
    InitStandardIOConsole();

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////
    g_camera3D->SetPosition({ 0.0f, 200.0, 400.0f });
    g_camera3D->Update();

    // ルートシグネチャを作成
    RootSignature rootSignature;
    InitRootSignature(rootSignature);

    // ライトを初期化
    Light light;
    InitLight(light);

    // ティーポットのモデルと背景モデルを初期化
    Model teapotModel, bgModel;
    InitModel(teapotModel, bgModel);

    // G-Bufferを作成
    RenderTarget albedoRT;
    RenderTarget normalRT;
    RenderTarget depthRT;
    RenderTarget* gbuffers[] = {
        &albedoRT,      // 0番目のレンダリングターゲット
        &normalRT,      // 1番目のレンダリングターゲット
        &depthRT        // 2番目のレンダリングターゲット
    };
    InitGBuffers(albedoRT, normalRT, depthRT);

    // step-1 ライトカリング用のコンピュートシェーダーをロード

    // step-2 ライトカリング用のパイプラインステートを初期化

    // step-3 タイルごとのポイントライトの番号のリストを出力するUAVを初期化

    // step-4 ポイントライトの情報を送るための定数バッファを作成

    // step-5 ライトカリング用のディスクリプタヒープを作成

    // ポストエフェクト的にディファードライティングを行うためのスプライトを初期化
    Sprite defferdLightingSpr;
    InitDefferedLightingSprite(
        defferdLightingSpr,
        gbuffers,
        ARRAYSIZE(gbuffers),
        light,
        pointLightNoListInTileUAV
    );

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Stopwatch sw;

    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        sw.Start();

        // レンダリング開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////
        // ライトを回す
        Quaternion qRot;
        qRot.SetRotationDegY(1.0f);
        Matrix mView = g_camera3D->GetViewMatrix();
        for (int i = 0; i < light.numPointLight; i++)
        {
            auto& pt = light.pointLights[i];
            qRot.Apply(pt.position);
            pt.positionInView = pt.position;
            mView.Apply(pt.positionInView);
        }

        // G-Bufferにレンダリング
        RenderGBuffer(renderContext, gbuffers, ARRAYSIZE(gbuffers), teapotModel, bgModel);

        // step-6 ライトカリングのコンピュートシェーダーをディスパッチ

        // リソースバリア
        renderContext.TransitionResourceState(
            pointLightNoListInTileUAV.GetD3DResoruce(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // ディファードライティング
        DefferedLighting(renderContext, defferdLightingSpr);

        renderContext.TransitionResourceState(
            pointLightNoListInTileUAV.GetD3DResoruce(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        /////////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed() );
    }

    ::FreeConsole();
    return 0;
}

// ルートシグネチャの初期化
void InitRootSignature(RootSignature& rs)
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

/// <summary>
// / パイプラインステートの初期化
/// </summary>
/// <param name="rs"></param>
/// <param name="pipelineState"></param>
/// <param name="cs"></param>
void InitPipelineState(RootSignature& rs, PipelineState& pipelineState, Shader& cs)
{
    // パイプラインステートを作成
    D3D12_COMPUTE_PIPELINE_STATE_DESC  psoDesc = { 0 };
    psoDesc.pRootSignature = rs.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetCompiledBlob());
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psoDesc.NodeMask = 0;

    pipelineState.Init(psoDesc);
}

/// <summary>
/// 標準入出力コンソールを初期化
/// </summary>
void InitStandardＩＯConsole()
{
    ::AllocConsole();               // コマンドプロンプトが表示される
    freopen("CON", "w", stdout);    // 標準出力の割り当て

    auto fhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT rc;

    rc.Top = 0;
    rc.Left = 0;
    rc.Bottom = 30;
    rc.Right = 30;
    ::SetConsoleWindowInfo(fhandle, TRUE, &rc);
}

/// <summary>
/// ライトを初期化
/// </summary>
/// <param name="light"></param>
void InitLight(Light& light)
{
    std::random_device seed_gen;
    std::mt19937 random(seed_gen());

    light.eyePos = g_camera3D->GetPosition();
    light.specPow = 5.0f;
    light.mViewProjInv.Inverse(g_camera3D->GetViewProjectionMatrix());

    // ディレクションライトを初期化
    light.directionLights[0].direction.Set(1.0f, 0.0f, 0.0f);
    light.directionLights[0].color.Set(0.5f, 0.5f, 0.5f);

    light.directionLights[1].direction.Set(-1.0f, 0.0f, 0.0f);
    light.directionLights[1].color.Set(0.5f, 0.0f, 0.0f);

    light.directionLights[2].direction.Set(0.0f, 0.0f, 1.0f);
    light.directionLights[2].color.Set(0.0f, 0.5f, 0.0f);

    light.directionLights[3].direction.Set(0.0f, -1.0f, 0.0f);
    light.directionLights[3].color.Set(0.0f, 0.0f, 0.5f);

    // ポイントライトを初期化
    light.numPointLight = 1000;
    Matrix mView = g_camera3D->GetViewMatrix();
    for (int i = 0; i < light.numPointLight; i++)
    {
        auto& pt = light.pointLights[i];
        pt.position.x = static_cast<float>(random() % 1000) - 500.0f;
        pt.position.y = 20.0f; // 高さは20固定
        pt.position.z = static_cast<float>(random() % 1000) - 500.0f;
        pt.positionInView = pt.position;
        mView.Apply(pt.positionInView);

        pt.range = 50.0f;       // 影響範囲も50で固定しておく
        pt.color.x = static_cast<float>(random() % 255) / 255.0f;
        pt.color.y = static_cast<float>(random() % 255) / 255.0f;
        pt.color.z = static_cast<float>(random() % 255) / 255.0f;
    }
}

/// <summary>
/// モデルの初期化
/// </summary>
/// <param name="teapotModel"></param>
/// <param name="bgModel"></param>
void InitModel(Model& teapotModel, Model& bgModel)
{
    // ティーポットモデルを初期化
    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";
    teapotModelInitData.m_fxFilePath = "Assets/shader/renderGBuffer.fx";
    teapotModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    teapotModelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    teapotModelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32_FLOAT;
    teapotModel.Init(teapotModelInitData);

    // 背景のモデルを初期化
    ModelInitData bgModelInitData;

    // ユーザー拡張データとしてポイントライトのリストを渡す
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg.tkm";
    bgModelInitData.m_fxFilePath = "Assets/shader/renderGBuffer.fx";
    bgModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    bgModelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    bgModelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32_FLOAT;
    bgModel.Init(bgModelInitData);
}

/// <summary>
/// ディファードライティング用のスプライトを初期化
/// </summary>
/// <param name="defferedSprite"></param>
void InitDefferedLightingSprite(
    Sprite& defferedSprite,
    RenderTarget* gbuffers[],
    int numGbuffer,
    Light& light,
    IShaderResource& srv)
{
    SpriteInitData spriteInitData;

    // 画面全体にレンダリングするので幅と高さはフレームバッファの幅と高さと同じ
    spriteInitData.m_width = FRAME_BUFFER_W;
    spriteInitData.m_height = FRAME_BUFFER_H;
    spriteInitData.m_expandShaderResoruceView = &srv;

    // ディファードライティングで使用するテクスチャを設定
    for (int i = 0; i < numGbuffer; i++)
    {
        spriteInitData.m_textures[i] = &gbuffers[i]->GetRenderTargetTexture();
    }

    spriteInitData.m_fxFilePath = "Assets/shader/defferedLighting.fx";
    spriteInitData.m_expandConstantBuffer = &light;
    spriteInitData.m_expandConstantBufferSize = sizeof(light);
    defferedSprite.Init(spriteInitData);
}

/// <summary>
/// G-Bufferの初期化
/// </summary>
/// <param name="albedoRT"></param>
/// <param name="normalRT"></param>
/// <param name="depthRT"></param>
void InitGBuffers(RenderTarget& albedoRT, RenderTarget& normalRT, RenderTarget& depthRT)
{
    // アルベドカラー出力用のレンダリングターゲット
    albedoRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT);

    // 法線出力用のレンダリングターゲット
    normalRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_UNKNOWN);

    // Z値出力用のレンダリングターゲット
    depthRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN);
}

/// <summary>
/// GBufferにレンダリング
/// </summary>
/// <param name="renderContext"></param>
/// <param name="gbuffers"></param>
/// <param name="numGbuffer"></param>
/// <param name="ladyModel"></param>
/// <param name="bgModel"></param>
void RenderGBuffer(RenderContext& renderContext, RenderTarget* gbuffers[], int numGbuffer, Model& teapotModel, Model& bgModel)
{
    // まず、レンダリングターゲットとして設定できるようになるまで待つ
    renderContext.WaitUntilToPossibleSetRenderTargets(numGbuffer, gbuffers);

    // レンダリングターゲットを設定
    renderContext.SetRenderTargets(numGbuffer, gbuffers);

    // レンダリングターゲットをクリア
    renderContext.ClearRenderTargetViews(numGbuffer, gbuffers);

    teapotModel.Draw(renderContext);
    bgModel.Draw(renderContext);

    // レンダリングターゲットへの書き込み待ち
    renderContext.WaitUntilFinishDrawingToRenderTargets(numGbuffer, gbuffers);
}

/// <summary>
/// ディファードライティング
/// </summary>
/// <param name="renderContext"></param>
/// <param name="defferedSprite"></param>
void DefferedLighting(RenderContext& renderContext, Sprite& defferedSprite)
{
    // レンダリング先をフレームバッファに戻してスプライトをレンダリングする
    g_graphicsEngine->ChangeRenderTargetToFrameBuffer(renderContext);

    // G-Bufferの内容を元にしてスプライトをレンダリング
    defferedSprite.Draw(renderContext);
}
