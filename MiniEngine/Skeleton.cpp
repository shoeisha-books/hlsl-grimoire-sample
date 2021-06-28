/*!
*@brief	スケルトン
*/
#include "stdafx.h"
#include "Skeleton.h"
#include <comdef.h> 


void Bone::CalcWorldTRS(Vector3& trans, Quaternion& rot, Vector3& scale)
{
	Matrix mWorld = m_worldMatrix;
	//行列から拡大率を取得する。
	scale.x = mWorld.v[0].Length();
	scale.y = mWorld.v[1].Length();
	scale.z = mWorld.v[2].Length();
	m_scale = scale;
	//行列から平行移動量を取得する。
	trans.Set(mWorld.v[3]);
	m_positoin = trans;
	//行列から拡大率と平行移動量を除去して回転量を取得する。
	mWorld.v[0].Normalize();
	mWorld.v[1].Normalize();
	mWorld.v[2].Normalize();
	mWorld.v[3].Set(0.0f, 0.0f, 0.0f, 1.0f);
	rot.SetRotation(mWorld);
	m_rotation = rot;
}
Skeleton::Skeleton()
{
	//リザーブ。
	m_bones.reserve(BONE_MAX);
}
Skeleton::~Skeleton()
{
}
void Skeleton::UpdateBoneWorldMatrix(Bone& bone, const Matrix& parentMatrix)
{
	Matrix mBoneWorld;
	Matrix localMatrix = bone.GetLocalMatrix();
	mBoneWorld = localMatrix * parentMatrix;
	
	bone.SetWorldMatrix(mBoneWorld);
	for (auto childBone : bone.GetChildren()) {
		UpdateBoneWorldMatrix(*childBone, mBoneWorld);
	}
}
bool Skeleton::Init(const char* tksFilePath)
{
	//tksファイルをロードする。
	if (m_tksFile.Load(tksFilePath)) {
		//ボーン行列を構築する。
		BuildBoneMatrices();
		return true;
	}
	return false;
}
void Skeleton::BuildBoneMatrices()
{
	m_tksFile.QueryBone([&](TksFile::SBone & tksBone) {
		//バインドポーズ。
		Matrix bindPoseMatrix;
		memcpy(bindPoseMatrix.m[0], &tksBone.bindPose[0], sizeof(tksBone.bindPose[0]));
		memcpy(bindPoseMatrix.m[1], &tksBone.bindPose[1], sizeof(tksBone.bindPose[1]));
		memcpy(bindPoseMatrix.m[2], &tksBone.bindPose[2], sizeof(tksBone.bindPose[2]));
		memcpy(bindPoseMatrix.m[3], &tksBone.bindPose[3], sizeof(tksBone.bindPose[3]));
		bindPoseMatrix.m[0][3] = 0.0f;
		bindPoseMatrix.m[1][3] = 0.0f;
		bindPoseMatrix.m[2][3] = 0.0f;
		bindPoseMatrix.m[3][3] = 1.0f;

		//バインドポーズの逆行列。
		Matrix invBindPoseMatrix;
		memcpy(invBindPoseMatrix.m[0], &tksBone.invBindPose[0], sizeof(tksBone.invBindPose[0]));
		memcpy(invBindPoseMatrix.m[1], &tksBone.invBindPose[1], sizeof(tksBone.invBindPose[1]));
		memcpy(invBindPoseMatrix.m[2], &tksBone.invBindPose[2], sizeof(tksBone.invBindPose[2]));
		memcpy(invBindPoseMatrix.m[3], &tksBone.invBindPose[3], sizeof(tksBone.invBindPose[3]));
		invBindPoseMatrix.m[0][3] = 0.0f;
		invBindPoseMatrix.m[1][3] = 0.0f;
		invBindPoseMatrix.m[2][3] = 0.0f;
		invBindPoseMatrix.m[3][3] = 1.0f;

		wchar_t boneName[256];
		mbstowcs(boneName, tksBone.name.get(), 256);
		BonePtr bone = std::make_unique<Bone>(
			boneName,
			bindPoseMatrix,
			invBindPoseMatrix,
			tksBone.parentNo,
			tksBone.no
			);
#if BUILD_LEVEL != BUILD_LEVEL_MASTER
		//ボーンのバリデーションチェック。
		//maxScriptでやりたいところではあるが、とりあえずこっち。
		auto it = std::find_if(m_bones.begin(), m_bones.end(), [&](auto & bone) {return wcscmp(boneName, bone->GetName()) == 0;  });
		if (it != m_bones.end()) {
			//同名のボーンが見つかった。
			_bstr_t b(boneName);
			const char* c = b;
			TK_WARNING("同名のボーンが見つかりました。未定の動作です。データを修正してください。%s", c);
		}
#endif
		m_bones.push_back(std::move(bone));
		});
	for (auto& bone : m_bones) {
		if (bone->GetParentBoneNo() != -1) {
			m_bones.at(bone->GetParentBoneNo())->AddChild(bone.get());
			//ローカルマトリクスを計算。
			const Matrix& parentMatrix = m_bones.at(bone->GetParentBoneNo())->GetInvBindPoseMatrix();
			Matrix localMatrix;
			localMatrix = bone->GetBindPoseMatrix() * parentMatrix;
			bone->SetLocalMatrix(localMatrix);
		}
		else {
			bone->SetLocalMatrix(bone->GetBindPoseMatrix());
		}
	}


	//ボーン行列を確保
	m_boneMatrixs = std::make_unique<Matrix[]>(m_bones.size());
	m_isInited = true;

}

void Skeleton::Update(const Matrix& mWorld)
{
	if (m_isPlayAnimation) {
		//ボーン行列をルートボーンの空間からワールド空間を構築していく。
		for (auto& bone : m_bones) {
			Matrix mBoneWorld;
			Matrix localMatrix = bone->GetLocalMatrix();
			//親の行列とローカル行列を乗算して、ワールド行列を計算する。
			mBoneWorld = localMatrix * mWorld;
			bone->SetWorldMatrix(mBoneWorld);
		}
	}
	else {
		//アニメーションが流し込まれていると、ボーン行列がルートボーン空間に
		//変換されているが、流されていないと親の骨の座標系のままなので、
		//ルートボーン空間→ワールド空間への変換を行う。
		for (auto& bone : m_bones) {
			if (bone->GetParentBoneNo() != -1) {
				continue;
			}
			//ルート。
			UpdateBoneWorldMatrix(*bone, mWorld);
		}
	}

	//ボーン行列を計算。
	int boneNo = 0;
	for (auto& bonePtr : m_bones) {
		Matrix mBone;
		mBone = bonePtr->GetInvBindPoseMatrix() * bonePtr->GetWorldMatrix();
		m_boneMatrixs[boneNo] = mBone;
		boneNo++;
	}	
}
