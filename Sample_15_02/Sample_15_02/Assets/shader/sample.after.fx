/*!
 * @brief モノクロ化コンピュートシェーダー
 */

// step-7 t0、u0に設定されているバッファーにアクセスするための変数を定義
StructuredBuffer<uint> inputImage : register(t0);
RWStructuredBuffer<uint> outputBuffer : register(u0);

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
/*!
 * @brief メイン関数
 */
[numthreads(4, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // step-8 入力データから画素を引っ張ってきてモノクロ化する
    int imageIndex = DTid.x;

    //画像データから
    uint iColor = inputImage[imageIndex];

    // 0～255の画像データを0.0～1.0に正規化する
    float4 color = UnpackedRGBA32ToFloat4(iColor);

    float Y = 0.29900 * color.r + 0.58700 * color.g + 0.11400 * color.b;
    color.r = Y;
    color.g = Y;
    color.b = Y;
    color.a = 1.0f;

    //float4をRGBA32ビットのフォーマットに変して出力
    outputBuffer[imageIndex] = PackedFloat4ToRGBA32(color);
}
