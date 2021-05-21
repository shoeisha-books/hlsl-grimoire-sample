#include "stdafx.h"
#include "system/system.h"

const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの数

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    Vector3 direction;      // ライトの方向
    float pad0;             // パディング
    Vector4 color;          // ライトのカラー
    Vector3 eyePos;         // カメラの位置
    float specPow;          // スペキュラの絞り
    Vector3 ambinetLight;   // 環境光
};

struct DispSetting
{
    Vector3 cameraPos;
    Vector3 cameraTarget;
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

    // ライトを用意する
    Light light;

    light.color.x = 0.6f;
    light.color.y = 0.6f;
    light.color.z = 0.6f;

    light.direction.x = 1.0f;
    light.direction.y = -1.0f;
    light.direction.z = -1.0f;

    light.ambinetLight.x = 0.3f;
    light.ambinetLight.y = 0.3f;
    light.ambinetLight.z = 0.3f;

    light.eyePos = g_camera3D->GetPosition();
    light.specPow = 5.0f;

    // 3Dモデルを作成
    Model model;
    ModelInitData initData;

    initData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    initData.m_fxFilePath = "Assets/shader/sample.fx";
    initData.m_expandConstantBuffer = &light;
    initData.m_expandConstantBufferSize = sizeof(light);

    model.Init(initData);
    model.UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);

    g_camera3D->SetPosition({ 0.0f, 150.0f, 300.0f });
    g_camera3D->SetTarget({ 0.0f, 150.0f, 0.0f });


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
        if (g_pad[0]->IsPress(enButtonRight))
        {
            qRot.SetRotationDegY(1.0f);
        }
        else if (g_pad[0]->IsPress(enButtonLeft))
        {
            qRot.SetRotationDegY(-1.0f);
        }

        qRot.Apply(light.direction);

        // カメラも回す
        qRot.SetRotationDegY(g_pad[0]->GetLStickXF());
        auto camPos = g_camera3D->GetPosition();
        qRot.Apply(camPos);
        g_camera3D->SetPosition(camPos);

        Vector3 rotAxis;
        auto toPos = g_camera3D->GetPosition() - g_camera3D->GetTarget();
        auto dir = toPos;
        dir.Normalize();
        rotAxis.Cross(dir, g_vec3AxisY);
        qRot.SetRotationDeg(rotAxis, g_pad[0]->GetLStickYF());
        qRot.Apply(toPos);
        g_camera3D->SetPosition(g_camera3D->GetTarget() + toPos);

        light.eyePos = g_camera3D->GetPosition();

        model.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
