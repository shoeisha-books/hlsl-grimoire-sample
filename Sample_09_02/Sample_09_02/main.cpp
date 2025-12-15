#include "stdafx.h"
#include "system/system.h"

// 頂点構造体
struct SimpleVertex
{
    Vector4 pos;    // 頂点座標
    Vector2 uv;     // UV座標
};

// 関数宣言
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

    // step-1 ワイプパラメータ構造体を定義する
    struct WipePram
    {
        Vector2 wipeDir;//【注目】ワイプする方向
		float wipeSize;// ワイプの大きさ
    };

    // step-2 ワイプパラメータを初期化する
	WipePram wipeParam;

    //ワイプする方向を設定する
    //今回は右下に向かってワイプする
    wipeParam.wipeDir.Set(1.0f, 1.0f);
	wipeParam.wipeDir.Normalize();

    //ワイプサイズを初期化する
	wipeParam.wipeSize = 0.0f;

    // まずはSpriteクラスの初期化オブジェクトを作成する
    SpriteInitData spriteInitData;

    // テクスチャのファイルパスを指定
    spriteInitData.m_ddsFilePath[0] = "Assets/image/test.dds";

    // シェーダーファイルのパスを指定
    spriteInitData.m_fxFilePath = "Assets/shader/sample.fx";

    // スプライトの幅と高さを指定
    spriteInitData.m_width = FRAME_BUFFER_W;
    spriteInitData.m_height = FRAME_BUFFER_H;
    spriteInitData.m_expandConstantBuffer = &wipeParam;
    spriteInitData.m_expandConstantBufferSize = sizeof(wipeParam);

    // Spriteクラスのオブジェクトを定義して初期化する
    Sprite test2D;

    // Init()に初期化オブジェクトを渡して初期化する
    test2D.Init(spriteInitData);

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
        wipeParam.wipeSize += 2.0f;

        // スプライトのドローコールを実行する
        test2D.Draw(renderContext);

        //////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
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
