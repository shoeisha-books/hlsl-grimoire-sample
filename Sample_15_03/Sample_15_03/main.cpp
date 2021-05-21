#include "stdafx.h"
#include "system/system.h"
#include "Bitmap.h"
#include "sub.h"

// 構造体定義
// テクスチャ情報用の定数バッファー
struct TexInfo
{
    int originalTexSize[2]; // オリジナルテクスチャのサイズ
    int xBlurTexSize[2];    // Xブラーの出力先のテクスチャのサイズ
    int yBlurTexSize[2];    // Yブラーの出力先のテクスチャのサイズ
};


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

    // ルートシグネチャの初期化
    RootSignature rs;
    InitRootSignature(rs);

    // step-1 画像データをメインメモリ上にロードする

    // step-2 ブレンディング係数を送るための定数バッファーを作成する

    // step-3 テクスチャ情報を送るための定数バッファーを作成する

    // step-4 各種構造化バッファーを作成する

    // step-5 各種ディスクリプタヒープを作成する

    // step-6 各種パイプラインステートを作成する

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム開始
        g_engine->BeginFrame();

        // ガウス関数を使って重みを計算し、グラフィックスメモリに転送
        float total = 0;
        float weights[8];
        for (int i = 0; i < 8; i++)
        {
            weights[i] = expf(-0.5f * (float)(i * i) / 100.0f);
            if (i == 0)
            {
                total += weights[i];
            }
            else
            {
                total += 2.0f * weights[i];
            }
        }
        // 規格化
        for (int i = 0; i < 8; i++)
        {
            weights[i] /= total;
        }

        // step-7 重みテーブルをVRAMに転送

        //////////////////////////////////////
        // ここからDirectComputeへのディスパッチ命令
        //////////////////////////////////////
        renderContext.SetComputeRootSignature(rs);

        // step-8 各種コンピュートシェーダーをディスパッチ

        // フレーム終了
        g_engine->EndFrame();

        // step-9 画像を取得し、保存する

        MessageBox(nullptr, L"完成", L"通知", MB_OK);
        // デストロイ
        DestroyWindow(g_hWnd);
    }
    return 0;
}
