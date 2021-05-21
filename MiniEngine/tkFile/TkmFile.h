/// <summary>
/// tkmファイル。
/// </summary>
/// <remarks>
/// tkmファイルは3Dモデルフォーマットです。
/// このクラスを利用することでtkmファイルを扱うことができます。。
/// </remarks>

#pragma once

	
/// <summary>
/// tkmファイルクラス。
/// </summary>
class  TkmFile {
public:
	/// <summary>
	/// マテリアル
	/// </summary>
	struct SMaterial {
		std::string albedoMapFileName;			//アルベドマップのファイル名。
		std::string normalMapFileName;			//法線マップのファイル名。
		std::string specularMapFileName;		//スペキュラマップのファイル名。
		std::string reflectionMapFileName;		//リフレクションマップのファイル名。
		std::string refractionMapFileName;		//屈折マップのファイル名。
		std::unique_ptr<char[]>	albedoMap;		//ロードされたアルベドマップ。(ddsファイル)
		unsigned int albedoMapSize;				//アルベドマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	normalMap;		//ロードされた法線マップ。(ddsファイル)
		unsigned int normalMapSize;				//法線マップのサイズ。
		std::unique_ptr<char[]>	specularMap;	//ロードされたスペキュラマップ。(ddsファイル)
		unsigned int specularMapSize;			//スペキュラマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	reflectionMap;	//ロードされたリフレクションマップ。(ddsファイル)
		unsigned int reflectionMapSize;			//リフレクションマップのサイズ。(ddsファイル)
		std::unique_ptr<char[]>	refractionMap;	//ロードされた屈折マップ。(ddsファイル)
		unsigned int refractionMapSize;			//屈折マップのサイズ。(ddsファイル)
	};
	/// <summary>
	/// 頂点。
	/// </summary>
	/// <remarks>
	/// ここを編集したら、レイトレのシェーダーで定義されている頂点構造体も変更する必要がある。
	/// </remarks>
	struct SVertex {
		Vector3 pos;			//座標。
		Vector3 normal;		//法線。
		Vector3 tangent;		//接ベクトル。
		Vector3 binormal;		//従ベクトル。
		Vector2 uv;			//UV座標。
		int indices[4];			//スキンインデックス。
		Vector4 skinWeights;	//スキンウェイト。
	};
	/// <summary>
	/// 32ビットのインデックスバッファ。
	/// </summary>
	struct SIndexBuffer32 {
		std::vector< uint32_t > indices;	//インデックス。
	};
	/// <summary>
	/// 16ビットのインデックスバッファ。
	/// </summary>
	struct SIndexbuffer16 {
		std::vector< uint16_t > indices;	//インデックス。
	};
	/// <summary>
	/// メッシュパーツ。
	/// </summary>
	struct SMesh {
		bool isFlatShading;							//フラットシェーディング？
		std::vector< SMaterial > materials;				//マテリアルの配列。
		std::vector< SVertex >	vertexBuffer;			//頂点バッファ。
		std::vector<SIndexBuffer32> indexBuffer32Array;	//インデックスバッファの配列。マテリアルの数分だけインデックスバッファはあるよ。
		std::vector< SIndexbuffer16> indexBuffer16Array;
	};
		
	/// <summary>
	/// 3Dモデルをロード。
	/// </summary>
	/// <param name="filePath">ファイルパス。</param>
	void Load(const char* filePath);
		
	/// <summary>
	/// メッシュパーツに対してクエリを行う。
	/// </summary>
	/// <param name="func">クエリ関数</param>
	void QueryMeshParts(std::function<void(const SMesh& mesh)> func) const
	{
		for (auto& mesh : m_meshParts) {
			func(mesh);
		}
	}
	/// <summary>
	/// メッシュパーツを取得。
	/// </summary>
	/// <returns></returns>
	const std::vector< SMesh>& GetMeshParts() const
	{
		return m_meshParts;
	}
	/// <summary>
	/// メッシュの数を取得。
	/// </summary>
	/// <returns></returns>
	int GetNumMesh() const
	{
		return (int)(m_meshParts.size());
	}
private:
	/// <summary>
	/// テクスチャ名をロード。
	/// </summary>
	/// <param name="fp"></param>
	/// <returns></returns>
	std::string LoadTextureFileName(FILE* fp);
	/// <summary>
	/// インデックスバッファをロード。
	/// </summary>
	template<class T>
	void LoadIndexBuffer(std::vector<T>& indexBuffer, int numIndex, FILE* fp);
	/// <summary>
	/// マテリアルを構築。
	/// </summary>
	/// <param name="tkmMat"></param>
	void BuildMaterial(SMaterial& tkmMat, FILE* fp, const char* filePath);
	/// <summary>
	/// 接ベクトルと従ベクトルを計算する。
	/// </summary>
	/// <remarks>
	/// 3dsMaxScriptでやるべきなんだろうけど、デバッグしたいので今はこちらでやる。
	/// </remarks>
	void BuildTangentAndBiNormal();
private:
	std::vector< SMesh>	m_meshParts;		//メッシュパーツ。
};
