#include "stdafx.h"
#include "RenderingEngine.h"

namespace myRenderer
{
    void RenderingEngine::Init()
    {
        InitZPrepassRenderTarget();
        InitMainRenderTarget();
        InitGBuffer();
        InitMainRTSnapshotRenderTarget();
        InitCopyMainRenderTargetToFrameBufferSprite();
        InitShadowMapRender();
        InitDeferredLighting();
        m_postEffect.Init(m_mainRenderTarget, m_zprepassRenderTarget);
    }

    void RenderingEngine::InitShadowMapRender()
    {
        // シャドウマップの描画処理の初期化
        for (auto& shadowMapRender : m_shadowMapRenders)
        {
            shadowMapRender.Init();
        }
    }

    void RenderingEngine::InitZPrepassRenderTarget()
    {
        m_zprepassRenderTarget.Create(
            g_graphicsEngine->GetFrameBufferWidth(),
            g_graphicsEngine->GetFrameBufferHeight(),
            1,
            1,
            DXGI_FORMAT_R32G32_FLOAT,
            DXGI_FORMAT_D32_FLOAT
        );

    }
    void RenderingEngine::InitMainRTSnapshotRenderTarget()
    {
        for (auto& snapshotRt : m_mainRTSnapshots)
        {
            snapshotRt.Create(
                g_graphicsEngine->GetFrameBufferWidth(),
                g_graphicsEngine->GetFrameBufferHeight(),
                1,
                1,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_UNKNOWN
            );
        }
    }

    void RenderingEngine::InitMainRenderTarget()
    {
        m_mainRenderTarget.Create(
            g_graphicsEngine->GetFrameBufferWidth(),
            g_graphicsEngine->GetFrameBufferHeight(),
            1,
            1,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_UNKNOWN
        );
    }

    void RenderingEngine::InitGBuffer()
    {
        int frameBuffer_w = g_graphicsEngine->GetFrameBufferWidth();
        int frameBuffer_h = g_graphicsEngine->GetFrameBufferHeight();

        // アルベドカラーを出力用のレンダリングターゲットを初期化する
        m_gBuffer[enGBufferAlbedo].Create(
            frameBuffer_w,
            frameBuffer_h,
            1,
            1,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_D32_FLOAT
        );

        // 法線出力用のレンダリングターゲットを初期化する
        m_gBuffer[enGBufferNormal].Create(
            frameBuffer_w,
            frameBuffer_h,
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_UNORM, //メモリ使用量とメモリ書き込み速度優先で、8bitの符号なし整数バッファを使用する。
            DXGI_FORMAT_UNKNOWN
        );

        // 金属度と滑らかさマップ出力用のレンダリングターゲットを初期化する
        m_gBuffer[enGBufferMetalSmooth].Create(
            frameBuffer_w,
            frameBuffer_h,
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_UNORM, //メモリ使用量メモリ書き込み速度優先で、8bitの符号なし整数バッファを使用する。。
            DXGI_FORMAT_UNKNOWN
        );

        // ワールド座標出力用のレンダリングターゲットを初期化する
        m_gBuffer[enGBufferWorldPos].Create(
            frameBuffer_w,
            frameBuffer_h,
            1,
            1,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_UNKNOWN
        );
        m_gBuffer[enGBUfferShadowParam].Create(
            frameBuffer_w,
            frameBuffer_h,
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT_UNKNOWN
        );
    }

    void RenderingEngine::InitCopyMainRenderTargetToFrameBufferSprite()
    {
        SpriteInitData spriteInitData;
        // テクスチャはyBlurRenderTargetのカラーバッファー
        spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();

        // レンダリング先がフレームバッファーなので、解像度はフレームバッファーと同じ
        spriteInitData.m_width = g_graphicsEngine->GetFrameBufferWidth();
        spriteInitData.m_height = g_graphicsEngine->GetFrameBufferHeight();

        // ボケ画像をそのまま貼り付けるだけなので、通常の2D描画のシェーダーを指定する
        spriteInitData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
        spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        // 初期化オブジェクトを使って、スプライトを初期化する
        m_copyMainRtToFrameBufferSprite.Init(spriteInitData);
    }

    void RenderingEngine::InitDeferredLighting()
    {
        int frameBuffer_w = g_graphicsEngine->GetFrameBufferWidth();
        int frameBuffer_h = g_graphicsEngine->GetFrameBufferHeight();

        // 太陽光
        m_deferredLightingCB.m_light.directionalLight[0].color.x = 4.8f;
        m_deferredLightingCB.m_light.directionalLight[0].color.y = 4.8f;
        m_deferredLightingCB.m_light.directionalLight[0].color.z = 4.8f;

        m_deferredLightingCB.m_light.directionalLight[0].direction.x = 1.0f;
        m_deferredLightingCB.m_light.directionalLight[0].direction.y = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[0].direction.z = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[0].direction.Normalize();
        m_deferredLightingCB.m_light.directionalLight[0].castShadow = true;

        //
        m_deferredLightingCB.m_light.directionalLight[1].color.x = 4.8f;
        m_deferredLightingCB.m_light.directionalLight[1].color.y = 4.8f;
        m_deferredLightingCB.m_light.directionalLight[1].color.z = 4.8f;

        m_deferredLightingCB.m_light.directionalLight[1].direction.x = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[1].direction.y = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[1].direction.z = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[1].direction.Normalize();
        m_deferredLightingCB.m_light.directionalLight[1].castShadow = true;

        //地面からの照り返し
        m_deferredLightingCB.m_light.directionalLight[2].color.x = 0.8f;
        m_deferredLightingCB.m_light.directionalLight[2].color.y = 0.8f;
        m_deferredLightingCB.m_light.directionalLight[2].color.z = 0.8f;

        m_deferredLightingCB.m_light.directionalLight[2].direction.x = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[2].direction.y = 1.0f;
        m_deferredLightingCB.m_light.directionalLight[2].direction.z = -1.0f;
        m_deferredLightingCB.m_light.directionalLight[2].direction.Normalize();

        m_deferredLightingCB.m_light.ambinetLight.x = 0.2f;
        m_deferredLightingCB.m_light.ambinetLight.y = 0.2f;
        m_deferredLightingCB.m_light.ambinetLight.z = 0.2f;
        m_deferredLightingCB.m_light.eyePos = g_camera3D->GetPosition();
        m_deferredLightingCB.m_light.specPow = 5.0f;

        // ポストエフェクト的にディファードライティングを行うためのスプライトを初期化
        SpriteInitData spriteInitData;

        // 画面全体にレンダリングするので幅と高さはフレームバッファーの幅と高さと同じ
        spriteInitData.m_width = frameBuffer_w;
        spriteInitData.m_height = frameBuffer_h;

        // ディファードライティングで使用するテクスチャを設定
        int texNo = 0;
        for (auto& gBuffer : m_gBuffer)
        {
            spriteInitData.m_textures[texNo++] = &gBuffer.GetRenderTargetTexture();
        }

        spriteInitData.m_fxFilePath = "Assets/shader/preset/DeferredLighting.fx";
        spriteInitData.m_expandConstantBuffer = &m_deferredLightingCB;
        spriteInitData.m_expandConstantBufferSize = sizeof(m_deferredLightingCB);
        for (int i = 0; i < NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT; i++)
        {
            for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
            {
                spriteInitData.m_textures[texNo++] = &m_shadowMapRenders[i].GetShadowMap(areaNo);
            }
        }
        spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

        // 初期化データを使ってスプライトを作成
        m_diferredLightingSprite.Init(spriteInitData);
    }

    void RenderingEngine::Execute(RenderContext& rc)
    {
        // シャドウマップへの描画
        RenderToShadowMap(rc);

        // ZPrepass
        ZPrepass(rc);

        // G-Bufferへのレンダリング
        RenderToGBuffer(rc);

        // ディファードライティング
        DeferredLighting(rc);

        // ディファードライティングが終わった時点でスナップショットを撮影する
        SnapshotMainRenderTarget(rc, EnMainRTSnapshot::enDrawnOpacity);

        // フォワードレンダリング
        ForwardRendering(rc);

        // ポストエフェクトを実行
        m_postEffect.Render(rc, m_mainRenderTarget);

        // メインレンダリングターゲットの内容をフレームバッファにコピー
        CopyMainRenderTargetToFrameBuffer(rc);

        // 登録されている3Dモデルをクリア
        m_renderToGBufferModels.clear();
        m_forwardRenderModels.clear();
        m_zprepassModels.clear();
    }

    void RenderingEngine::RenderToShadowMap(RenderContext& rc)
    {
        int ligNo = 0;
        for (auto& shadowMapRender : m_shadowMapRenders)
        {
            shadowMapRender.Render(
                rc,
                m_deferredLightingCB.m_light.directionalLight[ligNo].direction
            );
            ligNo++;
        }
    }

    void RenderingEngine::ZPrepass(RenderContext& rc)
    {
        // まず、レンダリングターゲットとして設定できるようになるまで待つ
        rc.WaitUntilToPossibleSetRenderTarget(m_zprepassRenderTarget);

        // レンダリングターゲットを設定
        rc.SetRenderTargetAndViewport(m_zprepassRenderTarget);

        // レンダリングターゲットをクリア
        rc.ClearRenderTargetView(m_zprepassRenderTarget);

        for (auto& model : m_zprepassModels)
        {
            model->Draw(rc);
        }

        rc.WaitUntilFinishDrawingToRenderTarget(m_zprepassRenderTarget);
    }

    void RenderingEngine::ForwardRendering(RenderContext& rc)
    {
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTarget(
            m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
            m_gBuffer[enGBufferAlbedo].GetDSVCpuDescriptorHandle()
        );
        for (auto& model : m_forwardRenderModels)
        {
            model->Draw(rc);
        }

        // メインレンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
    }

    void RenderingEngine::RenderToGBuffer(RenderContext& rc)
    {
        // レンダリングターゲットをG-Bufferに変更
        RenderTarget* rts[enGBufferNum] = {
            &m_gBuffer[enGBufferAlbedo],        // 0番目のレンダリングターゲット
            &m_gBuffer[enGBufferNormal],        // 1番目のレンダリングターゲット
            &m_gBuffer[enGBufferWorldPos],      // 2番目のレンダリングターゲット
            &m_gBuffer[enGBufferMetalSmooth],   // 3番目のレンダリングターゲット
            &m_gBuffer[enGBUfferShadowParam],   // 4番目のレンダリングターゲット
        };

        // まず、レンダリングターゲットとして設定できるようになるまで待つ
        rc.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリングターゲットを設定
        rc.SetRenderTargets(ARRAYSIZE(rts), rts);

        // レンダリングターゲットをクリア
        rc.ClearRenderTargetViews(ARRAYSIZE(rts), rts);
        for (auto& model : m_renderToGBufferModels)
        {
            model->Draw(rc);
        }

        // レンダリングターゲットへの書き込み待ち
        rc.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);
    }

    void RenderingEngine::SnapshotMainRenderTarget(RenderContext& rc, EnMainRTSnapshot enSnapshot)
    {
        // メインレンダリングターゲットの内容をスナップショット
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRTSnapshots[(int)enSnapshot]);
        rc.SetRenderTargetAndViewport(m_mainRTSnapshots[(int)enSnapshot]);
        m_copyMainRtToFrameBufferSprite.Draw(rc);
        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRTSnapshots[(int)enSnapshot]);
    }

    void RenderingEngine::DeferredLighting(RenderContext& rc)
    {
        // ディファードライティングに必要なライト情報を更新する
        m_deferredLightingCB.m_light.eyePos = g_camera3D->GetPosition();
        for (int i = 0; i < NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT; i++)
        {
            for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
            {
                m_deferredLightingCB.mlvp[i][areaNo] = m_shadowMapRenders[i].GetLVPMatrix(areaNo);
            }
        }

        // レンダリング先をメインレンダリングターゲットにする
        // メインレンダリングターゲットを設定
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTargetAndViewport(m_mainRenderTarget);

        // G-Bufferの内容を元にしてディファードライティング
        m_diferredLightingSprite.Draw(rc);

        // メインレンダリングターゲットへの書き込み終了待ち
        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
    }

    void RenderingEngine::CopyMainRenderTargetToFrameBuffer(RenderContext& rc)
    {
        // メインレンダリングターゲットの絵をフレームバッファーにコピー
        rc.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );

        // ビューポートを指定する
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = 1280;
        viewport.Height = 720;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        rc.SetViewportAndScissor(viewport);
        m_copyMainRtToFrameBufferSprite.Draw(rc);
    }
}
