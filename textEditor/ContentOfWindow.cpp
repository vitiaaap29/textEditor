#include "ContentOfWindow.h"

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->caretPos.x = 0;
	this->caretPos.y = 0;
	this->endTextPos.x = 1;
	this->endTextPos.y = 0;
	this->leftMouseButtonPressed = false;
	this->selectionFlag = false;
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

void ContentOfWindow::CaretPosByCoordinates(LPARAM lParam)
{
	caretPos = calculateCaretPosByCoordinates(lParam);
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
	indexesNewLines.clear();
	COLORREF color = GetBkColor(hDC);
	COLORREF highlightColor = RGB(0, 255, 255);
	for (int i = 0; i < textSize; i++)
	{
		if (selectionFlag && caretIncludeSelectArea(currentPos))
		{
			SetBkColor(hDC,highlightColor);
		}
		currentPos =  printCharOnDC(i,currentPos);
		SetBkColor(hDC,color);
	}
	SetCaretPos(caretPos.x * charSize.x, caretPos.y * charSize.y );
	ShowCaret(hWnd);
}

void ContentOfWindow::mouseSelection(WPARAM wParam, LPARAM lParam)
{
	POINT currentCaretPos = calculateCaretPosByCoordinates(lParam);
	POINT difference;
	if (wParam == MK_LBUTTON)
	{
		difference.x = currentCaretPos.x - caretPos.x;
		difference.y = currentCaretPos.y - caretPos.y;
		if (difference.x != 0 || difference.y != 0)
		{
			selectionFlag = true;
			caretPos = currentCaretPos;
			InvalidateRect(hWnd, NULL, false);
		}
	}
	else
	{
		selectionFlag = false;
	}
}

void ContentOfWindow::processorArrows(WORD wParam) 
{
	HideCaret(hWnd);
	switch (wParam)
	{
		case VK_LEFT:
			if (caretPos.x != 0 || caretPos.y !=0)
			{
				if (caretPos.x == 0)
				{
					caretPos.y--;
					caretPos.x = indexesNewLines[caretPos.y];
				}
				else
				{
					caretPos.x--;
				}
			}
			break;

		case VK_RIGHT:
			if (endTextPos.x != caretPos.x  || endTextPos.y !=caretPos.y)
			{
				if (caretPos.y < indexesNewLines.size())
				{
					if (caretPos.x == indexesNewLines[caretPos.y])
					{
						caretPos.x = 0;
						caretPos.y++;
					}
					else
					{
						caretPos.x++;
					}
				}
				else
				{
					caretPos.x++;
				}
			}
			break;

		case VK_UP:
			if (caretPos.y != 0)
			{
				caretPos.y--;
			//	caretPos.x == indexesNewLines[caretPos.y];
			}
			break;

		case VK_DOWN:
			if (endTextPos.y != caretPos.y && endTextPos.x != caretPos.x)
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
	case ID_SAVE_FILE:
		//тут Лёшина функция будет вызываться
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
				caretPos.x = indexesNewLines[caretPos.y - 1];
			}

			text.erase(index-1,1);
		}
		break;
	case '\t':
		for (int i = 0; i < 4; i++)
		{
			addCharToText(L' ');
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

void ContentOfWindow::setStartForSelection(LPARAM lParam)
{
	startForSelection = calculateCaretPosByCoordinates(lParam);
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
	wchar_t addedSymbol = wParam;
	int indexCharInText = indexInTextByCaret();

	if ( text.size() == indexCharInText)
	{
		text.append(&addedSymbol,1);
	}
	else
	{ 
		text.insert(indexCharInText,&addedSymbol,1);
	}

	if (addedSymbol == '\r' || caretPos.x == lengthLine)
	{
		caretPos.y++;
		caretPos.x = 0;
	}
	else if (caretPos.x < lengthLine)
	{
		caretPos.x++;
	}
}

POINT ContentOfWindow::calculateCaretPosByCoordinates(LPARAM lParam)
{
	POINT caretPosition;
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	caretPosition.y = y / charSize.y;
	caretPosition.x = x / charSize.x;
	return caretPosition;
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
	POINT currentEnd;
	currentEnd.x = 0;
	currentEnd.y = 0;
	int size = (int)text.size();
	for (int index = 0; index < size; index++)
	{
		if (text[index] == '\r' || currentEnd.x == lengthLine)
		{
			currentEnd.x = 0;
			currentEnd.y++;
		}
		else
		{
			currentEnd.x++;
		}
	}
	endTextPos = currentEnd;
}

bool ContentOfWindow::caretIncludeSelectArea(POINT position)
{
	bool result = false;
	if (startForSelection.y == caretPos.y && caretPos.y == position.y)
	{
		if ( position.x >= min(startForSelection.x,caretPos.x ) &&  position.x < max(startForSelection.x, caretPos.x) )
		{
			result = true;
		}
	}
	else
	{
		if (position.y > min(startForSelection.y, caretPos.y) && position.y < max(startForSelection.y, caretPos.y))
		{
			result = true;
		}
		else if(position.y == max(startForSelection.y, caretPos.y) )
		{
			POINT posMaxY = (startForSelection.y > caretPos.y) ? startForSelection: caretPos;
			if (position.x < posMaxY.x)
			{
				result = true;
			}
			else
			{
				result = false;
			}
		}
		else if (position.y == min(startForSelection.y, caretPos.y) )
		{
			POINT posMinY = (startForSelection.y < caretPos.y) ? startForSelection: caretPos;
			if (position.x >= posMinY.x)
			{
				result = true;
			}
			else
			{
				result = false;
			}
		}
	}
	return result;
}

int ContentOfWindow::indexInTextByCaret()
{
	POINT currentCaretPos;
	currentCaretPos.x = 0;
	currentCaretPos.y = 0;
	int size = (int)text.size();
	int index;
	for (index = 0; index < size && (currentCaretPos.x != caretPos.x || currentCaretPos.y != caretPos.y); index++)
	{
		if (text[index] == '\r' || currentCaretPos.x == lengthLine)
		{
			currentCaretPos.x = 0;
			currentCaretPos.y++;
		}
		else
		{
			currentCaretPos.x++;
		}
	}
	return index;
}

POINT ContentOfWindow::printCharOnDC(int indexCharInText, POINT currentPos)
{
	LPCWSTR printedChar = &text.c_str()[indexCharInText];
	wchar_t ch = text.at(indexCharInText);
	if (ch != '\r' && currentPos.x != lengthLine)
	{
		TextOut(hDC, currentPos.x * charSize.x, currentPos.y * charSize.y, printedChar, 1);
		currentPos.x++;
	}
	else if (currentPos.x == lengthLine)
	{
		TextOut(hDC, currentPos.x * charSize.x, currentPos.y * charSize.y, printedChar, 1);
		indexesNewLines.push_back(currentPos.x);
		currentPos.x = 0;
		currentPos.y++;
	}
	else
	{
		indexesNewLines.push_back(currentPos.x);
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