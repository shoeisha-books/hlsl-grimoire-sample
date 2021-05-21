#include "stdafx.h"
#include "system/system.h"
#include <random>
#include "util/stopwatch.h"

/////////////////////////////////////////////////////////////////
// 定数
/////////////////////////////////////////////////////////////////
const int TILE_WIDTH = 16;      // タイルの幅
const int TILE_HEIGHT = 16;     // タイルの高さ
const int NUM_TILE = (FRAME_BUFFER_W / TILE_WIDTH) * (FRAME_BUFFER_H / TILE_HEIGHT); // タイルの数

/////////////////////////////////////////////////////////////////
// 構造体
/////////////////////////////////////////////////////////////////
// ディレクションライト構造体
struct alignas(16) DirectionalLight
{
    Vector3  color;     // ライトのカラー
    float pad0;
    Vector3  direction; // ライトの方向
};

// ポイントライト構造体
struct alignas(16) PointLight
{
    Vector3 position;       // 座標
    float pad0;
    Vector3 positionInView; // カメラ空間での座標
    float pad1;
    Vector3 color;          // ライトのカラー
    float range;            // ライトの影響を与える範囲
};

const int MAX_POINT_LIGHT = 1000;   // ポイントライトの最大数
const int NUM_DIRECTION_LIGHT = 4;  // ディレクションライトの数

// ライト構造体
struct Light
{
    DirectionalLight directionLights[ NUM_DIRECTION_LIGHT]; // ディレクションライト
    PointLight pointLights[MAX_POINT_LIGHT];                // ポイントライト
    Matrix mViewProjInv;                                    // ビュープロジェクション行列の逆行列
    Vector4 screenParam;                                    // スクリーン情報
    Vector3 eyePos;                                         // 視点
    float specPow;                                          // スペキュラの絞り
    int numPointLight;                                      // ポイントライトの数
};

/////////////////////////////////////////////////////////////////
/// クラス
/////////////////////////////////////////////////////////////////
// ライトカリングクラス
class LightCulling
{
private:
    // ライトカリングで使用するカメラ情報
    struct alignas(16) LightCullingCameraData
    {
        Matrix mProj;           // プロジェクション行列
        Matrix mProjInv;        // プロジェクション行列の逆行列
        Matrix mCameraRot;      // カメラの回転行列
    };

    RootSignature* m_rootSignature = nullptr;   // ルートシグネチャ
    Shader m_cs;                                // コンピュートシェーダー
    PipelineState m_pipelineState;              // パイプラインステート
    RWStructuredBuffer m_pointLightNoListInTileUAV; // タイルごとのポイントライトの番号のリストを出力するUAVを初期化
    LightCullingCameraData m_cameraDataCPU;     // ライトカリングのためのカメラ情報を送る定数バッファ
    ConstantBuffer m_cameraDataGPU;             // ライトカリング用のカメラデータ
    ConstantBuffer m_lightGPU;                  //
    DescriptorHeap m_descriptorHeap;            // ディスクリプタヒープ
    Light* m_light = nullptr;

public:
    RWStructuredBuffer& GetPointLightNoListInTileUAV()
    {
        return m_pointLightNoListInTileUAV;
    }

    /// <summary>
    /// 初期化
    /// </summary>
    void Init(RootSignature& rs, Light& light, RenderTarget& depthRT)
    {
        m_light = &light;
        m_rootSignature = &rs;
        // ライトカリング用のコンピュートシェーダーをロード
        m_cs.LoadCS("Assets/shader/lightCulling.fx", "CSMain");

        // パイプラインステートを作成
        D3D12_COMPUTE_PIPELINE_STATE_DESC  psoDesc = { 0 };
        psoDesc.pRootSignature = rs.Get();
        psoDesc.CS = CD3DX12_SHADER_BYTECODE(m_cs.GetCompiledBlob());
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psoDesc.NodeMask = 0;

        m_pipelineState.Init(psoDesc);

        // タイルごとのポイントライトの番号のリストを出力するUAVを初期化
        m_pointLightNoListInTileUAV.Init(
            sizeof(int),
            MAX_POINT_LIGHT * NUM_TILE,
            nullptr
        );

        // ポイントライトの情報を送るための定数バッファを作成
        // ライトカリングのカメラ用の定数バッファを作成
        m_cameraDataCPU.mProj = g_camera3D->GetProjectionMatrix();
        m_cameraDataCPU.mProjInv.Inverse(g_camera3D->GetProjectionMatrix());
        m_cameraDataCPU.mCameraRot = g_camera3D->GetCameraRotation();

        m_cameraDataGPU.Init(sizeof(m_cameraDataCPU), &m_cameraDataCPU);

        // ライトカリングのライト用の定数バッファを作成
        m_lightGPU.Init(sizeof(light), &light);

        // ライトカリング用のディスクリプタヒープを作成
        m_descriptorHeap.RegistShaderResource(0, depthRT.GetRenderTargetTexture());
        m_descriptorHeap.RegistUnorderAccessResource(0, m_pointLightNoListInTileUAV);
        m_descriptorHeap.RegistConstantBuffer(0, m_cameraDataGPU);
        m_descriptorHeap.RegistConstantBuffer(1, m_lightGPU);
        m_descriptorHeap.Commit();
    }

    /// <summary>
    /// ディスパッチ
    /// </summary>
    /// <param name="renderContext"></param>
    void Dispatch(RenderContext& renderContext)
    {
        // ライトカリングをディスパッチ
        renderContext.SetComputeRootSignature(*m_rootSignature);
        m_lightGPU.CopyToVRAM(*m_light);
        renderContext.SetComputeDescriptorHeap(m_descriptorHeap);
        renderContext.SetPipelineState(m_pipelineState);

        // グループの数はタイルの数
        renderContext.Dispatch(
            FRAME_BUFFER_W / TILE_WIDTH,
            FRAME_BUFFER_H / TILE_HEIGHT,
            1
        );
    }
};

// step-1 ZPrepassクラスを作成
class ZPrepass
{
private:
    RenderTarget m_depthRT; // 深度値を書き込むレンダリングターゲット
    Model m_teapotModel;    // ティーポットのモデル
    Model m_bgModel;        // 背景モデル

public:
    RenderTarget& GetDepthRenderTarget()
    {
        return m_depthRT;
    }

    void Init()
    {
        // 深度値を書き込むレンダリングターゲットを作成
        m_depthRT.Create(
            FRAME_BUFFER_W,
            FRAME_BUFFER_H,
            1,
            1,
            DXGI_FORMAT_R32_FLOAT,
            DXGI_FORMAT_D32_FLOAT
        );

        // モデルを初期化
        ModelInitData teapotModelInitData;
        teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";

        // シェーダーをZPrepass用にする
        teapotModelInitData.m_fxFilePath = "Assets/shader/zprepass.fx";
        // 出力先のカラーバッファのフォーマットを指定する。
        teapotModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
        m_teapotModel.Init(teapotModelInitData);

        // 背景のモデルを初期化
        ModelInitData bgModelInitData;

        // ユーザー拡張データとしてポイントライトのリストを渡す
        bgModelInitData.m_tkmFilePath = "Assets/modelData/bg.tkm";

        // シェーダーをZPrepass用にする
        bgModelInitData.m_fxFilePath = "Assets/shader/zprepass.fx";
        // 出力先のカラーバッファのフォーマットを指定する。
        bgModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
        m_bgModel.Init(bgModelInitData);
    }

    void Draw(RenderContext& renderContext)
    {
        // レンダリングターゲットを切り替えてドロー
        RenderTarget* rts[] = {
            &m_depthRT
        };
        renderContext.WaitUntilToPossibleSetRenderTargets(1, rts);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargets(1, rts);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(1, rts);

        m_teapotModel.Draw(renderContext);
        m_bgModel.Draw(renderContext);

        // レンダリングターゲットへの書き込み待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(1, rts);

        // レンダリング先をフレームバッファに戻す
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(renderContext);
    }
};

/////////////////////////////////////////////////////////////////
//関数宣言
/////////////////////////////////////////////////////////////////
void InitRootSignature(RootSignature& rs);
void InitStandardＩＯConsole();
void InitLight(Light& light);
void InitModel(Model& teapotModel, Model& bgModel, Light& light, IShaderResource& pointLightNoListInTileUAV);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    // 標準入出力コンソールの初期化
    InitStandardＩＯConsole();

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////
    g_camera3D->SetPosition({ 0.0f, 200.0, 400.0f });
    g_camera3D->Update();

    // ルートシグネチャを作成
    RootSignature rootSignature;
    InitRootSignature(rootSignature);

    // ライトを初期化
    Light light;
    InitLight(light);

    // step-2 ZPrepassクラスのオブジェクトを作成して初期化する
    ZPrepass zprepass;
    zprepass.Init();

    // ライトカリングの初期化
    LightCulling lightCulling;
    lightCulling.Init(rootSignature, light, zprepass.GetDepthRenderTarget());

    // ティーポットのモデルと背景モデルを初期化
    Model teapotModel, bgModel;
    InitModel(teapotModel, bgModel, light, lightCulling.GetPointLightNoListInTileUAV());

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Stopwatch sw;
    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        sw.Start();
        // レンダリング開始
        g_engine->BeginFrame();
        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // ライトを回す
        Quaternion qRot;
        qRot.SetRotationDegY(1.0f);
        Matrix mView = g_camera3D->GetViewMatrix();
        for (int i = 0; i < light.numPointLight; i++)
        {
            auto& pt = light.pointLights[i];
            qRot.Apply(pt.position);
            pt.positionInView = pt.position;
            mView.Apply(pt.positionInView);
        }

        // step-3 ZPrepass⇒ライトカリング⇒フォワードレンダリングの実行
        // ZPrepass実行
        zprepass.Draw(renderContext);

        // ライトカリングをディスパッチ
        lightCulling.Dispatch(renderContext);

        // フォワードレンダリング
        teapotModel.Draw(renderContext);
        bgModel.Draw(renderContext);

        /////////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
        g_engine->EndFrame();
        sw.Stop();
        printf("fps = %0.2f\n", 1.0f / sw.GetElapsed() );
    }
    ::FreeConsole();
    return 0;
}

// ルートシグネチャの初期化
void InitRootSignature(RootSignature& rs)
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

/// <summary>
/// 標準入出力コンソールを初期化
/// </summary>
void InitStandardＩＯConsole()
{
    ::AllocConsole();               // コマンドプロンプトが表示される
    freopen("CON", "w", stdout);    // 標準出力の割り当て

    auto fhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT rc;

    rc.Top = 0;
    rc.Left = 0;
    rc.Bottom = 30;
    rc.Right = 30;
    ::SetConsoleWindowInfo(fhandle, TRUE, &rc);
}

/// <summary>
/// ライトを初期化
/// </summary>
/// <param name="light"></param>
void InitLight(Light& light)
{
    std::random_device seed_gen;
    std::mt19937 random(seed_gen());

    light.eyePos = g_camera3D->GetPosition();
    light.specPow = 5.0f;
    light.mViewProjInv.Inverse(g_camera3D->GetViewProjectionMatrix());

    // ディレクションライトを初期化
    light.directionLights[0].direction.Set(1.0f, 0.0f, 0.0f);
    light.directionLights[0].color.Set(0.5f, 0.5f, 0.5f);

    light.directionLights[1].direction.Set(-1.0f, 0.0f, 0.0f);
    light.directionLights[1].color.Set(0.5f, 0.0f, 0.0f);

    light.directionLights[2].direction.Set(0.0f, 0.0f, 1.0f);
    light.directionLights[2].color.Set(0.0f, 0.5f, 0.0f);

    light.directionLights[3].direction.Set(0.0f, -1.0f, 0.0f);
    light.directionLights[3].color.Set(0.0f, 0.0f, 0.5f);

    // ポイントライトを初期化
    light.numPointLight = 1000;
    Matrix mView = g_camera3D->GetViewMatrix();
    for (int i = 0; i < light.numPointLight; i++)
    {
        auto& pt = light.pointLights[i];
        pt.position.x = static_cast<float>(random() % 1000) - 500.0f;
        pt.position.y = 20.0f; // 高さは20固定
        pt.position.z = static_cast<float>(random() % 1000) - 500.0f;
        pt.positionInView = pt.position;
        mView.Apply(pt.positionInView);

        pt.range = 50.0f;       // 影響範囲も50で固定しておく
        pt.color.x = static_cast<float>(random() % 255) / 255.0f;
        pt.color.y = static_cast<float>(random() % 255) / 255.0f;
        pt.color.z = static_cast<float>(random() % 255) / 255.0f;
    }
    light.screenParam.x = g_camera3D->GetNear();
    light.screenParam.y = g_camera3D->GetFar();
    light.screenParam.z = FRAME_BUFFER_W;
    light.screenParam.w = FRAME_BUFFER_H;
}

/// <summary>
/// モデルの初期化
/// </summary>
/// <param name="teapotModel"></param>
/// <param name="bgModel"></param>
void InitModel(Model& teapotModel, Model& bgModel, Light& light, IShaderResource& pointLightNoListInTileUAV)
{
    // ティーポットのモデルを初期化
    ModelInitData teapotModelInitData;
    teapotModelInitData.m_tkmFilePath = "Assets/modelData/teapot.tkm";
    teapotModelInitData.m_fxFilePath = "Assets/shader/model.fx";
    teapotModelInitData.m_expandConstantBuffer = &light;
    teapotModelInitData.m_expandConstantBufferSize = sizeof(light);
    teapotModelInitData.m_expandShaderResoruceView[0] = &pointLightNoListInTileUAV;
    teapotModel.Init(teapotModelInitData);

    // 背景のモデルを初期化
    ModelInitData bgModelInitData;

    // ユーザー拡張データとしてポイントライトのリストを渡す
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg.tkm";
    bgModelInitData.m_fxFilePath = "Assets/shader/model.fx";
    bgModelInitData.m_expandConstantBuffer = &light;
    bgModelInitData.m_expandConstantBufferSize = sizeof(light);
    bgModelInitData.m_expandShaderResoruceView[0] = &pointLightNoListInTileUAV;
    bgModel.Init(bgModelInitData);
}
