#pragma once

namespace myRenderer {
    class Dof
    {
    public:
        /// <summary>
        /// 初期化
        /// </summary>
        /// <param name="mainRenderTarget">メインレンダリングターゲット</param>
        /// <param name="zprepassRenderTarget">ZPrepass</param>
        void Init(RenderTarget& mainRenderTarget, RenderTarget& zprepassRenderTarget);
        /// <summary>
        /// 描画
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        /// <param name="mainRenderTarget">メインレンダリングターゲット</param>
        void Render(RenderContext& rc, RenderTarget& mainRenderTarget);
    private:
        void InitCombimeBokeImageToSprite(Sprite& combineBokeImageSprite, Texture& bokeTexture, Texture& depthTexture);
    private:
        RenderTarget m_rtVerticalBlur;		//垂直ブラーをかけるためのレンダリングターゲット
        RenderTarget m_rtDiagonalBlur;		//対角線ブラーをかけるためのレンダリングターゲット
        RenderTarget m_rtPhomboidBlur;		//六角形ブラーをかけるためのレンダリングターゲット
        Sprite m_vertDIagonalBlurSprite;	//垂直、対角線ブラー用のスプライト
        Sprite m_phomboidBlurSprite;		//六角形ブラ用のスプライト
        Sprite m_combineBokeImageSprite;	//ボケ画像をメインレンダリングターゲットに合成するためのスプライト

    };
}

