#include "StdAfx.h"
#include "winMain.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ContentOfWindow *content;
	static int xClient;
	static int yClient;

	switch (message)
	{
	case WM_CREATE:
		content = new ContentOfWindow(hWnd);
		break;
	case WM_DESTROY:
		MessageBox(NULL, (LPCWSTR)"rgir[ijit", (LPCWSTR)"Errorrr",NULL);
		content->~ContentOfWindow();
        PostQuitMessage(0);
        break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		content->calculateCaretPos(lParam);
		InvalidateRect(hWnd,NULL,true);
		break;
	case WM_COMMAND:
		if ( ProcessingMenuMessages(LOWORD(wParam)) )
		{
			break;
		}
		break;

	case WM_SIZE:
		content->setSizeAreaType(lParam);
		break;
	case WM_SETFOCUS:
		content->workWithCaret(message);
		break;
	case WM_KILLFOCUS:
		content->workWithCaret(message);
		break;
	case WM_CHAR:
		content->processorWmChar(wParam);
		break;
	case WM_PAINT:
		if (content->Text().size() != 0)
		{
			content->drawText();
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
