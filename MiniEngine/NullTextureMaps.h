#pragma once

/// <summary>
/// Nullテクスチャマップを保持しているクラス。
/// </summary>
/// <remarks>
/// このクラスが保持しているのは、
/// スペキュラマップ、法線マップ、反射率マップなどの
/// マテリアルで使用するテクスチャマップです。
/// このクラスのマップはマテリアルに、上記のマップが
/// 貼り付けられていない場合です。
/// ですので、このクラスが保持しているマップは、スペキュラマップであれば
/// 鏡面反射率０、法線マップであれば凸凹なし、リフレクションマップであれば反射率０となります。
/// マテリアルにマップが貼り付けられていない場合に、このクラスのマップを利用することで、
/// シェーダーでの動的分岐を減らすことができ、シェーダ―プログラムをシンプルにすることができるようになっています。
/// このクラスでロードされているテクスチャは、Assets/modelData/presetの下に用意されています。
/// </remarks>
class NullTextureMaps {
public:
	/// <summary>
	/// 初期化。
	/// </summary>
	void Init();
	/// <summary>
	/// 法線マップを取得。
	/// </summary>
	/// <returns></returns>
	const std::unique_ptr<char[]>& GetNormalMap() const
	{
		return m_normalMap;
	}
	/// <summary>
	/// 法線マップのサイズを取得。
	/// </summary>
	/// <returns></returns>
	unsigned int GetNormalMapSize() const
	{
		return m_normalMapSize;
	}
	const char* GetNormalMapFilePath() const
	{
		return m_normalMapFilePath;
	}
	/// <summary>
	/// スペキュラマップを取得。
	/// </summary>
	/// <returns></returns>
	const std::unique_ptr<char[]>& GetSpecularMap() const
	{
		return m_specMap;
	}
	/// <summary>
	/// スペキュラマップのサイズを取得。
	/// </summary>
	/// <returns></returns>
	unsigned int GetSpecularMapSize() const
	{
		return m_specMapSize;
	}
	const char* GetSpecularMapFilePath() const
	{
		return m_specMapFilePath;
	}
	/// <summary> 
	/// アルベドマップを取得。
	/// </summary>
	/// <returns></returns>
	const std::unique_ptr<char[]>& GetAlbedoMap() const
	{
		return m_albedoMap;
	}
	/// <summary>
	/// アルベドマップのサイズを取得。
	/// </summary>
	/// <returns></returns>
	unsigned int GetAlbedoMapSize() const
	{
		return m_albedoMapSize;
	}
	const char* GetAlbedoMapFilePath() const
	{
		return m_albedoMapFilePath;
	}
	/// <summary>
	/// リフレクションマップを取得。
	/// </summary>
	/// <returns></returns>
	const std::unique_ptr<char[]>& GetReflectionMap() const
	{
		return m_zeroValueMap;
	}
	/// <summary>
	/// リフレクションマップのサイズを取得。
	/// </summary>
	/// <returns></returns>
	unsigned int GetReflectionMapSize() const
	{
		return m_zeroValueMapSize;
	}
	const char* GetReflectionMapFilePath() const
	{
		return m_zeroValueMapFilePath;
	}
	/// <summary>
	/// 屈折マップを取得。
	/// </summary>
	/// <returns></returns>
	const std::unique_ptr<char[]>& GetRefractionMap() const
	{
		return m_zeroValueMap;
	}
	/// <summary>
	/// 屈折マップのサイズを取得。
	/// </summary>
	/// <returns></returns>
	unsigned int GetRefractionMapSize() const
	{
		return m_zeroValueMapSize;
	}
	const char* GetRefractionMapFilePath() const
	{
		return m_zeroValueMapFilePath;
	}
private:
	std::unique_ptr<char[]> m_albedoMap;	//アルベドマップ。
	unsigned int m_albedoMapSize = 0;		//アルベドマップのサイズ。
	std::unique_ptr<char[]>	m_normalMap;	//法線マップ。
	unsigned int m_normalMapSize = 0;		//法線マップのサイズ。
	std::unique_ptr<char[]>	m_specMap;		//法線マップ。
	unsigned int m_specMapSize = 0;			//法線マップのサイズ。
	std::unique_ptr<char[]> m_zeroValueMap;	//0の値を格納しているマップ。
	unsigned int m_zeroValueMapSize = 0;	//0の値を格納しているマップのサイズ。
	const char*		m_albedoMapFilePath;
	const char*		m_normalMapFilePath;
	const char*		m_specMapFilePath;
	const char*		m_zeroValueMapFilePath;
};