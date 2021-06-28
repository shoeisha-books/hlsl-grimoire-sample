#include "stdafx.h"

#include "RaytracingEngine.h"
#include "Material.h"

namespace raytracing {
	using namespace std;

	ID3D12Resource* CreateBuffer(
		ID3D12Device5* pDevice,
		uint64_t size,
		D3D12_RESOURCE_FLAGS flags,
		D3D12_RESOURCE_STATES initState,
		const D3D12_HEAP_PROPERTIES& heapProps)
	{
		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Alignment = 0;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Flags = flags;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.Height = 1;
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufDesc.MipLevels = 1;
		bufDesc.SampleDesc.Count = 1;
		bufDesc.SampleDesc.Quality = 0;
		bufDesc.Width = size;

		ID3D12Resource* pBuffer;
		pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer));
		return pBuffer;
	}

	void Engine::CreateShaderResources()
	{
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();

		// レイトレの結果の出力先のテクスチャを作成。
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.DepthOrArraySize = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB formats can't be used with UAVs. We will convert to sRGB ourselves in the shader
		resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resDesc.Height = g_graphicsEngine->GetFrameBufferHeight();
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.Width = g_graphicsEngine->GetFrameBufferWidth();
		m_outputResource.Init(resDesc);

		//レイジェネレーション用の定数バッファ。
		Camera cam;
		cam.pos = g_camera3D->GetPosition();
		cam.mRot = g_camera3D->GetCameraRotation();
		cam.aspect = g_camera3D->GetAspect();
		cam.fNear = g_camera3D->GetNear();
		cam.fFar = g_camera3D->GetFar();
		m_rayGenerationCB.Init(sizeof(Camera), &cam);

	}

	void Engine::Dispatch(RenderContext& rc)
	{
		//カリカリ
		Camera cam;
		cam.pos = g_camera3D->GetPosition();
		cam.mRot = g_camera3D->GetCameraRotation();
		cam.aspect = g_camera3D->GetAspect();
		cam.fNear = g_camera3D->GetNear();
		cam.fFar = g_camera3D->GetFar();
		m_rayGenerationCB.CopyToVRAM(cam);

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_outputResource.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		rc.ResourceBarrier(barrier);

		D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
		raytraceDesc.Width = g_graphicsEngine->GetFrameBufferWidth();
		raytraceDesc.Height = g_graphicsEngine->GetFrameBufferHeight();
		raytraceDesc.Depth = 1;


		auto shaderTableEntrySize = m_shaderTable.GetShaderTableEntrySize();
		auto numRayGenShader = m_shaderTable.GetNumRayGenShader();
		auto numMissShader = m_shaderTable.GetNumMissShader();
		auto numHitShader = m_shaderTable.GetNumHitShader();

		// レイ生成シェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
		raytraceDesc.RayGenerationShaderRecord.StartAddress = m_shaderTable.GetGPUVirtualAddress();
		raytraceDesc.RayGenerationShaderRecord.SizeInBytes = shaderTableEntrySize;

		// ミスシェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
		size_t missOffset = numRayGenShader * shaderTableEntrySize;
		raytraceDesc.MissShaderTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + missOffset;
		raytraceDesc.MissShaderTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.MissShaderTable.SizeInBytes = shaderTableEntrySize * numMissShader;

		// ヒットグループシェーダーの開始アドレスとサイズを設定。
		size_t hitOffset = (numRayGenShader + numMissShader) * shaderTableEntrySize;
		raytraceDesc.HitGroupTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + hitOffset;
		raytraceDesc.HitGroupTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.HitGroupTable.SizeInBytes = shaderTableEntrySize * numHitShader * m_world.GetNumInstance();

		// グローバルルートシグネチャを設定。
		rc.SetComputeRootSignature(m_pipelineStateObject.GetGlobalRootSignature());

		// Dispatch
		//グローバルルートシグネチチャに登録されているディスクリプタヒープを登録する。
		const DescriptorHeap* descriptorHeaps[] = {
			&m_descriptorHeaps.GetSrvUavCbvDescriptorHeap(),
			&m_descriptorHeaps.GetSamplerDescriptorHeap()
		};
		rc.SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

		rc.SetPipelineState(m_pipelineStateObject);
		rc.DispatchRays(raytraceDesc);

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_outputResource.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		rc.ResourceBarrier(barrier);

		//レイトレの結果をフレームバッファに書き戻す。
		g_graphicsEngine->CopyToFrameBuffer(rc, m_outputResource.Get());

	}

	void Engine::CommitRegistGeometry(RenderContext& rc)
	{
		g_graphicsEngine->BeginRender();
		m_world.CommitRegistGeometry(rc);
		//シェーダーリソースを作成。
		CreateShaderResources();
		//各種リソースをディスクリプタヒープに登録する。
		m_descriptorHeaps.Init(m_world, m_outputResource, m_rayGenerationCB);
		//PSOを作成。
		m_pipelineStateObject.Init(m_descriptorHeaps);
		//シェーダーテーブルを作成。
		m_shaderTable.Init(m_world, m_pipelineStateObject, m_descriptorHeaps);
		
		g_graphicsEngine->EndRender();
	}
}//namespace raytracing