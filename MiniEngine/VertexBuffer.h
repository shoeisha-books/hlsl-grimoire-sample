#pragma once


/// <summary>
/// 頂点バッファ
/// </summary>
class VertexBuffer {
		
public:
	/// <summary>
	/// デストラクタ。
	/// </summary>
	~VertexBuffer();
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="size">頂点バッファのサイズ</param>
	/// <param name="stride">頂点バッファのストライド。一個の頂点のサイズ</param>
	void Init(int size, int stride);
	/// <summary>
	/// 頂点データを頂点バッファにコピー。
	/// </summary>
	/// <param name="srcVertices">コピー元の頂点データ。</param>
	void Copy(void* srcVertices) ;
	/// <summary>
	/// 頂点バッファビューを取得。
	/// </summary>
	/// <returns>ビュー</returns>
	const D3D12_VERTEX_BUFFER_VIEW& GetView() const
	{
		return m_vertexBufferView;
	}
	/// <summary>
	/// 頂点バッファのストライド(単位：バイト)を取得。
	/// </summary>
	/// <remarks>
	/// ストライドというのは、１要素のサイズのこと。
	/// ここでは、一つの頂点のサイズを取得することになる。
	/// </remarks>
	/// <returns></returns>
	UINT GetStrideInBytes() const
	{
		return m_vertexBufferView.StrideInBytes;
	}
	/// <summary>
	/// 頂点バッファのサイズ(単位：バイト)を取得。
	/// </summary>
	/// <returns></returns>
	UINT GetSizeInBytes() const
	{
		return m_vertexBufferView.SizeInBytes;
	}
	/// <summary>
	/// ID3D12Resourceのアドレスを取得します。
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* GetID3DResourceAddress() const
	{
		return m_vertexBuffer;
	}
private:
	ID3D12Resource* m_vertexBuffer = nullptr;		//頂点バッファ。
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;	//頂点バッファビュー。
};