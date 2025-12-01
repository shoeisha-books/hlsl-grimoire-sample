#include "stdafx.h"
#include "system/system.h"

struct DirectionLight
{
    Vector3 ligDirection;   // ライトの方向
    float pad0;
    Vector3 ligColor;       // ライトのカラー
    float pad1;

    // step-1 構造体に視点の位置を追加する
    Vector3 eyePos;//視点の位置
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

    g_camera3D->SetPosition({ 0.0f, 0.0f, 100.0f });
    g_camera3D->SetTarget({ 0.0f, 0.0f, 0.0f });

    // ディレクションライトのデータを作成する
    DirectionLight directionLig;

    // ライトは右側から当たっている
    directionLig.ligDirection.x = 1.0f;
    directionLig.ligDirection.y = -1.0f;
    directionLig.ligDirection.z = -1.0f;
    directionLig.ligDirection.Normalize();

    // ライトのカラーは白
    //ここで色を設定できる
    directionLig.ligColor.x = 0.5f;
    directionLig.ligColor.y = 0.5f;
    directionLig.ligColor.z = 0.5f;

    // step-2 視点の位置を設定する
    directionLig.eyePos = g_camera3D->GetPosition();

    // モデルを初期化する
    // モデルを初期化するための情報を構築する
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    // 使用するシェーダーファイルパスを設定する
    modelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報をディスクリプタヒープに
    // 定数バッファとして登録するためにモデルの初期化情報として渡す
    modelInitData.m_expandConstantBuffer = &directionLig;
    modelInitData.m_expandConstantBufferSize = sizeof(directionLig);

    // 初期化情報を使ってモデルを初期化する
    Model model;
    model.Init(modelInitData);

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
        Quaternion qRot;
        qRot.SetRotationDegY(g_pad[0]->GetLStickXF());
        qRot.Apply(directionLig.ligDirection);
        qRot.SetRotationDegX(g_pad[0]->GetLStickYF());
        qRot.Apply(directionLig.ligDirection);

        // ティーポットをドローする
        model.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
