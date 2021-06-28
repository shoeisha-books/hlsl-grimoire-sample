#include "stdafx.h"
#include "RaytracingPSO.h"
#include "RaytracingDescriptorHeaps.h"

namespace raytracing {
	namespace {

		
		static_assert(ARRAYSIZE(hitGroups) == eHitGroup_Num, "hitGroups arraySize is invalid!! hitGroups arraySize must be equal to eHitGoup_Num");

		ID3D12RootSignaturePtr CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc, const wchar_t* name)
		{
			auto d3dDevice = g_graphicsEngine->GetD3DDevice();
			ID3DBlobPtr pSigBlob;
			ID3DBlobPtr pErrorBlob;
			HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
			if (FAILED(hr))
			{
				MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "エラー", MB_OK);
				std::abort();
			}
			ID3D12RootSignaturePtr pRootSig;
			d3dDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig));
			pRootSig->SetName(name);
			return pRootSig;
		}
	}
	//サブオブジェクト作成のヘルパー。
	namespace BuildSubObjectHelper {
		/// <summary>
		/// ローカル√シグネチャのサブオブジェクト作成のヘルパー構造体。
		/// </summary>
		struct LocalRootSignatureSubobject {
			LocalRootSignatureSubobject()
			{
			}
			void Init(const D3D12_ROOT_SIGNATURE_DESC& desc, const wchar_t* name)
			{
				pRootSig = CreateRootSignature(desc, name);
				pInterface = pRootSig;
				subobject.pDesc = &pInterface;
				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			}
			ID3D12RootSignaturePtr pRootSig;
			ID3D12RootSignature* pInterface = nullptr;
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// ExportAssociationのサブオブジェクト作成のヘルパー構造体。
		/// </summary>
		struct ExportAssociationSubobject
		{
			void Init(const WCHAR* exportNames[], uint32_t exportCount, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
			{
				association.NumExports = exportCount;
				association.pExports = exportNames;
				association.pSubobjectToAssociate = pSubobjectToAssociate;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				subobject.pDesc = &association;
			}
			D3D12_STATE_SUBOBJECT subobject = {};
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = {};
		};
		struct ShaderConfigSubobject
		{
			void Init(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
			{
				shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
				shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
				subobject.pDesc = &shaderConfig;
			}
			D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// パイプライン設定のサブオブジェクト作成のヘルパー構造体。
		/// </summary>
		struct PipelineConfigSubobject
		{
			PipelineConfigSubobject()
			{
				config.MaxTraceRecursionDepth = MAX_TRACE_RECURSION_DEPTH;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
				subobject.pDesc = &config;
			}

			D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// グローバルルートシグネチャのサブオブジェクト作成のヘルパー構造体。
		/// </summary>
		struct GlobalRootSignatureSubobject
		{
			GlobalRootSignatureSubobject()
			{
				pRootSig = CreateRootSignature({}, L"GlobalRootSignature");
				pInterface = pRootSig;
				subobject.pDesc = &pInterface;
				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
			}
			ID3D12RootSignaturePtr pRootSig;
			ID3D12RootSignature* pInterface = nullptr;
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// ヒットグループサブオブジェクト
		/// </summary>
		struct HitGroupSubObject
		{
			HitGroupSubObject() {}
			void Init(const SHitGroup& hitgroup)
			{
				desc = {};
				desc.AnyHitShaderImport = hitgroup.anyHitShaderName;
				desc.ClosestHitShaderImport = hitgroup.chsHitShaderName;
				desc.HitGroupExport = hitgroup.name;

				subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				subObject.pDesc = &desc;
			}
			D3D12_HIT_GROUP_DESC desc;
			D3D12_STATE_SUBOBJECT subObject;
		};
	};
	PSO::RootSignatureDesc PSO::CreateRayGenRootSignatureesc()
	{
		// Create the root-signature
		RootSignatureDesc desc;
		desc.range.resize(3);
		// gOutput
		desc.range[0].BaseShaderRegister = 0;
		desc.range[0].NumDescriptors = 1;
		desc.range[0].RegisterSpace = 0;
		desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		desc.range[0].OffsetInDescriptorsFromTableStart = m_srvUavCbvHeap->GetOffsetUAVDescriptorFromTableStart();

		// gRtScene
		desc.range[1].BaseShaderRegister = 0;
		desc.range[1].NumDescriptors = (int)ESRV_OneEntry::eNumRayGenerationSRV;
		desc.range[1].RegisterSpace = 0;
		desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		desc.range[1].OffsetInDescriptorsFromTableStart = 1;

		desc.range[2].BaseShaderRegister = 0;
		desc.range[2].NumDescriptors = 1;
		desc.range[2].RegisterSpace = 0;
		desc.range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		desc.range[2].OffsetInDescriptorsFromTableStart = 0;

		desc.rootParams.resize(1);
		desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[0].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(desc.range.size());
		desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();

		// Create the desc
		desc.desc.NumParameters = 1;
		desc.desc.pParameters = desc.rootParams.data();
		desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		return desc;
	}
	PSO::RootSignatureDesc PSO::CreatePBRMatterialHitRootSignatureDesc()
	{
		RootSignatureDesc desc;

		enum ERange {
			eRange_SRV,		//SRV
			eRange_Sampler,	//サンプラ
			eRange_Num,		//範囲の数。
		};
		desc.range.resize(eRange_Num);

		desc.rootParams.resize(eHitShaderDescriptorTable_Num);

		desc.range[eRange_SRV].BaseShaderRegister = 0;
		desc.range[eRange_SRV].NumDescriptors = (int)ESRV_OneEntry::eNum;
		desc.range[eRange_SRV].RegisterSpace = 0;
		desc.range[eRange_SRV].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		desc.range[eRange_SRV].OffsetInDescriptorsFromTableStart = 0;

		desc.range[eRange_Sampler].BaseShaderRegister = 0;
		desc.range[eRange_Sampler].NumDescriptors = 1;
		desc.range[eRange_Sampler].RegisterSpace = 0;
		desc.range[eRange_Sampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		desc.range[eRange_Sampler].OffsetInDescriptorsFromTableStart = 0;

		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].DescriptorTable.NumDescriptorRanges = 1;
		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].DescriptorTable.pDescriptorRanges = &desc.range[0];

		desc.rootParams[eHitShaderDescriptorTable_Sampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[eHitShaderDescriptorTable_Sampler].DescriptorTable.NumDescriptorRanges = 1;
		desc.rootParams[eHitShaderDescriptorTable_Sampler].DescriptorTable.pDescriptorRanges = &desc.range[1];


		desc.desc.NumParameters = static_cast<UINT>(desc.rootParams.size());
		desc.desc.pParameters = desc.rootParams.data();
		desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		return desc;
	}

	void PSO::Init(const DescriptorHeaps& descriptorHeaps)
	{
		m_srvUavCbvHeap = &descriptorHeaps.GetSrvUavCbvDescriptorHeap();
		using namespace BuildSubObjectHelper;

		std::array<D3D12_STATE_SUBOBJECT, 14> subobjects;
		uint32_t index = 0;

		//DXILライブラリを作成。
		//レイトレーシング用のシェーダーをロード。
		Shader raytraceShader;
		raytraceShader.LoadRaytracing(L"Assets/shader/sample.fx");

		D3D12_EXPORT_DESC libExport[eShader_Num];
		for (int i = 0; i < eShader_Num; i++) {
			libExport[i].Name = shaderDatas[i].entryPointName;
			libExport[i].ExportToRename = nullptr;
			libExport[i].Flags = D3D12_EXPORT_FLAG_NONE;
		};

		D3D12_DXIL_LIBRARY_DESC dxLibdesc;
		auto pBlob = raytraceShader.GetCompiledDxcBlob();
		dxLibdesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
		dxLibdesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
		dxLibdesc.NumExports = ARRAYSIZE(libExport);
		dxLibdesc.pExports = libExport;

		subobjects[index].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobjects[index].pDesc = &dxLibdesc;

		index++;

		//HitGroupのサブオブジェクトを作成。
		std::vector< HitGroupSubObject> hitGroupSOs;
		hitGroupSOs.resize(eHitGroup_Num);
		for (int i = 0; i < eHitGroup_Num; i++) {
			hitGroupSOs[i].Init(hitGroups[i]);
			subobjects[index++] = hitGroupSOs[i].subObject; // 1 Hit Group
		}

		auto BuildAndRegistRootSignatureAndAssSubobjectFunc = [&](
			LocalRootSignatureSubobject& rsSO, ExportAssociationSubobject& ass, ELocalRootSignature eRS, const WCHAR* exportNames[]
			) {
			if (eRS == eLocalRootSignature_Raygen) {
				rsSO.Init(CreateRayGenRootSignatureesc().desc, L"RayGenRootSignature");
			}
			if (eRS == eLocalRootSignature_PBRMaterialHit) {
				rsSO.Init(CreatePBRMatterialHitRootSignatureDesc().desc, L"PBRMaterialHitGenRootSignature");
			}
			if (eRS == eLocalRootSignature_Empty) {
				D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
				emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
				rsSO.Init(emptyDesc, L"EmptyRootSignature");
			}
			subobjects[index] = rsSO.subobject;
			uint32_t rgSOIndex = index++;

			int useRootSignature = 0;
			for (auto& shaderData : shaderDatas) {
				if (shaderData.useLocalRootSignature == eRS) {
					//こいつは指定されたルートシグネチャを使う。
					exportNames[useRootSignature] = shaderData.entryPointName;
					useRootSignature++;
				}
			}
			ass.Init(exportNames, useRootSignature, &(subobjects[rgSOIndex]));
			subobjects[index++] = ass.subobject;
		};
		//ルートシグネチャとシェーダーの関連付けを行うサブオブジェクトを作っていく。
		LocalRootSignatureSubobject rayGenSignatureSO, modelSignatureSO, emptySignatureSO;
		ExportAssociationSubobject rayGenAssSO, modelAssSO, emptyAssSO;
		const WCHAR* rayGenExportName[eShader_Num];
		const WCHAR* modelExportName[eShader_Num];
		const WCHAR* emptyExportName[eShader_Num];

		BuildAndRegistRootSignatureAndAssSubobjectFunc(rayGenSignatureSO, rayGenAssSO, eLocalRootSignature_Raygen, rayGenExportName);
		BuildAndRegistRootSignatureAndAssSubobjectFunc(modelSignatureSO, modelAssSO, eLocalRootSignature_PBRMaterialHit, modelExportName);
		BuildAndRegistRootSignatureAndAssSubobjectFunc(emptySignatureSO, emptyAssSO, eLocalRootSignature_Empty, emptyExportName);


		// Payloadのサイズと引数の数はとりあえず固定で・・・。後で検討。
		ShaderConfigSubobject shaderConfig;
		struct RayPayload
		{
			Vector4 color;
			Vector4 reflectionColor;
			Vector4 hit_depth;
		};
		shaderConfig.Init(sizeof(float) * 2, sizeof(RayPayload));
		subobjects[index] = shaderConfig.subobject; // 

		uint32_t shaderConfigIndex = index++;
		ExportAssociationSubobject configAssociationSO;
		const WCHAR* entryPointNames[eShader_Num];
		for (int i = 0; i < eShader_Num; i++) {
			entryPointNames[i] = shaderDatas[i].entryPointName;
		}
		configAssociationSO.Init(entryPointNames, eShader_Num, &subobjects[shaderConfigIndex]);
		subobjects[index++] = configAssociationSO.subobject;

		// パイプライン設定のサブオブジェクトを作成。
		PipelineConfigSubobject config;
		subobjects[index++] = config.subobject;

		// グローバルルートシグネチャのサブオブジェクトを作成。
		GlobalRootSignatureSubobject root;
		m_emptyRootSignature = root.pRootSig;
		subobjects[index++] = root.subobject;

		// Create the state
		D3D12_STATE_OBJECT_DESC desc;
		desc.NumSubobjects = index;
		desc.pSubobjects = subobjects.data();
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		auto hr = d3dDevice->CreateStateObject(&desc, IID_PPV_ARGS(&m_pipelineState));
		if (FAILED(hr)) {
			MessageBox(nullptr, L"パイプラインステートの作成に失敗しました。\n", L"エラー", MB_OK);
			std::abort();
		}
	}
}