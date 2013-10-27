#include "StdAfx.h"
#include "ContentOfWindow.h"
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

bool RegWinClass(HINSTANCE hInstance, WNDPROC Proc, LPCTSTR szClassName);
bool initWindow(LPCTSTR szClassName, LPCTSTR szTitle, HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool ProcessingMenuMessages(WORD id);