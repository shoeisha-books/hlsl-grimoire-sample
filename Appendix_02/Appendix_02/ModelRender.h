#pragma once

#include "MyRenderer.h"

namespace myRenderer
{
    class RenderingEngine;
    /// <summary>
    /// フォワードレンダリング用のモデル初期化構造体。
    /// </summary>
    /// <remark>
    /// ModelInitDataを継承しています。
    /// フォワードレンダリングのために必要なデフォルト設定をコンストラクタで行ってくれます。
    /// </remark>
    struct ModelInitDataFR : public ModelInitData
    {
        ModelInitDataFR()
        {
            m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    };
    class ModelRender
    {
    public:
        
        /// <summary>
        /// 初期化
        /// </summary>
        /// <remark>
        /// 本関数を利用して初期化されたモデルは、
        /// ディファードレンダリングの描画パスで描画されます
        /// ディファードレンダリングでの描画はPBRシェーダーでライティングが計算されます
        /// </remark>
        /// <param name="renderingEngine">レンダリングエンジン。</param>
        /// <param name="tkmFilePath">tkmファイルパス。</param>
        /// <param name="isShadowReciever">シャドウレシーバーフラグ</param>
        void InitDeferredRendering(RenderingEngine& renderingEngine, const char* tkmFilePath, bool isShadowReciever);

        /// <summary>
        /// 初期化
        /// </summary>
        /// <remark>
        /// 本関数を利用して初期化されたモデルは
        /// フォワードレンダリングの描画パスで描画されます
        /// 特殊なシェーディングを行いたい場合は、こちらを使用してください
        /// </remark>
        /// <param name="renderingEngine">レンダリングエンジン</param>
        /// <param name="modelInitData"></param>
        void InitForwardRendering(RenderingEngine& renderingEngine, ModelInitDataFR& modelInitData);

        /// <summary>
        /// ワールド行列を更新する
        /// </summary>
        /// <param name="pos">座標</param>
        /// <param name="rot">回転クォータニオン</param>
        /// <param name="scale">拡大率</param>
        void UpdateWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale)
        {
            m_zprepassModel.UpdateWorldMatrix(pos, rot, scale);
            if (m_renderToGBufferModel.IsInited())
            {
                m_renderToGBufferModel.UpdateWorldMatrix(pos, rot, scale);
            }
            if (m_forwardRenderModel.IsInited())
            {
                m_forwardRenderModel.UpdateWorldMatrix(pos, rot, scale);
            }
        }

        /// <summary>
        /// 描画
        /// </summary>
        void Draw();

        /// <summary>
        /// シャドウキャスターのフラグを設定する
        /// </summary>
        void SetShadowCasterFlag(bool flag)
        {
            m_isShadowCaster = flag;
        }

    private:
        /// <summary>
        /// 共通の初期化処理
        /// </summary>
        /// <param name="renderingEngine">レンダリングエンジン</param>
        /// <param name="tkmFilePath">tkmファイルパス</param>
        void InitCommon(RenderingEngine& renderingEngine, const char* tkmFilePath);

    private:
        RenderingEngine* m_renderingEngine = nullptr;   //レンダリングエンジン
        Model m_zprepassModel;                  // ZPrepassで描画されるモデル
        Model m_forwardRenderModel;             // フォワードレンダリングの描画パスで描画されるモデル
        Model m_renderToGBufferModel;           // RenderToGBufferで描画されるモデル
        Model m_shadowModels[NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];	//シャドウマップに描画するモデル
        bool m_isShadowCaster = false;          // シャドウキャスターフラグ
    };
}
