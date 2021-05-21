#pragma once

#include "tkFile/TksFile.h"

/// <summary>
/// ボーン。
/// </summary>
class Bone {
public:
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <param name="boneName">ボーンの名前</param>
	/// <param name="bindPose">バインドポーズの行列</param>
	/// <param name="invBindPose">バインドポーズの逆行列</param>
	/// <param name="parentBoneNo">親のボーン番号</param>
	/// <param name="boneId">ボーン番号</param>
	Bone(
		const wchar_t* boneName,
		const Matrix& bindPose,
		const Matrix& invBindPose,
		int parentBoneNo,
		int boneId
	) :
		m_boneName(boneName),
		m_bindPose(bindPose),
		m_invBindPose(invBindPose),
		m_parentBoneNo(parentBoneNo),
		m_worldMatrix(bindPose),
		m_boneId(boneId)
	{
	}
	/// <summary>
	/// ローカル行列(親の座標系での行列)を設定。
	/// </summary>
	void SetLocalMatrix(const Matrix& m)
	{
		m_localMatrix = m;
	}
	/// <summary>
	/// ローカル行列(親の座標系での行列)を取得。
	/// </summary>
	const Matrix& GetLocalMatrix() const
	{
		return m_localMatrix;
	}
	/// <summary>
	/// ワールド行列を設定。
	/// </summary>
	void SetWorldMatrix(const Matrix& m)
	{
		m_worldMatrix = m;
	}
	/// <summary>
	/// ワールド行列を取得。
	/// </summary>
	const Matrix& GetWorldMatrix() const
	{
		return m_worldMatrix;
	}
	/// <summary>
	/// バインドポーズの行列を取得。
	/// </summary>
	const Matrix& GetBindPoseMatrix() const
	{
		return m_bindPose;
	}
	/// <summary>
	/// バインドポーズの逆行列を取得。
	/// </summary>
	const Matrix& GetInvBindPoseMatrix() const
	{
		return m_invBindPose;
	}
	/// <summary>
	/// 親のボーン番号を取得。
	/// </summary>
	/// <returns></returns>
	int GetParentBoneNo() const
	{
		return m_parentBoneNo;
	}
	/// <summary>
	/// ボーン番号を取得。
	/// </summary>
	/// <returns></returns>
	int GetNo() const
	{
		return m_boneId;
	}

	/*!
	*@brief	子供を追加。
	*/
	void AddChild(Bone* bone) 
	{
		m_children.push_back(bone);
	}
	/*!
	*@brief	子供を取得。
	*/
	std::list<Bone*>& GetChildren()
	{
		return m_children;
	}
	const Matrix& GetOffsetLocalMatrix() const
	{
		return m_offsetLocalMatrix;
	}
	/*!
		*@brief	名前の取得。
		*/
	const wchar_t* GetName() const
	{
		return m_boneName.c_str();
	}
	/*!
	*@brief	このボーンのワールド空間での位置と回転とスケールを計算する。
	*@param[out]	trans		平行移動量の格納先。
	*@param[out]	rot			回転量の格納先。
	*@param[out]	scale		拡大率の格納先。
	*/
	void CalcWorldTRS(Vector3& trans, Quaternion& rot, Vector3& scale);
		
private:
	
	std::wstring	m_boneName;
	int				m_parentBoneNo = -1;	//親のボーン番号。
	int				m_boneId = -1;			//ボーン番号。
	Matrix			m_bindPose;				//バインドポーズ。
	Matrix			m_invBindPose;			//バインドポーズの逆行列。
	Matrix			m_localMatrix;			//ローカル行列。
	Matrix			m_worldMatrix;			//ワールド行列。
	Matrix			m_offsetLocalMatrix;
	Vector3			m_positoin;				//このボーンのワールド空間での位置。最後にCalcWorldTRSを実行したときの結果が格納されている。
	Vector3			m_scale;				//このボーンの拡大率。最後にCalcWorldTRSを実行したときの結果が格納されている。
	Quaternion		m_rotation;				//このボーンの回転。最後にCalcWorldTRSを実行したときの結果が格納されている。
	std::list<Bone*>	m_children;			//子供のリスト。
};

/// <summary>
/// スケルトンクラス。
/// </summary>
class Skeleton  {
public:
	using OnPostProcessSkeletonUpdate = std::function<void()>;

	Skeleton();
	~Skeleton();
		
	/// <summary>
	/// ボーンのローカル行列を設定。
	/// </summary>
	/// <param name="boneNo">ボーン番号</param>
	/// <param name="m">行列</param>
	void SetBoneLocalMatrix(int boneNo, const Matrix& m)
	{
		if (boneNo > (int)m_bones.size() - 1) {
			MessageBoxA(nullptr, "boneNo is over m_bones.size() ", "error", MB_OK);
			return;
		}
		m_bones[boneNo]->SetLocalMatrix(m);
	}

	/// <summary>
	/// ボーンの数を取得。
	/// </summary>
	/// <returns></returns>
	int GetNumBones() const
	{
		return static_cast<int>(m_bones.size());
	}
	/// <summary>
	/// 初期化。
	/// </summary>
	/// <param name="tksFilePath">tksファイルのファイルパス</param>
	/// <returns>trueが返ってきたらロード成功。</returns>
	bool Init(const char* tksFilePath);

	/// <summary>
	/// 初期化済みか判定。
	/// </summary>
	/// <returns></returns>
	bool IsInited() const
	{
		return m_isInited;
	}
	/// <summary>
	/// ボーン行列の構築。
	/// 読み込みが完了した後で呼び出してください。
	/// </summary>
	void BuildBoneMatrices();
	/// <summary>
	/// ボーンの名前からボーン番号を検索。
	/// </summary>
	/// <param name="boneName">ボーンの名前</param>
	/// <returns>ボーン番号。見つからなかった場合は-1が返ってきます。</returns>
	int FindBoneID(const wchar_t* boneName) const
	{
		for (int i = 0; i < (int)m_bones.size(); i++) {
			if (wcscmp(m_bones[i]->GetName(), boneName) == 0) {
				return i;
			}
		}
		//見つからなかった。
		return -1;
	}
	/// <summary>
	/// ボーン番号からボーンを取得。
	/// </summary>
	/// <param name="boneNo">ボーン番号</param>
	/// <returns>ボーン</returns>
	Bone* GetBone(int boneNo) const
	{
		return m_bones[boneNo].get();
	}
	/// <summary>
	/// ボーン行列の先頭アドレスを取得。
	/// </summary>
	/// <returns></returns>
	Matrix* GetBoneMatricesTopAddress() const
	{
		return m_boneMatrixs.get();
	}
	/// <summary>
	/// アニメーションが再生されているマークを付ける。
	/// </summary>
	void SetMarkPlayAnimation()
	{
		m_isPlayAnimation = true;
	}
public:
		

	/// <summary>
	/// 更新。
	/// </summary>
	/// <param name="mWorld"></param>
	void Update(const Matrix& mWorld);
	
	/// <summary>
	/// ボーンのワールド行列の更新関数。
	/// </summary>
	/// <remarks>
	/// 通常はユーザーがこの関数を呼び出す必要はありません。
	/// </remarks>
	/// <param name="bone">更新するボーン。</param>
	/// <param name="parentMatrix">親のボーンのワールド行列。</param>
	static 	void UpdateBoneWorldMatrix(Bone& bone, const Matrix& parentMatrix);

private:
	TksFile m_tksFile;										//TKSファイル。
	static const int BONE_MAX = 512;				//ボーンの最大数。
	using BonePtr = std::unique_ptr<Bone>;
	std::vector<BonePtr>	m_bones;				//ボーンの配列。
	std::unique_ptr<Matrix[]>	m_boneMatrixs;	//ボーン行列。
	bool m_isInited = false;								//初期化済み？
	bool m_isPlayAnimation = false;					//アニメーションが流し込まれている？
};