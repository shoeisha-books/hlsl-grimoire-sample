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
    Bitmap imagebmp;
    imagebmp.Load("Assets/image/original.bmp");

    // step-2 画像データをグラフィックスメモリに送るためにストラクチャードバッファを作成
    StructuredBuffer inputImageBmpSB;
    inputImageBmpSB.Init(
        imagebmp.GetPixelSizeInBytes(), // 第一引数は1画素のサイズ
        imagebmp.GetNumPixel(),         // ピクセルの数を取得
        imagebmp.GetImageAddress()      // 画像データの先頭アドレス
    );

    // step-3 モノクロ化した画像を受け取るためのRWストラクチャバッファを作成
    RWStructuredBuffer outputImageBmpRWSB;
    outputImageBmpRWSB.Init(
        imagebmp.GetPixelSizeInBytes(), // 第一引数は1画素のサイズ
        imagebmp.GetNumPixel(),         // ピクセルの数を取得
        imagebmp.GetImageAddress()      // 画像データの先頭アドレス
    );

    // step-4 入力データと出力データをディスクリプタヒープに登録する
    DescriptorHeap ds;
    ds.RegistShaderResource(0, inputImageBmpSB);
    ds.RegistUnorderAccessResource(0, outputImageBmpRWSB);
    ds.Commit();

    //コンピュートシェーダのロード
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
        renderContext.SetComputeRootSignature(rs);
        renderContext.SetPipelineState(pipelineState);
        renderContext.SetComputeDescriptorHeap(ds);

        // ピクセル数は512×512 = 262,144ピクセル
        // 4つのスレッドを生成するコンピュートシェーダ―なので、
        // 262,144 ÷ 4 = 65,536個のスレッドグループを作成する
        renderContext.Dispatch(65536, 1, 1);

        // フレーム終了
        g_engine->EndFrame();

        // step-6 モノクロにした画像を保存
        imagebmp.Copy(outputImageBmpRWSB.GetResourceOnCPU());
        imagebmp.Save("Assets/image/monochrome.bmp");

        MessageBox(nullptr, L"完成", L"通知", MB_OK);
        // デストロイ
        DestroyWindow(g_hWnd);
    }
    return 0;
}
