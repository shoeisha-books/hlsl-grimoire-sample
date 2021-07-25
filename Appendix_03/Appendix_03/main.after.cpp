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
    g_camera3D->SetPosition({ 0.0f, 70.0f, 150.0f });
    g_camera3D->SetTarget({ 0.0f, 70.0f, 0.0f });

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // 背景モデルを初期化
    Light light;
    Model model;
    InitBGModel(model, light);

    // step-1 人型モデルを描画するレンダリングターゲットを初期化。
    // 【注目】αの値を0.0でクリアしている(クリアカラーの４番目の要素)。
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    RenderTarget drawHumanModelRT;
    drawHumanModelRT.Create(
        100,    // 横幅100
        100,    // 縦幅100
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // step-2 人型モデルを初期化。
    // モデルの初期化データを設定する。
    ModelInitData modelInitData;
    // tkmファイルのパスを指定。
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    // 使用するシェーダーファイルのパスを指定。
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    // 拡張定数バッファにライトの情報を渡す。
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    // 【注目】テクスチャのフィルタを縮小/拡大/MIPすべてでポイントサンプリングを指定する。
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    // 設定したデータでモデルを初期化。
    Model humanModel;
    humanModel.Init(modelInitData);

    // step-3 人型モデルのテクスチャを貼り付ける板ポリモデルを初期化。
    ModelInitData planeModelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/plane.tkm";
    // 【注目】ドット絵用の特別シェーダーを指定している。
    modelInitData.m_fxFilePath = "Assets/shader/samplePixelArt.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    // 【注目】テクスチャのフィルタを縮小/拡大/MIPすべてでポイントサンプリングを指定する。
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    Model planeModel;
    planeModel.Init(modelInitData);
    // 【注目】板ポリモデルのアルベドテクスチャを差し替える。
    planeModel.ChangeAlbedoMap("", drawHumanModelRT.GetRenderTargetTexture());
  

    // step-4 人型モデルを描画するためのカメラを作成。
    Camera drawHumanModelCamera;
    // 透視変換行列の計算方法を平行投影にする。
    drawHumanModelCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    // スクリーンに写すカメラ空間の横幅を指定。平行投影の時のみのパラメータ。
    // カメラの視点を中心として、カメラ空間でのx座標が-100～100の範囲のオブジェクトがスクリーンに投影される。
    drawHumanModelCamera.SetWidth( 200.0f );
    // スクリーンに写すカメラ空間の横幅を指定。平行投影の時のみのパラメータ。
    // カメラの視点を中心として、カメラ空間でのy座標が-100～100の範囲のオブジェクトがスクリーンに投影される。
    drawHumanModelCamera.SetHeight( 200.0f );
    // 近平面を指定。
    drawHumanModelCamera.SetNear(1.0f);
    // 遠平面を指定。
    drawHumanModelCamera.SetFar( 1000.0f );
    // 視点の座標を設定。
    drawHumanModelCamera.SetPosition(0.0f, 100.0f, 200.0f);
    // 注視点の座標を設定。
    drawHumanModelCamera.SetTarget(0.0f, 100.0f, 0.0f);
    // カメラの上方向を設定。
    drawHumanModelCamera.SetUp(0.0f, 1.0f, 0.0f);
    // ここまで設定された内容を使って、カメラ行列と透視変換行列を計算する。
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
        // 板ポリを描画。
        planeModel.Draw(renderContext);

        // 背景モデルをドロー
        model.Draw(renderContext);
        
        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}
