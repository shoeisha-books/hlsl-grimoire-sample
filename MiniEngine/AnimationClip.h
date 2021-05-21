/*!
 *@brief	アニメーションクリップ。
 */

#pragma once

#include "tkFile/TkaFile.h"

	
/*!
*@brief	キーフレーム。
*/
struct KeyFrame {
	uint32_t boneIndex;	//!<ボーンインデックス。
	float time;					//!<時間。
	Matrix transform;			//!<トランスフォーム。
};

/*!
*@brief	アニメーションイベント。
*/
class AnimationEvent {
public:
	
	/*!
	*@brief	イベント発生時間を設定。
	*/
	float GetInvokeTime() const
	{
		return m_invokeTime;
	}
	/*!
	*@brief	イベント名を取得。
	*/
	const wchar_t* GetEventName() const
	{
		return m_eventName.c_str();
	}
	/*!
	*@brief	イベント発生時間を設定。
	*/
	void SetInvokeTime(float time)
	{
		m_invokeTime = time;
	}
	/*!
	*@brief	イベント名を設定。
	*/
	void SetEventName(const wchar_t* name)
	{
		m_eventName = name;
	}
	/*!
	*@brief	イベントが発生済みか判定。
	*/
	bool IsInvoked() const
	{
		return m_isInvoked;
	}
	/*!
	*@brief	イベントが発生済みのフラグを設定する。
	*/
	void SetInvokedFlag(bool flag)
	{
		m_isInvoked = flag;
	}
private:
	bool m_isInvoked = false;	//!<イベント発生済み？
	float m_invokeTime = 0.0f;	//!<イベント発生時間。
	std::wstring m_eventName;	//!<イベント名。
};
/*!
	*@brief	アニメーションクリップ。
	*/
class AnimationClip  {
public:
	
	using keyFramePtrList = std::vector<KeyFrame*>;
	/*!
	* @brief	コンストラクタ
	*/
	AnimationClip()
	{
	}
	/*!
		*@brief	デストラクタ。
		*/
	~AnimationClip();
	/// <summary>
	/// アニメーションクリップを同期ロード。
	/// </summary>
	/// <param name="filePath"></param>
	void Load(const char* filePath);
	
	/// <summary>
	/// キーフレームとアニメーションイベントを構築する。
	/// </summary>
	/// <remarks>
	/// ロードが終わったときに呼び出してください。
	/// </remarks>
	void BuildKeyFramesAndAnimationEvents();
	/*!
	*@brief	ループする？
	*/
	bool IsLoop() const
	{
		return m_isLoop;
	}
	/*!
	*@brief	ループフラグを設定する。
	*/
	void SetLoopFlag(bool flag)
	{
		m_isLoop = flag;
	}
	/*!
	*@brief
	*/
	const std::vector<keyFramePtrList>& GetKeyFramePtrListArray() const
	{
		return m_keyFramePtrListArray;
	}
	const keyFramePtrList& GetTopBoneKeyFrameList() const
	{
		return *m_topBoneKeyFramList;
	}
	/*!
		*@brief	クリップ名を取得。
		*/
	const wchar_t* GetName() const
	{
		return m_clipName.c_str();
	}
	/*!
	*@brief	アニメーションイベントを取得。
	*/
	std::unique_ptr<AnimationEvent[]>& GetAnimationEvent()
	{
		return m_animationEvent;
	}

	/// <summary>
	/// アニメーションイベントの数を取得
	/// </summary>
	/// <returns></returns>
	int GetNumAnimationEvent() const
	{
		return m_tkaFile.GetNumAnimationEvent();
	}
private:
	using KeyframePtr = std::unique_ptr<KeyFrame>;
	std::wstring m_clipName;	//!<アニメーションクリップの名前。
	bool m_isLoop = false;	//!<ループフラグ。
	std::vector<KeyframePtr>			m_keyframes;				//キーフレーム。
	std::vector<keyFramePtrList>		m_keyFramePtrListArray;		//ボーンごとのキーフレームのリストを管理するための配列。
	std::unique_ptr<AnimationEvent[]>	m_animationEvent;			//アニメーションイベント。
	int									m_numAnimationEvent = 0;	//アニメーションイベントの数。
	keyFramePtrList*					m_topBoneKeyFramList = nullptr;
	TkaFile							m_tkaFile;			//tkaファイル
};
using AnimationClipPtr = std::unique_ptr<AnimationClip>;