#pragma once
#include "RaytracingInstance.h"
class RenderContext;

namespace raytracing {
	/// <summary>
	/// TLASBuffer
	/// </summary>
	/// <remark>
	/// TLAS( Top level acceleration structures )とは
	/// レイトレワールドに登録されているインスタンスのデータ構造です。
	/// インスタンスの使用するジオメトリ、シェーダーID、ワールド行列などの
	/// データを保持します。
	/// </remark>
	class TLASBuffer : public IShaderResource
	{
	public:
		/// <summary>
		/// TLASを構築。
		/// </summary>
		/// <param name="rc"></param>
		/// <param name="instances"></param>
		void Init(
			RenderContext& rc,
			const std::vector<InstancePtr>& instances,
			const std::vector< AccelerationStructureBuffers>& bottomLevelASBuffers
		);
		/// <summary>
		/// SRVに登録。
		/// </summary>
		/// <param name="descriptorHandle"></param>
		void RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo) override;
		/// <summary>
		/// VRAM上の仮想アドレスを取得。
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}
	private:
		AccelerationStructureBuffers m_topLevelASBuffers;
	};
}//namespace raytracing