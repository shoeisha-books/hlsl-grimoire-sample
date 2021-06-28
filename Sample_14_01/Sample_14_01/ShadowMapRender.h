#pragma once

#include "CascadeShadowMapMatrix.h"

namespace myRenderer
{
    namespace shadow
    {
        /// <summary>
        /// シャドウマップへの描画処理
        /// </summary>
        class ShadowMapRender
        {
        public:
            /// <summary>
            /// 初期化
            /// </summary>
            void Init();
            /// <summary>
        /// シャドウマップに描画するモデルを追加
        /// </summary>
        /// <param name="model0">近景用のシャドウマップに描画するモデル</param>
        /// <param name="model1">中景用のシャドウマップ1に描画するモデル</param>
        /// <param name="model2">遠景用のシャドウマップ2に描画するモデル</param>
            void Add3DModel(
                Model& model0,
                Model& model1,
                Model& model2)
            {
                m_modelsArray[0].push_back(&model0);
                m_modelsArray[1].push_back(&model1);
                m_modelsArray[2].push_back(&model2);
            }
            /// <summary>
            /// 描画
            /// </summary>
            /// <param name="rc">レンダリングコンテキスト</param>
            /// <param name="lightDirection">ライトの方向</param>
            void Render(RenderContext& rc, Vector3& lightDirection);
            /// <summary>
            /// シャドウマップを取得
            /// </summary>
            /// <param name="areaNo">エリア番号</param>
            /// <returns></returns>
            Texture& GetShadowMap(int areaNo)
            {
                return m_shadowMaps[areaNo].GetRenderTargetTexture();
            }
            /// <summary>
            /// ライトビュープロジェクション行列を取得
            /// </summary>
            /// <returns></returns>
            const Matrix& GetLVPMatrix(int areaNo) const
            {
                return m_cascadeShadowMapMatrix.GetLightViewProjectionCropMatrix(areaNo);
            }
        private:
            CascadeShadowMapMatrix m_cascadeShadowMapMatrix;	//カスケードシャドウマップの行列を扱うオブジェクト
            RenderTarget m_shadowMaps[NUM_SHADOW_MAP];			//シャドウマップ
            std::vector<Model*> m_modelsArray[NUM_SHADOW_MAP];	//シャドウマップに描画するモデルの配列

        };
    }
}

