#include "stdafx.h"
#include "ModelRender.h"
#include "RenderingEngine.h"

namespace myRenderer
{
    void ModelRender::InitDeferredRendering(RenderingEngine& renderingEngine, const char* tkmFilePath, bool isShadowReciever)
    {
        ModelInitData modelInitData;
        modelInitData.m_fxFilePath = "Assets/shader/preset/RenderToGBufferFor3DModel.fx";
        if (isShadowReciever)
        {
            modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
        }
        modelInitData.m_tkmFilePath = tkmFilePath;
        modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
        modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        modelInitData.m_colorBufferFormat[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
        modelInitData.m_colorBufferFormat[4] = DXGI_FORMAT_R8G8B8A8_UNORM;

        m_renderToGBufferModel.Init(modelInitData);

        InitCommon(renderingEngine, tkmFilePath);
    }

    void ModelRender::InitForwardRendering(RenderingEngine& renderingEngine, ModelInitDataFR& modelInitData)
    {
        m_forwardRenderModel.Init(modelInitData);
        InitCommon(renderingEngine, modelInitData.m_tkmFilePath);
    }

    void ModelRender::InitCommon(RenderingEngine& renderingEngine, const char* tkmFilePath)
    {
        m_renderingEngine = &renderingEngine;

        // ZPrepass描画用のモデルを初期化
        {
            ModelInitData modelInitData;
            modelInitData.m_tkmFilePath = tkmFilePath;
            modelInitData.m_fxFilePath = "Assets/shader/preset/ZPrepass.fx";
            modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32_FLOAT;

            m_zprepassModel.Init(modelInitData);
        }

        // シャドウマップ描画用のモデルを初期化
        {
            ModelInitData modelInitData;
            modelInitData.m_tkmFilePath = tkmFilePath;
            modelInitData.m_fxFilePath = "Assets/shader/preset/DrawShadowMap.fx";
            modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
            for (int ligNo = 0;
                 ligNo < NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT;
                 ligNo++)
            {
                m_shadowModels[ligNo][0].Init(modelInitData);
                m_shadowModels[ligNo][1].Init(modelInitData);
                m_shadowModels[ligNo][2].Init(modelInitData);
            }
        }
    }

    void ModelRender::Draw()
    {
        if (m_isShadowCaster)
        {
            // シャドウキャスター
            for (int ligNo = 0;
                 ligNo < NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT;
                 ligNo++)
            {
                m_renderingEngine->Add3DModelToRenderToShadowMap(
                    ligNo,
                    m_shadowModels[ligNo][0],
                    m_shadowModels[ligNo][1],
                    m_shadowModels[ligNo][2]
                );
            }
        }
        m_renderingEngine->Add3DModelToZPrepass(m_zprepassModel);
        if (m_renderToGBufferModel.IsInited())
        {
            m_renderingEngine->Add3DModelToRenderGBufferPass(m_renderToGBufferModel);
        }
        if (m_forwardRenderModel.IsInited())
        {
            m_renderingEngine->Add3DModelToForwardRenderPass(m_forwardRenderModel);
        }
    }
}
