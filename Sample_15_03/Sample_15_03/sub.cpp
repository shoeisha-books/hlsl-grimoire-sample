#include "stdafx.h"
#include "sub.h"

// ルートシグネチャの初期化
void InitRootSignature(RootSignature& rs)
{
    rs.Init(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

void InitPipelineState(RootSignature& rs, PipelineState& pipelineState, Shader& cs)
{
    // パイプラインステートを作成
    D3D12_COMPUTE_PIPELINE_STATE_DESC  psoDesc = { 0 };
    psoDesc.pRootSignature = rs.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetCompiledBlob());
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psoDesc.NodeMask = 0;

    pipelineState.Init(psoDesc);
}
