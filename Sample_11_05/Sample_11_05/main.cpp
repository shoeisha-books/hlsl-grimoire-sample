#include "stdafx.h"
#include "system/system.h"
#include "ModelStandard.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);
void InitLightCamera(Camera& lightCamera, Vector3& ligPos);
void MoveCamera();

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

    g_camera3D->SetPosition(0, 50.0f, 250.0f);
    g_camera3D->SetTarget(0, 0, 0);
    RootSignature rs;
    InitRootSignature(rs);

    // ライトの座標
    Vector3 ligPos = { 0, 300, 0 };

    // 影描画用のライトカメラを作成する
    Camera lightCamera;
    InitLightCamera(lightCamera, ligPos);

    // step-1 シャドウマップ描画用のレンダリングターゲットを作成する
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	RenderTarget shadowMap;
    shadowMap.Create(
        2048,
        2048,
        1,
        1,
        //【注目】シャドウマップのカラーバッファーのフォーマットを変更している
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // step-2 シャドウマップをぼかすためのオブジェクトを初期化する
    GaussianBlur shadowBlur;
    shadowBlur.Init(
		&shadowMap.GetRenderTargetTexture()//ぼかすテクスチャはシャドウマップのテクスチャ
    );

    // step-3 GPU側で利用するシャドウ用の構造体を定義する
    struct ShadowParam
    {
        Matrix mLVP;//ライトビュープロジェクション行列
		Vector3 lightrPos;//ライトの座標
    };

    // step-4 GPU側に送るデータを設定する
    ShadowParam sp;
	sp.mLVP = lightCamera.GetViewProjectionMatrix();
    sp.lightrPos.Set(ligPos);

    // step-5 シャドウマップに描画するモデルを初期化する
    //ティーポットモデルを初期化するための初期化情報を設定する
	ModelInitData teapotShadowModelInitData;

    //シャドウマップ描画用のシェーダーを指定する
	teapotShadowModelInitData.m_fxFilePath = "Assets/shader/sampleDrawShadowMap.fx";
	teapotShadowModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

    //【注目】影用のパラメータを拡張定数バッファーに設定する
    teapotShadowModelInitData.m_expandConstantBuffer = (void*)&sp;
	teapotShadowModelInitData.m_expandConstantBufferSize = sizeof(sp);

    //【注目】カラーバッファーのファーマっとに変更が入ったので、こちらも変更する
    teapotShadowModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32_FLOAT;

    //ティーポットモデルを初期化
    Model teapotShadowModel;
	teapotShadowModel.Init(teapotShadowModelInitData);
    teapotShadowModel.UpdateWorldMatrix(
        { 0,50,0 },
        g_quatIdentity,
        g_vec3One
    );

    // 通常描画のティーポットモデルを初期化
    ModelStandard teapotModel;
    teapotModel.Init("Assets/modelData/teapot.tkm");
    teapotModel.Update(
        { 0, 50, 0 },
        g_quatIdentity,
        g_vec3One
    );

    // step-6 影を受ける背景モデルを初期化
	ModelInitData bgModelInitData;

    //シャドウレシーバー（影が落とされるモデル）用のシェーダーを指定する
	bgModelInitData.m_fxFilePath = "Assets/shader/sampleShadowRecieve.fx";

    //【注目】影用のパラメーターを拡張定数バッファーに設定する
	bgModelInitData.m_expandConstantBuffer = (void*)&sp;
	bgModelInitData.m_expandConstantBufferSize = sizeof(sp);

    //【注目】シャドウマッップは、ガウシアンブラーでぼかしたものを利用する
    bgModelInitData.m_expandShaderResoruceView[0] = &shadowBlur.GetBokeTexture();

	bgModelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";

    Model bgModel;
	bgModel.Init(bgModelInitData);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        // カメラを動かす
        MoveCamera();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // シャドウマップにレンダリング
        // レンダリングターゲットをシャドウマップに変更する
        renderContext.WaitUntilToPossibleSetRenderTarget(shadowMap);
        renderContext.SetRenderTargetAndViewport(shadowMap);
        renderContext.ClearRenderTargetView(shadowMap);

        // 影モデルを描画
        teapotShadowModel.Draw(renderContext, lightCamera);

        // 書き込み完了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(shadowMap);

        // step-7 シャドウマップをぼかすためのガウシアンブラーを実行する
        shadowBlur.ExecuteOnGPU(renderContext, 5.0f);

        // 通常レンダリング
        // レンダリングターゲットをフレームバッファーに戻す
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        renderContext.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

        // ティーポットモデルを描画
        teapotModel.Draw(renderContext);

        // 影を受ける背景を描画
        bgModel.Draw(renderContext);

        //////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

// ルートシグネチャの初期化
void InitRootSignature( RootSignature& rs )
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

void InitLightCamera(Camera& lightCamera, Vector3& ligPos)
{
    // カメラの位置を設定。これはライトの位置
    lightCamera.SetPosition(ligPos);

    // カメラの注視点を設定。これがライトが照らしている場所
    lightCamera.SetTarget(0, 0, 0);

    // 上方向を設定。今回はライトが真下を向いているので、X方向を上にしている
    lightCamera.SetUp(1, 0, 0);

    // ライトビュープロジェクション行列を計算している
    lightCamera.Update();
}

void MoveCamera()
{
    Quaternion qAddRot;
    qAddRot.SetRotationDegX(g_pad[0]->GetRStickYF() * 0.5f);
    g_camera3D->RotateOriginTarget(qAddRot);
    auto pos = g_camera3D->GetPosition();
    auto target = g_camera3D->GetTarget();
    pos.z -= g_pad[0]->GetLStickYF() * 0.5f;
    target.z -= g_pad[0]->GetLStickYF() * 0.5f;
    g_camera3D->SetPosition(pos);
    g_camera3D->SetTarget(target);
}
