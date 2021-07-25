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

    // step-1 500体分のモデルの座標を計算する。
    const int width = 50;
    const int height = 10;
    const int numHumanModel = width * height;
    Vector3* humanPos = new Vector3[numHumanModel];    
    
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

    // step-2 500体分のワールド行列関係の各種バッファを確保。
    // まずは計算用のバッファをメインメモリ上に確保する。
    Matrix* worldMatrixArray = new Matrix[numHumanModel];
    // 続いて、シェーダー側でワールド行列を使用するためのストラクチャードバッファをVRAM上に確保する。
    StructuredBuffer worldMatrixSB;
    worldMatrixSB.Init(
        sizeof(Matrix), // 第一引数は１要素のサイズ。
        numHumanModel,  // 第二引数は要素数。
        nullptr         // 第三引数は初期値データ。初期値は指定しないので、今回はnullptr。
    );

    // step-3 人物のモデルを初期化。
    // モデルの初期化データを設定する。
    ModelInitData modelInitData;
    // tkmファイルのパスを指定。
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    // 使用するシェーダーファイルのパスを指定。
    modelInitData.m_fxFilePath = "Assets/shader/sample3DInstancing.fx";
    // 【注目】拡張SRVにストラクチャードバッファを渡す。
    modelInitData.m_expandShaderResoruceView[0] = &worldMatrixSB;
    // 設定したデータでモデルを初期化。
    Model humanModel;
    humanModel.Init(modelInitData);


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

        // step-4 ワールド行列を計算する。
        for (int i = 0; i < numHumanModel; i++) {
            // ワールド行列を計算する。
            worldMatrixArray[i] = humanModel.CalcWorldMatrix(humanPos[i], qRot, g_vec3One);
        }
        // step-5 ワールド行列の内容をグラフィックメモリにコピー。
        worldMatrixSB.Update(worldMatrixArray);
        
        // step-6 人物ののモデルをインスタンシグ描画。
        humanModel.DrawInstancing(renderContext, numHumanModel);

        // 1フレーム終了
        g_engine->EndFrame();

        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed());
    }
    return 0;
}
