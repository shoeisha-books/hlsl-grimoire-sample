#include "stdafx.h"
#include "system/system.h"
#include "sub.h"



///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)time(nullptr));
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // 背景モデルを初期化
    Light light;
    Model model;
    InitBGModel(model, light);

    // step-1 人型モデルを描画するレンダリングターゲットを初期化。
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    RenderTarget drawHumanModelRT;
    drawHumanModelRT.Create(
        100,
        100,
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // step-2 人型モデルを初期化。
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;

    Model humanModel;
    humanModel.Init(modelInitData);

    // step-3 人型モデルのテクスチャを貼り付ける板ポリモデルを初期化。
    ModelInitData planeModelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/plane.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/samplePixelArt.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    Model planeModel;
    planeModel.Init(modelInitData);
    // 板ポリモデルのアルベドテクスチャを差し替える。
    planeModel.ChangeAlbedoMap("", drawHumanModelRT.GetRenderTargetTexture());
  

    // step-4 人型モデルを描画するためのカメラを作成。
    Camera drawHumanModelCamera;
    drawHumanModelCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    drawHumanModelCamera.SetWidth( 200.0f );
    drawHumanModelCamera.SetHeight( 200.0f );
    drawHumanModelCamera.SetNear(1.0f);
    drawHumanModelCamera.SetFar( 1000.0f );
    drawHumanModelCamera.SetPosition(0.0f, 100.0f, 200.0f);
    drawHumanModelCamera.SetTarget(0.0f, 100.0f, 0.0f);
    drawHumanModelCamera.SetUp(0.0f, 1.0f, 0.0f);
    drawHumanModelCamera.Update();

   
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Quaternion qRot;

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        MoveCamera();

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

        // step-6 レンダリングターゲットをフレームバッファに戻す。
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        // step-7 ビューポートとシザリング矩形を指定する。
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 1280;
        viewport.Height = 720;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        renderContext.SetViewportAndScissor(viewport);

        // step-8 板ポリを描画。
        planeModel.Draw(renderContext);

        // 背景モデルをドロー
        model.Draw(renderContext);
        
        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}
