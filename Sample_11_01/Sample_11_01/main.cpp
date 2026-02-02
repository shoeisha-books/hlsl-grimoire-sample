#include "stdafx.h"
#include "system/system.h"
#include "ModelStandard.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);

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
    RootSignature rs;
    InitRootSignature(rs);

    // step-1 シャドウマップ描画用のレンダリングターゲットを作成する
    //カラーバッファーのクリアカラー
    //今回はカラーバッファーは真っ白にする
	float clearColor[4] = { 1.0f,1.0f, 1.0f, 1.0f };
    RenderTarget shadowMap;
    shadowMap.Create(
        1024,
        1024,
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // step-2 影描画用のライトカメラを作成する
    Camera lightCamera;

    //カメラの位置を設定。これはライトの位置
	lightCamera.SetPosition(0, 600, 0);

    //カメラ視点の注視点を設定。これがライトが照らしている場所
	lightCamera.SetTarget(0, 0, 0);

    //【注目】上方向を設定。今回はライトが真下を向いているので、X方向を上にしている
	lightCamera.SetUp(1, 0, 0);

    //今回のサンプルでは画角をせまめにしておく
	lightCamera.SetViewAngle(Math::DegToRad(20.0f));

    //ライトビュープロジェクション行列を計算している
	lightCamera.Update();

    // step-3 シャドウマップ描画用のモデルを用意する
	ModelInitData teapotShadowModelInitData;

    //【注目】シャドウマップ描画用のシェーダーを設定する
	teapotShadowModelInitData.m_fxFilePath = "Assets/shader/sampleDrawShadowMap.fx";
	teapotShadowModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";
	Model teapotShadowModel;
	teapotShadowModel.Init(teapotShadowModelInitData);
    teapotShadowModel.UpdateWorldMatrix(
        { 0,50,0 },
        g_quatIdentity,
        g_vec3One
    );

    // シャドウマップを表示するためのスプライトを初期化する
    SpriteInitData spriteInitData;
    spriteInitData.m_textures[0] = &shadowMap.GetRenderTargetTexture();
    spriteInitData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
    spriteInitData.m_width = 256;
    spriteInitData.m_height = 256;

    Sprite sprite;
    sprite.Init(spriteInitData);

    // 通常描画のティーポットモデルを初期化
    ModelStandard teapotModel;
    teapotModel.Init("Assets/modelData/teapot.tkm");
    teapotModel.Update(
        { 0, 50, 0 },
        g_quatIdentity,
        g_vec3One
    );

    // 背景モデルを初期化
    ModelStandard bgModel;
    bgModel.Init("Assets/modelData/bg/bg.tkm");

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // step-4 影を生成したいモデルをシャドウマップに描画する
        //レンダリングターゲットをシャドウマップに変更する
        renderContext.WaitUntilToPossibleSetRenderTarget(shadowMap);
		renderContext.SetRenderTargetAndViewport(shadowMap);
		renderContext.ClearRenderTargetView(shadowMap);

        //影モデルを描画
		teapotShadowModel.Draw(renderContext, lightCamera);

        //書き込み終了待ち
		renderContext.WaitUntilFinishDrawingToRenderTarget(shadowMap);

        // 通常レンダリング
        // レンダリングターゲットをフレームバッファに戻す
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        renderContext.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

        // ティーポットモデルを描画
        teapotModel.Draw(renderContext);

        // 背景を描画
        bgModel.Draw(renderContext);

        sprite.Update({ FRAME_BUFFER_W / -2.0f, FRAME_BUFFER_H / 2.0f,  0.0f }, g_quatIdentity, g_vec3One, { 0.0f, 1.0f });
        sprite.Draw(renderContext);

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
