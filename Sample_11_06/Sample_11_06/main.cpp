#include "stdafx.h"
#include "system/system.h"
#include "ModelStandard.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);
void MoveCamera();
void InitShadowCaster(Model& model);
void InitShadowReciever(Model& model, Matrix* lvpMatrix, RenderTarget* shadowMap);
void InitLightCamera(Camera& lightCamera);

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

    g_camera3D->SetPosition(0, 100.0f, 350.0f);
    g_camera3D->SetTarget(0, 100.0f, 0);
    RootSignature rs;
    InitRootSignature(rs);

    // 影描画用のライトカメラを作成する
    Camera lightCamera;
    InitLightCamera(lightCamera);

    // シャドウマップを3枚作成する
    float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // step-1 シャドウマップの枚数を定数で定義する

    // step-2 ライトビュープロジェクションクロップ行列の配列を定義する

    // step-3 シャドウマップを書き込むレンダリングターゲットを3枚用意する

    // step-4 分割エリアの最大深度値を定義する

    // 影を落とすモデルを初期化する
    Model testShadowModel[NUM_SHADOW_MAP];
    InitShadowCaster(testShadowModel[0]);
    InitShadowCaster(testShadowModel[1]);
    InitShadowCaster(testShadowModel[2]);

    // 通常描画のティーポットモデルを初期化
    ModelStandard teapotModel;
    teapotModel.Init("Assets/modelData/testModel.tkm");

    // step-5 影を受ける背景モデルを初期化

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        // カメラを動かす
        MoveCamera();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        const auto& lvpMatrix = lightCamera.GetViewProjectionMatrix();

        // step-6 カメラの前方向、右方向、上方向を求める

        // nearDepthはエリアの最小深度値を表す
        // 一番近いエリアの最小深度値はカメラのニアクリップ
        float nearDepth = g_camera3D->GetNear();
        for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
        {
            // step-7 エリアを内包する視錐台の8頂点を求める

            // step-8 8頂点を変換して最大値、最小値を求める

            // step-9 クロップ行列を求める

            // step-10 ライトビュープロジェクション行列にクロップ行列を乗算する

            // step-11 シャドウマップにレンダリング

            // 次のエリアの近平面までの距離を代入する
            nearDepth = cascadeAreaTbl[areaNo];
        }

        // 通常レンダリング
        // レンダリングターゲットをフレームバッファーに戻す
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        renderContext.SetViewportAndScissor(
            g_graphicsEngine->GetFrameBufferViewport());

        // ティーポットモデルを描画
        teapotModel.Draw(renderContext);

        // 影を受ける背景を描画
        bgModel.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

void InitLightCamera(Camera& lightCamera)
{
    // ライトまでのベクトル
    Vector3 toLigDir = { 1, 2, 1 };
    toLigDir.Normalize();
    toLigDir *= 5000.0f;    // ライトまでの距離

    // カメラの位置を設定。これはライトの位置
    lightCamera.SetPosition(toLigDir);

    // カメラの注視点を設定。これがライトが照らしている場所
    lightCamera.SetTarget({ 0, 0, 0 });

    // 上方向を設定。今回はライトが真下を向いているので、X方向を上にしている
    lightCamera.SetUp(1, 0, 0);

    lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    lightCamera.SetWidth(5000.0f);
    lightCamera.SetHeight(5000.0f);
    lightCamera.SetNear(1.0f);
    lightCamera.SetFar(10000.0f);

    // ライトビュープロジェクション行列を計算している
    lightCamera.Update();
}

// ルートシグネチャの初期化
void InitRootSignature( RootSignature& rs )
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

void MoveCamera()
{
    auto pos = g_camera3D->GetPosition();
    auto target = g_camera3D->GetTarget();
    pos.z -= g_pad[0]->GetLStickYF() * 2.0f;
    target.z -= g_pad[0]->GetLStickYF() * 2.0f;
    pos.y += g_pad[0]->GetRStickYF() * 2.0f;
    target.y += g_pad[0]->GetRStickYF() * 2.0f;
    g_camera3D->SetPosition(pos);
    g_camera3D->SetTarget(target);
}

void InitShadowCaster(Model& model)
{
    // シャドウキャスターモデルを初期化する
    // シャドウキャスターモデルを初期化するための初期化データを設定する
    ModelInitData modelInitData;

    // シャドウマップ描画用のシェーダーを指定する
    modelInitData.m_fxFilePath = "Assets/shader/sampleDrawShadowMap.fx";
    modelInitData.m_tkmFilePath = "Assets/modelData/testModel.tkm";

    // カラーバッファーのフォーマットに変更が入ったので、こちらも変更する
    modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;

    model.Init(modelInitData);
}

void InitShadowReciever(Model& model, Matrix* lvpMatrix, RenderTarget* shadowMap)
{
    // 影を受ける背景モデルを初期化
    ModelInitData bgModelInitData;

    // シャドウレシーバー(影が落とされるモデル)用のシェーダーを指定する
    bgModelInitData.m_fxFilePath = "Assets/shader/sampleShadowReciever.fx";

    // シャドウマップは、ガウシアンブラーでぼかしたものを利用する
    bgModelInitData.m_expandShaderResoruceView[0] = &shadowMap[0].GetRenderTargetTexture();
    bgModelInitData.m_expandShaderResoruceView[1] = &shadowMap[1].GetRenderTargetTexture();
    bgModelInitData.m_expandShaderResoruceView[2] = &shadowMap[2].GetRenderTargetTexture();

    // 影用のパラメータを拡張定数バッファーに設定する
    bgModelInitData.m_expandConstantBuffer = (void*)lvpMatrix;
    bgModelInitData.m_expandConstantBufferSize = sizeof(*lvpMatrix) * 3;
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";

    model.Init(bgModelInitData);
}
