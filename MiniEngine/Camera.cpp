/*!
* @brief	カメラ
*/
#include "stdafx.h"
#include "Camera.h"



void Camera::Update()
{
	//アスペクト比を計算する。
	m_aspect = (float)g_graphicsEngine->GetFrameBufferWidth() / (float)g_graphicsEngine->GetFrameBufferHeight();
	if(m_isNeedUpdateProjectionMatrix){
		if (m_updateProjMatrixFunc == enUpdateProjMatrixFunc_Perspective) {
			//透視変換行列を計算。
			m_projectionMatrix.MakeProjectionMatrix(
				m_viewAngle,
				m_aspect,
				m_near,
				m_far
			);
		}
		else {
			//平行投影行列を計算。
			m_projectionMatrix.MakeOrthoProjectionMatrix(m_width, m_height, m_near, m_far);
		}
	}
	//ビュー行列の算出
	m_viewMatrix.MakeLookAt( m_position, m_target, m_up );
	//ビュープロジェクション行列の作成。
	m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	//ビュー行列の逆行列を計算。
	m_viewMatrixInv.Inverse( m_viewMatrix );

	m_forward.Set(m_viewMatrixInv.m[2][0], m_viewMatrixInv.m[2][1], m_viewMatrixInv.m[2][2]);
	m_right.Set(m_viewMatrixInv.m[0][0], m_viewMatrixInv.m[0][1], m_viewMatrixInv.m[0][2]);
	//カメラの回転行列を取得。
	m_cameraRotation = m_viewMatrixInv;
	m_cameraRotation.m[3][0] = 0.0f;
	m_cameraRotation.m[3][1] = 0.0f;
	m_cameraRotation.m[3][2] = 0.0f;
	m_cameraRotation.m[3][3] = 1.0f;

	Vector3 toPos;
	toPos.Subtract(m_position, m_target);
	m_targetToPositionLen = toPos.Length();

	m_isDirty = false;
}
void Camera::CalcScreenPositionFromWorldPosition(Vector2& screenPos, const Vector3& worldPos) const
{
	float half_w = (float)g_graphicsEngine->GetFrameBufferWidth() * 0.5f;
	float half_h = (float)g_graphicsEngine->GetFrameBufferHeight() * 0.5f;
	Vector4 _screenPos;
	_screenPos.Set(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	m_viewProjectionMatrix.Apply(_screenPos);
	screenPos.x = (_screenPos.x / _screenPos.w)*half_w;
	screenPos.y = (_screenPos.y / _screenPos.w)*half_h;
}
void Camera::RotateOriginTarget(const Quaternion& qRot)
{
	Vector3 cameraPos = m_position;
	Vector3 cameraTarget = m_target;
	Vector3 toPos = cameraPos - cameraTarget;
	qRot.Apply(toPos);
	m_position = m_target + toPos;
	m_isDirty = true;
}
