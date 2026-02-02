#include "stdafx.h"
#include "system/system.h"

const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの数

/// <summary>
/// ディレクションライト
/// </summary>
struct DirectionalLight
{
    Vector3 direction;  // ライトの方向
    float pad0;         // パディング
    Vector4 color;      // ライトのカラー
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT]; // ディレクションライト
    Vector3 eyePos;         // カメラの位置
    float specPow;          // スペキュラの絞り
    Vector3 ambinetLight;   // 環境光
};

const int NUM_WEIGHTS = 8;

/// <summary>
/// ブラー用のパラメータ
/// </summary>
struct SBlurParam
{
    float weights[NUM_WEIGHTS];
};

// 関数宣言
void InitRootSignature(RootSignature& rs);
void InitModel(Model& plModel);

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

    RootSignature rs;
    InitRootSignature(rs);

    // 32ビットの浮動小数点のカラーバッファーを保持したメインレンダリングターゲットを作成する
    RenderTarget mainRenderTarget;
    mainRenderTarget.Create(
        1280,
        720,
        1,
        1,
        // 【注目】カラーバッファーのフォーマットを32bit浮動小数点にしている
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );

    // 強い光のライトを用意する
    Light light;

    // 光を強めに設定する
    light.directionalLight[0].color.x = 10.8f;
    light.directionalLight[0].color.y = 10.8f;
    light.directionalLight[0].color.z = 10.8f;

    light.directionalLight[0].direction.x = 0.0f;
    light.directionalLight[0].direction.y = 0.0f;
    light.directionalLight[0].direction.z = -1.0f;
    light.directionalLight[0].direction.Normalize();

    light.ambinetLight.x = 0.5f;
    light.ambinetLight.y = 0.5f;
    light.ambinetLight.z = 0.5f;
    light.eyePos = g_camera3D->GetPosition();

    // モデルの初期化情報を設定する
    ModelInitData plModelInitData;

    // tkmファイルを指定する
    plModelInitData.m_tkmFilePath = "Assets/modelData/sample.tkm";

    // シェーダーファイルを指定する
    plModelInitData.m_fxFilePath = "Assets/shader/sample3D.fx";

    // ユーザー拡張の定数バッファーに送るデータを指定する
    plModelInitData.m_expandConstantBuffer = &light;

    // ユーザー拡張の定数バッファーに送るデータのサイズを指定する
    plModelInitData.m_expandConstantBufferSize = sizeof(light);

    // レンダリングするカラーバッファーのフォーマットを指定する
    plModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // 設定した初期化情報をもとにモデルを初期化する
    Model plModel;
    plModel.Init(plModelInitData);

    // 輝度抽出用のレンダリングターゲットを作成
    RenderTarget luminnceRenderTarget;

    // 解像度、ミップマップレベル
    luminnceRenderTarget.Create(
        1280,       // 解像度はメインレンダリングターゲットと同じ
        720,        // 解像度はメインレンダリングターゲットと同じ
        1,
        1,
        // 【注目】カラーバッファーのフォーマットを32bit浮動小数点にしている
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );

    // 輝度抽出用のスプライトを初期化
    // 初期化情報を作成する
    SpriteInitData luminanceSpriteInitData;

    // 輝度抽出用のシェーダーのファイルパスを指定する
    luminanceSpriteInitData.m_fxFilePath = "Assets/shader/samplePostEffect.fx";

    // 頂点シェーダーのエントリーポイントを指定する
    luminanceSpriteInitData.m_vsEntryPointFunc = "VSMain";

    // ピクセルシェーダーのエントリーポイントを指定する
    luminanceSpriteInitData.m_psEntryPoinFunc = "PSSamplingLuminance";

    // スプライトの幅と高さはluminnceRenderTargetと同じ
    luminanceSpriteInitData.m_width = 1280;
    luminanceSpriteInitData.m_height = 720;

    // テクスチャはメインレンダリングターゲットのカラーバッファー
    luminanceSpriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();

    // 描き込むレンダリングターゲットのフォーマットを指定する
    luminanceSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // 作成した初期化情報をもとにスプライトを初期化する
    Sprite luminanceSprite;
    luminanceSprite.Init( luminanceSpriteInitData );

    // step-1 ガウシアンブラーを初期化
    GaussianBlur gaussianBlur[4];

    //gaussianBlur[0]は輝度テクスチャにガウシアンブラーをかける
    gaussianBlur[0].Init(&luminnceRenderTarget.GetRenderTargetTexture());

	//gaussianBlur[1]はgaussianBlur[0]のテクスチャにガウシアンブラーをかける
	gaussianBlur[1].Init(&gaussianBlur[0].GetBokeTexture());
	
	//gaussianBlur[2]はgaussianBlur[1]のテクスチャにガウシアンブラーをかける
    gaussianBlur[2].Init(&gaussianBlur[1].GetBokeTexture());
    
	//gaussianBlur[3]はgaussianBlur[2]のテクスチャにガウシアンブラーをかける
    gaussianBlur[3].Init(&gaussianBlur[2].GetBokeTexture());

    // step-2 ボケ画像を合成して書き込むためのスプライトを初期化
    //初期化情報を指定する
	SpriteInitData finalSpriteInitData;

    //【注目】ボケテクスチャを4枚指定する
	finalSpriteInitData.m_textures[0] = &gaussianBlur[0].GetBokeTexture();
	finalSpriteInitData.m_textures[1] = &gaussianBlur[1].GetBokeTexture();
	finalSpriteInitData.m_textures[2] = &gaussianBlur[2].GetBokeTexture();
	finalSpriteInitData.m_textures[3] = &gaussianBlur[3].GetBokeTexture();

	//解像度はmainRenderTargetと同じ幅と高さ
	finalSpriteInitData.m_width = 1280;
	finalSpriteInitData.m_height = 720;

    //【注目】ボケ画像を合成する必要があるので、２D用シェーダーではなく
    //専用シェーダーを指定
	finalSpriteInitData.m_fxFilePath = "Assets/shader/samplePostEffect.fx";
    finalSpriteInitData.m_psEntryPoinFunc = "PSBloomFinal";

	//ただし、加算合成で描画するので、アルファブレンディングモードを加算する
    finalSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;

	//カラーバッファーのフォーマットは例によって、32ビット浮動小数点バッファー
	finalSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    //初期化情報をもとに加算合成用のスプライトを初期化する
	Sprite finalSprite;
	finalSprite.Init(finalSpriteInitData);

    // mainRenderTargetのテクスチャをフレームバッファーに貼り付けるためのスプライトを初期化する
    // スプライトの初期化オブジェクトを作成する
    SpriteInitData spriteInitData;

    // テクスチャはmainRenderTargetのカラーバッファー
    spriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
    spriteInitData.m_width = 1280;
    spriteInitData.m_height = 720;

    // モノクロ用のシェーダーを指定する
    spriteInitData.m_fxFilePath = "Assets/shader/sample2D.fx";

    // 初期化オブジェクトを使って、スプライトを初期化する
    Sprite copyToFrameBufferSprite;
    copyToFrameBufferSprite.Init(spriteInitData);

    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////

        // レンダリングターゲットをmainRenderTargetに変更する
        // レンダリングターゲットとして利用できるまで待つ
        renderContext.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
        // レンダリングターゲットを設定
        renderContext.SetRenderTargetAndViewport(mainRenderTarget);
        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetView(mainRenderTarget);

        // mainRenderTargetに各種モデルを描画する
        plModel.Draw(renderContext);

        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);

        // 輝度抽出
        // 輝度抽出用のレンダリングターゲットに変更
        renderContext.WaitUntilToPossibleSetRenderTarget(luminnceRenderTarget);

        // レンダリングターゲットを設定
        renderContext.SetRenderTargetAndViewport(luminnceRenderTarget);

        // レンダリングターゲットをクリア
        renderContext.ClearRenderTargetView(luminnceRenderTarget);

        // 輝度抽出を行う
        luminanceSprite.Draw(renderContext);

        // レンダリングターゲットへの書き込み終了待ち
        renderContext.WaitUntilFinishDrawingToRenderTarget(luminnceRenderTarget);

        // step-3 ガウシアンブラーを4回実行する
        gaussianBlur[0].ExecuteOnGPU(renderContext, 10);
        gaussianBlur[1].ExecuteOnGPU(renderContext, 10);
        gaussianBlur[2].ExecuteOnGPU(renderContext, 10);
        gaussianBlur[3].ExecuteOnGPU(renderContext, 10);

        // step-4 ボケ画像を合成してメインレンダリングターゲットに加算合成
        //レンダリングターゲットとして利用できるまで待つ
		renderContext.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);

        //レンダリングターゲットを設定
		renderContext.SetRenderTargetAndViewport(mainRenderTarget);

        //最終合成
		finalSprite.Draw(renderContext);

        //レンダリングターゲットへの書き込み終了待ち
		renderContext.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);

        // メインレンダリングターゲットの絵をフレームバッファーにコピー
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        copyToFrameBufferSprite.Draw(renderContext);

        // ライトの強さを変更する
        light.directionalLight[0].color.x += g_pad[0]->GetLStickXF() * 0.5f;
        light.directionalLight[0].color.y += g_pad[0]->GetLStickXF() * 0.5f;
        light.directionalLight[0].color.z += g_pad[0]->GetLStickXF() * 0.5f;
        //////////////////////////////////////
        //絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

// ルートシグネチャの初期化
void InitRootSignature( RootSignature& rs )
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}
