/*!
* @brief	アニメータークラス。
*/
#include "stdafx.h"
#include "Animation.h"
#include "Model.h"


namespace {
	//DCCツールでのアニメーション上での1フレームの経過時間(単位は秒)
	const float DELTA_SEC_PER_FRAME_ON_DCC_TOOL = 1.0f / 30.0f;
}

void Animation::Init(
	Skeleton& skeleton, 
	AnimationClip* animClips,
	int numAnimClip)
{
	m_skeleton = &skeleton;
	for( int i = 0; i < numAnimClip; i++){
		m_animationClips.push_back(&animClips[i]);
	}
	//footstepボーンの番号を調べる。
	int footstepBoneNo = -1;
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {
		auto bone = m_skeleton->GetBone(boneNo);
		if (wcscmp(bone->GetName(), L"footstep") == 0) {
			//footstepボーンが見つかった。
			footstepBoneNo = boneNo;
			break;
		}
	}

	for (auto& ctr : m_animationPlayController) {
		ctr.Init(m_skeleton, footstepBoneNo);
	}

	Play(0);
	m_isInited = true;
}
/*!
	* @brief	ローカルポーズの更新。
	*/
void Animation::UpdateLocalPose(float deltaTime)
{
	m_interpolateTime += deltaTime;
	if (m_interpolateTime >= 1.0f) {
		//補間完了。
		//現在の最終アニメーションコントローラへのインデックスが開始インデックスになる。
		m_startAnimationPlayController = GetLastAnimationControllerIndex();
		m_numAnimationPlayController = 1;
		m_interpolateTime = 1.0f;
	}
	for (int i = 0; i < m_numAnimationPlayController-1; i++) {
		int index = GetAnimationControllerIndex(m_startAnimationPlayController, i );
		m_animationPlayController[index].Update(deltaTime, this);
	}
	//最後のポーズだけ進めていく。
	int lastIndex = GetLastAnimationControllerIndex();
	m_animationPlayController[lastIndex].Update(deltaTime, this);
		
}
Vector3 Animation::CalcFootstepDeltaValueInWorldSpace(Quaternion rotation, Vector3 scale) const
{

	auto footstepDeltaValueInWorldSpace = m_footstepDeltaValue;

	Matrix mBias = Matrix::Identity;
	mBias.MakeRotationX(Math::PI * -0.5f);
	mBias.Apply(footstepDeltaValueInWorldSpace);

	//フットステップの移動量を拡大する。
	footstepDeltaValueInWorldSpace.x *= scale.x;
	footstepDeltaValueInWorldSpace.y *= scale.y;
	footstepDeltaValueInWorldSpace.z *= scale.z;
	//フットステップの移動量を回す。
	rotation.Apply(footstepDeltaValueInWorldSpace);
	//フットステップの移動量をオイラー積分する。
	float t = m_deltaTimeOnUpdate / DELTA_SEC_PER_FRAME_ON_DCC_TOOL;
	footstepDeltaValueInWorldSpace *= t;
	return footstepDeltaValueInWorldSpace;
}
void Animation::UpdateGlobalPose()
{
	//グローバルポーズ計算用のメモリをスタックから確保。
	int numBone = m_skeleton->GetNumBones();
	Quaternion* qGlobalPose = (Quaternion*)alloca(sizeof(Quaternion) * numBone);
	Vector3* vGlobalPose = (Vector3*)alloca(sizeof(Vector3) * numBone);
	Vector3* vGlobalScale = (Vector3*)alloca(sizeof(Vector3) * numBone);
	m_footstepDeltaValue = g_vec3Zero;
	for (int i = 0; i < numBone; i++) {
		qGlobalPose[i] = Quaternion::Identity;
		vGlobalPose[i] = Vector3::Zero;
		vGlobalScale[i] = Vector3::One;
	}
	//グローバルポーズを計算していく。
	int startIndex = m_startAnimationPlayController;
	for (int i = 0; i < m_numAnimationPlayController; i++) {
		int index = GetAnimationControllerIndex(startIndex, i);
		float intepolateRate = m_animationPlayController[index].GetInterpolateRate();
		const auto& localBoneMatrix = m_animationPlayController[index].GetBoneLocalMatrix();
		auto deltaValueFootStep = m_animationPlayController[index].GetFootStepDeltaValueOnUpdate();
		//footstepの移動量の補完
		m_footstepDeltaValue.Lerp(intepolateRate, m_footstepDeltaValue, deltaValueFootStep);
		for (int boneNo = 0; boneNo < numBone; boneNo++) {
			//平行移動の補完
			Matrix m = localBoneMatrix[boneNo];
			vGlobalPose[boneNo].Lerp(
				intepolateRate, 
				vGlobalPose[boneNo], 
				*(Vector3*)m.m[3]
			);
			//平行移動成分を削除。
			m.m[3][0] = 0.0f;
			m.m[3][1] = 0.0f;
			m.m[3][2] = 0.0f;
				
			//拡大成分の補間。
			Vector3 vBoneScale;
			vBoneScale.x = (*(Vector3*)m.m[0]).Length();
			vBoneScale.y = (*(Vector3*)m.m[1]).Length();
			vBoneScale.z = (*(Vector3*)m.m[2]).Length();

			vGlobalScale[boneNo].Lerp(
				intepolateRate,
				vGlobalScale[boneNo],
				vBoneScale
			);
			//拡大成分を除去。
			m.m[0][0] /= vBoneScale.x;
			m.m[0][1] /= vBoneScale.x;
			m.m[0][2] /= vBoneScale.x;

			m.m[1][0] /= vBoneScale.y;
			m.m[1][1] /= vBoneScale.y;
			m.m[1][2] /= vBoneScale.y;

			m.m[2][0] /= vBoneScale.z;
			m.m[2][1] /= vBoneScale.z;
			m.m[2][2] /= vBoneScale.z;

			//回転の補完
			Quaternion qBone;
			qBone.SetRotation(m);
			qGlobalPose[boneNo].Slerp(intepolateRate, qGlobalPose[boneNo], qBone);		
		}
	}
	//グローバルポーズをスケルトンに反映させていく。
	for (int boneNo = 0; boneNo < numBone; boneNo++) {
			
		//拡大行列を作成。
		Matrix scaleMatrix;
		scaleMatrix.MakeScaling(vGlobalScale[boneNo]);
		//回転行列を作成。
		Matrix rotMatrix;
		rotMatrix.MakeRotationFromQuaternion(qGlobalPose[boneNo]);
		//平行移動行列を作成。
		Matrix transMat;
		transMat.MakeTranslation(vGlobalPose[boneNo]);

		//全部を合成して、ボーン行列を作成。
		Matrix boneMatrix;
		boneMatrix = scaleMatrix * rotMatrix;
		boneMatrix = boneMatrix * transMat;
			
		m_skeleton->SetBoneLocalMatrix(
			boneNo,
			boneMatrix
		);			
			
	}
		
	//最終アニメーション以外は補間完了していたら除去していく。
	int numAnimationPlayController = m_numAnimationPlayController;
	for (int i = 1; i < m_numAnimationPlayController; i++) {
		int index = GetAnimationControllerIndex(startIndex, i);
		if (m_animationPlayController[index].GetInterpolateRate() > 0.99999f) {
			//補間が終わっているのでアニメーションの開始位置を前にする。
			m_startAnimationPlayController = index;
			numAnimationPlayController = m_numAnimationPlayController - i;
		}
	}
	m_numAnimationPlayController = numAnimationPlayController;
}
	
void Animation::Progress(float deltaTime)
{
	if (m_numAnimationPlayController == 0) {
		return;
	}
	m_deltaTimeOnUpdate = deltaTime;
	//ローカルポーズの更新をやっていく。
	UpdateLocalPose(deltaTime);
		
	//グローバルポーズを計算していく。
	UpdateGlobalPose();
}
