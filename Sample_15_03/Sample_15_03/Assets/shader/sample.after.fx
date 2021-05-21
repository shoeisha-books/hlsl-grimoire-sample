/*!
 * @brief ガウシアンブラーのコンピュートシェーダー
 */

// step-10 各種定数バッファーにアクセスするための変数を定義
// 定数バッファー
cbuffer cb_0 : register(b0)
{
    float4 weights[2];
};

// テクスチャ情報用の定数バッファー
cbuffer texInfoCB : register(b1)
{
    int2 texSize;           // オリジナルテクスチャのサイズ
    int2 xBlurTexSize;      // Xブラーの出力先のテクスチャのサイズ
    int2 yBlurTexSize;      // Yブラーの出力先のテクスチャのサイズ
};

// step-11 入出力画像にアクセスするための変数を定義
StructuredBuffer<uint> inputImage : register(t0);
RWStructuredBuffer<uint> outputImage : register(u0);

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

    uint pixelIndex = GetPixelIndexFromXYCoord(x, y, texSize.x);
    return UnpackedRGBA32ToFloat4(inputImage[ pixelIndex]);
}

// step-12 Xブラーを実装
[numthreads(4, 4, 1)]
void XBlur(uint3 DTid : SV_DispatchThreadID)
{
    uint2 basepos = uint2(DTid.x * 2, DTid.y);

    float4 color = GetPixelColor(basepos.x, basepos.y, texSize) * weights[0].x;
    color += GetPixelColor(basepos.x + 1, basepos.y, texSize) * weights[0].y;
    color += GetPixelColor(basepos.x + 2, basepos.y, texSize) * weights[0].z;
    color += GetPixelColor(basepos.x + 3, basepos.y, texSize) * weights[0].w;
    color += GetPixelColor(basepos.x + 4, basepos.y, texSize) * weights[1].x;
    color += GetPixelColor(basepos.x + 5, basepos.y, texSize) * weights[1].y;
    color += GetPixelColor(basepos.x + 6, basepos.y, texSize) * weights[1].z;
    color += GetPixelColor(basepos.x + 7, basepos.y, texSize) * weights[1].w;

    color += GetPixelColor(basepos.x - 1, basepos.y, texSize) * weights[0].y;
    color += GetPixelColor(basepos.x - 2, basepos.y, texSize) * weights[0].z;
    color += GetPixelColor(basepos.x - 3, basepos.y, texSize) * weights[0].w;
    color += GetPixelColor(basepos.x - 4, basepos.y, texSize) * weights[1].x;
    color += GetPixelColor(basepos.x - 5, basepos.y, texSize) * weights[1].y;
    color += GetPixelColor(basepos.x - 6, basepos.y, texSize) * weights[1].z;
    color += GetPixelColor(basepos.x - 7, basepos.y, texSize) * weights[1].w;

    uint pixelIndex = GetPixelIndexFromXYCoord(DTid.x, DTid.y, xBlurTexSize.x);
    outputImage[pixelIndex] = PackedFloat4ToRGBA32(color);
}

// step-13 Yブラーを実装
[numthreads(4, 4, 1)]
void YBlur(uint3 DTid : SV_DispatchThreadID)
{
    uint2 basepos = uint2(DTid.x, DTid.y * 2);

    float4 color = GetPixelColor(basepos.x, basepos.y, xBlurTexSize) * weights[0].x;
    color += GetPixelColor(basepos.x, basepos.y + 1, xBlurTexSize) * weights[0].y;
    color += GetPixelColor(basepos.x, basepos.y + 2, xBlurTexSize) * weights[0].z;
    color += GetPixelColor(basepos.x, basepos.y + 3, xBlurTexSize) * weights[0].w;
    color += GetPixelColor(basepos.x, basepos.y + 4, xBlurTexSize) * weights[1].x;
    color += GetPixelColor(basepos.x, basepos.y + 5, xBlurTexSize) * weights[1].y;
    color += GetPixelColor(basepos.x, basepos.y + 6, xBlurTexSize) * weights[1].z;
    color += GetPixelColor(basepos.x, basepos.y + 7, xBlurTexSize) * weights[1].w;

    color += GetPixelColor(basepos.x, basepos.y - 1, xBlurTexSize) * weights[0].y;
    color += GetPixelColor(basepos.x, basepos.y - 2, xBlurTexSize) * weights[0].z;
    color += GetPixelColor(basepos.x, basepos.y - 3, xBlurTexSize) * weights[0].w;
    color += GetPixelColor(basepos.x, basepos.y - 4, xBlurTexSize) * weights[1].x;
    color += GetPixelColor(basepos.x, basepos.y - 5, xBlurTexSize) * weights[1].y;
    color += GetPixelColor(basepos.x, basepos.y - 6, xBlurTexSize) * weights[1].z;
    color += GetPixelColor(basepos.x, basepos.y - 7, xBlurTexSize) * weights[1].w;

    uint pixelIndex = GetPixelIndexFromXYCoord(DTid.x, DTid.y, yBlurTexSize.x);
    outputImage[pixelIndex] = PackedFloat4ToRGBA32(color);
}

// step-14 最終出力を実装
[numthreads(4, 4, 1)]
void Final(uint3 DTid : SV_DispatchThreadID)
{
    // バイリニアフィルタをかける
    uint2 basepos = uint2(DTid.x / 2, DTid. y /2);
    float4 color = GetPixelColor(basepos.x, basepos.y, yBlurTexSize);
    color += GetPixelColor(basepos.x, basepos.y + 1, yBlurTexSize);
    color += GetPixelColor(basepos.x + 1, basepos.y, yBlurTexSize);
    color += GetPixelColor(basepos.x + 1, basepos.y + 1, yBlurTexSize);

    // 加重平均を取る
    color /= 4.0f;
    uint pixelIndex = GetPixelIndexFromXYCoord(DTid.x, DTid.y, texSize.x);
    outputImage[pixelIndex] = PackedFloat4ToRGBA32(color);
}
