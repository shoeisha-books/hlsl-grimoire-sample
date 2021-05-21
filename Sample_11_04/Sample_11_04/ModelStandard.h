#pragma once

/// <summary>
/// Standardシェーダーを利用したモデル表示処理
/// </summary>
class ModelStandard
{
public:
	enum { NUM_DIRECTIONAL_LIGHT = 4 };	//ディレクションライトの数。

	/// <summary>
	/// ディレクションライト。
	/// </summary>
	struct DirectionalLight {
		Vector3 direction;	//ライトの方向。
		float pad0;			//パディング。
		Vector4 color;		//ライトのカラー。
	};
	/// <summary>
	/// ライト構造体。
	/// </summary>
	struct Light {
		DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];	//ディレクションライト。
		Vector3 eyePos;					//カメラの位置。
		float specPow;					//スペキュラの絞り。
		Vector3 ambinetLight;			//環境光。
	};
public: 
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="tkmFilePath">tkmファイルのファイルパス。</param>
	void Init(const char* tkmFilePath);
	/// <summary>
	/// 毎フレーム呼び出す必要がある更新処理。
	/// </summary>
	/// <param name="pos">座標</param>
	/// <param name="rot">回転</param>
	/// <param name="scale">拡大率</param>
	void Update(Vector3 pos, Quaternion rot, Vector3 scale)
	{
		m_light.eyePos = g_camera3D->GetPosition();
		m_model.UpdateWorldMatrix(pos, rot, scale);
	}
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="rc">レンダリングコンテキスト</param>
	void Draw(RenderContext& rc)
	{
		m_model.Draw(rc);
	}
private:
	Model m_model;	//モデル。
	Light m_light;	//ライト。
};

