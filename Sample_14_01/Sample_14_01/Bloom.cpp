#include "stdafx.h"
#include "Bloom.h"

namespace myRenderer
{
    void Bloom::Init(RenderTarget& mainRenderTarget)
    {
        m_luminanceRenderTarget.Create(
            mainRenderTarget.GetWidth(),   // 解像度はメインレンダリングターゲットと同じ
            mainRenderTarget.GetHeight(),  // 解像度はメインレンダリングターゲットと同じ
            1,
            1,
            mainRenderTarget.GetColorBufferFormat(),
            DXGI_FORMAT_D32_FLOAT
        );
        {
            // 輝度抽出用のスプライトを初期化
            // 初期化情報を作成する
            SpriteInitData spriteInitData;
            // 輝度抽出用のシェーダーのファイルパスを指定する
            spriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
            // 頂点シェーダーのエントリーポイントを指定する
            spriteInitData.m_vsEntryPointFunc = "VSMain";
            // ピクセルシェーダーのエントリーポイントを指定する
            spriteInitData.m_psEntryPoinFunc = "PSSamplingLuminance";
            // スプライトの幅と高さはluminnceRenderTargetと同じ
            spriteInitData.m_width = mainRenderTarget.GetWidth();
            spriteInitData.m_height = mainRenderTarget.GetHeight();
            // テクスチャはメインレンダリングターゲットのカラーバッファー
            spriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
            // 描き込むレンダリングターゲットのフォーマットを指定する
            spriteInitData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();

            m_luminanceSprite.Init(spriteInitData);
        }
        //ガウシアンブラーを初期化
        // gaussianBlur[0]は輝度テクスチャにガウシアンブラーをかける
        m_gaussianBlur[0].Init(&m_luminanceRenderTarget.GetRenderTargetTexture());
        // gaussianBlur[1]はgaussianBlur[0]のテクスチャにガウシアンブラーをかける
        m_gaussianBlur[1].Init(&m_gaussianBlur[0].GetBokeTexture());
        // gaussianBlur[2]はgaussianBlur[1]のテクスチャにガウシアンブラーをかける
        m_gaussianBlur[2].Init(&m_gaussianBlur[1].GetBokeTexture());
        // gaussianBlur[3]はgaussianBlur[2]のテクスチャにガウシアンブラーをかける
        m_gaussianBlur[3].Init(&m_gaussianBlur[2].GetBokeTexture());
        // 最終合成用のスプライトを初期化する
        {
            SpriteInitData spriteInitData;
            // ボケテクスチャを4枚指定する
            spriteInitData.m_textures[0] = &m_gaussianBlur[0].GetBokeTexture();
            spriteInitData.m_textures[1] = &m_gaussianBlur[1].GetBokeTexture();
            spriteInitData.m_textures[2] = &m_gaussianBlur[2].GetBokeTexture();
            spriteInitData.m_textures[3] = &m_gaussianBlur[3].GetBokeTexture();
            // 解像度はmainRenderTargetの幅と高さ
            spriteInitData.m_width = mainRenderTarget.GetWidth();
            spriteInitData.m_height = mainRenderTarget.GetHeight();
            // ぼかした画像を、通常の2Dとしてメインレンダリングターゲットに描画するので、
            // 2D用のシェーダーを使用する
            spriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
            spriteInitData.m_psEntryPoinFunc = "PSBloomFinal";

            // ただし、加算合成で描画するので、アルファブレンディングモードを加算にする
            spriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;
            spriteInitData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();

            m_finalSprite.Init(spriteInitData);
        }
    }
    void Bloom::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
    {
        // 輝度抽出
        // 輝度抽出用のレンダリングターゲットに変更
        rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRenderTarget);
        // レンダリングターゲットを設定
        rc.SetRenderTargetAndViewport(m_luminanceRenderTarget);
        // レンダリングターゲットをクリア
        rc.ClearRenderTargetView(m_luminanceRenderTarget);
        // 輝度抽出を行う
        m_luminanceSprite.Draw(rc);
        // レンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(m_luminanceRenderTarget);

        // ガウシアンブラーを4回実行する
        m_gaussianBlur[0].ExecuteOnGPU(rc, 10);
        m_gaussianBlur[1].ExecuteOnGPU(rc, 10);
        m_gaussianBlur[2].ExecuteOnGPU(rc, 10);
        m_gaussianBlur[3].ExecuteOnGPU(rc, 10);

        // 4枚のボケ画像を合成してメインレンダリングターゲットに加算合成
        // レンダリングターゲットとして利用できるまで待つ
        rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
        // レンダリングターゲットを設定
        rc.SetRenderTargetAndViewport(mainRenderTarget);
        // 最終合成
        m_finalSprite.Draw(rc);
        // レンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
    }
}