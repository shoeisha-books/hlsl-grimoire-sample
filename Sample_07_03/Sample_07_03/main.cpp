#include "stdafx.h"
#include "system/system.h"

const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの数

/// <summary>
/// ディレクションライト
/// </summary>
struct DirectionalLight
{
    Vector3 direction;  // ライトの方向
    float pad0;         // パディング
    Vector4 color;      // ライトのカラー
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT]; // ディレクションライト
    Vector3 eyePos;         // カメラの位置
    float specPow;          // スペキュラの絞り
    Vector3 ambinetLight;   // 環境光
};

struct DispSetting
{
    Vector3 cameraPos;
    Vector3 cameraTarget;
};

enum
{
    Monster,
    Human,
    Lantern,
    Num
};

DispSetting dispSettings[Num];

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

    // 太陽光
    light.directionalLight[0].color.x = 3.0f;
    light.directionalLight[0].color.y = 3.0f;
    light.directionalLight[0].color.z = 3.0f;

    light.directionalLight[0].direction.x = 2.0f;
    light.directionalLight[0].direction.y = -1.0f;
    light.directionalLight[0].direction.z = 3.0f;
    light.directionalLight[0].direction.Normalize();

    // 地面からの照り返し
    light.directionalLight[1].color.x = 1.5f;
    light.directionalLight[1].color.y = 1.5f;
    light.directionalLight[1].color.z = 1.5f;

    light.directionalLight[1].direction.x = 0.0f;
    light.directionalLight[1].direction.y = 0.0f;
    light.directionalLight[1].direction.z = -1.0f;
    light.directionalLight[1].direction.Normalize();

    light.ambinetLight.x = 0.4f;
    light.ambinetLight.y = 0.4f;
    light.ambinetLight.z = 0.4f;
    
    light.eyePos = g_camera3D->GetPosition();
    light.specPow = 5.0f;

    // 3Dモデルを作成
    Model models[Num];
    ModelInitData initData;

    initData.m_tkmFilePath = "Assets/modelData/monster.tkm";
    initData.m_fxFilePath = "Assets/shader/sample.fx";
    initData.m_expandConstantBuffer = &light;
    initData.m_expandConstantBufferSize = sizeof(light);

    models[Monster].Init(initData);

    initData.m_tkmFilePath = "Assets/modelData/Human.tkm";
    models[Human].Init(initData);

    initData.m_tkmFilePath = "Assets/modelData/lantern.tkm";
    models[Lantern].Init(initData);

    Vector3 pos, scale;


    dispSettings[Monster].cameraPos = { 0.0f, 85.0f, 50.0f };
    dispSettings[Monster].cameraTarget = { 0.0f, 85.0f, 0.0f };
    dispSettings[Human].cameraPos = { 0.0f, 160.0f, 50.0f };
    dispSettings[Human].cameraTarget = { 0.0f, 160.0f, 0.0f };
    dispSettings[Lantern].cameraPos = { 0.0f, 50.0f, 120.0f };
    dispSettings[Lantern].cameraTarget = { 0.0f, 50.0f, 0.0f };

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    int dispModelNo = Monster;

    g_camera3D->SetPosition(dispSettings[dispModelNo].cameraPos);
    g_camera3D->SetTarget(dispSettings[dispModelNo].cameraTarget);

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

        for (auto& lig : light.directionalLight)
        {
            qRot.Apply(lig.direction);
        }

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

        models[dispModelNo].Draw(renderContext);
        if (g_pad[0]->IsTrigger(enButtonA))
        {
            dispModelNo = (dispModelNo + 1) % Num;
            g_camera3D->SetPosition(dispSettings[dispModelNo].cameraPos);
            g_camera3D->SetTarget(dispSettings[dispModelNo].cameraTarget);
        }

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}
