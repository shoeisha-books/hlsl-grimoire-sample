#include "stdafx.h"
#include "ModelStandard.h"


void ModelStandard::Init(const char* tkmFilePath)
{
	ModelInitData initData;
	//スタンダードシェーダーを指定する。
	initData.m_fxFilePath = "Assets/shader/preset/modelStandard.fx";
	initData.m_tkmFilePath = tkmFilePath;
	initData.m_expandConstantBuffer = &m_light;
	initData.m_expandConstantBufferSize = sizeof(m_light);

	//光を強めに設定する。
	m_light.directionalLight[0].color.x = 2.0f;
	m_light.directionalLight[0].color.y = 2.0f;
	m_light.directionalLight[0].color.z = 2.0f;

	m_light.directionalLight[0].direction.x = 0.0f;
	m_light.directionalLight[0].direction.y = 0.0f;
	m_light.directionalLight[0].direction.z = -1.0f;
	m_light.directionalLight[0].direction.Normalize();

	m_light.ambinetLight.x = 0.5f;
	m_light.ambinetLight.y = 0.5f;
	m_light.ambinetLight.z = 0.5f;

	m_model.Init(initData);
}