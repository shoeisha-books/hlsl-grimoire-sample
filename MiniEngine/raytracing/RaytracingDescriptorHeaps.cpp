#include "stdafx.h"
#include "RaytracingDescriptorHeaps.h"
#include "Material.h"

namespace raytracing {
	void DescriptorHeaps::Init(
		World& world,
		GPUBuffer& outputBuffer,
		ConstantBuffer& rayGeneCB
	)
	{

		//レイトレの出力先をディスクリプタヒープに登録する。
		m_srvUavCbvHeap.RegistUnorderAccessResource(0, outputBuffer);
		m_srvUavCbvHeap.RegistConstantBuffer(0, rayGeneCB);
		int regNo = 0;
		world.QueryInstances([&](Instance& instance)
		{
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eTLAS,
				world.GetTLASBuffer()
			);
			//アルベドマップをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eAlbedoMap,
				instance.m_material->GetAlbedoMap()
			);
			//法線マップをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eNormalMap,
				instance.m_material->GetNormalMap()
			);
			//スペキュラマップをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eSpecularMap,
				instance.m_material->GetSpecularMap()
			);

			//リフレクションマップをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eReflectionMap,
				instance.m_material->GetReflectionMap()
			);

			//屈折マップをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eRefractionMap,
				instance.m_material->GetRefractionMap()
			);
			//頂点バッファをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eVertexBuffer,
				instance.m_vertexBufferRWSB
			);
			//インデックスバッファをディスクリプタヒープに登録。
			m_srvUavCbvHeap.RegistShaderResource(
				regNo + (int)ESRV_OneEntry::eIndexBuffer,
				instance.m_indexBufferRWSB
			);
			regNo += (int)ESRV_OneEntry::eNum;

		});

		//サンプラステートの扱いは仮。
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		//サンプラステートをディスクリプタヒープに登録する。
		m_samplerDescriptorHeap.RegistSamplerDesc(0, samplerDesc);
		m_samplerDescriptorHeap.CommitSamperHeap();
		m_srvUavCbvHeap.Commit();
	}
}