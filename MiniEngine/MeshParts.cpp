#include "stdafx.h"
#include "MeshParts.h"
#include "Skeleton.h"
#include "Material.h"
#include "IndexBuffer.h"

MeshParts::~MeshParts()
{
	for (auto& mesh : m_meshs) {
		//インデックスバッファを削除。
		for (auto& ib : mesh->m_indexBufferArray) {
			delete ib;
		}
		//マテリアルを削除。
		for (auto& mat : mesh->m_materials) {
			delete mat;
		}
		//メッシュを削除。
		delete mesh;
	}
}
void MeshParts::InitFromTkmFile(
	const TkmFile& tkmFile, 
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntryPointFunc,
	const char* psEntryPointFunc,
	void* expandData,
	int expandDataSize,
	const std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV>& expandShaderResourceView,
	const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
	D3D12_FILTER samplerFilter
)
{
	m_meshs.resize(tkmFile.GetNumMesh());
	int meshNo = 0;
	int materianNo = 0;
	tkmFile.QueryMeshParts([&](const TkmFile::SMesh& mesh) {
		//tkmファイルのメッシュ情報からメッシュを作成する。
		CreateMeshFromTkmMesh(
			mesh, 
			meshNo,
			materianNo,
			fxFilePath, 
			vsEntryPointFunc, 
			vsSkinEntryPointFunc, 
			psEntryPointFunc, 
			colorBufferFormat,
			samplerFilter
		);
		meshNo++;
	});
	//共通定数バッファの作成。
	m_commonConstantBuffer.Init(sizeof(SConstantBuffer), nullptr);
	//ユーザー拡張用の定数バッファを作成。
	if (expandData) {
		m_expandConstantBuffer.Init(expandDataSize, nullptr);
		m_expandData = expandData;
	}
	for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++) {
		m_expandShaderResourceView[i] = expandShaderResourceView[i];
	}
	//ディスクリプタヒープを作成。
	CreateDescriptorHeaps();
}

void MeshParts::CreateDescriptorHeaps()
{
	
	//ディスクリプタヒープを構築していく。
	int srvNo = 0;
	int cbNo = 0;
	for (auto& mesh : m_meshs) {
		for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
			
			//ディスクリプタヒープにディスクリプタを登録していく。
			m_descriptorHeap.RegistShaderResource(srvNo, mesh->m_materials[matNo]->GetAlbedoMap());			//アルベドマップ。
			m_descriptorHeap.RegistShaderResource(srvNo+1, mesh->m_materials[matNo]->GetNormalMap());		//法線マップ。
			m_descriptorHeap.RegistShaderResource(srvNo+2, mesh->m_materials[matNo]->GetSpecularMap());		//スペキュラマップ。
			m_descriptorHeap.RegistShaderResource(srvNo+3, m_boneMatricesStructureBuffer);					//ボーンのストラクチャードバッファ。
			for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++) {
				if (m_expandShaderResourceView[i]) {
					m_descriptorHeap.RegistShaderResource(srvNo + EXPAND_SRV_REG__START_NO + i, *m_expandShaderResourceView[i]);
				}
			}
			srvNo += NUM_SRV_ONE_MATERIAL;
			m_descriptorHeap.RegistConstantBuffer(cbNo, m_commonConstantBuffer);
			if (m_expandConstantBuffer.IsValid()) {
				m_descriptorHeap.RegistConstantBuffer(cbNo + 1, m_expandConstantBuffer);
			}
			cbNo += NUM_CBV_ONE_MATERIAL;
		}
	}
	m_descriptorHeap.Commit();
}
void MeshParts::CreateMeshFromTkmMesh(
	const TkmFile::SMesh& tkmMesh, 
	int meshNo,
	int& materialNum,
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntryPointFunc,
	const char* psEntryPointFunc,
	const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
	D3D12_FILTER samplerFilter
)
{
	//1. 頂点バッファを作成。
	int numVertex = (int)tkmMesh.vertexBuffer.size();
	int vertexStride = sizeof(TkmFile::SVertex);
	auto mesh = new SMesh;
	mesh->skinFlags.reserve(tkmMesh.materials.size());
	mesh->m_vertexBuffer.Init(vertexStride * numVertex, vertexStride);
	mesh->m_vertexBuffer.Copy((void*)&tkmMesh.vertexBuffer[0]);

	auto SetSkinFlag = [&](int index) {
		if (tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f) {
			//スキンがある。
			mesh->skinFlags.push_back(1);
		}
		else {
			//スキンなし。
			mesh->skinFlags.push_back(0);
		}
	};
	//2. インデックスバッファを作成。
	if (!tkmMesh.indexBuffer16Array.empty()) {
		//インデックスのサイズが2byte
		mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer16Array.size());
		for (auto& tkIb : tkmMesh.indexBuffer16Array) {
			auto ib = new IndexBuffer;
			ib->Init(static_cast<int>(tkIb.indices.size()) * 2, 2);
			ib->Copy((uint16_t*)&tkIb.indices.at(0));
				
			//スキンがあるかどうかを設定する。
			SetSkinFlag(tkIb.indices[0]);

			mesh->m_indexBufferArray.push_back(ib);
		}
	}
	else {
		//インデックスのサイズが4byte
		mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer32Array.size());
		for (auto& tkIb : tkmMesh.indexBuffer32Array) {
			auto ib = new IndexBuffer;
			ib->Init(static_cast<int>(tkIb.indices.size()) * 4, 4);
			ib->Copy((uint32_t*)&tkIb.indices.at(0));

			//スキンがあるかどうかを設定する。
			SetSkinFlag(tkIb.indices[0]);

			mesh->m_indexBufferArray.push_back(ib);
		}
	}
	//3. マテリアルを作成。
	mesh->m_materials.reserve(tkmMesh.materials.size());
	for (auto& tkmMat : tkmMesh.materials) {
		auto mat = new Material;
		mat->InitFromTkmMaterila(
			tkmMat, 
			fxFilePath, 
			vsEntryPointFunc, 
			vsSkinEntryPointFunc, 
			psEntryPointFunc, 
			colorBufferFormat,
			NUM_SRV_ONE_MATERIAL,
			NUM_CBV_ONE_MATERIAL,
			NUM_CBV_ONE_MATERIAL * materialNum,
			NUM_SRV_ONE_MATERIAL * materialNum,
			samplerFilter
		);
		//作成したマテリアル数をカウントする。
		materialNum++;
		mesh->m_materials.push_back(mat);
	}

	m_meshs[meshNo] = mesh;
	
}

void MeshParts::BindSkeleton(Skeleton& skeleton)
{
	m_skeleton = &skeleton;
	//構造化バッファを作成する。
	m_boneMatricesStructureBuffer.Init(
		sizeof(Matrix),
		m_skeleton->GetNumBones(),
		m_skeleton->GetBoneMatricesTopAddress()
	);
}
void MeshParts::DrawCommon(RenderContext& rc, const Matrix& mWorld, const Matrix& mView, const Matrix& mProj)
{
	//メッシュごとにドロー
	//プリミティブのトポロジーはトライアングルリストのみ。
	rc.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//定数バッファを更新する。
	SConstantBuffer cb;
	cb.mWorld = mWorld;
	cb.mView = mView;
	cb.mProj = mProj;
	m_commonConstantBuffer.CopyToVRAM(cb);

	if (m_expandData) {
		m_expandConstantBuffer.CopyToVRAM(m_expandData);
	}
	if (m_boneMatricesStructureBuffer.IsInited()) {
		//ボーン行列を更新する。
		m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
	}
}
void MeshParts::Draw(
	RenderContext& rc,
	const Matrix& mWorld,
	const Matrix& mView,
	const Matrix& mProj
)
{
	//定数バッファの設定、更新など描画の共通処理を実行する。
	DrawCommon(rc, mWorld, mView, mProj);
	
	int descriptorHeapNo = 0;
	for (auto& mesh : m_meshs) {
		//1. 頂点バッファを設定。
		rc.SetVertexBuffer(mesh->m_vertexBuffer);
		//マテリアルごとにドロー。
		for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
			//このマテリアルが貼られているメッシュの描画開始。
			mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);
			//2. ディスクリプタヒープを設定。
			rc.SetDescriptorHeap(m_descriptorHeap);
			//3. インデックスバッファを設定。
			auto& ib = mesh->m_indexBufferArray[matNo];
			rc.SetIndexBuffer(*ib);

			//4. ドローコールを実行。
			rc.DrawIndexed(ib->GetCount());
			descriptorHeapNo++;
		}
	}
}
void MeshParts::DrawInstancing(RenderContext& rc, int numInstance, const Matrix& mView, const Matrix& mProj)
{
	//定数バッファの設定、更新など描画の共通処理を実行する。
	DrawCommon(rc, g_matIdentity, mView, mProj);

	int descriptorHeapNo = 0;
	for (auto& mesh : m_meshs) {
		//1. 頂点バッファを設定。
		rc.SetVertexBuffer(mesh->m_vertexBuffer);
		//マテリアルごとにドロー。
		for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
			//このマテリアルが貼られているメッシュの描画開始。
			mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);
			//2. ディスクリプタヒープを設定。
			rc.SetDescriptorHeap(m_descriptorHeap);
			//3. インデックスバッファを設定。
			auto& ib = mesh->m_indexBufferArray[matNo];
			rc.SetIndexBuffer(*ib);

			//4. ドローコールを実行。
			rc.DrawIndexedInstanced(ib->GetCount(), numInstance);
			descriptorHeapNo++;
		}
	}

}
