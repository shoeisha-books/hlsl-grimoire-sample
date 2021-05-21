#include "stdafx.h"
#include "system/system.h"

// 関数宣言
void InitRootSignature(RootSignature& rs);

/// <summary>
/// ディレクショナルライト
/// </summary>
struct DirectionalLight
{
    Vector3  color;
    float pad0;     // パディング
    Vector3  direction;
    float pad1;     // パディング
    Vector3 eyePos; // 視点
    float specPow;  // スペキュラの絞り
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
    light.direction.y = -1.0f;
    light.direction.z = -1.0f;
    light.direction.Normalize();

    light.color.x = 1.0f;
    light.color.y = 1.0f;
    light.color.z = 1.0f;
    light.eyePos = g_camera3D->GetPosition();

    // モデルを初期化
    ModelInitData modelInitData;
    // 人型モデルを初期化
    modelInitData.m_tkmFilePath = "Assets/modelData/sample.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/model.fx";
    Model humanModel;
    humanModel.Init(modelInitData);
    humanModel.UpdateWorldMatrix({ 0.0f, 0.0f, 0.0f }, g_quatIdentity, g_vec3One);

    // 背景モデルを初期化
    modelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
    Model bgModel;
    bgModel.Init(modelInitData);

    // step-1 半透明の球体モデルを初期化
    ModelInitData transModelInitData;
    transModelInitData.m_tkmFilePath = "Assets/modelData/sphere.tkm";
    transModelInitData.m_fxFilePath = "Assets/shader/model.fx";
    // 半透明モデルはモデルを描くときにライティングを行うので、ライトの情報を渡す
    transModelInitData.m_expandConstantBuffer = &light;
    transModelInitData.m_expandConstantBufferSize = sizeof(light);
    // ピクセルシェーダのエントリーポイントが不透明モデルとは異なる
    // 不透明モデルはPSMain、半透明モデルはPSMainTransを使用する
    // ピクセルシェーダの実装は後で確認
    transModelInitData.m_psEntryPointFunc = "PSMainTrans";
    // 半透明の球体モデルを初期化
    Model sphereModel;
    sphereModel.Init(transModelInitData);

    Vector3 planePos = { 0.0f, 160.0f, 20.0f };

    // G-Bufferを作成
    RenderTarget albedRT; // アルベドカラー書き込み用のレンダリングターゲット
    albedRT.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
    RenderTarget normalRT; // 法線書き込み用のレンダリングターゲット
    normalRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_UNKNOWN
    );
    RenderTarget worldPosRT;
    worldPosRT.Create(
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT, // ワールド座標を記録するので、32ビット浮動小数点バッファーを利用する
        DXGI_FORMAT_UNKNOWN
    );

    // ポストエフェクト的にディファードライティングを行うためのスプライトを初期化
    SpriteInitData spriteInitData;
    // 画面全体にレンダリングするので幅と高さはフレームバッファーの幅と高さと同じ
    spriteInitData.m_width = FRAME_BUFFER_W;
    spriteInitData.m_height = FRAME_BUFFER_H;
    // ディファードライティングで使用するテクスチャを設定
    spriteInitData.m_textures[0] = &albedRT.GetRenderTargetTexture();
    spriteInitData.m_textures[1] = &normalRT.GetRenderTargetTexture();

    // ディファードライティングで使用するテクスチャにワールド座標テクスチャを追加
    spriteInitData.m_textures[2] = &worldPosRT.GetRenderTargetTexture();

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

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // レンダリング開始
        g_engine->BeginFrame();
        if (g_pad[0]->IsPress(enButtonRight))
        {
            planePos.x -= 1.0f;
        }
        if (g_pad[0]->IsPress(enButtonLeft))
        {
            planePos.x += 1.0f;
        }
        if (g_pad[0]->IsPress(enButtonUp) )
        {
            planePos.z -= 1.0f;
        }
        if (g_pad[0]->IsPress(enButtonDown))
        {
            planePos.z += 1.0f;
        }
        sphereModel.UpdateWorldMatrix(planePos, g_quatIdentity, g_vec3One);

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // レンダリングターゲットをG-Bufferに変更して書き込む
        RenderTarget* rts[] = {
            &albedRT, // 0番目のレンダリングターゲット
            &normalRT, // 1番目のレンダリングターゲット
            &worldPosRT // 2番目のレンダリングターゲット
        };

        // まず、レンダリングターゲットとして設定できるようになるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);
        // レンダリングターゲットを設定
        renderContext.SetRenderTargets(ARRAYSIZE(rts), rts);
        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetViews(ARRAYSIZE(rts), rts);
        humanModel.Draw(renderContext);
        bgModel.Draw(renderContext);

        // レンダリングターゲットへの書き込み待ち
        renderContext.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリング先をフレームバッファーに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(renderContext);
        // G-Bufferの内容を元にしてディファードライティング
        defferdLightinSpr.Draw(renderContext);

        // step-2 深度ステンシルビューをG-Bufferを作成したときのものに変更する
        renderContext.SetRenderTarget(g_graphicsEngine->GetCurrentFrameBuffuerRTV(), rts[0]->GetDSVCpuDescriptorHandle());
        // 半透明オブジェクトを描画！
        sphereModel.Draw(renderContext);

        /////////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////
        // レンダリング終了
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
