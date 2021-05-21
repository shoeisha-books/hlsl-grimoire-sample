/*!
* @brief	アニメーション
*/

#pragma once

#include "AnimationClip.h"
#include "AnimationPlayController.h"



class Skeleton;
using AnimationEventListener = std::function<void(const wchar_t* clipName, const wchar_t* eventName)>;

/// <summary>
/// アニメーションクラス。
/// </summary>
/// <remarks>
/// 提供する機能
/// １. シンプルなアニメーション再生。
/// ２. アニメーション補間。
/// ３. footstepボーンを活用したアニメーション移動量の計算。
///		スケルトンにfootstepボーンを追加すると、全体のボーンから、
///		footstepボーンの平行移動量を除外したアニメーション再生が行われます。
///		除外された移動量はCalcFootstepDeltaValueInWorldSpace関数を利用することで
///		計算することができます。
///		この機能を活用することで、アニメータが作成した歩きアニメーションなどに移動量を
///		含むことができ、アニメータが意図した移動を行うことができます。
/// </remarks>
class Animation {
public:

	/// <summary>
	/// 初期化済みか判定。
	/// </summary>
	/// <returns>trueが返ってきたら初期化済み。</returns>
	bool IsInited() const
	{
		return m_isInited;
	}

	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="skeleton">アニメーションさせるスケルトン</param>
	/// <param name="animClips">アニメーションクリップの配列</param>
	/// <param name="numAnimClip">アニメーションクリップの数</param>
	void Init(
		Skeleton& skeleton, 
		AnimationClip* animClips,
		int numAnimClip
	);
	/// <summary>
	/// アニメーションの再生。
	/// </summary>
	/// <param name="clipNo">アニメーションクリップの番号。Init関数に渡したanimClipListの並びとなる。</param>
	/// <param name="interpolateTime">補完時間(単位：秒)</param>
	void Play(int clipNo, float interpolateTime = 0.0f)
	{
		if (clipNo < m_animationClips.size()) {
			PlayCommon(m_animationClips[clipNo], interpolateTime);
		}
	}
	/// <summary>
	/// アニメーションクリップのループフラグを設定します。
	/// </summary>
	/// <param name="clipName">アニメーションクリップの名前</param>
	/// <param name="flag">フラグ</param>
	void SetAnimationClipLoopFlag(const wchar_t* clipName, bool flag)
	{
		auto it = std::find_if(
			m_animationClips.begin(),
			m_animationClips.end(),
			[clipName](auto& clip) {return clip->GetName() == clipName; }
		);
		if (it == m_animationClips.end()) {
			//見つからなかった。
			return;
		}
		(*it)->SetLoopFlag(flag);
	}
	/// <summary>
	/// アニメーションの再生中？
	/// </summary>
	/// <returns></returns>
	bool IsPlaying() const
	{
		int lastIndex = GetLastAnimationControllerIndex();
		return m_animationPlayController[lastIndex].IsPlaying();
	}

	/// <summary>
	/// アニメーションを進める。
	/// </summary>
	/// <remarks>
	/// エンジン内部から呼ばれます。
	/// ユーザーは使用しないでください。
	/// </remarks>
	/// <param name="deltaTime">アニメーションを進める時間(単位：秒)</param>
	void Progress(float deltaTime);
	/*!
	*@brief	アニメーションイベントリスナーを登録。
	*@return
	* 登録されたリスナー。
	*/
	void AddAnimationEventListener(AnimationEventListener eventListener)
	{
		m_animationEventListeners.push_back(eventListener);
	}
	
	/*!
	* @brief	アニメーションイベントをリスナーに通知。
	*/
	void NotifyAnimationEventToListener(const wchar_t* clipName, const wchar_t* eventName)
	{
		for (auto& listener : m_animationEventListeners) {
			listener(clipName, eventName);
		}
	}
	/// <summary>
	/// ワールド空間でのフットステップの移動量を計算する。
	/// </summary>
	/// <remarks>
	/// フットステップの移動量は、モデルのルートからの相対移動量です。
	/// そのため、ワールド空間に変換するのに平行移動量は不要です。
	/// モデルの回転クォータニオンと拡大率のみ指定してください。
	/// </remarks>
	/// <param name="rotation">モデルの回転</param>
	/// <param name="scale">モデルの拡大率</param>
	/// <returns>ワールド空間でのフットステップの移動量。</returns>
	Vector3 CalcFootstepDeltaValueInWorldSpace(Quaternion rotation, Vector3 scale) const;

private:
	void PlayCommon(AnimationClip* nextClip, float interpolateTime)
	{
		int index = GetLastAnimationControllerIndex();
		if (m_animationPlayController[index].GetAnimClip() == nextClip) {
			return;
		}
		if (interpolateTime == 0.0f) {
			//補完なし。
			m_numAnimationPlayController = 1;
		}
		else {
			//補完あり。
			m_numAnimationPlayController++;
		}
		index = GetLastAnimationControllerIndex();
		m_animationPlayController[index].ChangeAnimationClip(nextClip);
		m_animationPlayController[index].SetInterpolateTime(interpolateTime);
		m_interpolateTime = 0.0f;
		m_interpolateTimeEnd = interpolateTime;
	}
	/// <summary>
	/// ローカルポーズの更新。
	/// </summary>
	/// <param name="deltaTime">アニメーションを進める時間。単位：秒。</param>
	void UpdateLocalPose(float deltaTime);
	/// <summary>
	/// グローバルポーズの更新。
	/// </summary>
	void UpdateGlobalPose();
private:
		
	/*!
		*@brief	最終ポーズになるアニメーションのリングバッファ上でのインデックスを取得。
		*/
	int GetLastAnimationControllerIndex() const
	{
		return GetAnimationControllerIndex(m_startAnimationPlayController, m_numAnimationPlayController - 1);
	}
	/*!
	*@brief	アニメーションコントローラのリングバッファ上でのインデックスを取得。
	*@param[in]	startIndex		開始インデックス。
	*@param[in]	localIndex		ローカルインデックス。
	*/
	int GetAnimationControllerIndex(int startIndex, int localIndex) const
	{
		return (startIndex + localIndex) % ANIMATION_PLAY_CONTROLLER_NUM;
	}
		
private:
	static const int ANIMATION_PLAY_CONTROLLER_NUM = 32;	//!<アニメーションコントローラの数。
	std::vector<AnimationClip*>	m_animationClips;	//!<アニメーションクリップの配列。
	Skeleton* m_skeleton = nullptr;	//!<アニメーションを適用するスケルトン。
	AnimationPlayController	m_animationPlayController[ANIMATION_PLAY_CONTROLLER_NUM];	//!<アニメーションコントローラ。リングバッファ。
	int m_numAnimationPlayController = 0;		//!<現在使用中のアニメーション再生コントローラの数。
	int m_startAnimationPlayController = 0;		//!<アニメーションコントローラの開始インデックス。
	float m_interpolateTime = 0.0f;
	float m_interpolateTimeEnd = 0.0f;
	bool m_isInterpolate = false;								//!<補間中？
	std::vector<AnimationEventListener>	m_animationEventListeners;	//!<アニメーションイベントリスナーのリスト。
	Vector3 m_footstepDeltaValue = g_vec3Zero;					//footstepボーンの移動量。
	bool m_isInited = false;
	float m_deltaTimeOnUpdate = 0.0f;							//Update関数を実行したときのデルタタイム。
};