#include "ContentOfWindow.h"

ContentOfWindow::CharInfo::CharInfo(HFONT* pfont)
{
	this->pfont = pfont;
}

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
	// init fonts
}

ContentOfWindow::~ContentOfWindow(void)
{
	for (int i = 0; i < (int)images.size(); i++)
	{
		delete images.at(i);
	}
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
	
	pixelPos.x = 0;
	pixelPos.y = 0;
	POINT currentPos;
	currentPos.x = 0;
	currentPos.y = 0;
	int textSize = text._Mysize;

	indexesNewLines.clear();
	COLORREF color = GetBkColor(hDC);
	COLORREF highlightColor = RGB(0, 255, 255);
	LineInfo line;
	line.start = -charSize.y;
	int oldNumberLine = -1;
	for (int i = 0; i < textSize; i++)
	{
		if (oldNumberLine != currentPos.y)
		{
			oldNumberLine = currentPos.y;
			line.heigth = heigthLine(i);
			line.start += line.heigth;
		}

		if (selectionFlag && caretIncludeSelectArea(currentPos))
		{
			SetBkColor(hDC,highlightColor);
			waitingActionOnSelected = true;
		}
		else if (!selectionFlag)
		{
			waitingActionOnSelected = false;
		}

		if (text[i] != SYMBOL_SIGN_PICTURES)
		{
			currentPos =  printCharOnDC(i,currentPos, line);
		}
		else
		{
			int indexImage = text[++i] - '0';
			if ( indexImage < (int)images.size() )
			{
				if (pixelPos.x + images[indexImage]->GetWidth() < clientSize.x )
				{
					currentPos.x += 2;
				}
				else
				{
					pixelPos.x = 0;
					pixelPos.y + line.heigth;
					POINT newlineInfo = {currentPos.x, line.heigth};
					indexesNewLines.push_back(newlineInfo);
					currentPos.x = 1;
					currentPos.y++;
				}
				POINT upperLeftCorner = {pixelPos.x, line.start + charSize.y - line.heigth};
				drawImage(images[indexImage], upperLeftCorner);
				pixelPos.x += images[indexImage]->GetWidth();
				pixelPos.y += images[indexImage]->GetWidth();
			}
		}
		SetBkColor(hDC,color);
	}

	for (int i = 0; i < shiftCaretAfterDrawing; i++)
	{
		processorWkRight();
	}
	shiftCaretAfterDrawing = 0;

	POINT pixelCaret = pixelByIndex(indexInTextByCaret(caretPos));
	SetCaretPos(pixelCaret.x, pixelCaret.y);
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
				if (caretPos.x >= indexesNewLines[caretPos.y-1].x)
				{
					if (indexesNewLines[caretPos.y-1].x < indexesNewLines[caretPos.y].x)
					{
						caretPos.y--;
						caretPos.x = indexesNewLines[caretPos.y].x;
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
				if (caretPos.x >= indexesNewLines[caretPos.y-1].x)
				{
					if (indexesNewLines[caretPos.y-1].x < endTextPos.x)
					{
						caretPos.y--;
						caretPos.x = indexesNewLines[caretPos.y].x;
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
				if (caretPos.x > indexesNewLines[caretPos.y].x)
				{
					caretPos.x = indexesNewLines[caretPos.y].x;
				}

			}
			else if (caretPos.y < endTextPos.y)
			{
				caretPos.y++;
			}
			break;
	}
	POINT pixelCaret = pixelByIndex(indexInTextByCaret(caretPos));
	SetCaretPos(pixelCaret.x, pixelCaret.y );
	
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
			CharInfo chInfo(&font);
			//chars.insert(
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
				caretPos.x = indexesNewLines[caretPos.y].x;
			}

			text.erase(index-1,1);
			chars.erase(chars.begin() + index - 1);
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
	CharInfo charInfo;
	if ( text.size() == indexCharInText)
	{
		text.append(&addedSymbol,1);
		CharInfo charInfo(&font);
		chars.push_back(charInfo);
	}
	else
	{ 
		text.insert(indexCharInText,&addedSymbol,1);
		CharInfo charInfo(&font);
		chars.insert(chars.begin() + indexCharInText, charInfo);
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
	POINT result = {0,0};
	POINT pixelPos;
	pixelPos.x = LOWORD(lParam);
	pixelPos.y = HIWORD(lParam);
	POINT currentPixelPos = {0,0};
	if (!images.empty())
	{
		for(int i = 0; i < (int)text.size() && ( pixelPos.y != currentPixelPos.y && pixelPos.x != currentPixelPos.x); i++)
		{
			if (text.at(i) != SYMBOL_SIGN_PICTURES)
			{
				if (text[i] == '\r' || result.x == lengthLine)
				{
					result.x = 0;
					result.y++;
					currentPixelPos.x = 0;
					currentPixelPos.y += heigthLine(i);
				}
				else
				{
					result.x++;
					currentPixelPos.x += charSize.x;
				}
			}
			else
			{
				int indexImage = text[++i] - '0';
				if ( indexImage < (int)images.size() )
				{
					currentPixelPos.x += images[indexImage]->GetWidth();
					if (currentPixelPos.x > clientSize.x  && images[indexImage]->GetWidth() < clientSize.x)
					{
						result.x = 1;
						result.y++;
						currentPixelPos.x = images[indexImage]->GetWidth();
						currentPixelPos.y += heigthLine(i+1);
					}
				}
			}
		}
	}
	else
	{
		result.y = pixelPos.y / charSize.y;
		result.x = pixelPos.x / charSize.x;
	}

	if (result.y < indexesNewLines.size() && result.x > indexesNewLines[result.y].x)
	{
		result.x = indexesNewLines[result.y].x;
	}
	else if (result.y > endTextPos.y || (result.y == endTextPos.y && result.x > endTextPos.x))
	{
		result = endTextPos;
	}

	return result;
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

POINT ContentOfWindow::caretByPixel(POINT pixel)
{
	return calculateCaretPosByCoordinates(MAKELPARAM(pixel.x, pixel.y));
}

void ContentOfWindow::calculateEndTextPos()
{
	POINT pixelEndTextPos = pixelByIndex(text.size()-1);
	endTextPos = caretByPixel(pixelEndTextPos);
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
		chars.erase(chars.begin() + min(startDeletedIndex,endDeletedIndex),chars.begin() + max(startDeletedIndex,endDeletedIndex));
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

int ContentOfWindow::heigthLine(int startIndex)
{
	if (text[startIndex] == '\r')
	{
		if (startIndex >= text.size() - 1)
		{
			return charSize.y;
		}
		else
		{
			startIndex++;
		}
	}
	int result = charSize.y;
	int foundSignImage = text.find(SYMBOL_SIGN_PICTURES, startIndex);
	if (foundSignImage != wstring::npos)
	{
		int foundNewline = text.find('\r', startIndex);
		if ( (foundNewline != wstring::npos && foundNewline > foundSignImage) || (foundNewline == wstring::npos) )
		{
			int yCurrentPos = 0;
			int xInPixel = clientSize.x;
			int maxHeigth = charSize.x;
			int i = startIndex;
			int size = (int)text.size();
			bool signEndLine = xInPixel > charSize.x && (i < foundNewline || foundNewline == wstring::npos) && i < size;
			for (; signEndLine; i++)
			{
				if (text[i] != SYMBOL_SIGN_PICTURES)
				{
					xInPixel -= charSize.x;
				}
				else
				{
					int indexImage = text[++i] - '0'; 
					if (indexImage < (int)images.size())
					{
						xInPixel -= images[indexImage]->GetWidth();
						if (xInPixel > 0)
						{
							if (maxHeigth < images[indexImage]->GetHeight())
							{
								result = images[indexImage]->GetHeight();
								maxHeigth = result;
							}
						}
						else
						{
							result = maxHeigth;
						}
					}
				}
				signEndLine = xInPixel > charSize.x && (i < foundNewline || foundNewline == wstring::npos) && i < size;
			}
		}
	}
	return result;
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
	ofn.lpstrFilter = L"Image\0*.bmp\0*.jpg\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	return ofn;
}

POINT ContentOfWindow::pixelByIndex(int index)
{
	POINT result = {0,0};
	int heigth = heigthLine(0);
	int textSize = text.size();
	for (int i = 0; i < textSize && i <= index; i++)
	{
		if (text[i] != SYMBOL_SIGN_PICTURES)
		{
			if (text[i] != '\r' && clientSize.x - result.x >= charSize.x)
			{
				result.x += charSize.x;
			}
			else
			{
				heigth = heigthLine(i + 1);
				result.y += heigth;
				result.x = 0;
			}
		}
		else
		{
			int indexImage = text[++i] - '0';
			if ( indexImage < (int)images.size() )
			{
				if (result.x + images[indexImage]->GetWidth() < clientSize.x )
				{
					LPARAM pseudoLparam = MAKELPARAM(result.x, result.y);
					result.x += images[indexImage]->GetWidth();
					POINT caretStartImage = calculateCaretPosByCoordinates(pseudoLparam);
					result.y += heigthLine(indexInTextByCaret(caretStartImage) - caretStartImage.x) - charSize.y;
				}
				else
				{
					result.x = 0;
					result.y += images[indexImage]->GetHeight();
				}
			}
		}
	}

	return result;
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
			images.push_back(image);
			addCharToText(SYMBOL_SIGN_PICTURES);
			addCharToText((WORD)(images.size() - 1 + '0'));
			drawText();
		}
    }
}

POINT ContentOfWindow::printCharOnDC(int indexCharInText, POINT currentPos, LineInfo lineInfo)
{
	LPCWSTR printedChar = &text.c_str()[indexCharInText];
	POINT line;
	wchar_t ch = text.at(indexCharInText);
	if (ch != '\r' && currentPos.x != lengthLine)
	{
		TextOut(hDC, pixelPos.x, lineInfo.start, printedChar, 1);
		currentPos.x++;
		pixelPos.x += charSize.x;
	}	
	else if (currentPos.x == lengthLine)
	{
		TextOut(hDC, pixelPos.x, lineInfo.start, printedChar, 1);
		line.x = currentPos.x;
		line.y = lineInfo.start;
		indexesNewLines.push_back(line);
		currentPos.x = 0;
		currentPos.y++;
		pixelPos.x = 0;
	}
	else
	{
		line.x = currentPos.x;
		line.y = lineInfo.start;
		indexesNewLines.push_back(line);
		currentPos.x = 0;
		currentPos.y++;
		pixelPos.x = 0;
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
			caretPos.x = indexesNewLines[caretPos.y].x;
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
			if (caretPos.x == indexesNewLines[caretPos.y].x)
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
	POINT pixelEndCaret = pixelByIndex(text.size()-1);
	calculateEndTextPos();
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = clientSize.x;
	rect.bottom = pixelEndCaret.y;
	ValidateRect(hWnd, &rect);

	RECT lastLineRect;
	lastLineRect.left = 0;
	lastLineRect.top = pixelEndCaret.y;
	lastLineRect.right = pixelEndCaret.x;
	lastLineRect.bottom = lastLineRect.top + charSize.y;
	ValidateRect(hWnd,&lastLineRect);
}