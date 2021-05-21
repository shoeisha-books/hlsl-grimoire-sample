#pragma once

namespace raytracing {
	/// <summary>
	/// レイトレーシングで使用するディスクリプタヒープ
	/// </summary>
	class DescriptorHeaps {
	public:
		/// <summary>
		/// 各種リソースをディスクリプタヒープに登録していく。
		/// </summary>
		/// <param name="world"></param>
		/// <param name="outputBuffer"></param>
		/// <param name="rayGeneCB"></param>
		void Init(
			World& world, 
			GPUBuffer& outputBuffer, 
			ConstantBuffer& rayGeneCB
		);
		/// <summary>
		/// シェーダーリソース、アンオーダーアクセスシェーダーリソース、定数バッファの
		/// ディスクリプタが登録されているディスクリプタヒープを取得。
		/// </summary>
		/// <returns></returns>
		const DescriptorHeap& GetSrvUavCbvDescriptorHeap() const
		{
			return m_srvUavCbvHeap;
		}
		/// <summary>
		/// サンプラのディスクリプタが登録されているディスクリプタヒープを取得。
		/// </summary>
		/// <returns></returns>
		const DescriptorHeap& GetSamplerDescriptorHeap() const
		{
			return m_samplerDescriptorHeap;
		}
	private:
		DescriptorHeap m_srvUavCbvHeap;			//SRV_UAV_CBV用のディスクリプタヒープ。
		DescriptorHeap m_samplerDescriptorHeap;	//サンプラ用のディスクリプタヒープ。
	};
}