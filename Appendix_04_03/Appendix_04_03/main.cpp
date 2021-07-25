#include "stdafx.h"
#include "system/system.h"
#include "sub.h"
#include "util/stopwatch.h"


///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)time(nullptr));
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
    InitStandardIOConsole();

    g_camera3D->SetPosition({ 0.0f, 0.0f, 3000.0f });
    g_camera3D->SetTarget({ 0.0f, 0.0f, 0.0f });

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // 背景モデルを初期化
    Light light;
    Model model;
    InitBGModel(model, light);

    // 人型モデルを描画するレンダリングターゲットを初期化。
    
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    RenderTarget drawHumanModelRT;
    drawHumanModelRT.Create(
        2048,    // 横幅2048
        2048,    // 縦幅2048
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // 人型モデルを初期化。
    // モデルの初期化データを設定する。
    ModelInitData modelInitData;
    // tkmファイルのパスを指定。
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    // 使用するシェーダーファイルのパスを指定。
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    // 拡張定数バッファにライトの情報を渡す。
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    // 設定したデータでモデルを初期化。
    Model humanModel;
    humanModel.Init(modelInitData);
    
    // 人型モデルを描画するためのカメラを作成。
    Camera drawHumanModelCamera;
    drawHumanModelCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    drawHumanModelCamera.SetWidth(200.0f);
    drawHumanModelCamera.SetHeight(200.0f);
    drawHumanModelCamera.SetNear(1.0f);
    drawHumanModelCamera.SetFar(1000.0f);
    drawHumanModelCamera.SetPosition(0.0f, 100.0f, 200.0f);
    drawHumanModelCamera.SetTarget(0.0f, 100.0f, 0.0f);
    drawHumanModelCamera.SetUp(0.0f, 1.0f, 0.0f);
    drawHumanModelCamera.Update();


  

    const int width = 50;
    const int height = 10;
    const int numHumanModel = width * height;
    Vector3* humanPos = new Vector3[numHumanModel];
    Matrix* worldMatrixArray = new Matrix[numHumanModel];
    StructuredBuffer worldMatrixSB;
    worldMatrixSB.Init(sizeof(Matrix), numHumanModel, nullptr);
    int humanNo = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // 1000体のモデルが綺麗に並ぶように、座標を計算する。
            humanPos[humanNo].x = -2400.0f + 100.0f * x;
            humanPos[humanNo].y = -1250.0f + 250.0f * y;
            humanPos[humanNo].z = 0.0f;

            humanNo++;
        }
    }

    
    // step-3 人型モデルのテクスチャを貼り付ける板ポリモデルを初期化。
    ModelInitData planeModelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/plane.tkm";
    
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3DInstancing.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_expandShaderResoruceView[0] = &worldMatrixSB;
    
    Model planeModel;
    planeModel.Init(modelInitData);
    // 【注目】板ポリモデルのアルベドテクスチャを差し替える。
    planeModel.ChangeAlbedoMap("", drawHumanModelRT.GetRenderTargetTexture());

   

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Stopwatch sw;
    Quaternion qRot;
    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        sw.Start();

        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

        // 人間のモデルを回転させる。
        qRot.AddRotationY(0.01f);
        humanModel.UpdateWorldMatrix(g_vec3Zero, qRot, g_vec3One);

        // step-5 人型モデルを描画。
        // レンダリングターゲットとして利用できるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTarget(drawHumanModelRT);
        // レンダリングターゲットを設定
        renderContext.SetRenderTargetAndViewport(drawHumanModelRT);
        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetView(drawHumanModelRT);
        // 人間モデルを描画。
        humanModel.Draw(renderContext, drawHumanModelCamera);
        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(drawHumanModelRT);

        // step-6 板ポリを描画。
       // レンダリングターゲットをフレームバッファに戻す。
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        // ビューポートとシザリング矩形を指定する。
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 1280;
        viewport.Height = 720;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        renderContext.SetViewportAndScissor(viewport);

        for (int i = 0; i < numHumanModel; i++) {
            // ワールド行列を計算する。
            humanModel.UpdateWorldMatrix(humanPos[i], g_quatIdentity, g_vec3One);
            worldMatrixArray[i] = humanModel.GetWorldMatrix();
        }
        worldMatrixSB.Update(worldMatrixArray);
        planeModel.DrawInstancing(renderContext, numHumanModel);
        // 1フレーム終了
        g_engine->EndFrame();

        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed());
    }
    return 0;
}
