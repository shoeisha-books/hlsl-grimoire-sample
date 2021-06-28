#include "stdafx.h"
#include "tkEngine.h"
#include "GraphicsEngine.h"

TkEngine* g_engine = nullptr;

TkEngine::~TkEngine()
{
	if (m_graphicsEngine) {
		delete m_graphicsEngine;
	}
}
void TkEngine::Init(HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
{
	//グラフィックエンジンの初期化。
	m_graphicsEngine = new GraphicsEngine();
	m_graphicsEngine->Init(hwnd, frameBufferWidth, frameBufferHeight);
	//ゲームパッドの初期化。
	for (int i = 0; i < GamePad::CONNECT_PAD_MAX; i++) {
		g_pad[i] = &m_pad[i];
	}
}
void TkEngine::BeginFrame()
{
	m_graphicsEngine->BeginRender();
	for (auto& pad : m_pad) {
		pad.BeginFrame();
		pad.Update();
	}
	
}
void TkEngine::EndFrame()
{
	m_graphicsEngine->EndRender();
}