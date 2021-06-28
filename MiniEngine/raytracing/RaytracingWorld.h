#pragma once

#include "DescriptorHeap.h"
#include "RaytracingInstance.h"
#include "TLASBuffer.h"
#include "BLASBuffer.h"

class Model;
class RenderContext;

namespace raytracing {

	/// <summary>
	/// レイトレワールド
	/// </summary>
	class World
	{
	public:
		/// <summary>
		/// ジオメトリを登録。
		/// </summary>
		/// <param name="model">モデル</param>
		void RegistGeometry(Model& model);
		/// <summary>
		/// ジオメトリの登録を確定。
		/// </summary>
		void CommitRegistGeometry(RenderContext& rc);
		/// <summary>
		/// レイトレワールドのインスタンスに対してクエリを行う。
		/// </summary>
		/// <param name="queryFunc"></param>
		void QueryInstances(std::function<void(Instance&)> queryFunc) const
		{
			for (auto& instance : m_instances) {
				queryFunc(*instance);
			}
		}
		/// <summary>
		/// インスタンスの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return static_cast<int>(m_instances.size());
		}
		/// <summary>
		/// BLASバッファーを取得。
		/// </summary>
		/// <returns></returns>
		const BLASBuffer& GetBLASBuffer()
		{
			return m_blasBuffer;
		}
		/// <summary>
		/// TLASバッファーを取得。
		/// </summary>
		/// <returns></returns>
		TLASBuffer& GetTLASBuffer() 
		{
			return m_topLevelASBuffers;
		}
	private:
		/// <summary>
		/// カメラ
		/// </summary>
		struct Camera {
			Matrix mRot;	//回転行列
			Vector3 pos;	//視点。
			float aspect;	//アスペクト比。
			float fFar;		//遠平面。
			float fNear;	//近平面。
		};
		Camera m_camera;									//レイトレワールドのカメラ。
		std::vector<InstancePtr> m_instances;		//レイトレワールドのインスタンの配列。
		BLASBuffer m_blasBuffer;							//BLAS
		TLASBuffer m_topLevelASBuffers;						//TLAS
	};
}//namespace raytracing