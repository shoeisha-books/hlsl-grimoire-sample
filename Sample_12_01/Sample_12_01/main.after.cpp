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

    //ディレクションライト
    DirectionalLight light;
    light.direction.x = 1.0f;
    light.direction.y = 0.0f;
    light.direction.z = 0.0f;

    light.color.x = 1.0f;
    light.color.y = 1.0f;
    light.color.z = 1.0f;

    // step-1 モデルを初期化
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/model.fx";
    Model model;
    model.Init(modelInitData);

    // step-2 G-Bufferを作成
    RenderTarget albedRT;   // アルベドカラー書き込み用のレンダリングターゲット
    albedRT.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
    RenderTarget normalRT;  // 法線書き込み用のレンダリングターゲット
    normalRT.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);

    // step-3 ディファードライティングを行うためのスプライトを初期化
    SpriteInitData spriteInitData;

    // 画面全体にレンダリングするので幅と高さはフレームバッファの幅と高さと同じ
    spriteInitData.m_width = FRAME_BUFFER_W;
    spriteInitData.m_height = FRAME_BUFFER_H;

    // 使用するテクスチャはアルベドテクスチャと法線テクスチャ
    spriteInitData.m_textures[0] = &albedRT.GetRenderTargetTexture();
    spriteInitData.m_textures[1] = &normalRT.GetRenderTargetTexture();
    spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
    spriteInitData.m_expandConstantBuffer = &light;
    spriteInitData.m_expandConstantBufferSize = sizeof(light);

    // 初期化データを使ってスプライトを作成
    Sprite defferdLightinSpr;
    defferdLightinSpr.Init(spriteInitData);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    //  ここからゲームループ
    while (DispatchWindowMessage())
    {
        // フレーム終了
        g_engine->BeginFrame();

        // ライトを回す
        Quaternion rotLig;
        rotLig.SetRotationDegY(2.0f);
        rotLig.Apply(light.direction);
        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // step-4 レンダリングターゲットをG-Bufferに変更して書き込む
        RenderTarget* rts[] = {
            &albedRT,
            &normalRT
        };

        // まず、レンダリングターゲットとして設定できるようになるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTargets(2, rts);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargets(2, rts);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(2, rts);
        model.Draw(renderContext);

        // レンダリングターゲットへの書き込み待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);

        // step-5 レンダリング先をフレームバッファーに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(renderContext);

        // G-Bufferの内容を元にしてスプライトをレンダリング
        defferdLightinSpr.Draw(renderContext);

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
