#include "stdafx.h"
#include "BLASBuffer.h"

namespace raytracing {
	


	void BLASBuffer::Init(RenderContext& rc, const std::vector<InstancePtr>& instances)
	{
		for (auto& instance : instances) {
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			inputs.NumDescs = 1;
			inputs.pGeometryDescs = &instance->geometoryDesc;
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

			auto d3dDevice = g_graphicsEngine->GetD3DDevice();
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
			d3dDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

			AccelerationStructureBuffers asbuffer;
			asbuffer.pScratch = CreateBuffer(
				d3dDevice,
				info.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COMMON,
				kDefaultHeapProps);

			asbuffer.pResult = CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps);

			// Create the bottom-level AS
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
			asDesc.Inputs = inputs;
			asDesc.DestAccelerationStructureData = asbuffer.pResult->GetGPUVirtualAddress();
			asDesc.ScratchAccelerationStructureData = asbuffer.pScratch->GetGPUVirtualAddress();

			rc.BuildRaytracingAccelerationStructure(asDesc);

			//レイトレーシングアクセラレーション構造のビルド完了待ちのバリアを入れる。
			D3D12_RESOURCE_BARRIER uavBarrier = {};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = asbuffer.pResult;
			rc.ResourceBarrier(uavBarrier);

			m_bottomLevelASBuffers.push_back(std::move(asbuffer));
		}
	}
}//namespace raytracing