#include "stdafx.h"
#include "StructuredBuffer.h"


StructuredBuffer::~StructuredBuffer()
{
	//アンマーップ
	CD3DX12_RANGE readRange(0, 0);
	for (auto& buffer : m_buffersOnGPU) {
		if (buffer) {
			buffer->Unmap(0, &readRange);
			buffer->Release();
		}
	}
}
void StructuredBuffer::Init(int sizeOfElement, int numElement, void* initData)
{
	m_sizeOfElement = sizeOfElement;
	m_numElement = numElement;
	auto device = g_graphicsEngine->GetD3DDevice();

	int bufferNo = 0;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto rDesc = CD3DX12_RESOURCE_DESC::Buffer(m_sizeOfElement * m_numElement);
	for (auto& buffer : m_buffersOnGPU) {
		auto hr = device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&rDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);

		
		//構造化バッファをCPUからアクセス可能な仮想アドレス空間にマッピングする。
		//マップ、アンマップのオーバーヘッドを軽減するためにはこのインスタンスが生きている間は行わない。
		{
			CD3DX12_RANGE readRange(0, 0);        //     intend to read from this resource on the CPU.
			buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_buffersOnCPU[bufferNo]));
		}
		if (initData != nullptr) {
			memcpy(m_buffersOnCPU[bufferNo], initData, m_sizeOfElement * m_numElement);
		}
		
		bufferNo++;
	}
	m_isInited = true;
}
void StructuredBuffer::Update(void* data)
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	memcpy(m_buffersOnCPU[backBufferIndex], data, m_numElement * m_sizeOfElement);
}
ID3D12Resource* StructuredBuffer::GetD3DResoruce() 
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_buffersOnGPU[backBufferIndex];
}
void StructuredBuffer::RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
{
	if (!m_isInited) {
		return;
	}
	auto device = g_graphicsEngine->GetD3DDevice();
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_numElement);
	srvDesc.Buffer.StructureByteStride = m_sizeOfElement;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	device->CreateShaderResourceView(
		m_buffersOnGPU[bufferNo],
		&srvDesc,
		descriptorHandle
	);
}
