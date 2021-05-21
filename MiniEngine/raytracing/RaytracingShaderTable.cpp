#include "stdafx.h"
#include "RaytracingShaderTable.h"
#include "RaytracingPSO.h"

#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

namespace raytracing {

	void ShaderTable::CountupNumGeyGenAndMissAndHitShader()
	{
		//各シェーダーの数を調査。
		m_numRayGenShader = 0;
		m_numMissShader = 0;
		m_numHitShader = 0;
		for (auto& shaderData : shaderDatas) {
			if (shaderData.category == eShaderCategory_RayGenerator) m_numRayGenShader++;
			if (shaderData.category == eShaderCategory_Miss) m_numMissShader++;
			if (shaderData.category == eShaderCategory_ClosestHit) m_numHitShader++;
		}
	}

	void ShaderTable::CalcShaderTableEntrySize()
	{
		//シェーダーテーブルに登録されるデータの１要素分のサイズを計算。
		//シェーダー識別子。
		m_shaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		//ディスクリプタテーブルのアドレス。
		m_shaderTableEntrySize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE) * eHitShaderDescriptorTable_Num; // The hit shader
		//アライメントをそろえる。
		m_shaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, m_shaderTableEntrySize);
	}
	void ShaderTable::Init(
		const World& world,
		const PSO& pso,
		const DescriptorHeaps& descriptorHeaps
	)
	{
		
		//各シェーダーの数をカウントする。
		CountupNumGeyGenAndMissAndHitShader();
		
		//シェーダーテーブルに登録されるデータの１要素分のサイズを計算。
		CalcShaderTableEntrySize();

		//シェーダーテーブルのサイズを計算。
		int shaderTableSize = m_shaderTableEntrySize * (m_numRayGenShader + m_numMissShader + (m_numHitShader * world.GetNumInstance()));

		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		//シェーダーテーブル用のバッファを作成。
		m_shaderTable = CreateBuffer(d3dDevice, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);

		//バッファをシステムメモリにマップする。
		uint8_t* pData;
		m_shaderTable->Map(0, nullptr, (void**)&pData);

		CComPtr< ID3D12StateObjectProperties > pRtsoProps;
		pso.QueryInterface(pRtsoProps);

		uint8_t* pCurret = pData;

		auto ds_size_cbv_srv_uav = g_graphicsEngine->GetCbrSrvDescriptorSize(); 
		auto hitGroup_pbrCameraRaySrvHeapStride =
			ds_size_cbv_srv_uav * (int)ESRV_OneEntry::eNum;

		const auto& srvUavCbvDescriptorHeap = descriptorHeaps.GetSrvUavCbvDescriptorHeap();
		const auto& samplerDescriptorHeap = descriptorHeaps.GetSamplerDescriptorHeap();

		uint64_t hitGroup_pbrCameraRaySrvHeapStart = srvUavCbvDescriptorHeap.Get()->GetGPUDescriptorHandleForHeapStart().ptr + ds_size_cbv_srv_uav;
		//シェーダーテーブルにシェーダーを登録する関数。
		auto RegistShaderTblFunc = [&](const ShaderData& shaderData, EShaderCategory registCategory, Instance* instance) {
			if (shaderData.category == registCategory) {
				//まずシェーダーIDを設定する。
				void* pShaderId = nullptr;
				if (registCategory == eShaderCategory_ClosestHit) {
					pShaderId = pRtsoProps->GetShaderIdentifier(hitGroups[shaderData.hitgroup].name);
				}
				else {
					pShaderId = pRtsoProps->GetShaderIdentifier(shaderData.entryPointName);
				}
				memcpy(pCurret, pShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				uint8_t* pDst = pCurret + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
				if (shaderData.useLocalRootSignature == eLocalRootSignature_Raygen) {
					//デフォルトのルートシグネチャを使用する場合、シェーダーIDの後ろにディスクリプタヒープへのアドレスを設定する。
					*(uint64_t*)(pDst) = srvUavCbvDescriptorHeap.Get()->GetGPUDescriptorHandleForHeapStart().ptr;
				}
				if (shaderData.useLocalRootSignature == eLocalRootSignature_PBRMaterialHit) {
					//SRV_CBVのディスクリプタヒープ				
					*(uint64_t*)(pDst) = hitGroup_pbrCameraRaySrvHeapStart;
					pDst += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
					*(uint64_t*)(pDst) = samplerDescriptorHeap.GetSamplerResourceGpuDescritorStartHandle().ptr;

				}
				//次。
				pCurret += m_shaderTableEntrySize;
			}
		};
		// レイジェネレーションシェーダーをテーブルに登録していく。
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_RayGenerator, nullptr);
		}
		// 続いてミスシェーダー。
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_Miss, nullptr);
		}
		//最後にヒットシェーダー。ヒットシェーダーはヒットシェーダーの数　×　インスタンスの数だけ登録する。
		world.QueryInstances([&](Instance& instance) {
			for (auto& shader : shaderDatas) {
				RegistShaderTblFunc(shader, eShaderCategory_ClosestHit, &instance);
			};
			hitGroup_pbrCameraRaySrvHeapStart += hitGroup_pbrCameraRaySrvHeapStride;	//次
		});

		//Unmap
		m_shaderTable->Unmap(0, nullptr);
	}
}