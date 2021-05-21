#pragma once

class VertexBuffer;
class IndexBuffer;

/// <summary>
/// 構造化バッファ
/// </summary>
class RWStructuredBuffer : public IUnorderAccessResrouce {
public:
	
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~RWStructuredBuffer();
	/// <summary>
	/// 構造化バッファを初期化。
	/// </summary>
	/// <param name="sizeOfElement">エレメントのサイズ。</param>
	/// <param name="numElement">エレメントの数。</param>
	/// <param name="initData">初期データ。</param>
	void Init(int sizeOfElement, int numElement, void* initData);
	/// <summary>
	/// 頂点バッファを使用して構造化バッファを初期化。
	/// </summary>
	/// <param name="vb">頂点バッファ</param>
	/// <param name="isUpdateByCPU">
	/// CPU側で頂点バッファを更新するかどうか。
	/// CPU側で更新する場合、引数で渡された頂点バッファの複製を二つ複製します。
	/// そのため、メモリの使用量は３倍になります。
	/// </param>
	void Init(const VertexBuffer& vb, bool isUpdateByCPU);
	/// <summary>
	/// インデックスバッファを利用して構造化バッファを初期化。
	/// </summary>
	/// <param name="ib">インデックスバッファ</param>
	/// <param name="isUpdateByCPU">
	/// CPU側でインデックスバッファを更新するかどうか。
	/// CPU側で更新する場合、引数で渡されたインデックスバッファの複製を二つ複製します。
	/// そのため、メモリの使用量は３倍になります。
	/// </param>
	void Init(const IndexBuffer& ib, bool isUpdateByCPU);
	/// <summary>
	/// UAVに登録。
	/// </summary>
	/// <param name=""></param>
	/// <param name="bufferNo"></param>
	void RegistUnorderAccessView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo) override;
	/// <summary>
	/// SRVに登録。
	/// </summary>
	/// <param name="descriptorHandle"></param>
	/// <param name="bufferNo"></param>
	void RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo) override;
	/// <summary>
	/// 初期化されているか判定。
	/// </summary>
	/// <returns></returns>
	bool IsInited() const
	{
		return m_isInited;
	}
	/// <summary>
	/// CPUからアクセス可能なリソースを取得する。
	/// </summary>
	/// <returns></returns>
	void* GetResourceOnCPU();
	
	ID3D12Resource* GetD3DResoruce() ;
private:
	ID3D12Resource* m_buffersOnGPU[2] = {nullptr};
	void* m_buffersOnCPU[2] = { nullptr };		//CPU側からアクセスできるするストラクチャバッファのアドレス。
	int m_numElement = 0;				//要素数。
	int m_sizeOfElement = 0;			//エレメントのサイズ。
	bool m_isInited = false;			//初期化済み？
};