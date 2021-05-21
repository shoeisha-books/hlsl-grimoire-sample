#pragma once

inline void RenderContext::SetDescriptorHeap(DescriptorHeap& descHeap)
{
	m_descriptorHeaps[0] = descHeap.Get();
	m_commandList->SetDescriptorHeaps(1, m_descriptorHeaps);

	//ディスクリプタテーブルに登録する。
	if (descHeap.IsRegistConstantBuffer()) {
		SetGraphicsRootDescriptorTable(0, descHeap.GetConstantBufferGpuDescritorStartHandle());
	}
	if (descHeap.IsRegistShaderResource()) {
		SetGraphicsRootDescriptorTable(1, descHeap.GetShaderResourceGpuDescritorStartHandle());
	}
	if (descHeap.IsRegistUavResource()) {
		SetGraphicsRootDescriptorTable(2, descHeap.GetUavResourceGpuDescritorStartHandle());
	}
}
inline void RenderContext::SetComputeDescriptorHeap(DescriptorHeap& descHeap)
{
	m_descriptorHeaps[0] = descHeap.Get();
	m_commandList->SetDescriptorHeaps(1, m_descriptorHeaps);

	//ディスクリプタテーブルに登録する。
	if (descHeap.IsRegistConstantBuffer()) {
		SetComputeRootDescriptorTable(0, descHeap.GetConstantBufferGpuDescritorStartHandle());
	}
	if (descHeap.IsRegistShaderResource()) {
		SetComputeRootDescriptorTable(1, descHeap.GetShaderResourceGpuDescritorStartHandle());
	}
	if (descHeap.IsRegistUavResource()) {
		SetComputeRootDescriptorTable(2, descHeap.GetUavResourceGpuDescritorStartHandle());
	}
}
inline void RenderContext::WaitUntilFinishDrawingToRenderTarget(RenderTarget& renderTarget)
{
	WaitUntilFinishDrawingToRenderTarget(renderTarget.GetRenderTargetTexture().Get());
}
inline void RenderContext::WaitUntilToPossibleSetRenderTarget(RenderTarget& renderTarget)
{
	WaitUntilToPossibleSetRenderTarget(renderTarget.GetRenderTargetTexture().Get());
}
inline void RenderContext::WaitUntilToPossibleSetRenderTargets(int numRt, RenderTarget* renderTargets[])
{
	for (int i = 0; i < numRt; i++) {
		WaitUntilToPossibleSetRenderTarget(*renderTargets[i]);
	}
}
inline void RenderContext::WaitUntilFinishDrawingToRenderTargets(int numRt, RenderTarget* renderTargets[])
{
	for (int i = 0; i < numRt; i++) {
		WaitUntilFinishDrawingToRenderTarget(*renderTargets[i]);
	}
}
inline void RenderContext::SetRenderTargets(UINT numRT, RenderTarget* renderTargets[])
{
	//d
	D3D12_CPU_DESCRIPTOR_HANDLE rtDSHandleTbl[32];
	int rtNo = 0;
	for( UINT rtNo = 0; rtNo < numRT; rtNo++){
		rtDSHandleTbl[rtNo] = renderTargets[rtNo]->GetRTVCpuDescriptorHandle();
	}
	if (renderTargets[0]->IsExsitDepthStencilBuffer()) {
		//深度バッファがある。
		D3D12_CPU_DESCRIPTOR_HANDLE dsDS = renderTargets[0]->GetDSVCpuDescriptorHandle();
		m_commandList->OMSetRenderTargets(numRT, rtDSHandleTbl, FALSE, &dsDS);
	}
	else {
		//深度バッファがない。
		m_commandList->OMSetRenderTargets(numRT, rtDSHandleTbl, FALSE, nullptr);
	}

}
inline void RenderContext::SetRenderTargetAndViewport(RenderTarget& renderTarget)
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(renderTarget.GetWidth());
	viewport.Height = static_cast<float>(renderTarget.GetHeight());
	viewport.MinDepth = D3D12_MIN_DEPTH;
	viewport.MaxDepth = D3D12_MAX_DEPTH;
	SetViewportAndScissor(viewport);
	
	SetRenderTarget(renderTarget);
}
inline void RenderContext::SetRenderTargetsAndViewport(UINT numRT, RenderTarget* renderTargets[])
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(renderTargets[0]->GetWidth());
	viewport.Height = static_cast<float>(renderTargets[0]->GetHeight());
	viewport.MinDepth = D3D12_MIN_DEPTH;
	viewport.MaxDepth = D3D12_MAX_DEPTH;
	SetViewportAndScissor(viewport);
	SetRenderTargets(numRT, renderTargets);
}
inline void RenderContext::ClearRenderTargetViews(int numRt, RenderTarget* renderTargets[])
{
	if (renderTargets[0]->IsExsitDepthStencilBuffer()) {
		//深度バッファがある。
		ClearDepthStencilView(renderTargets[0]->GetDSVCpuDescriptorHandle(), renderTargets[0]->GetDSVClearValue());
	}
	for (int i = 0; i < numRt; i++) {
		ClearRenderTargetView(renderTargets[i]->GetRTVCpuDescriptorHandle(), renderTargets[i]->GetRTVClearColor());
	}
}
inline void RenderContext::SetPipelineState(raytracing::PSO& pso)
{
	m_commandList->SetPipelineState1(pso.Get());
}
