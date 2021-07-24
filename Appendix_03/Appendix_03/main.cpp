#include "stdafx.h"
#include "system/system.h"
#include "sub.h"



///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)time(nullptr));
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // 背景モデルを初期化
    Light light;
    Model model;
    InitBGModel(model, light);

    // step-1 人型モデルを描画するレンダリングターゲットを初期化。

    // step-2 人型モデルを初期化。

    // step-3 人型モデルのテクスチャを貼り付ける板ポリモデルを初期化。

    // step-4 人型モデルを描画するためのカメラを作成。
   
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Quaternion qRot;

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        MoveCamera();

        // 人間のモデルを回転させる。
        qRot.AddRotationY(0.01f);
        humanModel.UpdateWorldMatrix(g_vec3Zero, qRot, g_vec3One);

        // step-5 人型モデルを描画。

        // step-6 レンダリングターゲットをフレームバッファに戻す。
        
        // step-7 ビューポートとシザリング矩形を指定する。

        // step-8 板ポリを描画。

        // 背景モデルをドロー
        model.Draw(renderContext);
        
        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}
