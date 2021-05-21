#pragma once


class ConstantBuffer{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	ConstantBuffer() {}
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~ConstantBuffer();
	/// <summary>
	/// ムーブコンストラクタ
	/// </summary>
	/// <param name="r"></param>
	ConstantBuffer(ConstantBuffer&& r)
	{
		m_constantBuffer[0] = r.m_constantBuffer[0];
		m_constantBuffer[1] = r.m_constantBuffer[1];
		m_constBufferCPU[0] = r.m_constBufferCPU[0];
		m_constBufferCPU[1] = r.m_constBufferCPU[1];
		m_size = r.m_size;
		m_allocSize = r.m_allocSize;
		m_isValid = r.m_isValid;

		r.m_constantBuffer[0] = nullptr;
		r.m_constantBuffer[1] = nullptr;
		r.m_constBufferCPU[0] = nullptr;
		r.m_constBufferCPU[1] = nullptr;
	}
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="size">定数バッファのサイズ。</param>
	/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
	void Init(int size, void* srcData = nullptr);
	/// <summary>
	/// 利用可能なバッファかどうかを判定。
	/// </summary>
	/// <returns></returns>
	bool IsValid() const
	{
		return m_isValid;
	}
	/// <summary>
	/// データをVRAMにコピーする。
	/// </summary>
	/// <param name="data"></param>
	void CopyToVRAM(void* data);
	template< class T>
	void CopyToVRAM(T& data)
	{
		CopyToVRAM(&data);
	}
	/// <summary>
	/// ディスクリプタヒープにConstantBufferViewを登録。
	/// </summary>
	/// <param name="descriptorHandle"></param>
	void RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
	void RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo);
	/// <summary>
	/// VRAM上の仮想アドレスを取得する。
	/// </summary>
	/// <returns></returns>
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();
	
private:
	ID3D12Resource* m_constantBuffer[2] = {nullptr};//定数バッファ。
	void* m_constBufferCPU[2] = { nullptr };		//CPU側からアクセスできるする定数バッファのアドレス。
	int m_size = 0;									//定数バッファのサイズ。
	int m_allocSize = 0;
	bool m_isValid = false;							//利用可能？
};
