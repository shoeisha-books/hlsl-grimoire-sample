#pragma once

#include "Bloom.h"
#include "Dof.h"

namespace myRenderer
{
    class PostEffect
    {
    public:
        void Init(RenderTarget& mainRenderTarget, RenderTarget& zprepassRenderTarget);

        void Render( RenderContext& rc, RenderTarget& mainRenderTarget );

    private:
        Bloom m_bloom;  //ƒuƒ‹[ƒ€
        Dof m_dof;      //”íÊŠE[“x
    };
}
