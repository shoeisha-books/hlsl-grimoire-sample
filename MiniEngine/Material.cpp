#include "stdafx.h"
#include "Material.h"

//ルートシグネチャとパイプラインステート周りはカリカリカリ
enum {
	enDescriptorHeap_CB,
	enDescriptorHeap_SRV,
	enNumDescriptorHeap
};
	
void Material::InitTexture(const TkmFile::SMaterial& tkmMat)
{

	const auto& nullTextureMaps = g_graphicsEngine->GetNullTextureMaps();

	const char* filePath = nullptr;
	char* map = nullptr;
	unsigned int mapSize;

	//アルベドマップ。
	{
		if (tkmMat.albedoMap != nullptr)
		{
			filePath = tkmMat.albedoMapFilePath.c_str();
			map = tkmMat.albedoMap.get();
			mapSize = tkmMat.albedoMapSize;
		}
		else
		{
			filePath = nullTextureMaps.GetAlbedoMapFilePath();
			map = nullTextureMaps.GetAlbedoMap().get();
			mapSize = nullTextureMaps.GetAlbedoMapSize();
		}
		auto albedoMap = g_engine->GetTextureFromBank(filePath);

		if (albedoMap == nullptr)
		{
			albedoMap = new Texture();
			albedoMap->InitFromMemory(map, mapSize);
			g_engine->RegistTextureToBank(filePath, albedoMap);
		}
		m_albedoMap = albedoMap;
	}


	//法線マップ。
	{
		if (tkmMat.normalMap != nullptr)
		{
			filePath = tkmMat.normalMapFilePath.c_str();
			map = tkmMat.normalMap.get();
			mapSize = tkmMat.normalMapSize;
		}
		else
		{
			filePath = nullTextureMaps.GetNormalMapFilePath();
			map = nullTextureMaps.GetNormalMap().get();
			mapSize = nullTextureMaps.GetNormalMapSize();
		}
		auto normalMap = g_engine->GetTextureFromBank(filePath);

		if (normalMap == nullptr)
		{
			normalMap = new Texture();
			normalMap->InitFromMemory(map, mapSize);
			g_engine->RegistTextureToBank(filePath, normalMap);
		}
		m_normalMap = normalMap;
	}



	//スペキュラマップ。
	{
		if (tkmMat.specularMap != nullptr)
		{
			filePath = tkmMat.specularMapFilePath.c_str();
			map = tkmMat.specularMap.get();
			mapSize = tkmMat.specularMapSize;
		}
		else
		{
			filePath = nullTextureMaps.GetSpecularMapFilePath();
			map = nullTextureMaps.GetSpecularMap().get();
			mapSize = nullTextureMaps.GetSpecularMapSize();
		}
		auto specularMap = g_engine->GetTextureFromBank(filePath);

		if (specularMap == nullptr)
		{
			specularMap = new Texture();
			specularMap->InitFromMemory(map, mapSize);
			g_engine->RegistTextureToBank(filePath, specularMap);
		}
		m_specularMap = specularMap;
	}

	//反射マップ。
	{
		if (tkmMat.reflectionMap != nullptr)
		{
			filePath = tkmMat.reflectionMapFilePath.c_str();
			map = tkmMat.reflectionMap.get();
			mapSize = tkmMat.reflectionMapSize;
		}
		else
		{
			filePath = nullTextureMaps.GetReflectionMapFilePath();
			map = nullTextureMaps.GetReflectionMap().get();
			mapSize = nullTextureMaps.GetReflectionMapSize();
		}
		auto reflectionMap = g_engine->GetTextureFromBank(filePath);

		if (reflectionMap == nullptr)
		{
			reflectionMap = new Texture();
			reflectionMap->InitFromMemory(map, mapSize);
			g_engine->RegistTextureToBank(filePath, reflectionMap);
		}
		m_reflectionMap = reflectionMap;
	}

	//屈折マップ。
	{
		if (tkmMat.refractionMap != nullptr)
		{
			filePath = tkmMat.refractionMapFilePath.c_str();
			map = tkmMat.refractionMap.get();
			mapSize = tkmMat.refractionMapSize;
		}
		else
		{
			filePath = nullTextureMaps.GetRefractionMapFilePath();
			map = nullTextureMaps.GetRefractionMap().get();
			mapSize = nullTextureMaps.GetRefractionMapSize();
		}
		auto refractionMap = g_engine->GetTextureFromBank(filePath);

		if (refractionMap == nullptr)
		{
			refractionMap = new Texture();
			refractionMap->InitFromMemory(map, mapSize);
			g_engine->RegistTextureToBank(filePath, refractionMap);
		}
		m_refractionMap = refractionMap;
	}


}
void Material::InitFromTkmMaterila(
	const TkmFile::SMaterial& tkmMat,
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntryPointFunc,
	const char* psEntryPointFunc,
	const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
	int numSrv,
	int numCbv,
	UINT offsetInDescriptorsFromTableStartCB,
	UINT offsetInDescriptorsFromTableStartSRV,
	D3D12_FILTER samplerFilter
)
{
	//テクスチャをロード。
	InitTexture(tkmMat);
	
	//定数バッファを作成。
	SMaterialParam matParam;
	matParam.hasNormalMap = m_normalMap->IsValid() ? 1 : 0;
	matParam.hasSpecMap = m_specularMap->IsValid() ? 1 : 0;
	m_constantBuffer.Init(sizeof(SMaterialParam), &matParam);

	//ルートシグネチャを初期化。
	D3D12_STATIC_SAMPLER_DESC samplerDescArray[2];
	//デフォルトのサンプラ
	samplerDescArray[0].Filter = samplerFilter;
	samplerDescArray[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].MipLODBias = 0;
	samplerDescArray[0].MaxAnisotropy = 0;
	samplerDescArray[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDescArray[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDescArray[0].MinLOD = 0.0f;
	samplerDescArray[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescArray[0].ShaderRegister = 0;
	samplerDescArray[0].RegisterSpace = 0;
	samplerDescArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//シャドウマップ用のサンプラ。
	samplerDescArray[1] = samplerDescArray[0];
	//比較対象の値が小さければ０、大きければ１を返す比較関数を設定する。
	samplerDescArray[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDescArray[1].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
	samplerDescArray[1].MaxAnisotropy = 1;
	samplerDescArray[1].ShaderRegister = 1;

	m_rootSignature.Init(
		samplerDescArray,
		2,
		numCbv,
		numSrv,
		8,
		offsetInDescriptorsFromTableStartCB,
		offsetInDescriptorsFromTableStartSRV
	);

	if (fxFilePath != nullptr && strlen(fxFilePath) > 0) {
		//シェーダーを初期化。
		InitShaders(fxFilePath, vsEntryPointFunc, vsSkinEntryPointFunc, psEntryPointFunc);
		//パイプラインステートを初期化。
		InitPipelineState(colorBufferFormat);
	}
}
void Material::InitPipelineState(const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat)
{
	// 頂点レイアウトを定義する。
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//パイプラインステートを作成。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel->GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel->GetCompiledBlob());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
#ifdef SAMPLE_11
	// 背面を描画していないと影がおかしくなるため、
	// シャドウのサンプルのみカリングをオフにする。
	// 本来はアプリ側からカリングモードを渡すのがいいのだけど、
	// 書籍に記載しているコードに追記がいるので、エンジン側で吸収する。
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
#else
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
#endif
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
#ifdef TK_ENABLE_ALPHA_TO_COVERAGE
	psoDesc.BlendState.AlphaToCoverageEnable = TRUE;
#endif
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	int numRenderTarget = 0;
	for (auto& format : colorBufferFormat) {
		if (format == DXGI_FORMAT_UNKNOWN) {
			//フォーマットが指定されていない場所が来たら終わり。
			break;
		}
		psoDesc.RTVFormats[numRenderTarget] = colorBufferFormat[numRenderTarget];
		numRenderTarget++;
	}
	psoDesc.NumRenderTargets = numRenderTarget;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	m_skinModelPipelineState.Init(psoDesc);

	//続いてスキンなしモデル用を作成。
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel->GetCompiledBlob());
	m_nonSkinModelPipelineState.Init(psoDesc);

	//続いて半透明マテリアル用。
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel->GetCompiledBlob());
	psoDesc.BlendState.IndependentBlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	
	m_transSkinModelPipelineState.Init(psoDesc);

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel->GetCompiledBlob());
	m_transNonSkinModelPipelineState.Init(psoDesc);

}
void Material::InitShaders(
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntriyPointFunc,
	const char* psEntryPointFunc
)
{
	//スキンなしモデル用のシェーダーをロードする。
	m_vsNonSkinModel = g_engine->GetShaderFromBank(fxFilePath, vsEntryPointFunc);
	if (m_vsNonSkinModel == nullptr) {
		m_vsNonSkinModel = new Shader;
		m_vsNonSkinModel->LoadVS(fxFilePath, vsEntryPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, vsEntryPointFunc, m_vsNonSkinModel);
	}
	//スキンありモデル用のシェーダーをロードする。
	m_vsSkinModel = g_engine->GetShaderFromBank(fxFilePath, vsSkinEntriyPointFunc);
	if (m_vsSkinModel == nullptr) {
		m_vsSkinModel = new Shader;
		m_vsSkinModel->LoadVS(fxFilePath, vsSkinEntriyPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, vsSkinEntriyPointFunc, m_vsSkinModel);
	}

	m_psModel = g_engine->GetShaderFromBank(fxFilePath, psEntryPointFunc);
	if (m_psModel == nullptr) {
		m_psModel = new Shader;
		m_psModel->LoadPS(fxFilePath, psEntryPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, psEntryPointFunc, m_psModel);
	}
}
void Material::BeginRender(RenderContext& rc, int hasSkin)
{
	rc.SetRootSignature(m_rootSignature);
	
	if (hasSkin) {
	//	rc.SetPipelineState(m_skinModelPipelineState);
		rc.SetPipelineState(m_transSkinModelPipelineState);
	}
	else {
	//	rc.SetPipelineState(m_nonSkinModelPipelineState);
		rc.SetPipelineState(m_transNonSkinModelPipelineState);
	}
}
