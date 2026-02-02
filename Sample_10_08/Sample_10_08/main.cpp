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

    // メインレンダリングターゲットと深度レンダリングターゲットを作成
    RenderTarget mainRenderTarget, depthRenderTarget;;
    InitMainDepthRenderTarget(mainRenderTarget, depthRenderTarget);

    // step-1 各種レンダリングターゲットを初期化する
	RenderTarget rtVerticalBlur;    // 垂直ブラー用レンダリングターゲット
	RenderTarget rtDiagonalBlur;    // 対角線ブラー用レンダリングターゲット
	RenderTarget rtPhomboidBlur;   // 六角形ブラー用レンダリングターゲット
    rtVerticalBlur.Create(1280, 720, 1, 1,
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	rtDiagonalBlur.Create(1280, 720, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	rtPhomboidBlur.Create(1280, 720, 1, 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);

    // step-2 垂直、対角線ブラーをかけるためのスプライトを初期化
    SpriteInitData vertDiagonalBlurSpriteInitData;
    vertDiagonalBlurSpriteInitData.m_textures[0] =
        &mainRenderTarget.GetRenderTargetTexture();
    vertDiagonalBlurSpriteInitData.m_width = 1280;
	vertDiagonalBlurSpriteInitData.m_height = 720;
	vertDiagonalBlurSpriteInitData.m_fxFilePath = "Assets/shader/sample.fx";

    //垂直、対角線ブラー用のピクセルシェーダーウィ指定する
    vertDiagonalBlurSpriteInitData.m_psEntryPoinFunc = "PSVerticalDiagonalBlur";
	vertDiagonalBlurSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    vertDiagonalBlurSpriteInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;

	Sprite vertDIagonalBlurSrite;
	vertDIagonalBlurSrite.Init(vertDiagonalBlurSpriteInitData);

    // step-3 六角形ブラーをかけるためのスプライトを初期化
	SpriteInitData phomboidBlurSpriteInitData;
	phomboidBlurSpriteInitData.m_textures[0] = &rtVerticalBlur.GetRenderTargetTexture();
	phomboidBlurSpriteInitData.m_textures[1] = &rtDiagonalBlur.GetRenderTargetTexture();
	phomboidBlurSpriteInitData.m_width = 1280;
	phomboidBlurSpriteInitData.m_height = 720;
	phomboidBlurSpriteInitData.m_fxFilePath = "Assets/shader/sample.fx";

    //六角形ブラー用のピクセルシェーダーを指定する
	phomboidBlurSpriteInitData.m_psEntryPoinFunc = "PSRhomboidBlur";
	phomboidBlurSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

	Sprite phomboidBlurSprite;
	phomboidBlurSprite.Init(phomboidBlurSpriteInitData);

    // ボケ画像をメインレンダリングターゲットに合成するためののスプライトを初期化する
    Sprite combineBokeImageSprite;
    InitCombimeBokeImageToSprite(
        combineBokeImageSprite,                     // 初期化されるスプライト
        rtPhomboidBlur.GetRenderTargetTexture(),    // ボケテクスチャ
        depthRenderTarget.GetRenderTargetTexture()  // 深度テクスチャ
    );

    // メインレンダリングターゲットの絵をフレームバッファにコピーするためのスプライトを初期化
    // スプライトの初期化オブジェクトを作成する
    Sprite copyToFrameBufferSprite;
    InitCopyToFrameBufferTargetSprite(
        copyToFrameBufferSprite,                    // 初期化されるスプライト
        mainRenderTarget.GetRenderTargetTexture()   // メインレンダリングターゲットのテクスチャ
    );

    // 背景モデルを初期化
    Light light;
    Model model;
    InitBGModel(model, light);

    // 球体モデルを初期化
    Light sphereLight[50];
    Model sphereModels[50];
    InitSphereModels(sphereModels, sphereLight, ARRAYSIZE(sphereModels));

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
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        MoveCamera();

        // 2枚のレンダリングターゲットを設定して、モデルを描画する
        // 2枚のレンダリングターゲットのポインタを持つ配列を定義する
        RenderTarget* rts[] = {
            &mainRenderTarget,
            &depthRenderTarget
        };

        // レンダリングターゲットとして利用できるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTargets(2, rts);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargetsAndViewport(2, rts);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(2, rts);

        // モデルをドロー
        model.Draw(renderContext);
        for (auto& sphereModel : sphereModels) {
            sphereModel.Draw(renderContext);
        }

        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);

        // step-4 垂直、対角線ブラーをかける
        RenderTarget* blurRts[] = {
            &rtVerticalBlur,
            &rtDiagonalBlur
        };

		//レンダリングターゲットとして利用できるまで待つ
		renderContext.WaitUntilToPossibleSetRenderTargets(2, blurRts);

        //レンダリングターゲットを設定
		renderContext.SetRenderTargetsAndViewport(2, blurRts);

        //レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(2, blurRts);

		vertDIagonalBlurSrite.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
		renderContext.WaitUntilFinishDrawingToRenderTargets(2, blurRts);

        // step-5 六角形ブラーをかける
		renderContext.WaitUntilToPossibleSetRenderTarget(rtPhomboidBlur);
        renderContext.SetRenderTargetAndViewport(rtPhomboidBlur);

        //レンダリングターゲットをクリア
		renderContext.ClearRenderTargetView(rtPhomboidBlur);

		phomboidBlurSprite.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
		renderContext.WaitUntilFinishDrawingToRenderTarget(rtPhomboidBlur);

        // ボケ画像と深度テクスチャを利用して、ボケ画像を描きこんでいく
        // メインレンダリングターゲットを設定
        renderContext.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
        renderContext.SetRenderTargetAndViewport(mainRenderTarget);

        // スプライトを描画
        combineBokeImageSprite.Draw(renderContext);

        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);

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
        copyToFrameBufferSprite.Draw(renderContext);

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
