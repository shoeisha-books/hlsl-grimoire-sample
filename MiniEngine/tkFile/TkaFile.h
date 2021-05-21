/// <summary>
/// tkaファイル
/// </summary>
/// <remarks>
/// tkaファイルはキーフレームアニメーションデータのフォーマットです。
/// 本クラスを利用するとこではtkaファイルを扱うことができます。
/// </remarks>
#pragma once

class TkaFile {
public:
	/// <summary>
	/// アニメーションクリップのヘッダー。
	/// </summary>
	struct AnimClipHeader {
		std::uint32_t numKey;				//キーフレームの数。
		std::uint32_t numAnimationEvent;	//アニメーションイベントの数。
	};
	/// <summary>
	/// アニメーションイベント。
	/// </summary>
	struct AnimationEvent {
		float	invokeTime;					//アニメーションイベントが発生する時間(単位:秒)
		std::string eventName;
	};

	/// <summary>
	/// キーフレーム。
	/// </summary>
	struct KeyFrame {
		std::uint32_t boneIndex;	//ボーンインデックス。
		float time;					//時間。
		Vector3 transform[4];		//トランスフォーム。
	};
	/// <summary>
	/// 読み込み処理。
	/// </summary>
	/// <param name="filePath"></param>
	void Load(const char* filePath) ;
	/// <summary>
	/// アニメーションイベントにクエリを行う。
	/// </summary>
	/// <param name="query">クエリ関数</param>
	void QueryAnimationEvents(std::function<void(const AnimationEvent& animEvent) > query)
	{
		for (auto& animEvent : m_animationEvents) {
			query(animEvent);
		}
	}
	/// <summary>
	/// キーフレームにクエリを行う。
	/// </summary>
	/// <param name="query">クエリ関数。</param>
	void QueryKeyFrames(std::function<void(const KeyFrame& keyFrame)> query)
	{
		for (auto& keyFrame : m_keyFrames) {
			query(keyFrame);
		}
	}
	/// <summary>
	/// アニメーションイベントの数を取得。
	/// </summary>
	/// <returns></returns>
	int GetNumAnimationEvent() const
	{
		return static_cast<int>( m_animationEvents.size() );
	}
	/// <summary>
	/// キーフレームの数を取得。
	/// </summary>
	/// <returns></returns>
	int GetNumKeyFrame() const
	{
		return static_cast<int>(m_keyFrames.size());
	}
private:
	std::vector< AnimationEvent> m_animationEvents;	//アニメーションイベント。
	std::vector< KeyFrame> m_keyFrames;				//キーフレーム。
};

