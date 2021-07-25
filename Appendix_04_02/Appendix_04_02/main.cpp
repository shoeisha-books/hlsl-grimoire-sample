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

    
    // 人型モデルを初期化。
    // モデルの初期化データを設定する。
    ModelInitData modelInitData;
    // tkmファイルのパスを指定。
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    // 使用するシェーダーファイルのパスを指定。
    modelInitData.m_fxFilePath = "Assets/shader/sample3D.fx";
    // 拡張定数バッファにライトの情報を渡す。
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
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

        for (int i = 0; i < numHumanModel; i++) {
            // ワールド行列を計算する。
            humanModel.UpdateWorldMatrix(humanPos[i], qRot, g_vec3One);
            worldMatrixArray[i] = humanModel.GetWorldMatrix();
        }
        worldMatrixSB.Update(worldMatrixArray);
        humanModel.DrawInstancing(renderContext, numHumanModel);
        // 1フレーム終了
        g_engine->EndFrame();

        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed());
    }
    return 0;
}
