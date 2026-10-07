#include "stdafx.h"
#include "system/system.h"

// 頂点構造体
struct SimpleVertex
{
	float pos[3];       // 頂点座標
	float color[3];     // 頂点カラー
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

	// 1. ルートシグネチャを作成
	RootSignature rootSignature;
	InitRootSignature(rootSignature);

	// 2. シェーダーをロード
	Shader vs, ps;
	vs.LoadVS("Assets/shader/sample.fx", "VSMain");
	ps.LoadPS("Assets/shader/sample.fx", "PSMain");

	// 3. パイプラインステートを作成
	PipelineState pipelineState;
	InitPipelineState(pipelineState, rootSignature, vs, ps);

	// 4. 三角形の頂点バッファを作成
	// 頂点配列を定義
	SimpleVertex vertices[] = {
		////元の三角形
		//{
		//    {-0.5f, -0.5f, 0.0f},   //左下：0
		//    { 1.0f, 0.0f, 0.0f }    //赤
		//},
		//{
		//    { 0.0f, 0.5f, 0.0f },   //上：1
		//    { 1.0f, 1.0f, 1.0f }    //白
		//},
		//{
		//    { 0.5f, -0.5f, 0.0f },  //右下：2
		//    { 0.0f, 0.0f, 1.0f }    //青
		//}

		////二等辺三角形
		//{
		//    {-0.5f, -0.5f, 0.0f},   //左下：0
		//    { 1.0f, 0.0f, 0.0f }    //赤
		//},
		//{
		//    { 0.0f, 1.0f, 0.0f },   //上：1
		//    { 1.0f, 1.0f, 1.0f }    //白
		//},
		//{
		//    { 0.5f, -0.5f, 0.0f },  //右下：2
		//    { 0.0f, 0.0f, 1.0f }    //青
		//},

		////長方形
		//{
		//    { -0.5f, 0.5f, 0.0f },  //左上：0
		//    { 0.0f, 0.0f, 0.0f }    //黒
		//},
		//{
		//    { -0.5f, -0.5f, 0.0f }, //左下：1
		//    { 0.0f, 0.0f, 1.0f }    //青
		//},
		//{
		//    {0.5f, 0.5f, 0.0f},     //右上：2
		//    { 1.0f, 0.0f, 0.0f }    //赤
		//},
		//{
		//    { 0.5f, -0.5f, 0.0f },  //右下:3
		//    { 1.0f, 1.0f, 1.0f }    //白
		//}

		////五角形
		//{
		//	{ -0.4f, 0.25f, 0.0f },   //左上：0
		//	{ 0.0f, 0.0f, 0.0f }      //黒
		//},
		//{
		//	{ -0.25f, -0.5f, 0.0f },  //左下：1
		//	{ 0.0f, 0.0f, 1.0f }      //青
		//},
		//{
		//	{ 0.4f, 0.25f, 0.0f},     //右上：2
		//	{ 1.0f, 0.0f, 0.0f }      //赤
		//},
		//{
		//	{ 0.25f, -0.5f, 0.0f },   //右下:3
		//	{ 1.0f, 1.0f, 1.0f }      //白
		//},
		//{
		//	{ 0.0f, 0.75f, 0.0f },    //上:4
		//	{ 0.0f, 1.0f, 0.0f }      //緑
		//}

		//縞模様
		//左
		{
			{ -0.5f, 0.5f, 0.0f },    //0
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ -0.5f, 0.0f, 0.0f },    //1
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ -0.5f, -0.5f, 0.0f},    //2
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		//真ん中
		{
			{ 0.0f, 0.5f, 0.0f },     //3
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ 0.0f, 0.0f, 0.0f },     //4
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ 0.0f, -0.5f, 0.0f },    //5
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		//右
		{
			{ 0.5f, 0.5f, 0.0f},      //6
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ 0.5f, 0.0f, 0.0f },     //7
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ 0.5f, -0.5f, 0.0f },    //8
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		//五角形
		{
			{ -0.25f, 0.75f, 0.0f },  //9
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ 0.0f, 1.0f, 0.0f },     //10
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		//おまけ※ひし形
		{
			{ 0.0f, -1.0f, 0.0f },    //11
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ 0.25f, -0.75f, 0.0f },  //12
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ 0.75f, -0.25f, 0.0f },  //13
			{ 1.0f, 1.0f, 1.0f }      //白
		},
		{
			{ 1.0f, 0.0f, 0.0f },     //14
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ -1.0f, 0.0f, 0.0f },    //15
			{ 0.0f, 0.0f, 0.0f }      //黒
		},
		{
			{ -0.75f, 0.25f, 0.0f },  //16
			{ 1.0f, 1.0f, 1.0f }      //白
		}


	};

	VertexBuffer triangleVB;
	triangleVB.Init(sizeof(vertices), sizeof(vertices[0]));
	triangleVB.Copy(vertices);

	// 5. 三角形のインデックスバッファを作成
	//インデックス配列
	uint16_t indices[] = {
		////三角形
		//0,1,2

		////長方形
		//0,1,3,
		//0,2,3

		////五角形
		//0,1,3,
		//0,2,3,
		//0,2,4

		//縞模様
		0,1,4,
		0,3,4,
		1,2,5,
		1,4,5,
		3,4,7,
		3,6,7,
		4,5,8,
		4,7,8,
		0,3,9,
		//+五角形
		9,3,10,
		10,3,6,
		//+おまけ（ひし形）
		2,5,11,
		5,11,12,
		5,8,12,

		6,7,14,
		7,8,13,
		7,13,14,
		
		1,2,15,
		1,15,16,
		0,1,16,
	};
	IndexBuffer triangleIB;
	triangleIB.Init(sizeof(indices), 2);
	triangleIB.Copy(indices);

	//////////////////////////////////////
	// 初期化を行うコードを書くのはここまで！！！
	//////////////////////////////////////
	auto& renderContext = g_graphicsEngine->GetRenderContext();

	// ここからゲームループ
	while (DispatchWindowMessage())
	{
		// レンダリング開始
		g_engine->BeginFrame();

		//////////////////////////////////////
		// ここから絵を描くコードを記述する
		//////////////////////////////////////

		// 1. ルートシグネチャを設定
		renderContext.SetRootSignature(rootSignature);
		// 2. パイプラインステートを設定
		renderContext.SetPipelineState(pipelineState);
		// 3. プリミティブのトポロジーを設定
		renderContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 4. 頂点バッファを設定
		renderContext.SetVertexBuffer(triangleVB);
		// 5. インデックスバッファを設定
		renderContext.SetIndexBuffer(triangleIB);
		// 6. ドローコール
		////※三角形
		//renderContext.DrawIndexed(3);
		////※四角形
		//renderContext.DrawIndexed(6);
		////※五角形
		//renderContext.DrawIndexed(9);

		//下のにすればいちいち変更する必要がない
		renderContext.DrawIndexed(sizeof(indices)/sizeof(indices[0]));

		/// //////////////////////////////////////
		// 絵を描くコードを書くのはここまで！！！
		//////////////////////////////////////
		// レンダリング終了
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
