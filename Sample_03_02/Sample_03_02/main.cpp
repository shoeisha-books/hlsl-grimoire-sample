#include "stdafx.h"
#include "system/system.h"
#include "TrianglePolygon.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////

    // ルートシグネチャを作成
    RootSignature rootSignature;
    InitRootSignature(rootSignature);

    // 定数バッファを作成
    ConstantBuffer cb;

    cb.Init(sizeof(Matrix));
    // 三角形ポリゴンを定義
    TrianglePolygon triangle;
    triangle.Init(rootSignature);

    // step-1 三角形ポリゴンにUV座標を設定
	triangle.SetUVCoord(0, 0.0f, 1.0f); //頂点の番号,U座標,V座標
	triangle.SetUVCoord(1, -0.5f, 0.0f); //頂点1の番号,U座標,V座標//U座標をマイナスにして左右反転
	triangle.SetUVCoord(2, -1.0f, 1.0f); //頂点2の番号,U座標,V座標

    // step-2 テクスチャをロード
    Texture tex;
    tex.InitFromDDSFile(L"Assets/image/sample_00.dds");

    // ディスクリプタヒープを作成
    DescriptorHeap ds;
    ds.RegistConstantBuffer(0, cb); // ディスクリプタヒープに定数バッファを登録

    // step-3 テクスチャをディスクリプタヒープに登録
	ds.RegistShaderResource(0, tex);//レジスタ番号,レジスタに設定するテクスチャ

    ds.Commit();                    //ディスクリプタヒープへの登録を確定

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // ルートシグネチャを設定
        renderContext.SetRootSignature(rootSignature);

        // ワールド行列を作成
        Matrix mWorld;

        // ワールド行列をグラフィックメモリにコピー
        cb.CopyToVRAM(mWorld);

        //ディスクリプタヒープを設定
        renderContext.SetDescriptorHeap(ds);

        //三角形をドロー
        triangle.Draw(renderContext);

        renderContext.DrawIndexed(6);

        /// //////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        //フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

//ルートシグネチャの初期化
void InitRootSignature(RootSignature& rs)
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}
