#include "stdafx.h"
#include "system/system.h"
#include "Bitmap.h"
#include "sub.h"


///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    //  ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // step-1 画像データをメインメモリ上にロードする

    // step-2 画像データをグラフィックスメモリに送るために構造化バッファーを作成

    // step-3 モノクロ化した画像を受け取るための読み書き可能な構造化バッファーを作成

    // step-4 入力データと出力データをディスクリプタヒープに登録する

    // コンピュートシェーダーのロード
    Shader cs;
    cs.LoadCS("Assets/shader/sample.fx", "CSMain");

    RootSignature rs;
    InitRootSignature(rs, cs);

    PipelineState pipelineState;
    InitPipelineState(rs, pipelineState, cs);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここからDirectComputeへのディスパッチ命令
        //////////////////////////////////////
        // step-5 ディスパッチコールを実行する

        // フレーム終了
        g_engine->EndFrame();

        // step-6 モノクロにした画像を保存

        MessageBox(nullptr, L"完成", L"通知", MB_OK);
        // デストロイ
        DestroyWindow(g_hWnd);
    }
    return 0;
}
