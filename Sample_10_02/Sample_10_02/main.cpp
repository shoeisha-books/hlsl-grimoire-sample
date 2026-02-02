#include "stdafx.h"
#include "system/system.h"

//関数宣言
void InitRootSignature(RootSignature& rs);
void InitPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

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

    // step-1 オフスクリーン描画用のレンダリングターゲットを作成
    //RenderTargetクラスのオブジェクトを定義
    RenderTarget offscreenRenderTarget;

    //RenderTarget::Create()を利用して、レンダリングターゲットを作成する
    offscreenRenderTarget.Create(
        1280,                       //テクスチャの幅
        720,                        //テクスチャの高さ
        1,                          //Mipmapレベル
        1,                          //テクスチャ配列のサイズ
        DXGI_FORMAT_R8G8B8A8_UNORM, //カラーバッファーのフォーマット
        DXGI_FORMAT_D32_FLOAT       //デプスステンシルバッファーのフォーマット
    );

    // 各種モデルを初期化する
    // 背景モデルを初期化
    ModelInitData bgModelInitData;
    bgModelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
    bgModelInitData.m_fxFilePath = "Assets/shader/sample3D.fx";

    // 背景モデルを初期化
    Model bgModel;
    bgModel.Init(bgModelInitData);

    // プレイヤーモデルを初期化
    ModelInitData plModelInitData;
    plModelInitData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    plModelInitData.m_fxFilePath = "Assets/shader/sample3D.fx";
    Model plModel;
    plModel.Init(plModelInitData);

    // step-2 ポストエフェクト実行用のスプライトを初期化する
    //スプライトの初期化オブジェクトを生成する
    SpriteInitData spriteInitData;

    //【重要】テクスチャはオフスクリーンレンダリングされた絵
    spriteInitData.m_textures[0] = &offscreenRenderTarget.GetRenderTargetTexture();

    //【重要】全画面描画なのでスプライトのサイズはフレームバッファーと同じになる
    spriteInitData.m_width = 1280;
    spriteInitData.m_height = 720;

    //【重要】モノクロ用のシェーダーを指定する
    spriteInitData.m_fxFilePath = "Assets/shader/samplePostEffect.fx";

    //初期化オブジェクトを使って、スプライトを初期化する
    Sprite monochromeSprite;
    monochromeSprite.Init(spriteInitData);
    
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // step-3 レンダリングターゲットを変更する
        RenderTarget* rtArray[] = { &offscreenRenderTarget };

        //レンダリングターゲットとして利用できるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTargets(1, rtArray);

        //レンダリングターゲットを設定
        renderContext.SetRenderTargets(1, rtArray);

        //レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(1, rtArray);

        // step-4 offscreenRenderTargetに各種モデルを描画する
        //背景モデルをドロー
        bgModel.Draw(renderContext);

        //プレイヤーをドロー
        plModel.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(1, rtArray);

        // step-5 画面に表示されるレンダリングターゲットに戻す
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );

        // step-6 フルスクリーン表示のスプライトを描画する
        monochromeSprite.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

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

    //  頂点レイアウトを定義する
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
