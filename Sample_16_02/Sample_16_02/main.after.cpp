#include "stdafx.h"
#include "system/system.h"
#include <random>
#include "util/stopwatch.h"

//関数宣言
void InitRootSignature(RootSignature& rs);
void InitStandardIOConsole();

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
    Vector3 position;   // 座標
    float pad0;
    Vector3 color;      // ライトのカラー
    float range;        // ライトの影響を与える範囲
};

const int NUM_POINT_LIGHT = 1000;   // ポイントライトの数
const int NUM_DIRECTION_LIGHT = 4;  // ディレクションライトの数

// ライト構造体
struct Light
{
    DirectionalLight directionLights[ NUM_DIRECTION_LIGHT]; // ディレクションライト
    PointLight pointLights[NUM_POINT_LIGHT];                // ポイントライト
    Matrix mViewProjInv;    // ビュープロジェクション行列の逆行列
    Vector3 eyePos;         // 視点
    float specPow;          // スペキュラの絞り
};

///////////////////////////////////////////////////////////////////
//  ウィンドウプログラムのメイン関数
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
    std::random_device seed_gen;
    std::mt19937 random(seed_gen());

    g_camera3D->SetPosition({ 0.0f, 200.0, 400.0f });
    g_camera3D->Update();

    // ルートシグネチャを作成
    RootSignature rootSignature;
    InitRootSignature(rootSignature);

    Light light;

    // ライトの共通パラメータを設定
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
    for (auto& pt : light.pointLights)
    {
        pt.position.x = static_cast<float>(random() % 1000) - 500.0f;
        pt.position.y = 20.0f;  // 高さは20固定
        pt.position.z = static_cast<float>(random() % 1000) - 500.0f;
        pt.range = 50.0f;       // 影響範囲も50で固定しておく
        pt.color.x = static_cast<float>(random() % 255) / 255.0f;
        pt.color.y = static_cast<float>(random() % 255) / 255.0f;
        pt.color.z = static_cast<float>(random() % 255) / 255.0f;
    }

    // ティーポットモデルを初期化
    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";
    teapotModelInitData.m_fxFilePath = "Assets/shader/renderGBuffer.fx";
    teapotModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    teapotModelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    teapotModelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32_FLOAT;
    Model teapotModel;
    teapotModel.Init(teapotModelInitData);

    // 背景のモデルを初期化
    ModelInitData bgModelInitData;

    // ユーザー拡張データとしてポイントライトのリストを渡す
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg.tkm";
    bgModelInitData.m_fxFilePath = "Assets/shader/renderGBuffer.fx";
    bgModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    bgModelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    bgModelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32_FLOAT;
    Model bgModel;
    bgModel.Init(bgModelInitData);

    // G-Bufferを作成
    // アルベドカラー出力用のレンダリングターゲット
    RenderTarget albedoRT;
    albedoRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT);

    // 法線出力用のレンダリングターゲット
    RenderTarget normalRT;
    normalRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_UNKNOWN);

    // step-1 射影空間でのZ値を出力するためのG-Bufferを作成する
    RenderTarget depthRT;
    depthRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN);

    // レンダリングターゲットをG-Bufferに変更して書き込む
    RenderTarget* rts[] = {
        &albedoRT,      // 0番目のレンダリングターゲット
        &normalRT,      // 1番目のレンダリングターゲット
        // step-2 RenderGBufferのパスのレンダリングターゲットにdepthRTを追加する
        & depthRT		//2番目のレンダリングターゲット
    };

    // ポストエフェクト的にディファードライティングを行うためのスプライトを初期化
    SpriteInitData spriteInitData;

    // 画面全体にレンダリングするので幅と高さはフレームバッファの幅と高さと同じ
    spriteInitData.m_width = FRAME_BUFFER_W;
    spriteInitData.m_height = FRAME_BUFFER_H;

    // ディファードライティングで使用するテクスチャを設定
    spriteInitData.m_textures[0] = &albedoRT.GetRenderTargetTexture();
    spriteInitData.m_textures[1] = &normalRT.GetRenderTargetTexture();

    // step-3 ディファードライティングで使用するテクスチャを追加する
    spriteInitData.m_textures[2] = &depthRT.GetRenderTargetTexture();

    spriteInitData.m_fxFilePath = "Assets/shader/defferedLighting.fx";
    spriteInitData.m_expandConstantBuffer = &light;
    spriteInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化データを使ってスプライトを作成
    Sprite defferdLightingSpr;
    defferdLightingSpr.Init( spriteInitData );

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Vector4 pos = { 20.0f, 10.0f, 30.0f, 1.0f };
    Matrix mViewProj = g_camera3D->GetViewProjectionMatrix();
    mViewProj.Apply(pos);
    mViewProj.Inverse(mViewProj);
    pos.x /= pos.w;
    pos.y /= pos.w;
    pos.z /= pos.w;
    pos.w = 1.0f;
    mViewProj.Apply(pos);
    pos.x /= pos.w;
    pos.y /= pos.w;
    pos.z /= pos.w;

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
        for (auto& pt : light.pointLights)
        {
            qRot.Apply(pt.position);
        }

        // まず、レンダリングターゲットとして設定できるようになるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

        teapotModel.Draw(renderContext);
        bgModel.Draw(renderContext);

        // レンダリングターゲットへの書き込み待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリング先をフレームバッファに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(renderContext);

        // G-Bufferの内容を元にしてスプライトをレンダリング
        defferdLightingSpr.Draw(renderContext);

        /////////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
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
// / 標準入出力コンソールを初期化
/// </summary>
void InitStandardIOConsole()
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