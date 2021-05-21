#pragma once

class RenderContext;

//頂点構造体
struct SimpleVertex {
	float pos[3];		//頂点座標。
	float color[3];		//頂点カラー。
};

/// <summary>
/// 三角形ポリゴン。
/// </summary>
class TrianglePolygon
{
public:
	/// <summary>
	/// 初期化。
	/// </summary>
	/// /// <param name="rs">ルートシグネチャ</param>
	void Init( RootSignature& rs );
	/// <summary>
	/// 描画。
	/// </summary>
	/// <param name="rc">レンダリングコンテキスト</param>
	void Draw(RenderContext& rc);
private:
	/// <summary>
	/// パイプラインステートを初期化。
	/// </summary>
	/// <param name="rs">ルートシグネチャ</param>
	void InitPipelineState(RootSignature& rs);
	/// <summary>
	/// シェーダーをロード。
	/// </summary>
	void LoadShaders();
	/// <summary>
	/// 頂点バッファを作成。
	/// </summary>
	void InitVertexBuffer();
	/// <summary>
	/// インデックスバッファを作成。
	/// </summary>
	void InitIndexBuffer();
private:
	VertexBuffer m_vertexBuffer;		//頂点バッファ。
	IndexBuffer m_indexBuffer;		//インデックスバッファ。
	Shader m_vertexShader;			//頂点シェーダー。
	Shader m_pixelShader;				//ピクセルシェーダー。
	PipelineState m_pipelineState;	//パイプラインステート。
};

