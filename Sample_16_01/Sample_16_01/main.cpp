#include "stdafx.h"
#include "system/system.h"
#include <random>
#include "util/stopwatch.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);
void InitStandardIOConsole();

// step-1 ポイントライト構造体を定義する

// step-2 ポイントライトの数を表す定数を定義する

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
    //  ここから初期化を行うコードを記述する
    //////////////////////////////////////
    std::random_device seed_gen;
    std::mt19937 random(seed_gen());

    g_camera3D->SetPosition({ 0.0f, 200.0, 400.0f });

    // ルートシグネチャを作成
    RootSignature rootSignature;
    InitRootSignature(rootSignature);

    // step-3 ポイントライトをランダムな位置とカラーで初期化する

    // step-4 表示するモデルを初期化する

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

        // step-5 ポイントライトを毎フレーム回す

        // step-6 モデルのドローコールを実行する

        /////////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed());
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
