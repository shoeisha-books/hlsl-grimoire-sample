#pragma once

#include "Bloom.h"
#include "Dof.h"

namespace myRenderer {
    class PostEffect
    {
    public:
        void Init(RenderTarget& mainRenderTarget, RenderTarget& zprepassRenderTarget);
        void Render( RenderContext& rc, RenderTarget& mainRenderTarget );
    private:
        Bloom m_bloom;	//ブルーム
        Dof m_dof;		//被写界深度
    };
}

