#include "StdAfx.h"
#include "resource.h"
#include "winMain.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
	TCHAR className[] = _T("VitekTextEditor");
	TCHAR title[] = _T("VMAText");
	LPCSTR errorMesage = "Это порнуха";
	if (!RegWinClass(hInstance, WndProc, className))
	{
		MessageBox(NULL, (LPCWSTR)errorMesage, (LPCWSTR)"Errorrr",NULL);
		return FALSE;
	}

	if (!initWindow(className,title, hInstance,nCmdShow) )
	{
		MessageBox(NULL, (LPCWSTR)errorMesage, (LPCWSTR)"Errorrr",NULL);
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, reinterpret_cast<LPCTSTR>(IDR_MAINFRAME));

	MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }

    return (int) msg.wParam;

}

bool RegWinClass(HINSTANCE hInstance, WNDPROC Proc, LPCTSTR szClassName) 
{ 
    WNDCLASS wc; 
    wc.style=wc.cbClsExtra=wc.cbWndExtra=0; 
    wc.lpfnWndProc = Proc; 
    wc.hInstance = hInstance; 
	wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); 
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINFRAME); 
	wc.lpszClassName = szClassName; 
    return (RegisterClass(&wc) != 0);
}

bool initWindow(LPCTSTR szClassName, LPCTSTR szTitle, HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hWnd = CreateWindow(
		szClassName, 
		szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		900, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd,nCmdShow);
	if (!UpdateWindow(hWnd))
	{
		return false;
	}
	return true;
}

