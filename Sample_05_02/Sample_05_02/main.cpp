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

    // ライト構造体にポイントライト用のメンバ変数を追加
    Vector3 ptPosition;     // 位置
    float pad2;             // パディング
    Vector3 ptColor;        // カラー
    float ptRange;          // 影響範囲

    // step-1 ライト構造体にスポットライト用のメンバ変数を追加
	Vector3 spPosition;     // 位置
	float pad3;             // パディング
	Vector3 spColor;        // カラー
	float spRange;          // 影響範囲
	Vector3 spDirection;    // 射出方向
	float spAngle;        // 射出角度

    Vector3 eyePos;         // 視点の位置
    float pad4;

    Vector3 ambientLight;   // アンビエントライト
};
//////////////////////////////////////
//関数宣言
//////////////////////////////////////
void InitModel(Model& bgModel, Model& teapotModel, Model& lightModel, Light& light);
void InitDirectionLight(Light& light);
void InitPointLight(Light& light);
void InitAmbientLight(Light& light);

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
    // ディレクションライトを初期化する
    InitDirectionLight(light);
    // ポイントライトを初期化する
    InitPointLight(light);
    // アンビエントライトを初期化する
    InitAmbientLight(light);

    // step-2 スポットライトのデータを初期化する
	//初期座標はX=0、Y=50、Z=0にする
    light.spPosition.x = 0.0f;
    light.spPosition.y = 50.0f;
    light.spPosition.z = 0.0f;

    //ライトのカラーを設定。R=10、G=10、B=10
    light.spColor.x = 10.0f;
    light.spColor.y = 10.0f;
    light.spColor.z = 10.0f;

    //初期方向は斜め下にする
    light.spDirection.x = 1.0f;
	light.spDirection.y = -1.0f;
	light.spDirection.z = 1.0f;

    //方向データなので、大きさを1にする必要があるので正規化する
	light.spDirection.Normalize();

    //射出範囲
    light.spRange = 300.0f;
    //射出角度
    light.spAngle = Math::DegToRad(25.0f);

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

        // step-3 コントローラー左スティックでスポットライトを移動させる
        //左スティックで動かす
        light.spPosition.x -= g_pad[0] -> GetLStickXF();
        if (g_pad[0]->IsPress(enButtonB)) 
        {
            //Bボタンが一緒に押されたらY軸方向に動かす
            light.spPosition.y += g_pad[0]->GetLStickYF();
        }
        else
        {
            //Z軸方向に動かす
			light.spPosition.z -= g_pad[0]->GetLStickYF();
        }

        // step-4 コントローラー右スティックでスポットライトを回転させる
        //Y軸周りの回転クォータニオンを計算する
        Quaternion qRotY;
        qRotY.SetRotationY(g_pad[0]->GetRStickXF() * 0.01f);

        //計算したクォータニオンでライトの方向を回す
        qRotY.Apply(light.spDirection);

		//X軸周りの回転クォータニオンを計算する
        Vector3 rotAxis;
        rotAxis.Cross(g_vec3AxisY, light.spDirection);
        Quaternion qRotX;
        qRotX.SetRotation(rotAxis, g_pad[0]->GetRStickYF() * 0.01f);

        //計算したクォータニオンでライトの方向を回す
        qRotX.Apply(light.spDirection);

        //スポットライトモデルの回転クォータニオンを求める
        Quaternion qRot;
		qRot.SetRotation({ 0.0f,0.0f,-1.0f }, light.spDirection);

        //スポットライトモデルのワールド行列を更新する
        lightModel.UpdateWorldMatrix(light.spPosition, qRot, g_vec3One);
		
        // 背景モデルをドロー
        bgModel.Draw(renderContext);

        // スポットライトモデルをドロー
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

    // ディレクションライトの情報を定数バッファとしてディスクリプタヒープに登録するために
    // モデルの初期化情報として渡す
    bgModelInitData.m_expandConstantBuffer = &light;
    bgModelInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化情報を使ってモデルを初期化する
    bgModel.Init(bgModelInitData);

    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    // 使用するシェーダーファイルパスを設定する
    teapotModelInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // ディレクションライトの情報を定数バッファとしてディスクリプタヒープに登録するために
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
}

void InitPointLight(Light& light)
{
    // ポイントライトの座標を設定する
    light.ptPosition.x = 0.0f;
    light.ptPosition.y = 50.0f;
    light.ptPosition.z = 50.0f;

    // ポイントライトのカラーを設定する
    light.ptColor.x = 0.0f;
    light.ptColor.y = 0.0f;
    light.ptColor.z = 0.0f;

    // ポイントライトの影響範囲を設定する
    light.ptRange = 100.0f;
}

void InitAmbientLight(Light& light)
{
    // アンビエントライト
    light.ambientLight.x = 0.3f;
    light.ambientLight.y = 0.3f;
    light.ambientLight.z = 0.3f;
}
