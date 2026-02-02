#include "stdafx.h"
#include "system/system.h"
#include "sub.h"

const int NUM_DIRECTIONAL_LIGHT = 4;

/// <summary>
/// ディレクションライト
/// </summary>
struct DirectionalLight
{
    Vector3 direction;  // ライトの方向
    float pad0;         // パディング
    Vector4 color;      // ライトのカラー
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT]; // ディレクションライト
    Vector3 eyePos;                 // カメラの位置
    float specPow;                  // スペキュラの絞り
    Vector3 ambinetLight;           // 環境光
};

// 関数宣言
void InitRootSignature(RootSignature& rs);
void InitPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);
void InitModel(Model& model, Light& light);

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

    Light light;

    // step-1 メインレンダリングターゲットと深度レンダリングターゲットを作成
	RenderTarget mainRenderTarget;
    mainRenderTarget.Create(1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );

    //
	RenderTarget depthRenderTarget;
    depthRenderTarget.Create(1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN
	);

    // step-2 シーンテクスチャをぼかすためのガウシアンブラーオブジェクトを初期化
    GaussianBlur blur;
    blur.Init(&mainRenderTarget.GetRenderTargetTexture());

    // step-3 ボケ画像合成用のスプライトを初期化する
	SpriteInitData combineBokeImagerSpriteInitData;

    //使用するテクスチャは２枚
	combineBokeImagerSpriteInitData.m_textures[0] = &blur.GetBokeTexture();
	combineBokeImagerSpriteInitData.m_textures[1] = &depthRenderTarget.GetRenderTargetTexture();
	combineBokeImagerSpriteInitData.m_width = 1280;
	combineBokeImagerSpriteInitData.m_height = 720;

    //合成用シェーダーをしていする
	combineBokeImagerSpriteInitData.m_fxFilePath = "Assets/shader/samplePostEffect.fx";
	combineBokeImagerSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    //距離を利用してボケ画像をアルファブレンディングするので、半透明合成モードにする
    combineBokeImagerSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Trans;

	//初期化オブジェクトを利用してスプライトを初期化する
    Sprite combineBokeImageSprite;
	combineBokeImageSprite.Init(combineBokeImagerSpriteInitData);

    // メインレンダリングターゲットの絵をフレームバッファにコピーするためのスプライトを初期化
    // スプライトの初期化オブジェクトを作成する
    SpriteInitData spriteInitData;

    // テクスチャはyBlurRenderTargetのカラーバッファー
    spriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();

    // レンダリング先がフレームバッファーなので、解像度はフレームバッファーと同じ
    spriteInitData.m_width = 1280;
    spriteInitData.m_height = 720;

    // ボケ画像をそのまま貼り付けるだけなので、通常の2D描画のシェーダーを指定する
    spriteInitData.m_fxFilePath = "Assets/shader/preset/sample2D.fx";

    // 初期化オブジェクトを使って、スプライトを初期化する
    Sprite copyToFrameBufferSprite;
    copyToFrameBufferSprite.Init(spriteInitData);

    //モデルを初期化
    Model model;
    InitModel(model, light);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //カメラを動かす。
        MoveCamera();

        // step-4 2枚のレンダリングターゲットを設定して、モデルを描画する
        //２枚のレンダリングターゲットのポインタを持つ配列を定義する
        RenderTarget* rts[] = {
            &mainRenderTarget,
            &depthRenderTarget
        };

        //レンダリングターゲットとして利用できるまで待つ
		renderContext.WaitUntilToPossibleSetRenderTargets(2, rts);

        //レンダリングターゲットを設定
		renderContext.SetRenderTargetsAndViewport(2, rts);

        //レンダリングターゲットをクリア
		renderContext.ClearRenderTargetViews(2, rts);

        //モデルをドロー
		model.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
		renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);

        // step-5 メインレンダリングターゲットのボケ画像を作成
        blur.ExecuteOnGPU(renderContext, 5);

        // step-6 ボケ画像と深度テクスチャを利用して、ボケ画像を描きこんでいく
        //レンダリングターゲットを設定
        renderContext.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
		renderContext.SetRenderTargetAndViewport(mainRenderTarget);

        //スプライトを描画
		combineBokeImageSprite.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
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

void InitModel(Model& model, Light& light)
{
    // 光を強めに設定する
    light.directionalLight[0].color.x = 2.0f;
    light.directionalLight[0].color.y = 2.0f;
    light.directionalLight[0].color.z = 2.0f;

    light.directionalLight[0].direction.x = 0.0f;
    light.directionalLight[0].direction.y = 0.0f;
    light.directionalLight[0].direction.z = -1.0f;
    light.directionalLight[0].direction.Normalize();

    light.ambinetLight.x = 0.5f;
    light.ambinetLight.y = 0.5f;
    light.ambinetLight.z = 0.5f;

    light.eyePos = g_camera3D->GetPosition();

    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/sample3D.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R32_FLOAT;

    model.Init(modelInitData);
}
