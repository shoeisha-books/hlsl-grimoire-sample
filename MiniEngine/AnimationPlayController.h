/*!
* @brief	アニメーション再生コントローラ。
*/

#pragma once


class Skeleton;
class AnimationClip;
class Animation;
class Bone;

/*!
* @brief	アニメーションの再生コントローラ。
* @details
*  一つのアニメーションクリップに対してアニメーションを進めて、ローカルポーズを計算します。
*/
class AnimationPlayController{
public:
	/*!
	* @brief	コンストラクタ。
	*/
	AnimationPlayController()
	{
	}
	/*!
	* @brief	デストラクタ。
	*/
	~AnimationPlayController()
	{
	}
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="skeleton">スケルトン</param>
	/// <param name="footStepBoneNo">footstepボーンの番号</param>
	void Init(Skeleton* skeleton, int footStepBoneNo);
	/*!
	* @brief	アニメーションクリップの変更。
	*/
	void ChangeAnimationClip(AnimationClip* clip);
	
	void SetInterpolateTime(float interpolateTime)
	{
		if (interpolateTime < 0.0f) {
			//interpolateTime is negative!!
			return;
		}
		m_interpolateEndTime = interpolateTime;
		m_interpolateTime = 0.0f;
	}
	/*!
		*@brief	補完率を取得。
		*/
	float GetInterpolateRate() const
	{
		if (m_interpolateEndTime <= 0.0f) {
			return 1.0f;
		}
		return min( 1.0f, m_interpolateTime / m_interpolateEndTime );
	}
	/*!
	* @brief	アニメーションを進める。
	*@param[in]	deltaTime		アニメーションを進める時間。
	*/
	void Update(float deltaTime, Animation* animation);
	/*!
		* @brief	ローカルボーン行列を取得。
		*/
	const std::vector<Matrix>& GetBoneLocalMatrix() const
	{
		return m_boneMatrix;
	}
	AnimationClip* GetAnimClip() const
	{
		return m_animationClip;
	}
	/*!
	* @brief	再生中？
	*/
	bool IsPlaying() const
	{
		return m_isPlaying;
	}
	/// <summary>
	/// Update関数を呼び出したときの、footstepボーンの移動量を取得。
	/// </summary>
	/// <returns></returns>
	Vector3 GetFootStepDeltaValueOnUpdate() const
	{
		return m_footstepDeltaValue;
	}
private:
	/*!
	*@brief	アニメーションイベントを起動する。
	*/
	void InvokeAnimationEvent(Animation* animation);
	/*!
	*@brief	ループ再生開始する時の処理。
	*/
	void StartLoop();
	/// <summary>
	/// ルートのボーン空間でのボーン行列を計算する。
	/// </summary>
	/// <param name="bone">計算する骨</param>
	/// <param name="parentMatrix">親の行列</param>
	void CalcBoneMatrixInRootBoneSpace(Bone& bone, Matrix parentMatrix);
	/// <summary>
	/// ボーン行列をアニメーションクリップからサンプリングする。
	/// </summary>
	void SamplingBoneMatrixFromAnimationClip();
	/// <summary>
	/// ルートボーン空間での行列を計算する。
	/// </summary>
	void CalcBoneMatrixInRootBoneSpace();
	/// <summary>
	/// footstepボーンの変化量をサンプリングする。
	/// </summary>
	void SamplingDeltaValueFootstepBone();
	/// <summary>
	/// footstepボーンの座標を全体の骨から減算する。
	/// </summary>
	void SubtractFootstepbonePosFromAllBone();
	/// <summary>
	/// キーフレーム番号を進める。
	/// </summary>
	void ProgressKeyframeNo(float deltaTime);
private:
	AnimationClip*			m_animationClip = nullptr;			//アニメーションクリップ。
	int						m_currentKeyFrameNoLastFrame = 0;	//一フレーム前のキーフレーム番号。
	int						m_currentKeyFrameNo = 0;			//現在再生中のキーフレーム番号。
	float					m_time = 0.0f;
	std::vector<Matrix>		m_boneMatrix;						//!<このコントローラで再生中のアニメーションのボーン行列。
	float					m_interpolateTime;					//!<補完時間
	float					m_interpolateEndTime;				//!<補完終了時間
	bool					m_isPlaying = false;				//!<再生中？
	Skeleton*				m_skeleton = nullptr;				//!<スケルトン。
	Vector3					m_footstepDeltaValue = g_vec3Zero;	//フットステップの移動ベクトル。
	Vector3					m_footstepPos = g_vec3Zero;			//フットステップボーンの座標。
	int						m_footstepBoneNo = -1;				//フットステップのボーンの番号。
};
