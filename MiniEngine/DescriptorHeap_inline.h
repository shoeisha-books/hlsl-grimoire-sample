#pragma once

inline ID3D12DescriptorHeap* DescriptorHeap::Get()const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_descriptorHeap[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetConstantBufferGpuDescritorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_cbGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetShaderResourceGpuDescritorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_srGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetUavResourceGpuDescritorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_uavGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSamplerResourceGpuDescritorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_samplerGpuDescriptorStart[backBufferIndex];
}