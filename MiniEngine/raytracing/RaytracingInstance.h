#pragma once

class Material;

namespace raytracing {
	using ID3D12DescriptorHeapPtr = CComPtr<ID3D12DescriptorHeap>;

	/// <summary>
	/// レイトレのインスタンスデータ。
	/// </summary>
	struct Instance {
		D3D12_RAYTRACING_GEOMETRY_DESC geometoryDesc;	//ジオメトリ情報。
		RWStructuredBuffer m_vertexBufferRWSB;			//頂点バッファ。
		RWStructuredBuffer m_indexBufferRWSB;			//インデックスバッファ。
		Material* m_material = nullptr;					//マテリアル。		
	};
}//namespace raytracing