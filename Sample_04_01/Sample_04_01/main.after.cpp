#include "stdafx.h"
#include "system/system.h"

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    g_camera3D->SetPosition({ 0.0f, 50.0f, 100.0f });

    // step-1 3Dモデルをロードするための情報を設定する
    // まず、モデルを初期化するための情報を設定する
    ModelInitData initData;
    // .tkmファイルのファイルパスを設定する
    initData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    // 使用するシェーダーファイルパスを設定する
    initData.m_fxFilePath = "Assets/shader/sample.fx";

    // step-2 初期化情報を使ってモデル表示処理を初期化する
    Model charaModel;
    charaModel.Init(initData);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // レンダリング開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // step-3 モデルのドローコールを実行する
        charaModel.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
