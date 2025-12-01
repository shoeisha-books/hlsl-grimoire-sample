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

    // step-1 ライト構造体にポイントライト用のメンバ変数を追加する
    Vector3 ptPosition;//位置
	float pad2;//パディング
	Vector3 ptColor;//カラー
	float ptRange;//影響範囲

    Vector3 eyePos;         // 視点の位置
    float pad3;
    Vector3 ambientLight;   // アンビエントライト
};

//////////////////////////////////////
// 関数宣言
//////////////////////////////////////
void InitModel(Model& bgModel, Model& teapotModel, Model& lightModel, Light& light);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
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
    
    // ディレクションライトのデータを設定する
    // ライトは右側から当たっている
    light.dirDirection.x = 1.0f;
    light.dirDirection.y = -1.0f;
    light.dirDirection.z = -1.0f;
    light.dirDirection.Normalize();

    // ライトのカラーは白
    light.dirColor.x = 0.5f;
    light.dirColor.y = 0.5f;
    light.dirColor.z = 0.5f;

    // 視点の位置を設定する
    light.eyePos = g_camera3D->GetPosition();

    // 環境光
    light.ambientLight.x = 0.3f;
    light.ambientLight.y = 0.3f;
    light.ambientLight.z = 0.3f;

    // step-2 ポイントライトの初期座標を設定する
    light.ptPosition.x = 0.0f;
	light.ptPosition.y = 50.0f;
	light.ptPosition.z = 50.0f;

    // step-3 ポイントライトの初期カラーを設定する
	light.ptColor.x = 15.0f;
	light.ptColor.y = 0.0f;
	light.ptColor.z = 0.0f;

    // step-4 ポイントライトの影響範囲を設定する
	light.ptRange = 500.0f;

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

        // step-5 コントローラーでポイントライトを動かす
        light.ptPosition.x -= g_pad[0]->GetLStickXF();
        if (g_pad[0]->IsPress(enButtonB))
        {
			light.ptPosition.y += g_pad[0]->GetLStickYF();
        }
        else 
        {
			light.ptPosition.z -= g_pad[0]->GetLStickYF();
        }

		//電球モデルのワールド行列を更新する
		lightModel.UpdateWorldMatrix(light.ptPosition, g_quatIdentity, g_vec3One);
        
        // 背景モデルをドロー
        bgModel.Draw(renderContext);

        // ティーポットモデルをドロー
        teapotModel.Draw(renderContext);

        // 電球モデルをドロー
        lightModel.Draw(renderContext);

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

    // ディレクションライトの情報をディスクリプタヒープに
    // 定数バッファとして登録するためにモデルの初期化情報として渡す
    bgModelInitData.m_expandConstantBuffer = &light;
    bgModelInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化情報を使ってモデルを初期化する
    bgModel.Init(bgModelInitData);

    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    // 使用するシェーダーファイルパスを設定する
    teapotModelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報をディスクリプタヒープに
    // 定数バッファとして登録するためモデルの初期化情報として渡す
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
