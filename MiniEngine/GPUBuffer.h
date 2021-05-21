#pragma once

/// <summary>
/// GPUバッファクラス
/// </summary>
/// <remarks>
/// 汎用的なGPUバッファクラスです。
/// テクスチャ、ストラクチャードバッファ、頂点バッファすべてのバッファを作成することができます。
/// </remarks>
class GPUBuffer : public IUnorderAccessResrouce {
public:
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~GPUBuffer()
	{
		if (m_buffer != nullptr) {
			m_buffer->Release();
		}
	}
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="desc"></param>
	void Init(D3D12_RESOURCE_DESC desc);
	/// <summary>
	/// UAVとしてディスクリプタヒープに登録
	/// </summary>
	/// <param name="cpuHandle">登録するディスクリプタヒープのCPUハンドル</param>
	/// <param name="bufferNo">バッファ番号</param>
	void RegistUnorderAccessView(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, int bufferNo) override;
	/// <summary>
	/// SRVとしてディスクリプタヒープに登録。
	/// </summary>
	/// <param name="descriptorHandle">登録するディスクリプタヒープのCPUハンドル</param>
	/// <param name="bufferNo">バッファ番号</param>
	void RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo) override;
	
	ID3D12Resource* Get()
	{
		return m_buffer;
	}
private:
	ID3D12Resource* m_buffer = nullptr;
	D3D12_RESOURCE_DESC m_desc;
};