#pragma once

/// <summary>
/// インデックスバッファ。
/// </summary>
/// <remarks>
/// </remarks>
class IndexBuffer {
public:
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~IndexBuffer(); 
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="size">インデックスバッファのサイズ。</param>
	/// <param name="stride">ストライド。</param>
	void Init(int size, int stride) ;
	/// <summary>
	/// インデックスデータをインデックスバッファにコピー。
	/// </summary>
	/// <param name="srcIndecies">コピー元のインデックスデータ。</param>
	void Copy(uint16_t* srcIndecies) ;
	/// <summary>
	/// インデックスデータをインデックスバッファにコピー。
	/// </summary>
	/// <param name="srcIndecies">コピー元のインデックスデータ。</param>
	void Copy(uint32_t* srcIndecies);
	/// <summary>
	/// インデックスバッファビューを取得。
	/// </summary>
	/// <returns></returns>
	const D3D12_INDEX_BUFFER_VIEW& GetView() const
	{
		return m_indexBufferView;
	}
	/// <summary>
	/// インデックスの数を取得。
	/// </summary>
	/// <returns>インデックスの数。</returns>
	int GetCount() const
	{
		return m_count;
	}
	/// <summary>
	/// インデックスバッファのストライドを取得。
	/// </summary>
	/// ストライドというのは、１要素のサイズのこと。
	/// ここでは、一つのインデックスのサイズを取得することになる。
	/// 2か4が返ってきます。
	/// <returns></returns>
	UINT GetStrideInBytes() const
	{
		return static_cast<UINT>(m_strideInBytes);
	}
	/// <summary>
	/// インデックスバッファのサイズ(単位：バイト)を取得。
	/// </summary>
	/// <returns></returns>
	UINT GetSizeInBytes() const
	{
		return static_cast<UINT>(m_sizeInBytes);
	}
	/// <summary>
	/// ID3D12Resourceのアドレスを取得します。
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetID3DResourceAddress() const
	{
		return m_indexBuffer;
	}
private:
private:
	ID3D12Resource* m_indexBuffer = nullptr;	//インデックスバッファ。
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;	//インデックスバッファビュー。
	int m_count = 0;							//インデックスの数。
	int m_strideInBytes = 0;					//ストライド(単位：バイト)。
	int m_sizeInBytes = 0;						//サイズ(単位：バイト)。
};