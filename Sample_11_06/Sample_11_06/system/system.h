#pragma once

class GraphicsEngine;
extern HWND			g_hWnd ;				//ウィンドウハンドル。

//ゲームの初期化。
void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName);
//ウィンドウメッセージをディスパッチ。falseが返ってきたら、ゲーム終了。
bool DispatchWindowMessage();
