#include "ContentOfWindow.h"

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->caretPos.x = 0;
	this->caretPos.y = 0;
	this->endTextPos.x = 1;
	this->endTextPos.y = 0;
	this->autoMoveNextlineFlag = true;
	hDC = GetDC(hWnd);
	SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
	if (!GetClientRect(hWnd, &clientRect))
	{
		//тут надо генерировать исключение, что у нас трындец
		// отлавливать исключение и выводить по нему MessageBox в WinProс
		//это оставлю для Максима или Макрычева
		//http://msdn.microsoft.com/en-us/library/windows/desktop/ms633503(v=vs.85).aspx
	}
}

ContentOfWindow::~ContentOfWindow(void)
{
	ReleaseDC(hWnd,hDC);
}

void ContentOfWindow::calulateCaretPosByCoordinates(LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	caretPos.y = y / charSize.y;
	caretPos.x = x / charSize.x;
}

void ContentOfWindow::drawText()
{
	HideCaret(hWnd);
	FillRect(hDC, &clientRect, (HBRUSH) (COLOR_WINDOW+1));
	validateRectsForPaint();
	POINT currentPos;
	currentPos.x = 0;
	currentPos.y = 0;
	int textSize = text._Mysize;
	for (int i = 0; i < textSize; i++)
	{
		currentPos =  printCharOnDC(i,currentPos);
	}
	SetCaretPos(caretPos.x * charSize.x, caretPos.y * charSize.y );
	ShowCaret(hWnd);
}

void ContentOfWindow::processorArrows(WORD wParam) 
{
	HideCaret(hWnd);
	switch (wParam)
	{
		case VK_LEFT:
			if (caretPos.x == 0)
				break;
			else
			{
				caretPos.x--;
				break;
			}


		case VK_RIGHT:
			caretPos.x++;
			break;

		case VK_UP:
			caretPos.y--;
			break;

		case VK_DOWN:
			caretPos.y++;
			break;
	}
		SetCaretPos(caretPos.x * charSize.x, caretPos.y * charSize.y );
		ShowCaret(hWnd);
}
bool ContentOfWindow::processorMenuMessages(WORD id)
{
	switch (id)
	{
	case ID_FILE_EXIT:
		PostQuitMessage(0);
		MessageBeep(MB_OK);  
		break;
	case ID_CTRL_V:
		if (OpenClipboard(hWnd))
		{
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);
			wstring fromClipboard = (wchar_t*)GlobalLock(hData);
			GlobalUnlock(hData);
			CloseClipboard();
			text.insert(indexInTextByCaret(), &fromClipboard.at(0), fromClipboard.size());
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	default:
		return false;
	}
	return true;
}

void ContentOfWindow::processorWmChar(WORD wParam)
{
	calculateLengthLine();
	switch (wParam)
	{
	case '\b':
		if (caretPos.x > 0 || caretPos.y != 0)
		{
			int index = indexInTextByCaret();
			
			if (caretPos.x != 0)
			{
				caretPos.x--;
			}
			else
			{
				caretPos.y--;
				if (vectorIndexesNewLines.empty()  || caretPos.y == 0)
				{
					caretPos.x = vectorIndexesNewLines.at(caretPos.y);
				}
				else
				{
					caretPos.x = vectorIndexesNewLines[caretPos.y] - vectorIndexesNewLines[caretPos.y -1] - 1;
				}
			}

			if (text[index-1] == '\r')
			{
				vector<int>::iterator it = vectorIndexesNewLines.begin();
				vectorIndexesNewLines.erase(it + caretPos.y);
			}
			text.erase(index-1,1);
			changeIndexesNewLines(caretPos.y,-1);
		}
		break;
	case '\t':
		for (int i = 0; i < 4; i++)
		{
			addCharToText(L' ');///error надо пробелы
		}
		break;
	case '\r':
	case '\n':
		addCharToText(wParam);
		break;
	default:
		addCharToText(wParam);
		break;
	}
	InvalidateRect(hWnd,NULL, false);
}

void ContentOfWindow::setSizeAreaType(LPARAM param)
{
	clientSize.x = LOWORD(param);
	clientSize.y = HIWORD(param);
	calculateLengthLine();
}

void ContentOfWindow::workWithCaret(WORD message)
{
	calculateCharSize();
	if (message == WM_SETFOCUS)
	{
		CreateCaret(hWnd,NULL, NULL,charSize.y);
		SetCaretPos(caretPos.x * charSize.x, caretPos.y * charSize.y );
		ShowCaret(hWnd);
	}
	if (message == WM_KILLFOCUS)
	{
		HideCaret(hWnd);
		DestroyCaret();
	}
}


void ContentOfWindow::addCharToText(WORD wParam)
{
	autoNewLine();
	wchar_t addedSymbol = wParam;
	int indexCharInText = indexInTextByCaret();
	changeIndexesNewLines(caretPos.y,1);

	if ( text.size() == indexCharInText)
	{
		text.append(&addedSymbol,1);
	}
	else
	{ 
		text.insert(indexCharInText,&addedSymbol,1);
	}

	if (addedSymbol == '\r')
	{
		vectorIndexesNewLines.insert(vectorIndexesNewLines.begin() + caretPos.y, indexCharInText);
		caretPos.y++;
		caretPos.x = 0;
	}
	else if (caretPos.x < lengthLine)
	{
		caretPos.x++;
	}
}

void ContentOfWindow::autoNewLine()
{
	static bool flagGo = true;
	calculateLengthLine();
	if (autoMoveNextlineFlag && caretPos.x == lengthLine && flagGo)
	{
		flagGo = false;
		addCharToText('\r');
		flagGo = true;
	}
}

void ContentOfWindow::calculateCharSize()
{
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	charSize.x = tm.tmAveCharWidth;
	charSize.y = tm.tmHeight;
}

void ContentOfWindow::calculateLengthLine()
{
	lengthLine = clientSize.x / charSize.x;
}

void ContentOfWindow::calculateEndTextPos()
{	
	if (!vectorIndexesNewLines.empty())
	{
		endTextPos.y = vectorIndexesNewLines.size();
		endTextPos.x = text.size() - (vectorIndexesNewLines.back() + 1);
	}
	else
	{
		endTextPos.y = 0;
		endTextPos.x = text.size();
	}
}

void ContentOfWindow::changeIndexesNewLines(int start, int additional)
{
	for (int i = start; i < (int)vectorIndexesNewLines.size(); i++)
	{
		vectorIndexesNewLines[i] += additional;
	}
}

int ContentOfWindow::indexInTextByCaret()
{
	//если мы не попадаем  область текста, то автоматом будет добовляться в конец
	//это временное решение
	int indexCharInText = text.size();
	if ( (caretPos.y < endTextPos.y) || (caretPos.y == endTextPos.y && caretPos.x <= endTextPos.x) )
	{
		indexCharInText = caretPos.x;
		if (!vectorIndexesNewLines.empty() && caretPos.y > 0)
		{
			indexCharInText += vectorIndexesNewLines[caretPos.y-1] + 1;
		}
	}
	return indexCharInText;
}

POINT ContentOfWindow::printCharOnDC(int indexCharInText, POINT currentPos)
{
	LPCWSTR printedChar = &text.c_str()[indexCharInText];
	if (text[indexCharInText] != '\r')
	{
		TextOut(hDC, currentPos.x * charSize.x, currentPos.y * charSize.y, printedChar, 1);
		currentPos.x++;
	}
	else
	{
		currentPos.x = 0;
		currentPos.y++;
	}
	return currentPos;
}

void ContentOfWindow::validateRectsForPaint()
{
	calculateEndTextPos();
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = clientSize.x;
	rect.bottom = endTextPos.y * charSize.y;
	ValidateRect(hWnd, &rect);

	RECT lastLineRect;
	lastLineRect.left = 0;
	lastLineRect.top = endTextPos.y * charSize.y;
	lastLineRect.right = endTextPos.x * charSize.x;
	lastLineRect.bottom = lastLineRect.top + charSize.y;
	ValidateRect(hWnd,&lastLineRect);
}