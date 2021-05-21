
#pragma once

#include "math/Vector.h"

/// <summary>
/// 行列クラス。
/// </summary>
class Matrix {
public:

	union {
		DirectX::XMFLOAT4X4 mat;
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		Vector4 v[4];
		float m[4][4];
	};
	//単位行列
	static const Matrix Identity;
public:
	/// <summary>
	/// DirectX::XMMATRIX型への暗黙の型変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMMATRIX() const
	{
		return DirectX::XMLoadFloat4x4(&mat);
	}
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <remarks>
	/// 単位行列として初期化されます。
	/// </remarks>
	Matrix() {
		mat._11 = 1.0f;
		mat._12 = 0.0f;
		mat._13 = 0.0f;
		mat._14 = 0.0f;

		mat._21 = 0.0f;
		mat._22 = 1.0f;
		mat._23 = 0.0f;
		mat._24 = 0.0f;

		mat._31 = 0.0f;
		mat._32 = 0.0f;
		mat._33 = 1.0f;
		mat._34 = 0.0f;

		mat._41 = 0.0f;
		mat._42 = 0.0f;
		mat._43 = 0.0f;
		mat._44 = 1.0f;
	}
	Matrix(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) :
		mat(m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33)
	{

	}
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="m"></param>
	Matrix(const DirectX::XMFLOAT4X4& m)
	{
		mat = m;
	}
	/// <summary>
	/// 代入演算子。
	/// </summary>
	/// <param name="_m"></param>
	/// <returns></returns>
	Matrix& operator=(const Matrix& _m)
	{
		mat = _m.mat;
		return *this;
	}
	/// <summary>
	/// ベクトルと3x3行列を乗算。
	/// </summary>
	/// <param name="vOut">乗算されるベクトル。</param>
	void Apply3x3(Vector3& vOut) const
	{
		Vector3 vTmp = vOut;
		vOut.x = vTmp.x * m[0][0] + vTmp.y * m[1][0] + vTmp.z * m[2][0];
		vOut.y = vTmp.x * m[0][1] + vTmp.y * m[1][1] + vTmp.z * m[2][1];
		vOut.z = vTmp.x * m[0][2] + vTmp.y * m[1][2] + vTmp.z * m[2][2];
	}

	/// <summary>
	/// ベクトルに行列を乗算。
	/// </summary>
	/// <param name="vOut">乗算されるベクトル。</param>
	void Apply(Vector3& vOut) const
	{
		DirectX::XMStoreFloat3(
			&vOut.vec,
			DirectX::XMVector3Transform(vOut, *this)
		);
	}
	/// <summary>
	/// ベクトルに行列を乗算。
	/// </summary>
	/// <param name="vOut">乗算されるベクトル。</param>
	void Apply(Vector4& vOut) const
	{
		DirectX::XMStoreFloat4(
			&vOut.vec,
			DirectX::XMVector4Transform(vOut, *this)
		);
	}
	/// <summary>
	/// 平行移動行列を作成。
	/// </summary>
	/// <param name="trans">平行移動。</param>
	void MakeTranslation(const Vector3& trans)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixTranslationFromVector(trans)
		);
	}
	void MakeTranslation(float x, float y, float z)
	{
		MakeTranslation(Vector3(x, y, z));
	}
	/// <summary>
	/// Y軸周りの回転行列を作成。
	/// </summary>
	/// <param name="angle">回転角度(単位：ラジアン)</param>
	void MakeRotationY(float angle)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixRotationY(angle)
		);
	}
	/// <summary>
	/// Z軸周りの回転行列を作成。
	/// </summary>
	/// <param name="angle">回転角度(単位：ラジアン)</param>
	void MakeRotationZ(float angle)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixRotationZ(angle)
		);
	}
	/// <summary>
	/// X軸周りの回転行列を作成。
	/// </summary>
	/// <param name="angle">回転角度(単位：ラジアン)</param>
	void MakeRotationX(float angle)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixRotationX(angle)
		);
	}
	/// <summary>
	/// クォータニオンから回転行列を作成。
	/// </summary>
	/// <param name="q">クォータニオン。</param>
	void MakeRotationFromQuaternion(const Quaternion& q)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixRotationQuaternion(q)
		);
	}

	/// <summary>
	/// 任意の軸周りの回転行列を作成。
	/// </summary>
	/// <param name="axis">回転軸。</param>
	/// <param name="angle">回転角度。</param>
	void MakeRotationAxis(const Vector3& axis, float angle)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixRotationAxis(axis, angle)
		);
	}
	/// <summary>
	/// 拡大行列を作成。
	/// </summary>
	/// <param name="scale">拡大率。</param>
	void MakeScaling(const Vector3& scale)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixScalingFromVector(scale)
		);
	}
	/// <summary>
	/// 透視変換行列を作成。
	/// </summary>
	/// <param name="viewAngle">画角(単位：ラジアン)</param>
	/// <param name="aspect">アスペクト比</param>
	/// <param name="fNear">近平面。</param>
	/// <param name="fFar">遠平面。</param>
	void MakeProjectionMatrix(
		float viewAngle,
		float aspect,
		float fNear,
		float fFar
	)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixPerspectiveFovLH(viewAngle, aspect, fNear, fFar)
		);
	}
	/// <summary>
	/// 平行投影行列を作成。
	/// </summary>
	/// <param name="w">ビューボリュームの幅。</param>
	/// <param name="h">ビューボリュームの高さ。</param>
	/// <param name="fNear">近平面。</param>
	/// <param name="fFar">遠平面。</param>
	void MakeOrthoProjectionMatrix(float w, float h, float fNear, float fFar)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixOrthographicLH(w, h, fNear, fFar)
		);
	}
	/// <summary>
	/// カメラ行列を作成。
	/// </summary>
	/// <param name="position">視点</param>
	/// <param name="target">注視点</param>
	/// <param name="up">カメラの上方向</param>
	void MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& up)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixLookAtLH(position, target, up)
		);
	}
	/// <summary>
	/// 行列と行列の乗算
	/// </summary>
	/// <remarks>
	/// *this = m0 * m1;
	/// </remarks>
	void Multiply(const Matrix& m0, const Matrix& m1)
	{
		DirectX::XMFLOAT4X4 lm;
		DirectX::XMStoreFloat4x4(
			&lm,
			DirectX::XMMatrixMultiply(m0, m1)
		);
		mat = lm;
	}

	/// <summary>
	/// 行列の代入演算子
	/// </summary>
	/// <remarks>
	/// *this *= _m;
	/// return *this;
	/// </remarks>
	const Matrix& operator*=(const Matrix& _m)
	{
		Multiply(*this, _m);
		return *this;
	}
	/// <summary>
	/// 逆行列を計算。
	/// </summary>
	/// <param name="_m">元になる行列。</param>
	void Inverse(const Matrix& _m)
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixInverse(NULL, _m)
		);
	}
	/// <summary>
	/// 自身の逆行列を計算。
	/// </summary>
	void Inverse()
	{
		Inverse(*this);
	}
	/// <summary>
	/// 転置行列を計算。
	/// </summary>
	void Transpose()
	{
		DirectX::XMStoreFloat4x4(
			&mat,
			DirectX::XMMatrixTranspose(*this)
		);
	}
		
};
/// <summary>
/// 行列同士の乗算
/// </summary>
/// <remarks>
/// 乗算は左から右に向かってかかっていく。
/// </remarks>
static inline Matrix operator*(const Matrix& m1, const Matrix m2)
{
	Matrix mRet;
	mRet.Multiply(m1, m2);
	return mRet;
}
/// <summary>
/// 単位行列
/// </summary>
const Matrix g_matIdentity = {
	1.0f, 0.0f, 0.0f, 0.0f ,
		0.0f, 1.0f, 0.0f, 0.0f ,
		0.0f, 0.0f, 1.0f, 0.0f ,
		0.0f, 0.0f, 0.0f, 1.0f
};
