#include "ContentOfWindow.h"

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->caretPos.x = 0;
	this->caretPos.y = 0;
	this->endTextPos.x = 1;
	this->endTextPos.y = 0;
	this->text;
	//this->iteratorIndexes = vectorIndexesNewLines.begin();
	this->autoMoveNextlineFlag = true;
	hDC = GetDC(hWnd);
	SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
}

ContentOfWindow::~ContentOfWindow(void)
{
	ReleaseDC(hWnd,hDC);
}

void ContentOfWindow::processorWmChar(WORD wParam)
{
	calculateLengthLine();
	switch (wParam)
	{
	case '\b':
		if (caretPos.x > 0 || caretPos.y != 0)
		{
			int index = indexCharByLinesLength();
			
			if (caretPos.x != 0)
			{
				caretPos.x--;
			}
			else
			{
				caretPos.y--;
				caretPos.x = vectorIndexesNewLines.at(caretPos.y);
			}

			if (text[index-1] == '\n')
			{
				vector<int>::iterator it = vectorIndexesNewLines.begin();
				vectorIndexesNewLines.erase(it + caretPos.y);
			}
			text.erase(index-1,1);
		}
		break;
	case '\t':
		for (int i = 0; i < 4; i++)
		{
			addCharToText(wParam);
		}
		break;
	case '\n':
		addCharToText(wParam);
		break;
	default:
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
	currentPos.x = 0;
	currentPos.y = 0;
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
	autoNewLine();
	wchar_t addedSymbol = wParam;
	int indexCharInText = indexCharByLinesLength();
	incrementIndexesNewLines(caretPos.y, vectorIndexesNewLines.size());

	if ( text.size() == indexCharInText)
	{
		text.append(&addedSymbol,1);
	}
	else
	{ 
		text.insert(indexCharInText,&addedSymbol,1);
	}

	if (addedSymbol == '\n')
	{
		vectorIndexesNewLines.insert(vectorIndexesNewLines.begin() + caretPos.y,indexCharInText);
		incrementIndexesNewLines(caretPos.y + 1, vectorIndexesNewLines.size());
		caretPos.y++;
		caretPos.x = 0;
	}
	else if (caretPos.x < lengthLine)
	{
		caretPos.x++;
	}
}

void ContentOfWindow::printCharOnDC(int index)
{
	LPCWSTR currentChar;
	switch (text[index])
	{
	case '\n':
		currentPos.x = 0;
		currentPos.y++;
		break;
	default:
		currentChar = &text.c_str()[index];
		TextOut(hDC, currentPos.x * charSize.x, currentPos.y * charSize.y, currentChar, 1);
		if (lengthLine > currentPos.x)
		{
			currentPos.x++;
		}
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

void ContentOfWindow::calulateCaretPosByCoordinates(LPARAM lParam)
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

int ContentOfWindow::indexCharByLinesLength()
{
	int indexCharInText;
	if (caretPos.y < (int)vectorIndexesNewLines.size())
	{
		indexCharInText = vectorIndexesNewLines[caretPos.y-1] + caretPos.x + 1;
	}
	else if (caretPos.y == vectorIndexesNewLines.size())
	{
		if (caretPos.y != 0)
		{
			indexCharInText = vectorIndexesNewLines[caretPos.y-1] + caretPos.x + 1;
		}
		else
		{
			indexCharInText = caretPos.x;
		}
	}
	return indexCharInText;
}

void ContentOfWindow::autoNewLine()
{
	static bool flagGo = true;
	calculateLengthLine();
	if (autoMoveNextlineFlag && caretPos.x == lengthLine && flagGo)
	{
		flagGo = false;
		addCharToText('\n');
		flagGo = true;
	}
}

void ContentOfWindow::incrementIndexesNewLines(int start, int end)
{
	if (end <= (int)vectorIndexesNewLines.size())
	{
		for (int i = start; i < end; i++)
		{
			vectorIndexesNewLines[i]++;
		}
	}
}

void ContentOfWindow::calculateEndTextPos()
{
	if (vectorIndexesNewLines.size() == 0)
	{
		endTextPos.y = 0;
		endTextPos.x = text.size();
	}
	else if (text.size() == vectorIndexesNewLines.back() + 1)
	{
		endTextPos.y = vectorIndexesNewLines.size();
		endTextPos.x = 0;
	}
	else
	{
		endTextPos.y = vectorIndexesNewLines.size();
		endTextPos.x = text.size() - (vectorIndexesNewLines.back() + 1);
	}
}