#pragma once

inline ID3D12DescriptorHeap* DescriptorHeap::Get()const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_descriptorHeap[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetConstantBufferGpuDescriptorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_cbGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetShaderResourceGpuDescriptorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_srGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetUavResourceGpuDescriptorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_uavGpuDescriptorStart[backBufferIndex];
}
inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSamplerResourceGpuDescriptorStartHandle() const
{
	auto backBufferIndex = g_graphicsEngine->GetBackBufferIndex();
	return m_samplerGpuDescriptorStart[backBufferIndex];
}