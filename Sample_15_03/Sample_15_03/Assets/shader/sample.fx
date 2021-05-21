/*!
 * @brief ガウシアンブラーのコンピュートシェーダー
 */

// step-8 各種定数バッファーにアクセスするための変数を定義

// step-9 入出力画像にアクセスするための変数を定義

/*!
 * @brief float4をRGBA32フォーマットに変換
 */
uint PackedFloat4ToRGBA32(float4 In)
{
    return (uint)(In.a * 255) << 24
         | (uint)(In.b * 255) << 16
         | (uint)(In.g * 255) << 8
         | (uint)(In.r * 255);
}

float4 UnpackedRGBA32ToFloat4(uint In)
{
    float4 fColor;
    fColor.a = (float)(In >> 24 & 0xFF);
    fColor.b = (float)((In >> 16) & 0xFF);
    fColor.g = (float)((In >> 8) & 0xFF);
    fColor.r = (float)(In & 0xFF);

    return fColor/255.0f;
}

// XY座標からピクセルインデックスを取得する
uint GetPixelIndexFromXYCoord(int x, int y, int width)
{
    return width * y + x;
}

float4 GetPixelColor(int x, int y, int2 texSize)
{
    x = clamp(0, texSize.x, x);
    y = clamp(0, texSize.y, y);

    uint pixelIndex = GetPixelIndexFromXYCoord(x, y, texSize.y);
    return UnpackedRGBA32ToFloat4(inputImage[ pixelIndex]);
}

// step-10 Xブラーを実装

// step-11 Yブラーを実装

// step-12 最終出力を実装
