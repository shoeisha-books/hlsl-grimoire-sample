#include "stdafx.h"
#include "Dof.h"

namespace myRenderer {
    void Dof::InitCombimeBokeImageToSprite(Sprite& combineBokeImageSprite, Texture& bokeTexture, Texture& depthTexture)
    {
        SpriteInitData combineBokeImageSpriteInitData;
        //使用するテクスチャは２枚
        combineBokeImageSpriteInitData.m_textures[0] = &bokeTexture;
        combineBokeImageSpriteInitData.m_textures[1] = &depthTexture;
        combineBokeImageSpriteInitData.m_width = 1280;
        combineBokeImageSpriteInitData.m_height = 720;
        combineBokeImageSpriteInitData.m_fxFilePath = "Assets/shader/preset/dof.fx";
        combineBokeImageSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        // 距離を利用してボケ画像をアルファブレンディングするので、半透明合成モードにする
        combineBokeImageSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Trans;
        // 初期化オブジェクトを利用してスプライトを初期化する

        combineBokeImageSprite.Init(combineBokeImageSpriteInitData);
    }
    void Dof::Init(RenderTarget& mainRenderTarget, RenderTarget& zprepassRenderTarget)
    {
        m_rtVerticalBlur.Create(
            mainRenderTarget.GetWidth(), 
            mainRenderTarget.GetHeight(), 
            1, 
            1, 
            mainRenderTarget.GetColorBufferFormat(), 
            DXGI_FORMAT_UNKNOWN
        );
        m_rtDiagonalBlur.Create(
            mainRenderTarget.GetWidth(),
            mainRenderTarget.GetHeight(),
            1,
            1,
            mainRenderTarget.GetColorBufferFormat(),
            DXGI_FORMAT_UNKNOWN
        );
        m_rtPhomboidBlur.Create(
            mainRenderTarget.GetWidth(),
            mainRenderTarget.GetHeight(),
            1,
            1,
            mainRenderTarget.GetColorBufferFormat(),
            DXGI_FORMAT_UNKNOWN
        );

        // step-2 垂直、対角線ブラーをかけるためのスプライトを初期化
        SpriteInitData vertDiagonalBlurSpriteInitData;
        vertDiagonalBlurSpriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
        vertDiagonalBlurSpriteInitData.m_width = mainRenderTarget.GetWidth();
        vertDiagonalBlurSpriteInitData.m_height = mainRenderTarget.GetHeight();
        vertDiagonalBlurSpriteInitData.m_fxFilePath = "Assets/shader/preset/hexaBlur.fx";

        // 垂直、対角線ブラー用のピクセルシェーダーを指定する
        vertDiagonalBlurSpriteInitData.m_psEntryPoinFunc = "PSVerticalDiagonalBlur";
        vertDiagonalBlurSpriteInitData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
        vertDiagonalBlurSpriteInitData.m_colorBufferFormat[1] = mainRenderTarget.GetColorBufferFormat();
        
        m_vertDIagonalBlurSprite.Init(vertDiagonalBlurSpriteInitData);

        // step-3 六角形ブラーをかけるためのスプライトを初期化
        SpriteInitData phomboidBlurSpriteInitData;
        phomboidBlurSpriteInitData.m_textures[0] = &m_rtVerticalBlur.GetRenderTargetTexture();
        phomboidBlurSpriteInitData.m_textures[1] = &m_rtDiagonalBlur.GetRenderTargetTexture();
        phomboidBlurSpriteInitData.m_width = mainRenderTarget.GetWidth();
        phomboidBlurSpriteInitData.m_height = mainRenderTarget.GetHeight();
        phomboidBlurSpriteInitData.m_fxFilePath = "Assets/shader/preset/hexaBlur.fx";

        // 六角形ブラー用のピクセルシェーダーを指定する
        phomboidBlurSpriteInitData.m_psEntryPoinFunc = "PSRhomboidBlur";
        phomboidBlurSpriteInitData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();

        m_phomboidBlurSprite.Init(phomboidBlurSpriteInitData);

        InitCombimeBokeImageToSprite(
            m_combineBokeImageSprite,                     // 初期化されるスプライト
            m_rtPhomboidBlur.GetRenderTargetTexture(),    // ボケテクスチャ
            zprepassRenderTarget.GetRenderTargetTexture()  // 深度テクスチャ
        );

    }
    void Dof::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
    {
        //step-4 垂直、対角線ブラーをかける
        RenderTarget* blurRts[] = {
            &m_rtVerticalBlur,
            &m_rtDiagonalBlur
        };

        //レンダリングターゲットとして利用できるまで待つ
        rc.WaitUntilToPossibleSetRenderTargets(2, blurRts);
        //レンダリングターゲットを設定
        rc.SetRenderTargetsAndViewport(2, blurRts);
        // レンダリングターゲットをクリア
        rc.ClearRenderTargetViews(2, blurRts);
        //
        m_vertDIagonalBlurSprite.Draw(rc);
        // レンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTargets(2, blurRts);

        //step-5 六角形ブラーをかける
        rc.WaitUntilToPossibleSetRenderTarget(m_rtPhomboidBlur);
        rc.SetRenderTargetAndViewport(m_rtPhomboidBlur);

        m_phomboidBlurSprite.Draw(rc);

        // レンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(m_rtPhomboidBlur);

        // ボケ画像と深度テクスチャを利用して、ボケ画像を描きこんでいく
        // メインレンダリングターゲットを設定
        rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
        rc.SetRenderTargetAndViewport(mainRenderTarget);

        // スプライトを描画&
        m_combineBokeImageSprite.Draw(rc);

        // レンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
    }
}