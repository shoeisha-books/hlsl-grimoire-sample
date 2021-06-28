#pragma once

namespace raytracing {
	extern 	ID3D12Resource* CreateBuffer(
		ID3D12Device5* pDevice,
		uint64_t size,
		D3D12_RESOURCE_FLAGS flags,
		D3D12_RESOURCE_STATES initState,
		const D3D12_HEAP_PROPERTIES& heapProps
	);

	struct AccelerationStructureBuffers {
		ID3D12Resource* pScratch = nullptr;
		ID3D12Resource* pResult = nullptr;
		ID3D12Resource* pInstanceDesc = nullptr;
	};

	//ヒットグループ。
	enum EHitGroup {
		eHitGroup_Undef = -1,
		eHitGroup_PBRCameraRay,	//PBRマテリアルにカメラレイが衝突するときのヒットグループ。
		eHitGroup_PBRShadowRay,	//PBRマテリアルにシャドウレイが衝突するときのヒットグループ。
		eHitGroup_Num,			//ヒットグループの数。
	};

	const D3D12_HEAP_PROPERTIES kUploadHeapProps =
	{
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0,
	};

	const D3D12_HEAP_PROPERTIES kDefaultHeapProps =
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

	const int MAX_TRACE_RECURSION_DEPTH = 4;	//レイトレースの再帰呼び出しの最大数。
													//これがTraceRayを再帰的に呼び出せる最大数です。
													//
		//ローカルルートシグネチャ
	enum ELocalRootSignature {
		eLocalRootSignature_Empty,				//空のローカルルートシグネチャ。
		eLocalRootSignature_Raygen,				//レイ生成シェーダー用のローカルルートシグネチャ。
		eLocalRootSignature_PBRMaterialHit,		//PBRマテリアルにヒットしたときのローカルルートシグネチャ。
	};
	//シェーダー
	enum EShader {
		eShader_Raygeneration,		//カメラレイを生成するシェーダー。
		eShader_Miss,				//カメラレイがどこにもぶつからなかった時に呼ばれるシェーダー。
		eShader_PBRChs,				//もっとも近いポリゴンとカメラレイが交差したときに呼ばれるシェーダー。
		eShader_ShadowChs,			//もっとも近いポリゴンとシャドウレイが交差したときに呼ばれるシェーダー。
		eShader_ShadowMiss,			//シャドウレイがどこにもぶつからなかった時に呼ばれるシェーダー。
		eShader_Num,				//シェーダーの数。
	};
	//シェーダーのカテゴリ。
	enum EShaderCategory {
		eShaderCategory_RayGenerator,	//レイを生成するシェーダー。
		eShaderCategory_Miss,			//ミスシェーダー。
		eShaderCategory_ClosestHit,		//もっとも近いポリゴンとレイが交差したときに呼ばれるシェーダー。
	};


	//シェーダーデータ構造体。
	struct ShaderData {
		const wchar_t* entryPointName;				//エントリーポイントの名前。
		ELocalRootSignature useLocalRootSignature;	//使用するローカルルートシグネチャ。
		EShaderCategory category;					//カテゴリー。
		EHitGroup hitgroup;							//ヒットグループ。
													//カテゴリがeShaderCategory_Miss、eShaderCategory_RayGeneratorの場合、このパラメーターは無視されます。
	};
	const ShaderData shaderDatas[] = {
		//entryPointName	useLocalRootSignature				category						hitgroup
		{ L"rayGen",		eLocalRootSignature_Raygen,			eShaderCategory_RayGenerator,	eHitGroup_Undef },
		{ L"miss",			eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
		{ L"chs",			eLocalRootSignature_PBRMaterialHit,	eShaderCategory_ClosestHit,		eHitGroup_PBRCameraRay },
		{ L"shadowChs",		eLocalRootSignature_PBRMaterialHit,	eShaderCategory_ClosestHit,		eHitGroup_PBRShadowRay },
		{ L"shadowMiss",	eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
	};

	static_assert(ARRAYSIZE(shaderDatas) == eShader_Num, "shaderDatas arraySize is invalid!! shaderDatas arraySize must be equal to eShader_Num");


	struct SHitGroup {
		const wchar_t* name;				//ヒットグループの名前。
		const wchar_t* chsHitShaderName;	//最も近いポリゴンにヒットしたときに呼ばれるシェーダーの名前。
		const wchar_t* anyHitShaderName;	//any shader???
	};
	//ヒットグループの名前の配列。
	const SHitGroup hitGroups[] = {
		{ L"HitGroup",			shaderDatas[eShader_PBRChs].entryPointName,	nullptr },
		{ L"ShadowHitGroup",	shaderDatas[eShader_ShadowChs].entryPointName, nullptr },
	};


	/// <summary>
	/// シェーダーテーブルに登録されているSRVの1要素
	/// </summary>
	/// <remarks>
	/// この列挙子の並びがtレジスタの番号になります。
	/// シェーダーテーブルには各インスタンスごとにシェーダーリソースのディスクリプタが登録されています。
	/// この列挙子が各インスタンスに割り当てられているシェーダーリソースを表しています。
	/// </remarks>
	enum class ESRV_OneEntry {
		eStartRayGenerationSRV ,				//レイジェネレーションシェーダーで利用するSRVの開始番号。
			eTLAS = eStartRayGenerationSRV,		//TLAS
		eEndRayGenerationSRV,					//レイジェネレーションで使用されるSRVの数。
			eAlbedoMap = eEndRayGenerationSRV,	//アルベドマップ。
			eNormalMap,							//法線マップ。
			eSpecularMap,						//スペキュラマップ。
			eReflectionMap,						//リフレクションマップ。
			eRefractionMap,						//屈折マップ。
			eVertexBuffer,						//頂点バッファ。
			eIndexBuffer,						//インデックスバッファ。
		eNum,			//SRVの数。
		eNumRayGenerationSRV = eEndRayGenerationSRV - eStartRayGenerationSRV,	//レイジェネレーションシェーダーで使用するSRVの数。
	};
	/// <summary>
	/// ヒットシェーダーのディスクリプタテーブル
	/// </summary>
	enum EHitShaderDescriptorTable{
		eHitShaderDescriptorTable_SRV_CBV,	//SRVとCBV
		eHitShaderDescriptorTable_Sampler,	//サンプラ
		eHitShaderDescriptorTable_Num       //テーブルの数。
	};
	struct Instance;
	using InstancePtr = std::unique_ptr< Instance>;
	using ID3D12ResourcePtr = CComPtr<ID3D12Resource>;
	using ID3D12RootSignaturePtr = CComPtr<ID3D12RootSignature>;
	using ID3DBlobPtr = CComPtr<ID3DBlob>;
	using ID3D12DescriptorHeapPtr = CComPtr<ID3D12DescriptorHeap>;
}//namespace raytracing

#include "RaytracingWorld.h"
