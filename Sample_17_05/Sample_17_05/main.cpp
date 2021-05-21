#include "stdafx.h"
#include "system/system.h"
#include <time.h>

const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの数

/// <summary>
/// ディレクションライト
/// </summary>
struct DirectionalLight
{
    Vector3 direction;  // ライトの方向
    float pad0;         // パディング
    Vector4 color;      // ライトのカラー
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT]; // ディレクションライト
    Vector3 eyePos;             // カメラの位置
    float specPow;              // スペキュラの絞り
    Vector3 ambinetLight;       // 環境光
};

#define USE_UNITY_CHAN 0
#define USE_NINJA 1
#define USE_ROBO 2

#define USE_MODEL 2

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    srand(time(nullptr) );
    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // 3Dモデルを作成
    Model model, bgModel;
    ModelInitData initData;

    initData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    initData.m_fxFilePath = "Assets/shader/NoAnimModel_PBR.fx";

    model.Init(initData);

    initData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
    bgModel.Init(initData);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    g_camera3D->SetPosition(0.0f, 50.0f, 120.0f);
    g_camera3D->SetTarget(0.0f, 50.0f, 200.0f);

    g_graphicsEngine->RegistModelToRaytracingWorld(model);
    g_graphicsEngine->RegistModelToRaytracingWorld(bgModel);
    g_graphicsEngine->BuildRaytracingWorld(renderContext);

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // レンダリング開始
        g_engine->BeginFrame();

        if (g_pad[0]->IsPress(enButtonLB1))
        {
            g_camera3D->MoveUp(g_pad[0]->GetLStickYF());
        }
        else
        {
            g_camera3D->MoveForward(-g_pad[0]->GetLStickYF());
        }
        g_camera3D->MoveRight(g_pad[0]->GetLStickXF());
        Quaternion qRotX, qRotY;
        qRotX.SetRotationX(g_pad[0]->GetRStickYF() * -0.005f);
        g_camera3D->RotateOriginTarget(qRotX);
        qRotY.SetRotationY(g_pad[0]->GetRStickXF() * 0.005f);
        g_camera3D->RotateOriginTarget(qRotY);

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////
        g_graphicsEngine->DispatchRaytracing(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}

