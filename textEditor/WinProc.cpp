#include "StdAfx.h"
#include "winMain.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ContentOfWindow *content;

	switch (message)
	{
	case WM_CREATE:
		content = new ContentOfWindow(hWnd);
		break;
	case WM_DESTROY:
		content->~ContentOfWindow();
        PostQuitMessage(0);
        break;
	case WM_LBUTTONDOWN:
		content->CaretPosByCoordinates(lParam);
		content->setStartForSelection(lParam);
		content->invertSelectionFlag();
		InvalidateRect(hWnd,NULL,true);
		break;
	case WM_LBUTTONUP:
		content->invertSelectionFlag();
		break;
	case WM_MOUSEMOVE:
		content->mouseSelection(wParam,lParam);
		break;
	case WM_KEYDOWN:
		content->processorArrows(wParam);
		break;
	case WM_COMMAND:
		content->processorMenuMessages(LOWORD(wParam));
		break;
	case WM_SIZE:
		content->setSizeAreaType(lParam);
		break;
	case WM_SETFOCUS:
		content->workWithCaret(message);
		break;
	case WM_MOUSEWHEEL:
		content->changeScalling(wParam);
		break;
	case WM_KILLFOCUS:
		content->workWithCaret(message);
		break;
	case WM_CHAR:
		content->processorWmChar(wParam);
		break;
	case WM_PAINT:
		content->drawText();

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
