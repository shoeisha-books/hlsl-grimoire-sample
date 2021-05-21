#include "stdafx.h"
#include "Bitmap.h"
#include <fstream>

using namespace std;


/// <summary>
/// コンストラクタ
/// </summary>
Bitmap::Bitmap()
{
}

/// <summary>
/// デストラクタ。
/// </summary>
Bitmap::~Bitmap()
{
}

/// <summary>
/// Bitmapをロード。
/// Bitmapの解像度は512×512である必要があります。
/// </summary>
/// <param name="filePath">読み込むファイルパス。</param>
/// <returns>
/// 読み込みに成功したらtrue、
/// 失敗したらfalseを返します。
/// </returns>
bool Bitmap::Load(const char* filePath)
{
	//ファイルストリームを作成。
	ifstream fs(filePath, std::ios::binary);

	if (fs.fail() == true) {
		//ファイルの読み込みに失敗した。
		return false;
	}
	//ファイルサイズを計算する。
	fs.seekg(0, fstream::end);		//ファイルの終端に移動する。
	uint32_t endPos = fs.tellg();	//現在のファイルの位置を取得。
	fs.clear();
	fs.seekg(0, fstream::beg);		//ファイルの先頭に戻どる。
	uint32_t begPos = fs.tellg();	//現在のファイルの位置を取得。
	uint32_t fileSize = endPos - begPos;	//末尾-銭湯でファイルサイズを計算。

	//読み込むぜ。
	char* buf = new char[fileSize];
	fs.read(buf, fileSize);

	//ファイルヘッダーのアドレスを代入。
	BITMAPFILEHEADER* header = (BITMAPFILEHEADER*)buf;
	//ファイル情報ヘッダーのアドレスを代入。
	BITMAPINFOHEADER* infoHeader = (BITMAPINFOHEADER*)(buf + sizeof(BITMAPFILEHEADER));
	//ほんとは色々チェックがいるのだが、
	//フォーマットは24bitカラーで、解像度は512×512で決め打ちだっぜ！
	//適当にイメージにジャンプ。
	char* pImage = buf + header->bfOffBits;

	//ビットマップの情報をメンバ変数にコピー。
	memcpy(&m_bitmapHeader, header, sizeof(m_bitmapHeader));
	memcpy(&m_bitmapInfoHeader, infoHeader, sizeof(m_bitmapInfoHeader));
	//RGB24ビットしか考慮してないよ。
	m_numPixel = m_bitmapInfoHeader.biWidth * m_bitmapInfoHeader.biHeight;
	m_imageSizeInBytes = m_numPixel * sizeof(SRgbRow);
	m_imageRow = std::make_unique<SRgbRow[]>(m_numPixel);
	memcpy(m_imageRow.get(), pImage, m_imageSizeInBytes);

	m_image = std::make_unique<SRgba[]>(m_numPixel);
	for (int i = 0; i < m_numPixel; i++) {
		m_image[i].r = m_imageRow[i].r;
		m_image[i].g = m_imageRow[i].g;
		m_image[i].b = m_imageRow[i].b;
		m_image[i].a = 255;
	}
	


	//コピーが終わったのでもういらないので、もろもろ削除。
	//解放解放。
	delete[] buf;
	//ファイルを閉じる。
	fs.close();

	//読み込みが成功したのでtrueを返す。
	return true;
}
/// <summary>
/// Bitmapの保存。
/// </summary>
/// <param name="filePath">保存するファイルパス。</param>
/// <returns>
/// 保存に成功したらtrue、
/// 失敗したらfalseを返す。
/// </returns>
bool Bitmap::Save(const char* filePath)
{
	ofstream fs(filePath, std::ios::binary);

	if (fs.fail() == true) {
		//ファイルのオープンに失敗した。
		return false;
	}

	//書き込め―。
	fs.write((char*)&m_bitmapHeader, sizeof(m_bitmapHeader));
	fs.write((char*)&m_bitmapInfoHeader, sizeof(m_bitmapInfoHeader));
	std::unique_ptr< SRgbRow[]> imageRow = std::make_unique<SRgbRow[]>(m_numPixel);
	for (int i = 0; i < m_numPixel; i++) {
		imageRow[i].r = m_image[i].r;
		imageRow[i].g = m_image[i].g;
		imageRow[i].b = m_image[i].b;
	}

	fs.write((char*)imageRow.get(), m_imageSizeInBytes);

	fs.close();
	//保存が成功したので、trueを返す。
	return true;
}
