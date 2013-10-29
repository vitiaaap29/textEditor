#include "ContentOfWindow.h"

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->caretPos.x = 0;
	this->caretPos.y = 0;
	this->text;
	hDC = GetDC(hWnd);
	SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
}

ContentOfWindow::~ContentOfWindow(void)
{
	ReleaseDC(hWnd,hDC);
}

void ContentOfWindow::processorWmChar(WORD wParam)
{
	int index;
	int lengthText = 0;
	switch (wParam)
	{
	case '\b':
		if (caretPos.x > 0 || caretPos.y != 0)
		{
			calculateLengthLine();
			index = caretPos.x + caretPos.y * lengthLine;
			text.erase(index-1,1);
			
			if (caretPos.x != 0)
			{
				caretPos.x--;
			}
		}
		break;
	case '\t':
		for (int i = 0; i < 4; i++)
		{
			addCharToText(wParam);
		}
		break;
	case '\n':
		break;
	default:
		calculateLengthLine();
		addCharToText(wParam);
		break;
	}
	InvalidateRect(hWnd,NULL,true);
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

void ContentOfWindow::drawText()
{
	validateRectsForPaint();

	HideCaret(hWnd);
	int textSize = text._Mysize;
	for (int i = 0; i < textSize; i++)
	{
		printCharOnDC(i);
	}
	SetCaretPos(caretPos.x * charSize.x, caretPos.y * charSize.y );
	ShowCaret(hWnd);
}


void ContentOfWindow::addCharToText(WORD wParam)
{
	wchar_t addedSymbol = wParam;
	int indexCaret = caretPos.x + caretPos.y * lengthLine;
	if ( text.size() == indexCaret )
	{
		text.append(&addedSymbol,1);
	}
	else
	{ 
		text.insert(indexCaret,&addedSymbol,1);
	}
	//maybe indexCaret require do field, because it's often used
	if (caretPos.x == (lengthLine-1))
	{
		caretPos.x = 0;
		caretPos.y++;
	}
	else
	{
		caretPos.x++;
	}
}

void ContentOfWindow::printCharOnDC(int index)
{
	switch (text[index])
	{
	case '\n':
		break;
	default:
		int yIndex = index / lengthLine;
		int xIndex = index % lengthLine;
		LPCWSTR currentChar = &text.c_str()[index];
		TextOut(hDC, xIndex * charSize.x, yIndex * charSize.y, currentChar, 1);
		break;
	}
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

void ContentOfWindow::calculateCaretPos(LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	caretPos.y = y / charSize.y;
	caretPos.x = x / charSize.x;
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
	endTextPos.x = text.size() % lengthLine;
	endTextPos.y = text.size() / lengthLine;
}