#include "stdafx.h"
#include "ConstantBuffer.h"

ConstantBuffer::~ConstantBuffer()
{
	//アンマーップ
	CD3DX12_RANGE readRange(0, 0);
	for (auto& cb : m_constantBuffer) {
		if (cb != nullptr) {
			cb->Unmap(0, &readRange);
			cb->Release();
		}
	}
}
void ConstantBuffer::Init(int size, void* srcData)
{
	m_size = size;

	//D3Dデバイスを取得。
	auto device = g_graphicsEngine->GetD3DDevice();


	//定数バッファは256バイトアライメントが要求されるので、256の倍数に切り上げる。
	m_allocSize = (size + 256) & 0xFFFFFF00;
	//定数バッファの作成。
	int bufferNo = 0;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto rDesc = CD3DX12_RESOURCE_DESC::Buffer(m_allocSize);
	for( auto& cb : m_constantBuffer ){
		device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&rDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&cb)
		);
		//定数バッファをCPUからアクセス可能な仮想アドレス空間にマッピングする。
		//マップ、アンマップのオーバーヘッドを軽減するためにはこのインスタンスが生きている間は行わない。
		{
			CD3DX12_RANGE readRange(0, 0);        //     intend to read from this resource on the CPU.
			cb->Map(0, &readRange, reinterpret_cast<void**>(&m_constBufferCPU[bufferNo]));
		}
		if (srcData != nullptr) {
			memcpy(m_constBufferCPU[bufferNo], srcData, m_size);
		}
		bufferNo++;
	}
	//利用可能にする。
	m_isValid = true;
}
void ConstantBuffer::RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
{
	//D3Dデバイスを取得。
	auto device = g_graphicsEngine->GetD3DDevice();
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = m_constantBuffer[bufferNo]->GetGPUVirtualAddress();
	desc.SizeInBytes = m_allocSize;
	device->CreateConstantBufferView(&desc, descriptorHandle);
}
void ConstantBuffer::RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	RegistConstantBufferView(descriptorHandle, backBufferIndex);
}
void ConstantBuffer::CopyToVRAM(void* data)
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	memcpy(m_constBufferCPU[backBufferIndex], data, m_size);
}
D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAddress()
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_constantBuffer[backBufferIndex]->GetGPUVirtualAddress();
}