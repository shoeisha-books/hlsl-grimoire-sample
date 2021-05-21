#include "stdafx.h"
#include "Texture.h"

Texture::Texture(const wchar_t* filePath)
{
	InitFromDDSFile(filePath);
}
Texture::~Texture()
{
	if (m_texture) {
		m_texture->Release();
	}
}
void Texture::InitFromDDSFile(const wchar_t* filePath)
{
	//DDSファイルからテクスチャをロード。
	LoadTextureFromDDSFile(filePath);
	
}
void Texture::InitFromD3DResource(ID3D12Resource* texture)
{
	if (m_texture) {
		m_texture->Release();
	}
	m_texture = texture;
	m_texture->AddRef();
	m_textureDesc = m_texture->GetDesc();
}
void Texture::InitFromMemory(const char* memory, unsigned int size)
{
	//DDSファイルからテクスチャをロード。
	LoadTextureFromMemory(memory, size);

}
void Texture::LoadTextureFromMemory(const char* memory, unsigned int size
)
{
	auto device = g_graphicsEngine->GetD3DDevice();
	DirectX::ResourceUploadBatch re(device);
	re.Begin();
	ID3D12Resource* texture;
	auto hr = DirectX::CreateDDSTextureFromMemoryEx(
		device,
		re,
		(const uint8_t*)memory,
		size,
		0,
		D3D12_RESOURCE_FLAG_NONE,
		0,
		&texture
	);
	re.End(g_graphicsEngine->GetCommandQueue());

	if (FAILED(hr)) {
		//テクスチャの作成に失敗しました。
		return;
	}

	m_texture = texture;
	m_textureDesc = m_texture->GetDesc();
}
void Texture::LoadTextureFromDDSFile(const wchar_t* filePath)
{
	auto device = g_graphicsEngine->GetD3DDevice();
	DirectX::ResourceUploadBatch re(device);
	re.Begin();
	ID3D12Resource* texture;
	auto hr = DirectX::CreateDDSTextureFromFileEx(
		device,
		re,
		filePath,
		0,
		D3D12_RESOURCE_FLAG_NONE,
		0,
		&texture
	);
	re.End(g_graphicsEngine->GetCommandQueue());

	if (FAILED(hr)) {
		//テクスチャの作成に失敗しました。
		return;
	}

	m_texture = texture;
	m_textureDesc = m_texture->GetDesc();
}
	
void Texture::RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
{
	if (m_texture) {
		auto device = g_graphicsEngine->GetD3DDevice();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = m_textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_textureDesc.MipLevels;
		device->CreateShaderResourceView(m_texture, &srvDesc, descriptorHandle);
	}
}
