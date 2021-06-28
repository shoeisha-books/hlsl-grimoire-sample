#include "stdafx.h"
#include "ShadowMapRender.h"


namespace myRenderer {
    namespace shadow {
        void ShadowMapRender::Init()
        {
            float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

            //近景用のシャドウマップ
            m_shadowMaps[0].Create(
                2048,
                2048,
                1,
                1,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_D32_FLOAT,
                clearColor
            );
            //中景用のシャドウマップ
            m_shadowMaps[1].Create(
                1024,
                1024,
                1,
                1,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_D32_FLOAT,
                clearColor
            );
            //遠景用のシャドウマップ
            m_shadowMaps[2].Create(
                512,
                512,
                1,
                1,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_D32_FLOAT,
                clearColor
            );
        }

        void ShadowMapRender::Render(
            RenderContext& rc, 
            Vector3& lightDirection
        )
        {
            if (lightDirection.LengthSq() < 0.001f) {
                return;
            }
            m_cascadeShadowMapMatrix.CalcLightViewProjectionCropMatrix(lightDirection);
            
            int shadowMapNo = 0;
            for (auto& shadowMap : m_shadowMaps) {
                rc.WaitUntilToPossibleSetRenderTarget(shadowMap);
                rc.SetRenderTargetAndViewport(shadowMap);
                rc.ClearRenderTargetView(shadowMap);

                for (auto& model : m_modelsArray[shadowMapNo]) {
                    model->Draw(
                        rc, 
                        g_matIdentity,
                        m_cascadeShadowMapMatrix.GetLightViewProjectionCropMatrix(shadowMapNo)
                    );
                }
                //描画が終わったらクリア
                m_modelsArray[shadowMapNo].clear();
                // 書き込み完了待ち
                rc.WaitUntilFinishDrawingToRenderTarget(shadowMap);
                shadowMapNo++;
            }
        }
    }
}