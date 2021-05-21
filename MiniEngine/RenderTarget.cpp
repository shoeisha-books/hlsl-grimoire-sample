#include "stdafx.h"
#include "RenderTarget.h"
#include "GraphicsEngine.h"

RenderTarget::~RenderTarget()
{
	if (m_renderTargetTextureDx12) {
		m_renderTargetTextureDx12->Release();
	}
	if (m_depthStencilTexture) {
		m_depthStencilTexture->Release();
	}
	if (m_rtvHeap) {
		m_rtvHeap->Release();
	}
	if (m_dsvHeap) {
		m_dsvHeap->Release();
	}
}
bool RenderTarget::Create(
	int w,
	int h,
	int mipLevel,
	int arraySize,
	DXGI_FORMAT colorFormat,
	DXGI_FORMAT depthStencilFormat,
	float clearColor[4]
)
{
	auto d3dDevice = g_graphicsEngine->GetD3DDevice();
	m_width = w;
	m_height = h;
	//レンダリングターゲットとなるテクスチャを作成する。
	if (!CreateRenderTargetTexture(*g_graphicsEngine, d3dDevice, w, h, mipLevel, arraySize, colorFormat, clearColor)) {
	//	TK_ASSERT(false, "レンダリングターゲットとなるテクスチャの作成に失敗しました。");
		MessageBoxA(nullptr, "レンダリングターゲットとなるテクスチャの作成に失敗しました。", "エラー", MB_OK);
		return false;
	}
	//深度ステンシルバッファとなるテクスチャを作成する。
	if (depthStencilFormat != DXGI_FORMAT_UNKNOWN) {
		if (!CreateDepthStencilTexture(*g_graphicsEngine, d3dDevice, w, h, depthStencilFormat)) {
			MessageBoxA(nullptr, "レンダリングターゲットとなるテクスチャの作成に失敗しました。", "エラー", MB_OK);
			return false;
		}
	}
	if (!CreateDescriptorHeap(*g_graphicsEngine, d3dDevice)) {
		//ディスクリプタヒープの作成に失敗した。
		MessageBoxA(nullptr, "レンダリングターゲットとなるテクスチャの作成に失敗しました。", "エラー", MB_OK);
		return false;
	}
	//ディスクリプタを作成する。
	CreateDescriptor(d3dDevice);
	if (clearColor) {
		memcpy(m_rtvClearColor, clearColor, sizeof(m_rtvClearColor));
	}
	return true;
}
bool RenderTarget::CreateDescriptorHeap(GraphicsEngine& ge, ID3D12Device5*& d3dDevice)
{
		
	//RTV用のディスクリプタヒープを作成する。
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = GraphicsEngine::FRAME_BUFFER_COUNT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
	if (m_rtvHeap == nullptr) {
		//RTV用のディスクリプタヒープの作成に失敗した。
		return false;
	}
	//ディスクリプタのサイズを取得。
	m_rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (m_depthStencilTexture) {
		//DSV用のディスクリプタヒープを作成する。
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap));
		if (m_dsvHeap == nullptr) {
			//DSV用のディスクリプタヒープの作成に失敗した。
			return false;
		}
		//ディスクリプタのサイズを取得。
		m_dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}
	return true;
}
bool RenderTarget::CreateRenderTargetTexture(
	GraphicsEngine& ge,
	ID3D12Device5*& d3dDevice,
	int w,
	int h,
	int mipLevel,
	int arraySize,
	DXGI_FORMAT format,
	float clearColor[4]
)
{
	CD3DX12_RESOURCE_DESC desc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(w),
		static_cast<UINT>(h),
		arraySize,
		mipLevel,
		format,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	if (clearColor != nullptr) {
		clearValue.Color[0] = clearColor[0];
		clearValue.Color[1] = clearColor[1];
		clearValue.Color[2] = clearColor[2];
		clearValue.Color[3] = clearColor[3];
	}
	else {
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;
	}
	//リソースを作成。
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = d3dDevice->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_renderTargetTextureDx12)
	);

	if (FAILED(hr)) {
		//作成に失敗。
		return false;
	}
	m_renderTargetTexture.InitFromD3DResource(m_renderTargetTextureDx12);
	return true;
}
bool RenderTarget::CreateDepthStencilTexture(
	GraphicsEngine& ge,
	ID3D12Device5*& d3dDevice,
	int w,
	int h,
	DXGI_FORMAT format)
{
	D3D12_CLEAR_VALUE dsvClearValue;
	dsvClearValue.Format = format;
	dsvClearValue.DepthStencil.Depth = 1.0f;
	dsvClearValue.DepthStencil.Stencil = 0;

	CD3DX12_RESOURCE_DESC desc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(w),
		static_cast<UINT>(h),
		1,
		1,
		format,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = d3dDevice->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClearValue,
		IID_PPV_ARGS(&m_depthStencilTexture)
	);
	if (FAILED(hr)) {
		//深度ステンシルバッファの作成に失敗。
		return false;
	}
	return true;
}
void RenderTarget::CreateDescriptor(ID3D12Device5*& d3dDevice)
{
	//カラーテクスチャのディスクリプタを作成。
	auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	d3dDevice->CreateRenderTargetView(m_renderTargetTexture.Get(), nullptr, rtvHandle);
	if (m_depthStencilTexture) {
		//深度テクスチャのディスクリプタを作成
		auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		d3dDevice->CreateDepthStencilView(m_depthStencilTexture, nullptr, dsvHandle);
	}
}
