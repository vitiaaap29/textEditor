#include "ContentOfWindow.h"

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->xCaretPos = 0;
	this->yCaretPos = 0;
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
		if (xCaretPos > 0 || yCaretPos != 0)
		{
			calculateLengthLine();
			index = xCaretPos + yCaretPos * lengthLine;
			text.erase(index-1,1);
			
			if (xCaretPos != 0)
			{
				xCaretPos--;
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
	xClient = LOWORD(param);
	yClient = HIWORD(param);
}

void ContentOfWindow::workWithCaret(WORD message)
{
	calculateCharSize();
	if (message == WM_SETFOCUS)
	{
		CreateCaret(hWnd,NULL, NULL,yCharSize);
		SetCaretPos(xCaretPos * xCharSize, yCaretPos * yCharSize );
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
	SetCaretPos(xCaretPos * xCharSize, yCaretPos * yCharSize );
	ShowCaret(hWnd);
}


void ContentOfWindow::addCharToText(WORD wParam)
{
	wchar_t addedSymbol = wParam;
	int indexCaret = xCaretPos + yCaretPos * lengthLine;
	if ( text.size() == indexCaret )
	{
		text.append(&addedSymbol,1);
	}
	else
	{ 
		text.insert(indexCaret,&addedSymbol,1);
	}
	//maybe indexCaret require do field, because it's often used
	if (xCaretPos == (lengthLine-1))
	{
		xCaretPos = 0;
		yCaretPos++;
	}
	else
	{
		xCaretPos++;
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
		TextOut(hDC, xIndex * xCharSize, yIndex * yCharSize, currentChar, 1);
		break;
	}
}

void ContentOfWindow::validateRectsForPaint()
{
	calculateEndTextPos();
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = xClient;
	rect.bottom = endTextPos.y * yCharSize;
	ValidateRect(hWnd, &rect);

	RECT lastLineRect;
	lastLineRect.left = 0;
	lastLineRect.top = endTextPos.y * yCharSize;
	lastLineRect.right = endTextPos.x * xCharSize;
	lastLineRect.bottom = lastLineRect.top + yCharSize;
	ValidateRect(hWnd,&lastLineRect);
}

void ContentOfWindow::calculateCaretPos(LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	yCaretPos = y / yCharSize;
	xCaretPos = x / xCharSize;
}

void ContentOfWindow::calculateCharSize()
{
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	xCharSize = tm.tmAveCharWidth;
	yCharSize = tm.tmHeight;
}

void ContentOfWindow::calculateLengthLine()
{
	lengthLine = xClient / xCharSize;
}

void ContentOfWindow::calculateEndTextPos()
{
	endTextPos.x = text.size() % lengthLine;
	endTextPos.y = text.size() / lengthLine;
}