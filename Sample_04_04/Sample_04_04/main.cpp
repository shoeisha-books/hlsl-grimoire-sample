#include "stdafx.h"
#include "system/system.h"

/// <summary>
/// ディレクションライト構造体
/// </summary>
struct DirectionLight
{
    Vector3 direction;  //ライトの方向
    float pad0;
    Vector3 color;      //ライトのカラー
    float pad1;
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionLight directionLight;  // ディレクションライト
    Vector3 eyePos;                 // 視点の位置
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

    // ライトのデータを作成する
    Light light;

    // ライトは右側から当たっている
    light.directionLight.direction.x = 1.0f;
    light.directionLight.direction.y = -1.0f;
    light.directionLight.direction.z = -1.0f;
    light.directionLight.direction.Normalize();

    // ライトのカラーは白
    //数値を変えると色を変えれる
    light.directionLight.color.x = 0.0f;
    light.directionLight.color.y = 1.0f;
    light.directionLight.color.z = 0.5f;

    // 視点の位置を設定する
    light.eyePos = g_camera3D->GetPosition();

    // モデルを初期化する
    // モデルを初期化するための情報を構築する
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    // 使用するシェーダーファイルパスを設定する
    modelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報を定数バッファとしてディスクリプタヒープに登録するために
    // モデルの初期化情報として渡す
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);

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
        qRot.Apply(light.directionLight.direction);
        qRot.SetRotationDegX(g_pad[0]->GetLStickYF());
        qRot.Apply(light.directionLight.direction);

        // ティーポットをドロー
        model.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
