#include "stdafx.h"
#include "system/system.h"
#include "sub.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);
void InitPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)time(nullptr));
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////


    
    // 背景モデルを初期化
    Light light;
    Model model, coneModel;
    InitBGModel(model, light);

    //　コーンモデルを初期化。
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/human.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;

    coneModel.Init(modelInitData);

    // 板ポリモデルを初期化。
    ModelInitData planeModelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/plane.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/preset/pixelArt.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_samplerFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;

    Model planeModel;
    planeModel.Init(modelInitData);

    // コーンモデルをレンダリングするためのレンダリングターゲットを初期化。
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    RenderTarget drawConeModelRT;
    drawConeModelRT.Create(
        100.0f,
        100.0f,
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        clearColor
    );
    // コーンモデルを描画するためのカメラを作成。
    Camera drawConeModelCamera;
    drawConeModelCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    drawConeModelCamera.SetWidth( 200.0f );
    drawConeModelCamera.SetHeight( 200.0f );
    drawConeModelCamera.SetNear(1.0f);
    drawConeModelCamera.SetFar( 1000.0f );
    drawConeModelCamera.SetPosition(0.0f, 100.0f, 200.0f);
    drawConeModelCamera.SetTarget(0.0f, 100.0f, 0.0f);
    drawConeModelCamera.SetUp(0.0f, 1.0f, 0.0f);
    drawConeModelCamera.Update();

    planeModel.ChangeAlbedoMap("", drawConeModelRT.GetRenderTargetTexture());
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    Quaternion qRot;

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        MoveCamera();

        qRot.AddRotationY(0.01f);
        coneModel.UpdateWorldMatrix(g_vec3Zero, qRot, g_vec3One);

        // コーンモデルを描画。
         // レンダリングターゲットとして利用できるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTarget(drawConeModelRT);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargetAndViewport(drawConeModelRT);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetView(drawConeModelRT);

        coneModel.Draw(renderContext, drawConeModelCamera);

        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(drawConeModelRT);


        // メインレンダリングターゲットの絵をフレームバッファーにコピー
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        // ビューポートを指定する
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 1280;
        viewport.Height = 720;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        renderContext.SetViewportAndScissor(viewport);

        
        // モデルをドロー
        model.Draw(renderContext);
        planeModel.Draw(renderContext);
        

        // 1フレーム終了
        g_engine->EndFrame();
    }
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

// パイプラインステートの初期化
void InitPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps)
{

    // 頂点レイアウトを定義する
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // パイプラインステートを作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = rs.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.GetCompiledBlob());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.GetCompiledBlob());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineState.Init(psoDesc);
}
