#include "stdafx.h"
#include "sub.h"

void InitMainDepthRenderTarget(RenderTarget& mainRenderTarget, RenderTarget& depthRenderTarget)
{
    //メインレンダリングターゲットと深度レンダリングターゲットを作成。
   //シーンのカラーを描きこむメインレンダリングターゲットを作成。
    mainRenderTarget.Create(
        1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );
    //シーンのカメラ空間でのZ値を書きこむレンダリングターゲットを作成。
    float clearColor[4] = { 10000.0f, 10000.0f, 10000.0f, 1.0f };
    depthRenderTarget.Create(
        1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN,
        clearColor
    );
}
void InitCombimeBokeImageToSprite(Sprite& combineBokeImageSprite, Texture& bokeTexture, Texture& depthTexture)
{
    SpriteInitData combineBokeImageSpriteInitData;
    //使用するテクスチャは２枚。
    combineBokeImageSpriteInitData.m_textures[0] = &bokeTexture;
    combineBokeImageSpriteInitData.m_textures[1] = &depthTexture;
    combineBokeImageSpriteInitData.m_width = 1280;
    combineBokeImageSpriteInitData.m_height = 720;
    combineBokeImageSpriteInitData.m_fxFilePath = "Assets/shader/preset/samplePostEffect.fx";
    combineBokeImageSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    // 距離を利用してボケ画像をアルファブレンディングするので、半透明合成モードにする。
    combineBokeImageSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Trans;
    // 初期化オブジェクトを利用してスプライトを初期化する。
   
    combineBokeImageSprite.Init(combineBokeImageSpriteInitData);
}
void InitCopyToFrameBufferTargetSprite(Sprite& copyToFrameBufferSprite, Texture& srcTexture)
{
    SpriteInitData spriteInitData;
    // テクスチャはyBlurRenderTargetのカラーバッファー
    spriteInitData.m_textures[0] = &srcTexture;
    // レンダリング先がフレームバッファーなので、解像度はフレームバッファーと同じ
    spriteInitData.m_width = 1280;
    spriteInitData.m_height = 720;
    // ボケ画像をそのまま貼り付けるだけなので、通常の2D描画のシェーダーを指定する
    spriteInitData.m_fxFilePath = "Assets/shader/preset/sample2D.fx";
    spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    // 初期化オブジェクトを使って、スプライトを初期化する
    copyToFrameBufferSprite.Init(spriteInitData);
}
void InitBGModel(Model& model, Light& light)
{
    //光を強めに設定する。
    light.directionalLight[0].color.x = 2.0f;
    light.directionalLight[0].color.y = 2.0f;
    light.directionalLight[0].color.z = 2.0f;

    light.directionalLight[0].direction.x = 0.0f;
    light.directionalLight[0].direction.y = 0.0f;
    light.directionalLight[0].direction.z = -1.0f;
    light.directionalLight[0].direction.Normalize();

    light.ambinetLight.x = 0.5f;
    light.ambinetLight.y = 0.5f;
    light.ambinetLight.z = 0.5f;

    light.eyePos = g_camera3D->GetPosition();

    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
    modelInitData.m_expandConstantBuffer = &light;
    modelInitData.m_expandConstantBufferSize = sizeof(light);
    modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R32_FLOAT;

    model.Init(modelInitData);
}
void InitSphereModels(Model* sphereModels, Light* sphereLight, int numModel)
{
  
    ModelInitData modelInitData;
    modelInitData.m_tkmFilePath = "Assets/modelData/sphere.tkm";
    modelInitData.m_fxFilePath = "Assets/shader/preset/sample3D.fx";
   
    modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R32_FLOAT;

    for (int i = 0; i < numModel; i++) {
        sphereLight[i].directionalLight[0].direction.x = 0.0f;
        sphereLight[i].directionalLight[0].direction.y = 0.0f;
        sphereLight[i].directionalLight[0].direction.z = -1.0f;
        sphereLight[i].directionalLight[0].direction.Normalize();

       
        sphereLight[i].eyePos = g_camera3D->GetPosition();

        switch (rand() % 3) {
        case 0:
            sphereLight[i].directionalLight[0].color.x = 200.0f;
            sphereLight[i].directionalLight[0].color.y = 0.0f;
            sphereLight[i].directionalLight[0].color.z = 0.0f;
            sphereLight[i].ambinetLight.x = 0.5f;
            sphereLight[i].ambinetLight.y = 0.0f;
            sphereLight[i].ambinetLight.z = 0.0f;

            break;
        case 1:
            sphereLight[i].directionalLight[0].color.x = 0.0f;
            sphereLight[i].directionalLight[0].color.y = 200.0f;
            sphereLight[i].directionalLight[0].color.z = 0.0f;
            sphereLight[i].ambinetLight.x = 0.0f;
            sphereLight[i].ambinetLight.y = 0.5f;
            sphereLight[i].ambinetLight.z = 0.0f;

            break;
        case 2:
            sphereLight[i].directionalLight[0].color.x = 0.0f;
            sphereLight[i].directionalLight[0].color.y = 0.0f;
            sphereLight[i].directionalLight[0].color.z = 200.0f;
            sphereLight[i].ambinetLight.x = 0.0f;
            sphereLight[i].ambinetLight.y = 0.0f;
            sphereLight[i].ambinetLight.z = 0.5f;

            break;
        }
        float t = ( rand() % 100 )/ 100.0f;

        modelInitData.m_expandConstantBuffer = &sphereLight[i];
        modelInitData.m_expandConstantBufferSize = sizeof(sphereLight[i]);

        sphereModels[i].Init(modelInitData);
        float x = Math::Lerp(t, -250.0f, 250.0f);
        t = (rand() % 100) / 100.0f;
        float y = Math::Lerp(t, 20.0f, 200.0f);
        t = (rand() % 100) / 100.0f;
        float z = Math::Lerp(t, 0.0f, -2000.0f);
        


        sphereModels[i].UpdateWorldMatrix(
            { x, y, z }, 
            g_quatIdentity, 
            { 0.5f, 0.5f, 0.5f }
        );
    }
}
void MoveCamera()
{
    g_camera3D->MoveForward(g_pad[0]->GetLStickYF() * 3.0f );
    g_camera3D->MoveRight(g_pad[0]->GetLStickXF() * 3.0f );
}