#include "stdafx.h"
#include "system/system.h"
#include <time.h>

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    srand(time(nullptr) );

    g_camera3D->SetPosition(0.0f, 50.0f, 120.0f);
    g_camera3D->SetTarget(0.0f, 50.0f, 200.0f);

    auto& renderContext = g_graphicsEngine->GetRenderContext();

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // step-1 モデルをレイトレワールドに追加する

    // step-2 登録されたモデルを使ってレイトレワールドを構築

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // レンダリング開始
        g_engine->BeginFrame();

        // カメラを動かす
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

        // step-3 レイをディスパッチ

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
