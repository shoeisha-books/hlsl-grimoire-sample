#include "stdafx.h"
#include "system/system.h"
#include "TrianglePolygon.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);

/// <summary>
/// ディレクショナルライト
/// </summary>
struct DirectionalLight
{
    Vector3  color;
    float pad0;         // パディング
    Vector3  direction;
    float pad1;         // パディング
};

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

    // ディレクションライト
    DirectionalLight light;
    light.direction.x = 1.0f;
    light.direction.y = 0.0f;
    light.direction.z = 0.0f;

    light.color.x = 1.0f;
    light.color.y = 1.0f;
    light.color.z = 1.0f;

    // step-1 モデルを初期化

    // step-2 G-Bufferを作成

    // step-3 ディファードライティングを行うためのスプライトを初期化

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム開始
        g_engine->BeginFrame();

        // ライトを回す
        Quaternion rotLig;
        rotLig.SetRotationDegY(2.0f);
        rotLig.Apply(light.direction);
        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // step-4 レンダリングターゲットをG-Bufferに変更して書き込む

        // step-5 レンダリング先をフレームバッファーに戻してスプライトをレンダリングする

        /////////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

// ルートシグネチャの初期化
void InitRootSignature(RootSignature& rs)
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}
