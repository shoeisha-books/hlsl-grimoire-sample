// 頂点シェーダーへの入力頂点構造体
struct VSInput
{
    float4 pos : POSITION;
};

// 頂点シェーダーの出力
struct VSOutput
{
    float4 pos : SV_POSITION;
};

// 頂点シェーダー
// 1. 引数は変換前の頂点情報
// 2. 戻り値は変換後の頂点情報
VSOutput VSMain(VSInput In)
{
    VSOutput vsOut = (VSOutput)0;

    // step-1 入力された頂点座標を出力データに代入する
    vsOut.pos = In.pos;

    // step-2 入力された頂点座標を2倍に拡大する

    //vsOut.pos.x *= 2.0f;
    //vsOut.pos.y *= 2.0f;
    //vsOut.pos.xy *= 2.0f; // x,y両方を2倍にする場合はこちらの方が簡潔
    // step-3 入力されたX座標を1.5倍、Y座標を0.5倍にして出力
    vsOut.pos.x *= 1.5f;
    vsOut.pos.y *= 0.5f;

    return vsOut;
}

// ピクセルシェーダー
float4 PSMain(VSOutput vsOut) : SV_Target0
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}