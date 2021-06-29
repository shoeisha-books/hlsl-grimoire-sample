///////////////////////////////////////
// ZPrepass
///////////////////////////////////////


///////////////////////////////////////
// 構造体。
///////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;          //頂点座標。
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       //座標。
    float3 depth : TEXCOORD0;       //深度値。xにはプロジェクション空間、yにはカメラ空間での正規化されたZ値、zにはカメラ空間でのZ値
};



///////////////////////////////////////
// 定数バッファ。
///////////////////////////////////////
// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};




///////////////////////////////////////
// 関数
///////////////////////////////////////

// モデル用の頂点シェーダーのエントリーポイント
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos); // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos); // ワールド座標系からカメラ座標系に変換
    psIn.depth.z = psIn.pos.z;
    psIn.pos = mul(mProj, psIn.pos); // カメラ座標系からスクリーン座標系に変換
    psIn.depth.x = psIn.pos.z / psIn.pos.w;
    psIn.depth.y = saturate( psIn.pos.w / 1000.0f );
    
    return psIn;
}

// モデル用のピクセルシェーダーのエントリーポイント
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return float4( psIn.depth.x, psIn.depth.y, psIn.depth.z, 1.0f );
}

