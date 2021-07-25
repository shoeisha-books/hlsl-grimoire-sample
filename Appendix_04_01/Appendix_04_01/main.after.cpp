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

    // step-1 人物のモデルを初期化。
    // モデルの初期化データを設定する。
    ModelInitData modelInitData;
    // tkmファイルのパスを指定。
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    // 使用するシェーダーファイルのパスを指定。
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    
    const int width = 50;  // 列の数。
    const int height = 10; // 行の数。
    // 表示する人物のモデルは50×10の500体
    const int numHumanModel = width * height;
    // 500体分のModelクラスのインタンスを確保して、初期化する。
    Model* humanModel = new Model[numHumanModel];
    for (int i = 0; i < numHumanModel; i++) {
        humanModel[i].Init(modelInitData);
    }
    // 続いて、500体分の座標データを確保。
    Vector3* humanPos = new Vector3[numHumanModel];
    // 500体が綺麗に並ぶように座標を計算する。
    int humanNo = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            humanPos[humanNo].x = -2400.0f + 100.0f * x;
            humanPos[humanNo].y = -1250.0f + 250.0f * y;
            humanPos[humanNo].z = 0.0f;
            humanNo++;
        }
    }

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

        // step-2 500体分の人間モデルを描画する。
        for (int i = 0; i < numHumanModel; i++) {
            // 人間モデルを描画。
            humanModel[i].Draw(renderContext);
            humanModel[i].UpdateWorldMatrix(humanPos[i], qRot, g_vec3One);

        }

        // 1フレーム終了
        g_engine->EndFrame();

        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed());
    }
    return 0;
}
