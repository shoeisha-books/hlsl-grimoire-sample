#include "stdafx.h"
#include "system/system.h"

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    // ディレクションライト用のメンバ
    Vector3 dirDirection;   // ライトの方向
    float pad0;
    Vector3 dirColor;       // ライトのカラー
    float pad1;

    Vector3 eyePos;         // 視点の位置
    float pad2;
    Vector3 ambientLight;   // アンビエントライト
    float pad3;

    // step-1 地面色と天球色、地面の法線を追加する
    Vector3 groundColor;
    float pad4;
    Vector3 skyColor;
	float pad5;
    Vector3 groundNormal;
};

//////////////////////////////////////
// 関数宣言
//////////////////////////////////////
void InitModel(Model& bgModel, Model& teapotModel, Model& lightModel, Light& light);
void InitDirectionLight(Light& light);
void InitPointLight(Light& light);
void InitAmbientLight(Light& light);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    g_camera3D->SetPosition({ 0.0f, 50.0f, 200.0f });
    g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////
    // ライトのデータを作成する
    Light light;

    // ディレクションライトを初期化する
    InitDirectionLight(light);

    // アンビエントライトを初期化する
    InitAmbientLight(light);

    // step-2 地面色、天球色、地面の法線のデータを設定する
	//地面色を設定
    /*light.groundColor.x = 0.7f;
    light.groundColor.y = 0.5f;
    light.groundColor.z = 0.3f;*/
    light.groundColor = { 0.7f ,0.5f,0.3f };

	//天球色を設定
    light.skyColor.x = 0.15f;
    light.skyColor.y = 0.7f;
    light.skyColor.z = 0.95f;

	//地面の法線を設定
	light.groundNormal.x = 0.0f;
	light.groundNormal.y = 1.0f;
	light.groundNormal.z = 0.0f;

    // モデルを初期化する
    // モデルを初期化するための情報を構築する
    Model lightModel, bgModel, teapotModel;
    InitModel(bgModel, teapotModel, lightModel , light);

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

        // ライトを回す
        Quaternion qRotY;
        qRotY.SetRotation(g_vec3AxisY, g_pad[0]->GetLStickXF() * 0.02f);
        qRotY.Apply(light.dirDirection);

        // 背景モデルをドロー
        // bgModel.Draw(renderContext);

        // ティーポットモデルをドロー
        teapotModel.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
    }
    return 0;
}

/// <summary>
/// モデルを初期化
/// </summary>
/// <param name="bgModel"></param>
/// <param name="teapotModel"></param>
/// <param name="lightModel"></param>
/// <param name="light"></param>
void InitModel(Model& bgModel, Model& teapotModel, Model& lightModel, Light& light)
{
    ModelInitData bgModelInitData;
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg.tkm";

    // 使用するシェーダーファイルパスを設定する
    bgModelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報を定数バッファーとしてディスクリプタヒープに登録するために
    // モデルの初期化情報として渡す
    bgModelInitData.m_expandConstantBuffer = &light;
    bgModelInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化情報を使ってモデルを初期化する
    bgModel.Init(bgModelInitData);
    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    // 使用するシェーダーファイルパスを設定する
    teapotModelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報を定数バッファーとしてディスクリプタヒープに登録するために
    // モデルの初期化情報として渡す
    teapotModelInitData.m_expandConstantBuffer = &light;
    teapotModelInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化情報を使ってモデルを初期化する
    teapotModel.Init(teapotModelInitData);

    teapotModel.UpdateWorldMatrix(
        { 0.0f, 20.0f, 0.0f },
        g_quatIdentity,
        g_vec3One
    );

    ModelInitData lightModelInitData;
    lightModelInitData.m_tkmFilePath = "Assets/modelData/light.tkm";

    // 使用するシェーダーファイルパスを設定する
    lightModelInitData.m_fxFilePath = "Assets/shader/other/light.fx";
    lightModelInitData.m_expandConstantBuffer = &light;
    lightModelInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化情報を使ってモデルを初期化する
    lightModel.Init(lightModelInitData);
}

void InitDirectionLight(Light& light)
{
    // ライトは右側から当たっている
    light.dirDirection.x = 0.0f;
    light.dirDirection.y = 0.0f;
    light.dirDirection.z = 1.0f;
    light.dirDirection.Normalize();

    // ライトのカラーは白
    light.dirColor.x = 0.5f;
    light.dirColor.y = 0.5f;
    light.dirColor.z = 0.5f;

    // 視点の位置を設定する
    light.eyePos = g_camera3D->GetPosition();
}

void InitAmbientLight(Light& light)
{
    // アンビエントライト
    light.ambientLight.x = 0.3f;
    light.ambientLight.y = 0.3f;
    light.ambientLight.z = 0.3f;
}
