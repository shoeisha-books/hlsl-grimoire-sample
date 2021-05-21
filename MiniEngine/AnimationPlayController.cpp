/*!
* @brief	アニメーション再生コントローラ。
*/

#include "stdafx.h"
#include "AnimationPlayController.h"
#include "AnimationClip.h"
#include "Animation.h"
#include "Skeleton.h"

void AnimationPlayController::Init(Skeleton* skeleton, int footStepBoneNo)
{
	m_footstepBoneNo = footStepBoneNo;
	int numBones = skeleton->GetNumBones();
	//ボーン行列をバシッと確保。
	m_boneMatrix.resize(numBones);
	m_skeleton = skeleton;
}
void AnimationPlayController::ChangeAnimationClip(AnimationClip* clip)
{
	m_animationClip = clip;
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;
	m_isPlaying = true;
	m_footstepPos = g_vec3Zero;
	m_footstepDeltaValue = g_vec3Zero;
	//アニメーションイベントの発生フラグをすべてfalseにする。
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		animEventArray[i].SetInvokedFlag(false);
	}
}
void AnimationPlayController::InvokeAnimationEvent(Animation* animation)
{
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		if (m_time > animEventArray[i].GetInvokeTime()
			&& animEventArray[i].IsInvoked() == false) {
			//アニメーションの起動時間を過ぎている且つ、まだイベント起動していない。
			animation->NotifyAnimationEventToListener(
				m_animationClip->GetName(), animEventArray[i].GetEventName()
			);
			animEventArray[i].SetInvokedFlag(true);
		}
	}
}

void AnimationPlayController::StartLoop()
{
	m_footstepPos = g_vec3Zero;
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;
	//アニメーションイベントをすべて未発生にする。
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		animEventArray[i].SetInvokedFlag(false);
	}
}
void AnimationPlayController::CalcBoneMatrixInRootBoneSpace(Bone& bone, Matrix parentMatrix)
{
	//ワールド行列を計算する。
	auto& mBoneInRootSpace = m_boneMatrix[bone.GetNo()];
	Matrix localMatrix = m_boneMatrix[bone.GetNo()];
	//親の行列とローカル行列を乗算して、ワールド行列を計算する。
	mBoneInRootSpace = localMatrix * parentMatrix;

	//子供のワールド行列も計算する。
	for (auto& childBone : bone.GetChildren()) {
		CalcBoneMatrixInRootBoneSpace(*childBone, mBoneInRootSpace);
	}
}
void AnimationPlayController::SamplingBoneMatrixFromAnimationClip()
{
	const auto& keyFramePtrListArray = m_animationClip->GetKeyFramePtrListArray();
	for (const auto& keyFrameList : keyFramePtrListArray) {
		if (keyFrameList.size() == 0) {
			continue;
		}
		//現在再生中のキーフレームを取ってくる。
		KeyFrame* keyframe = keyFrameList.at(m_currentKeyFrameNo);
		m_boneMatrix[keyframe->boneIndex] = keyframe->transform;
	}
}
void AnimationPlayController::CalcBoneMatrixInRootBoneSpace()
{
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {
		//ルートの骨を検索する。
		auto bone = m_skeleton->GetBone(boneNo);
		if (bone->GetParentBoneNo() != -1) {
			continue;
		}
		CalcBoneMatrixInRootBoneSpace(*bone, g_matIdentity);
	}
}
void AnimationPlayController::SamplingDeltaValueFootstepBone()
{
	if (m_currentKeyFrameNoLastFrame == m_currentKeyFrameNo) {
		//キーフレームが進んでいない。
		return;
	}
	if (m_footstepBoneNo == -1) {
		return;
	}
	int numBone = m_skeleton->GetNumBones();

	for (int boneNo = 0; boneNo < numBone; boneNo++) {
		auto bone = m_skeleton->GetBone(boneNo);
		if (m_footstepBoneNo == bone->GetNo()) {
			auto mat = m_boneMatrix[bone->GetNo()];
			Vector3 footstepBonePos;
			footstepBonePos.x = mat.m[3][0];
			footstepBonePos.y = mat.m[3][1];
			footstepBonePos.z = mat.m[3][2];
			//このフレームでのfootstepの移動量を計算する。
			m_footstepDeltaValue = footstepBonePos - m_footstepPos;
			//このフレームでのfootstepの座標を更新する。
			m_footstepPos = footstepBonePos;
			break;
		}
	}
}
void AnimationPlayController::SubtractFootstepbonePosFromAllBone()
{
	if (m_footstepBoneNo == -1) {
		return;
	}
	int numBone = m_skeleton->GetNumBones();

	for (int boneNo = 0; boneNo < numBone; boneNo++) {
		auto bone = m_skeleton->GetBone(boneNo);
		m_boneMatrix[bone->GetNo()].m[3][0] -= m_footstepPos.x;
		m_boneMatrix[bone->GetNo()].m[3][1] -= m_footstepPos.y;
		m_boneMatrix[bone->GetNo()].m[3][2] -= m_footstepPos.z;
	}
}
void AnimationPlayController::ProgressKeyframeNo(float deltaTime)
{
	//1フレーム前のキーフレーム番号を記録しておく。
	m_currentKeyFrameNoLastFrame = m_currentKeyFrameNo;

	const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();

	//補完時間も進めていく。
	m_interpolateTime = min(1.0f, m_interpolateTime + deltaTime);
	while (true) {
		if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
			//終端まで行った。
			if (m_animationClip->IsLoop()) {
				//ループ。
				StartLoop();
			}
			else {
				//ワンショット再生。
				m_currentKeyFrameNo--;
				m_isPlaying = false;	//再生終わり。
			}
			break;
		}
		if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
			//終わり。
			break;
		}
		//次へ。
		m_currentKeyFrameNo++;
	}
}
void AnimationPlayController::Update(float deltaTime, Animation* animation)
{
	if(m_animationClip == nullptr){
		return ;
	}

	m_time += deltaTime;
		
	//アニメーションイベントの発生。
	InvokeAnimationEvent(animation);
	//キーフレーム番号を進める。
	ProgressKeyframeNo(deltaTime);
		
	//ボーン行列をアニメーションクリップからサンプリングしていく。
	SamplingBoneMatrixFromAnimationClip();
		
	//親の骨座標系になっているボーン行列をルートのボーンの空間に変換していく。
	CalcBoneMatrixInRootBoneSpace();
		
	//footstepボーンの移動量を取得する。
	SamplingDeltaValueFootstepBone();
		
	//footstepボーンの移動量を全体の骨から減算する。
	SubtractFootstepbonePosFromAllBone();

	//アニメーション再生した印をスケルトンにつける。
	m_skeleton->SetMarkPlayAnimation();
}
	
