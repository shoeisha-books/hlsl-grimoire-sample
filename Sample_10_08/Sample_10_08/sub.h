#pragma once

const int NUM_DIRECTIONAL_LIGHT = 4;
/// <summary>
/// ディレクションライト。
/// </summary>
struct DirectionalLight {
    Vector3 direction;	//ライトの方向。
    float pad0;			//パディング。
    Vector4 color;		//ライトのカラー。
};
/// <summary>
/// ライト構造体。
/// </summary>
struct Light {
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];	//ディレクションライト。
    Vector3 eyePos;					//カメラの位置。
    float specPow;					//スペキュラの絞り。
    Vector3 ambinetLight;			//環境光。
};

void InitMainDepthRenderTarget(RenderTarget& mainRenderTarget, RenderTarget& depthRenderTarget);
void InitCombimeBokeImageToSprite(Sprite& combineBokeImageSprite, Texture& bokeTexture, Texture& depthTexture);
void InitCopyToFrameBufferTargetSprite(Sprite& sprite, Texture& srcTexture);
void InitBGModel(Model& model, Light& light);
void InitSphereModels(Model* model, Light* light, int numModel);
void MoveCamera();