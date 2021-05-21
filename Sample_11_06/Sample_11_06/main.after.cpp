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
    const int NUM_SHADOW_MAP = 3;

    // step-2 ライトビュープロジェクションクロップ行列の配列を定義する
    Matrix lvpcMatrix[NUM_SHADOW_MAP];

    // step-3 シャドウマップを書き込むレンダリングターゲットを３枚用意する
    RenderTarget shadowMaps[NUM_SHADOW_MAP];

    // 近影用のシャドウマップ
    shadowMaps[0].Create(
        2048,
        2048,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // 中影用のシャドウマップ
    shadowMaps[1].Create(
        1024,
        1024,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // 遠影用のシャドウマップ
    shadowMaps[2].Create(
        512,
        512,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );

    // step-4 分割エリアの最大深度値を定義する
    float cascadeAreaTbl[NUM_SHADOW_MAP] = {
        500,                    // 近影を映す最大深度値
        2000,                   // 中影を映す最大深度値
        g_camera3D->GetFar(),   // 遠影を映す最大深度値。最大深度はカメラのFarクリップ
    };

    // 影を落とすモデルを初期化する
    Model testShadowModel[NUM_SHADOW_MAP];
    InitShadowCaster(testShadowModel[0]);
    InitShadowCaster(testShadowModel[1]);
    InitShadowCaster(testShadowModel[2]);

    // 通常描画のティーポットモデルを初期化
    ModelStandard teapotModel;
    teapotModel.Init("Assets/modelData/testModel.tkm");

    // step-5 影を受ける背景モデルを初期化
    ModelInitData bgModelInitData;

    // シャドウレシーバー（影が落とされるモデル）用のシェーダーを指定する
    bgModelInitData.m_fxFilePath = "Assets/shader/sampleShadowReciever.fx";

    // 【注目】シャドウマップを拡張SRVに設定する
    bgModelInitData.m_expandShaderResoruceView[0] = &shadowMaps[0].GetRenderTargetTexture();
    bgModelInitData.m_expandShaderResoruceView[1] = &shadowMaps[1].GetRenderTargetTexture();
    bgModelInitData.m_expandShaderResoruceView[2] = &shadowMaps[2].GetRenderTargetTexture();

    // 【注目】ライトビュープロジェクションクロップ行列を拡張定数バッファーに設定する
    bgModelInitData.m_expandConstantBuffer = (void*)lvpcMatrix;
    bgModelInitData.m_expandConstantBufferSize = sizeof(lvpcMatrix);
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";

    Model bgModel;
    bgModel.Init(bgModelInitData);

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
        // 前方向と右方向はすでに計算済みなので、それを引っ張ってくる
        const auto& cameraForward = g_camera3D->GetForward();
        const auto& cameraRight = g_camera3D->GetRight();

        // カメラの上方向は前方向と右方向の外積で求める
        Vector3 cameraUp;
        cameraUp.Cross( cameraForward, cameraRight );

        // nearDepthはエリアの最小深度値を表す
        // 一番近いエリアの最小深度値はカメラのニアクリップ
        float nearDepth = g_camera3D->GetNear();
        for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
        {
            // step-7 エリアを内包する視錐台の８頂点を求める
            // エリアの近平面の中心からの上面、下面までの距離を求める
            float nearY = tanf(g_camera3D->GetViewAngle() * 0.5f) * nearDepth;

            // エリアの近平面の中心からの右面、左面までの距離を求める
            float nearX = nearY * g_camera3D->GetAspect();

            // エリアの遠平面の中心からの上面、下面までの距離を求める
            float farY = tanf(g_camera3D->GetViewAngle()*0.5f) * cascadeAreaTbl[areaNo];

            // エリアの遠平面の中心からの右面、左面までの距離を求める
            float farX = farY * g_camera3D->GetAspect();

            // エリアの近平面の中心座標を求める
            Vector3 nearPos = g_camera3D->GetPosition() + cameraForward * nearDepth;

            // エリアの遠平面の中心座標を求める
            Vector3 farPos = g_camera3D->GetPosition() + cameraForward * cascadeAreaTbl[areaNo];

            // 8頂点を求める
            Vector3 vertex[8];

            // 近平面の右上の頂点
            vertex[0] += nearPos + cameraUp *  nearY + cameraRight *  nearX;

            // 近平面の左上の頂点
            vertex[1] += nearPos + cameraUp *  nearY + cameraRight * -nearX;

            // 近平面の右下の頂点
            vertex[2] += nearPos + cameraUp * -nearY + cameraRight *  nearX;

            // 近平面の左下の頂点
            vertex[3] += nearPos + cameraUp * -nearY + cameraRight * -nearX;

            // 遠平面の右上の頂点
            vertex[4] += farPos + cameraUp *  farY + cameraRight *  farX;

            // 遠平面の左上の頂点
            vertex[5] += farPos + cameraUp *  farY + cameraRight * -farX;

            // 遠平面の右下の頂点
            vertex[6] += farPos + cameraUp * -farY + cameraRight *  farX;

            // 遠平面の左下の頂点
            vertex[7] += farPos + cameraUp * -farY + cameraRight * -farX;

            // step-8 8頂点を変換して最大値、最小値を求める
            Vector3 vMax, vMin;
            vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
            vMin = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
            for (auto& v : vertex)
            {
                lvpMatrix.Apply(v);
                vMax.Max(v);
                vMin.Min(v);
            }

            // step-9 クロップ行列を求める
            float xScale = 2.0f / (vMax.x - vMin.x);
            float yScale = 2.0f / (vMax.y - vMin.y);
            float xOffset = (vMax.x + vMin.x) * -0.5f * xScale;
            float yOffset = (vMax.y + vMin.y) * -0.5f * yScale;
            Matrix clopMatrix;
            clopMatrix.m[0][0] = xScale;
            clopMatrix.m[1][1] = yScale;
            clopMatrix.m[3][0] = xOffset;
            clopMatrix.m[3][1] = yOffset;

            // step-10 ライトビュープロジェクション行列にクロップ行列を乗算する
            lvpcMatrix[areaNo] = lvpMatrix * clopMatrix;

            // step-11 シャドウマップにレンダリング
            // レンダリングターゲットをシャドウマップに変更する
            renderContext.WaitUntilToPossibleSetRenderTarget(shadowMaps[areaNo]);
            renderContext.SetRenderTargetAndViewport(shadowMaps[areaNo]);
            renderContext.ClearRenderTargetView(shadowMaps[areaNo]);

            // 影モデルを描画
            testShadowModel[areaNo].Draw(renderContext, g_matIdentity, lvpcMatrix[areaNo]);

            // 書き込み完了待ち
            renderContext.WaitUntilFinishDrawingToRenderTarget(shadowMaps[areaNo]);

            // 次のエリアの近平面までの距離を代入する
            nearDepth = cascadeAreaTbl[areaNo];
        }

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
    toLigDir *= 5000.0f; // ライトまでの距離

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

    // シャドウレシーバー（影が落とされるモデル）用のシェーダーを指定する
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
