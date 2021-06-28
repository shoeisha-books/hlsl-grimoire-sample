#include "stdafx.h"
#include "tkFile/TksFile.h"

bool TksFile::Load(const char* filePath)
{
	auto fp = fopen(filePath, "rb");
	if (fp == nullptr) {
		return false;
	}
	//骨の数を取得。
	fread(&m_numBone, sizeof(m_numBone), 1, fp);
	m_bones.resize(m_numBone);
	for (int i = 0; i < m_numBone; i++) {
		auto& bone = m_bones.at(i);
		size_t nameCount = 0;
		//骨の名前を取得。
		fread(&nameCount, 1, 1, fp);
		bone.name = std::make_unique<char[]>(nameCount + 1);
		fread(bone.name.get(), nameCount + 1, 1, fp);
		//親のIDを取得。
		fread(&bone.parentNo, sizeof(bone.parentNo), 1, fp);
		//バインドポーズを取得。
		fread(bone.bindPose, sizeof(bone.bindPose), 1, fp);
		//バインドポーズの逆数を取得。
		fread(bone.invBindPose, sizeof(bone.invBindPose), 1, fp);
		//ボーンの番号。
		bone.no = i;
	}

	fclose(fp);
	return true;
}
