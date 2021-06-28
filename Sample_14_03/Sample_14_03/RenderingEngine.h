#pragma once

#include "MyRenderer.h"
#include "ShadowMapRender.h"
#include "PostEffect.h"

namespace myRenderer
{
    // レンダリングエンジン
    class RenderingEngine
    {
    public:
        // ディレクションライト
        struct DirectionalLight
        {
            Vector3 direction;  // ライトの方向
            int castShadow;     // 影をキャストする？
            Vector4 color;      // ライトのカラー
        };

        // ライト構造体
        struct Light
        {
            std::array<DirectionalLight, NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT> directionalLight; // ディレクションライト
            Vector3 eyePos;         // カメラの位置
            float specPow;          // スペキュラの絞り
            Vector3 ambinetLight;   // 環境光
        };

        // メインレンダリングターゲットのスナップショット
        enum class EnMainRTSnapshot
        {
            enDrawnOpacity,     // 不透明オブジェクトの描画完了時点
            enNum,              // スナップショットの数
        };

        // レンダリングパス
        enum class EnRenderingPass
        {
            enRenderToShadowMap,    // シャドウマップへの描画パス
            enZPrepass,             // ZPrepass
            enRenderToGBuffer,      // G-Bufferへの描画パス
            enForwardRender,        // フォワードレンダリングの描画パス
        };

        /// <summary>
        /// レンダリングパイプラインを初期化
        /// </summary>
        void Init();

        /// <summary>
        /// シャドウマップへの描画パスにモデルを追加
        /// </summary>
        /// <param name="ligNo">シャドウマップを生成するライトの番号</param>
        /// <param name="model0">近景用のシャドウマップに描画するモデル</param>
        /// <param name="model1">中景用のシャドウマップ1に描画するモデル</param>
        /// <param name="model2">遠景用のシャドウマップ2に描画するモデル</param>
        void Add3DModelToRenderToShadowMap(
            int ligNo,
            Model& model0,
            Model& model1,
            Model& model2
        )
        {
            m_shadowMapRenders[ligNo].Add3DModel(model0, model1, model2);
        }

        /// <summary>
        /// ZPrepassの描画パスにモデルを追加
        /// </summary>
        /// <param name="model"></param>
        void Add3DModelToZPrepass(Model& model)
        {
            m_zprepassModels.push_back(&model);
        }

        /// <summary>
        /// GBufferの描画パスにモデルを追加
        /// </summary>
        /// <param name="model"></param>
        void Add3DModelToRenderGBufferPass(Model& model)
        {
            m_renderToGBufferModels.push_back(&model);
        }

        /// <summary>
        /// フォワードレンダリングの描画パスにモデルを追加
        /// </summary>
        /// <param name="model"></param>
        void Add3DModelToForwardRenderPass(Model& model)
        {
            m_forwardRenderModels.push_back(&model);
        }

        /// <summary>
        /// ZPrepassで作成された深度テクスチャを取得
        /// </summary>
        /// <returns></returns>
        Texture& GetZPrepassDepthTexture()
        {
            return m_zprepassRenderTarget.GetRenderTargetTexture();
        }

        /// <summary>
        /// GBufferのアルベドテクスチャを取得
        /// </summary>
        /// <returns></returns>
        Texture& GetGBufferAlbedoTexture()
        {
            return m_gBuffer[enGBufferAlbedo].GetRenderTargetTexture();
        }

        /// <summary>
        /// 不透明オブジェクトの描画完了時のメインレンダリングターゲットのスナップショットを取得
        /// </summary>
        /// <returns></returns>
        Texture& GetMainRenderTargetSnapshotDrawnOpacity()
        {
            return m_mainRTSnapshots[(int)EnMainRTSnapshot::enDrawnOpacity].GetRenderTargetTexture();
        }

        /// <summary>
        /// レンダリングパイプラインを実行
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト。</param>
        void Execute(RenderContext& rc);

        /// <summary>
        /// ディレクションライトのパラメータを設定
        /// </summary>
        /// <param name="lightNo"></param>
        /// <param name="direction"></param>
        /// <param name="color"></param>
        void SetDirectionLight(int lightNo, Vector3 direction, Vector3 color)
        {
            m_deferredLightingCB.m_light.directionalLight[lightNo].direction = direction;
            m_deferredLightingCB.m_light.directionalLight[lightNo].color = color;
        }

    private:
        /// <summary>
        /// G-Bufferを初期化
        /// </summary>
        void InitGBuffer();

        /// <summary>
        /// ディファードライティングの初期化
        /// </summary>
        void InitDeferredLighting();

        /// <summary>
        /// シャドウマップに描画
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        void RenderToShadowMap(RenderContext& rc);

        /// <summary>
        /// ZPrepass
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        void ZPrepass(RenderContext& rc);

        /// <summary>
        /// G-Bufferへの描画
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト。</param>
        void RenderToGBuffer(RenderContext& rc);

        /// <summary>
        /// ディファードライティング
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        void DeferredLighting(RenderContext& rc);

        /// <summary>
        /// メインレンダリングターゲットの内容をフレームバッファにコピーする
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        void CopyMainRenderTargetToFrameBuffer(RenderContext& rc);

        /// <summary>
        /// フォワードレンダリング
        /// </summary>
        /// <param name="rc">レンダリングコンテキスト</param>
        void ForwardRendering(RenderContext& rc);

        /// <summary>
        /// メインレンダリングターゲットを初期化
        /// </summary>
        void InitMainRenderTarget();

        /// <summary>
        /// メインレンダリングターゲットののスナップショットを取るためのレンダリングターゲットを初期化
        /// </summary>
        void InitMainRTSnapshotRenderTarget();

        /// <summary>
        /// メインレンダリングターゲットのカラーバッファの内容を
        /// フレームバッファにコピーするためのスプライトを初期化する
        /// </summary>
        void InitCopyMainRenderTargetToFrameBufferSprite();

        /// <summary>
        /// ZPrepass用のレンダリングターゲットを初期化
        /// </summary>
        void InitZPrepassRenderTarget();

        /// <summary>
        /// メインレンダリングターゲットのスナップショットを撮影
        /// </summary>
        void SnapshotMainRenderTarget(RenderContext& rc, EnMainRTSnapshot enSnapshot);

        /// <summary>
        /// シャドウマップへの描画処理を初期化
        /// </summary>
        void InitShadowMapRender();

    private:
        //GBufferの定義
        enum EnGBuffer
        {
            enGBufferAlbedo,        // アルベド
            enGBufferNormal,        // 法線
            enGBufferWorldPos,      // ワールド座標
            enGBufferMetalSmooth,   // 金属度と滑らかさ。xに金属度、wに滑らかさが記録されている。
            enGBUfferShadowParam,   // 影パラメータ
            enGBufferNum,           // G-Bufferの数
        };

        // ディファードライティング用の定数バッファ
        struct SDeferredLightingCB
        {
            Light m_light;      // ライト
            float pad;          // パディング
            Matrix mlvp[NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];
        };

        shadow::ShadowMapRender m_shadowMapRenders[NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT];	//シャドウマップへの描画処理
        SDeferredLightingCB m_deferredLightingCB;   // ディファードライティング用の定数バッファ
        Sprite m_copyMainRtToFrameBufferSprite;     // メインレンダリングターゲットをフレームバッファにコピーするためのスプライト
        Sprite m_diferredLightingSprite;            // ディファードライティングを行うためのスプライト
        RenderTarget m_zprepassRenderTarget;        // ZPrepass描画用のレンダリングターゲット
        RenderTarget m_mainRenderTarget;            // メインレンダリングターゲット
        RenderTarget m_mainRTSnapshots[(int)EnMainRTSnapshot::enNum];   // メインレンダリングターゲットのスナップショット
        RenderTarget m_gBuffer[enGBufferNum];                           // G-Buffer
        PostEffect m_postEffect;                                        // ポストエフェクト
        std::vector< Model* > m_zprepassModels;                         // ZPrepassの描画パスで描画されるモデルのリスト
        std::vector< Model* > m_renderToGBufferModels;                  // Gバッファへの描画パスで描画するモデルのリスト
        std::vector< Model* > m_forwardRenderModels;                    // フォワードレンダリングの描画パスで描画されるモデルのリスト
    };
}
