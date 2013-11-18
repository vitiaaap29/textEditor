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
	this->waitingActionOnSelected = false;
	this->shiftCaretAfterDrawing = 0;
	hDC = GetDC(hWnd);
	SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
	GetClientRect(hWnd, &clientRect);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

ContentOfWindow::~ContentOfWindow(void)
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
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
			waitingActionOnSelected = true;
		}
		else if (!selectionFlag)
		{
			waitingActionOnSelected = false;
		}
		currentPos =  printCharOnDC(i,currentPos);
		SetBkColor(hDC,color);
	}

	for (int i = 0; i < shiftCaretAfterDrawing; i++)
	{
		processorWkRight();
	}
	shiftCaretAfterDrawing = 0;
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
	else if (selectionFlag)
	{
		selectionFlag = false;
	}
}

void ContentOfWindow::processorArrows(WPARAM wParam) 
{
	HideCaret(hWnd);
	switch (wParam)
	{
		case VK_LEFT:
			processorVkLeft();
			break;

		case VK_RIGHT:
			processorWkRight();
			break;

		case VK_UP:
			if (caretPos.y != 0 && caretPos.y < endTextPos.y)
			{
				if (caretPos.x >= indexesNewLines[caretPos.y-1])
				{
					if (indexesNewLines[caretPos.y-1] < indexesNewLines[caretPos.y])
					{
						caretPos.y--;
						caretPos.x = indexesNewLines[caretPos.y];
					}
					else
					{
						caretPos.y--;
					}
				}
				else
				{
					caretPos.y--;	
				}
			}
			else if(caretPos.y == endTextPos.y)
			{
				if (caretPos.x >= indexesNewLines[caretPos.y-1])
				{
					if (indexesNewLines[caretPos.y-1] < endTextPos.x)
					{
						caretPos.y--;
						caretPos.x = indexesNewLines[caretPos.y];
					}
					else
					{
						caretPos.y--;
					}
				}
				else
				{
					caretPos.y--;	
				}
			}
			break;

		case VK_DOWN:
			if (caretPos.y < endTextPos.y-1)
			{
				caretPos.y++;
				if (caretPos.x > indexesNewLines[caretPos.y])
				{
					caretPos.x = indexesNewLines[caretPos.y];
				}

			}
			else if (caretPos.y < endTextPos.y)
			{
				caretPos.y++;
			}
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
			deleteSelectedText();
			text.insert(indexInTextByCaret(caretPos), &fromClipboard.at(0), fromClipboard.size());
			shiftCaretAfterDrawing = fromClipboard.size();
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case ID_CTRL_C:
		if (OpenClipboard(hWnd))
		{
			EmptyClipboard();
			int startCopyIndex = indexInTextByCaret(startForSelection);
			int endCopyIndex = indexInTextByCaret(caretPos);
			int minIndex = min(startCopyIndex,endCopyIndex);
			int difference = max(startCopyIndex,endCopyIndex) - minIndex + 1;
			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, difference * sizeof(wchar_t));
			wchar_t* lptstrCopy = (wchar_t*)GlobalLock(hglbCopy); 
			memcpy(lptstrCopy,&text[minIndex],difference * sizeof(wchar_t));
			GlobalUnlock(hglbCopy);
			SetClipboardData(CF_UNICODETEXT, hglbCopy);
			CloseClipboard();
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case ID_IMAGE_LOAD:
		openImage();
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
		if ( !deleteSelectedText() && (caretPos.x > 0 || caretPos.y != 0) )
		{
			int index = indexInTextByCaret(caretPos);
			
			if (caretPos.x != 0)
			{
				caretPos.x--;
			}
			else
			{
				caretPos.y--;
				caretPos.x = indexesNewLines[caretPos.y];
			}

			text.erase(index-1,1);
		}
		break;
	case '\t':
		deleteSelectedText();
		for (int i = 0; i < 4; i++)
		{
			addCharToText(L' ');
		}
		break;
	case '\r':
	case '\n':
		deleteSelectedText();
		addCharToText(wParam);
		break;
	default:
		deleteSelectedText();
		addCharToText(wParam);
		break;
	}
	InvalidateRect(hWnd,NULL, false);
}

void ContentOfWindow::setSizeAreaType(LPARAM param)
{
	clientSize.x = LOWORD(param);
	clientSize.y = HIWORD(param);
	GetClientRect(hWnd, &clientRect);
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
	int indexCharInText = indexInTextByCaret(caretPos);

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

bool ContentOfWindow::deleteSelectedText()
{
	bool result = false;
	if (waitingActionOnSelected)
	{
		int startDeletedIndex = indexInTextByCaret(startForSelection);
		int endDeletedIndex = indexInTextByCaret(caretPos);
		int difference = max(startDeletedIndex,endDeletedIndex) - min(startDeletedIndex,endDeletedIndex);
		text.erase(min(startDeletedIndex,endDeletedIndex), difference);
		if (startDeletedIndex < endDeletedIndex)
		{
			for (int i = 0; i < difference; i++)
			{
				processorVkLeft();
			}
		}
		selectionFlag = false;
		result = true;
	}
	return result;
}

void ContentOfWindow::drawImage(Gdiplus::Image* pImage, POINT start)
{
	RECT rect = {start.x, start.y, pImage->GetWidth(), pImage->GetHeight()};
	Gdiplus::Rect imageRect(start.x, start.y, pImage->GetWidth(), pImage->GetHeight());
	ValidateRect(hWnd, &rect);
	Gdiplus::Graphics graphics(hWnd);
	graphics.DrawImage(pImage, imageRect);
}

int ContentOfWindow::indexInTextByCaret(POINT caretPos)
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

OPENFILENAME ContentOfWindow::initializeStructOpenFilename(wchar_t *filename)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(*filename) * 256;
	ofn.lpstrFilter = NULL;//L"All\0*.*\0Image\0*.PNG\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	return ofn;
}

void ContentOfWindow::openImage()
{
	wchar_t filename[256] = {0};
	OPENFILENAME ofn = initializeStructOpenFilename(filename);
	if(GetOpenFileName(&ofn))
    {
		POINT start;
		start.x = caretPos.x * charSize.x;
		start.y = caretPos.y * charSize.y;
		Gdiplus::Image* image = Gdiplus::Image::FromFile((WCHAR*)filename);
		if (GetLastError() == ERROR_SUCCESS)
		{
			drawImage(image,start);
		}
		delete image;
    }
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

void ContentOfWindow::processorVkLeft()
{
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
}

void ContentOfWindow::processorWkRight()
{
	if (endTextPos.x != caretPos.x  || endTextPos.y !=caretPos.y)
	{
		if (caretPos.y < (int)indexesNewLines.size())
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