#include "stdafx.h"
#include "NullTextureMaps.h"

void NullTextureMaps::Init()
{
	//各種テクスチャをロード。
	auto TexLoad = [&](
		const char* loadTexFilePath,
		std::unique_ptr<char[]>& outTexData,
		unsigned int& outTexSize
		) {
		FILE* fp = fopen(loadTexFilePath, "rb");
		if (fp == nullptr) {
			//nullテクスチャのロードに失敗。
			MessageBoxA(nullptr, "nullテクスチャのロードに失敗しました。", "エラー", MB_OK);
			std::abort();
		}
		//テクスチャサイズを計算。
		fseek(fp, 0L, SEEK_END);
		outTexSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		//メモリを確保
		outTexData = std::make_unique<char[]>(outTexSize);
		fread(outTexData.get(), outTexSize, 1, fp);
		fclose(fp);
	};

	//アルベドマップをロード。
	TexLoad(
		"Assets/modelData/preset/NullAlbedoMap.DDS",
		m_albedoMap,
		m_albedoMapSize);

	//法線マップをロード。
	TexLoad(
		"Assets/modelData/preset/NullNormalMap.DDS",
		m_normalMap,
		m_normalMapSize);

	//スペキュラマップをロード
	TexLoad(
		"Assets/modelData/preset/specMap_None.DDS",
		m_specularMap,
		m_specularMapSize
	);

	//０の値を格納しているマップをロード。
	TexLoad(
		"Assets/modelData/preset/ZeroValueMap.DDS",
		m_zeroValueMap,
		m_zeroValueMapSize);

}