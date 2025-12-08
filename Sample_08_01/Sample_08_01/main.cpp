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

	// step-1 ルートシグネチャを作成
	RootSignature rootSignature;
	InitRootSignature(rootSignature);

	// step-2 シェーダーをロード
	Shader vs, ps;

	//頂点シェーダーをロード
	vs.LoadVS("Assets/shader/sample.fx", "VSMain");

	//ピクセルシェーダーをロード
	ps.LoadPS("Assets/shader/sample.fx", "PSMain");

	// step-3 パイプラインステートを作成
	PipelineState pipelineState;
	InitPipelineState(pipelineState, rootSignature, vs, ps);

	// step-4 四角形の板ポリの頂点バッファを作成
	//頂点配列を定義
	SimpleVertex vertices[] = {
		{
			{-1.0f,-1.0f,0.0f,1.0f},//座標
			{0.0f,1.0f}//左上             //UV座標
		},
		{
			{ 1.0f,1.0f,0.0f,1.0f },//座標
			{1.0f,0.0f}//右下             //UV座標
		},
		{
			{ 1.0f,-1.0f,0.0f,1.0f },//座標
			{ 1.0f,1.0f }//右上           //UV座標
		},
		{
			{ -1.0f,1.0f,0.0f,1.0f },//座標
			{ 0.0f,0.0f }//左下            //UV座標
		}
	};

	//頂点配列から頂点バッファを作成
	VertexBuffer triangleVB;
	triangleVB.Init(sizeof(vertices), sizeof(vertices[0]));
	triangleVB.Copy(vertices);

	// step-5 板ポリのインデックスバッファを作成
	//インデックス配列
	uint16_t indices[]={
		0, 1, 2,//三角形1つ目
		0, 3, 1 //三角形2つ目
	};

	//インデックス配列からインデックスバッファーを作成する
	IndexBuffer triangleIB;
	triangleIB.Init(sizeof(indices), 2);
	triangleIB.Copy(indices);

	// step-6 テクスチャをロード
	Texture texture;
	texture.InitFromDDSFile(L"Assets/image/test.dds");

	// step-7 ディスクリプタヒープを作成
	DescriptorHeap ds;
	ds.RegistShaderResource(0, texture);
	ds.Commit();

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

		// step-8 ドローコールを実行
		//ルートシグネチャを設定
		renderContext.SetRootSignature(rootSignature);

		//パイプラインステートを設定
		renderContext.SetPipelineState(pipelineState);

		//プリミティブのトポロジーを設定
		renderContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点バッファーを設定
		renderContext.SetVertexBuffer(triangleVB);

		//インデックスバッファーを設定
		renderContext.SetIndexBuffer(triangleIB);

		//ディスクリプタヒープを登録
		renderContext.SetDescriptorHeap(ds);

		//ドローコール
		renderContext.DrawIndexed(6);//引数はインデックスの数

		/// //////////////////////////////////////
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
