
#include "stdafx.h"
#include "RaytracingWorld.h"
#include "Material.h"

#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)
namespace raytracing {

	void World::RegistGeometry(Model& model)
	{
		model.QueryMeshAndDescriptorHeap([&](const SMesh& mesh, const DescriptorHeap& ds) {

			for (int i = 0; i < mesh.m_materials.size(); i++) {
				const auto& vertexBufferView = mesh.m_vertexBuffer.GetView();
				const auto& indexBufferView = mesh.m_indexBufferArray[i]->GetView();
				D3D12_RAYTRACING_GEOMETRY_DESC desc;
				memset(&desc, 0, sizeof(desc));
				desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				desc.Triangles.Transform3x4 = 0;
				desc.Triangles.VertexBuffer.StartAddress = vertexBufferView.BufferLocation;
				desc.Triangles.VertexBuffer.StrideInBytes = vertexBufferView.StrideInBytes;
				desc.Triangles.VertexCount = vertexBufferView.SizeInBytes / vertexBufferView.StrideInBytes;
				desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
				desc.Triangles.IndexBuffer = indexBufferView.BufferLocation;
				desc.Triangles.IndexCount = mesh.m_indexBufferArray[i]->GetCount();
				desc.Triangles.IndexFormat = indexBufferView.Format;
				InstancePtr instance = std::make_unique<Instance>();
				instance->geometoryDesc = desc;
				instance->m_material = mesh.m_materials[i];
				instance->m_vertexBufferRWSB.Init(mesh.m_vertexBuffer, false);
				instance->m_indexBufferRWSB.Init(*mesh.m_indexBufferArray[i], false);

				m_instances.emplace_back(std::move(instance));
			}
		});
	}

	void World::CommitRegistGeometry(RenderContext& rc)
	{
		//BLASを構築。
		m_blasBuffer.Init(rc, m_instances);
		//TLASを構築。
		m_topLevelASBuffers.Init(rc, m_instances, m_blasBuffer.Get());
	}
}//namespace raytracing