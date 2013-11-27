#include "ContentOfWindow.h"

/*—ей код попирает правило ѕаши Ћебедева: не использовать в именах больше четырЄх слов, ибо тогда
говном обращаетс€ код.

Ќа правило Ћебедева ложитс€, ибо есть правило —урокова: "»спользуйте столько слов, сколько требуетс€
дл€ полного понимани€ сути"*/

ContentOfWindow::CharInfo::CharInfo(wchar_t symbol, HFONT* pfont, POINT size)
{
	this->symbol = symbol;
	this->pfont = pfont;
	this->size  = size;
	this->image = NULL;
}

void ContentOfWindow::CharInfo::SetImage(Gdiplus::Image* image)
{
	this->image = image;
	this->symbol = SYMBOL_SIGN_PICTURES;
	POINT imageSize = {image->GetWidth(), image->GetHeight()};
	this->size = imageSize;
}

ContentOfWindow::LineInfo::LineInfo()
{
	this->heigth = 0;
	this->lengthByX = 0;
	this->maxHeigthChar = 0;
	this->startInText = 0;
	this->upperLeftCorner = 0;
}

ContentOfWindow::ContentOfWindow(HWND hWnd)
{
	this->hWnd = hWnd;
	this->caretIndex = 0;
	this->endTextPos.x = 1;
	this->endTextPos.y = 0;
	this->leftMouseButtonPressed = false;
	this->selectionFlag = false;
	this->waitingActionOnSelected = false;
	this->shiftCaretAfterDrawing = 0;
	this->currentFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	hDC = GetDC(hWnd);
	//SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
	GetClientRect(hWnd, &clientRect);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
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
	POINT pixel = lParamToPixel(lParam);
	caretIndex = indexByCaret(pixel);
}

void ContentOfWindow::drawText()
{
	int textSize = (int)text.size();
	if (textSize > 0)
	{
		HideCaret(hWnd);
		FillRect(hDC, &clientRect, (HBRUSH) (COLOR_WINDOW+1));
		validateRectsForPaint();
	
		POINT lowLeftAngle = {0, lines.at(0).heigth};
	
		COLORREF color = GetBkColor(hDC);
		COLORREF highlightColor = RGB(0, 255, 255);
	
		for (int i = 0; i < textSize; i++)
		{
			CharInfo symbol = text.at(i);
 
			if (selectionFlag && caretIncludeSelectArea(lowLeftAngle))
			{
				SetBkColor(hDC,highlightColor);
				waitingActionOnSelected = true;
			}
			else if (!selectionFlag)
			{
				waitingActionOnSelected = false;
			}
		
			int indexLine = numberLineByIndex(i);
			lowLeftAngle = printCharOnDC(symbol, lowLeftAngle, indexLine);
		
			SetBkColor(hDC,color);
		}

		for (int i = 0; i < shiftCaretAfterDrawing; i++)
		{
			processorWkRight();
		}
		shiftCaretAfterDrawing = 0;
		POINT caret = pixelUpperCornerByIndex(caretIndex);
		SetCaretPos(caret.x, caret.y);
		ShowCaret(hWnd);
	}
}

void ContentOfWindow::mouseSelection(WPARAM wParam, LPARAM lParam)
{
	if (wParam == MK_LBUTTON)
	{
		POINT pixelCaretPos = lParamToPixel(lParam);
		int pixelIndex = indexByCaret(pixelCaretPos);
		int intervalSelectedSymbol = pixelIndex - caretIndex;
		if (intervalSelectedSymbol != 0)
		{
			selectionFlag = true;
			caretIndex = pixelIndex;
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
	calculateEndTextPos();
	switch (wParam)
	{
		case VK_LEFT:
			processorVkLeft();
			break;

		case VK_RIGHT:
			processorWkRight();
			break;

		case VK_UP:
			break;
		case VK_DOWN:
			break;
	}
	POINT caret = pixelUpperCornerByIndex(caretIndex);
	SetCaretPos(caret.x, caret.y );
	
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
			int pastedIndex = caretIndex;
			for(int i = 0; i < (int)fromClipboard.size(); i++)
			{
				if (fromClipboard.at(i) != '\n')
				{
					CharInfo pastedChar(fromClipboard.at(i),&currentFont,charSize);
					text.insert(text.begin() + pastedIndex,pastedChar);
				}
			}
			shiftCaretAfterDrawing = fromClipboard.size();
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case ID_CTRL_C:
		if (OpenClipboard(hWnd))
		{
			EmptyClipboard();
			int startCopyIndex = indexByCaret(startForSelection);
			int endCopyIndex = caretIndex;
			int minIndex = min(startCopyIndex,endCopyIndex);
			int maxIndex =  max(startCopyIndex,endCopyIndex);
			wstring copyText = new wchar_t[minIndex];
			for (int i = minIndex; i <= maxIndex; i++)
			{
				copyText[i] = text[i].GetSymbol();
			}
			int find;
			while( (find = copyText.find('\r')) != wstring::npos)
			{
				copyText.insert(copyText.begin() + find + 1,'\n');
			}
			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, copyText.size() * sizeof(copyText[0]));
			wchar_t* lptstrCopy = (wchar_t*)GlobalLock(hglbCopy); 
			memcpy(lptstrCopy,&copyText, copyText.size() * sizeof(copyText[0]));
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
		//тут ЋЄшина функци€ будет вызыватьс€
		break;
	default:
		return false;
	}
	return true;
}

void ContentOfWindow::processorWmChar(WORD wParam)
{
	int indexAccordingCaret;
	switch (wParam)
	{
	case '\b':
		indexAccordingCaret = caretIndex;
		if ( !deleteSelectedText() && indexAccordingCaret > 0 )
		{
			text.erase(text.begin() + indexAccordingCaret - 1);
			if (caretIndex > 0)
			{
				caretIndex--;
			}
			getLinesInfo();
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
	getLinesInfo();
}

void ContentOfWindow::setStartForSelection(LPARAM lParam)
{
	startForSelection = lParamToPixel(lParam);
	//возможно тут лучше нормировать
}

void ContentOfWindow::workWithCaret(WORD message)
{
	calculateCharSize();
	if (message == WM_SETFOCUS)
	{
		int heigthCaret;
		int inText = caretIndex;
		if (inText < (int)text.size())
		{
			heigthCaret = text.at(inText).GetSize().y;
		}
		else
		{
			calculateCharSize();
			heigthCaret = charSize.y;
		}
		
		CreateCaret(hWnd,NULL, NULL, heigthCaret);
		POINT caret = pixelUpperCornerByIndex(inText);
		SetCaretPos(caret.x, caret.y );
		ShowCaret(hWnd);
	}
	if (message == WM_KILLFOCUS)
	{
		HideCaret(hWnd);
		DestroyCaret();
	}
}


void ContentOfWindow::addCharToText(WORD wParam, Gdiplus::Image* image)
{
	wchar_t addedSymbol = wParam;
	CharInfo* pCharInfo; 
	getLinesInfo();
	int indexCharInText = caretIndex;
	if (addedSymbol != '\r')
	{
		pCharInfo = new CharInfo(addedSymbol,&currentFont, charSize);
	}
	else
	{
		POINT size = {0, charSize.y};
		pCharInfo = new CharInfo(addedSymbol,&currentFont, size);
	}
	if (image != NULL)
	{
		pCharInfo->SetImage(image);
	}
	ABC abc;
	int iAbc;
	bool iii = GetCharABCWidths(hDC, addedSymbol,addedSymbol, &abc);
	bool ii = GetCharWidth32W(hDC, addedSymbol, addedSymbol, &iAbc);
	int charWidth = iAbc;
	pCharInfo->SetSizeX(charWidth);
	text.insert( text.begin() + indexCharInText, *pCharInfo);
	getLinesInfo();
	processorWkRight();
}

bool ContentOfWindow::belongsPixelToLineByX(POINT pixel, LineInfo line)
{
	bool result = false;
	if (pixel.x <= line.lengthByX )
	{
		result = true;
	}
	return result;
}

bool ContentOfWindow::belongsPixelToLineByY(POINT pixel, LineInfo line)
{
	bool result = false;
	if (pixel.y >= line.upperLeftCorner && pixel.y <= line.upperLeftCorner + line.heigth )
	{
		result = true;
	}
	return result;
}

POINT ContentOfWindow::lParamToPixel(LPARAM lParam)
{
	POINT result = {0,0};
	result.x = LOWORD(lParam);
	result.y = HIWORD(lParam);
	return result;
}

void ContentOfWindow::calculateCharSize()
{
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	charSize.x = tm.tmAveCharWidth;
	charSize.y = tm.tmHeight + tm.tmExternalLeading;
}

void ContentOfWindow::calculateEndTextPos()
{
	endTextPos = pixelUpperCornerByIndex(text.size()-1);
}

bool ContentOfWindow::caretIncludeSelectArea(POINT position)
{
	bool result = false;
	int startIndex = indexByCaret(startForSelection);
	int index = indexByCaret(position);
	int min = min(startIndex, caretIndex);
	int max = max(startIndex, caretIndex);
	if (min >= index && index <= max)
	{
		result = true;
	}
	return result;
}

bool ContentOfWindow::deleteSelectedText()
{
	bool result = false;
	if (waitingActionOnSelected)
	{
		int startDeletedIndex = indexByCaret(startForSelection);
		int endDeletedIndex = caretIndex;
		int difference = max(startDeletedIndex,endDeletedIndex) - min(startDeletedIndex,endDeletedIndex);
		vector<CharInfo>::iterator it = text.begin();
		text.erase(it + min(startDeletedIndex,endDeletedIndex),it + max(startDeletedIndex,endDeletedIndex));
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

void ContentOfWindow::getLinesInfo()
{
	lines.clear();
	LineInfo line;
	POINT pixelPos = {0, 0};
	if (text.size() > 0)
	{
		line.heigth = text.at(line.startInText).GetSize().y;
		for (int i = 0; i < (int)text.size(); i++)
		{
			bool endWindow = clientSize.x - pixelPos.x < text.at(i).GetSize().x;
			if (text.at(i).GetSymbol() != '\r' && !endWindow) 
			{
				if (text.at(i).GetSize().y > line.heigth )
				{
					line.heigth = text.at(i).GetSize().y;
				}
				//возможно сей кусок не нужен, ибо по левому нижнему углу повелеваем буквой
				if (text.at(i).GetSize().y > line.maxHeigthChar && text.at(i).GetImage() == NULL)
				{
					line.maxHeigthChar = text.at(i).GetSize().y;
				}
				//до строчки сверху не нужен
				pixelPos.x += text.at(i).GetSize().x;
			}
			else if (!endWindow)
			{
				line.endInText = i;
				line.lengthByX = pixelPos.x;
				lines.push_back(line);
				pixelPos.x = 0;
				if (i+1 < (int)text.size())
				{
					line.startInText = i + 1;
					line.upperLeftCorner += line.heigth;
					line.lengthByX = 0;
					line.heigth = text.at(line.startInText).GetSize().y;
					line.maxHeigthChar = 0;
				}
			}
			else
			{
				line.endInText = i;
				line.lengthByX = pixelPos.x;
				lines.push_back(line);
				pixelPos.x = 0;

				line.startInText = i + 1;
				line.upperLeftCorner += line.heigth;
				line.lengthByX = 0;
				if ( i + 1 < text.size())
				{
					line.heigth = text.at(line.startInText).GetSize().y;
				}
				else
				{
					line.heigth = charSize.y;
				}
				line.maxHeigthChar = 0;
			}
		}
		
		if (text.at(text.size() - 1).GetSymbol() != '\r')
		{
			line.endInText = text.size() - 1;
			line.lengthByX = pixelPos.x;
			lines.push_back(line);
		}
	}
	else
	{
		line.endInText = 0;
		lines.push_back(line);
	}

}

/*
определ€ет индекс в тексте по положению каретки - пикселю
*/
int ContentOfWindow::indexByCaret(POINT caretPos)
{
	int index = -1;
	if (!lines.empty())
	{
		int textSize = text.size();
		int lineSize = (int) lines.size();
		int indexLine = 0;
		for(int indexLine = 0; indexLine < lineSize && index == -1; indexLine++)
		{
			LineInfo line = lines.at(indexLine);
			if ( belongsPixelToLineByY(caretPos, line))
			{
				if ( belongsPixelToLineByX(caretPos, line))
				{
					int a = 0;
					int b = a;
					for (int i = line.startInText; i < line.endInText ;i++)
					{
						b = a + text.at(i).GetSize().x;
						if (caretPos.x >= a && caretPos.x < b )
						{
							index = i;
							break;
						}
						a = b;
					}
				}
				else
				{
					if (indexLine < lineSize - 1)
					{
						index = line.endInText;
					}
					else
					{
						index = text.size();
					}
				}
			}
		}
	}
	// если всЄ это не заработает, то можно просто пройтись по lines
	//ло нужной строки, а потом по буквам
	if (index == -1)
	{
		index = text.size();
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
	ofn.lpstrFilter = L"Image\0*.bmp;*.jpg\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	return ofn;
}

/*
ѕринадлежит ли пиксель букве, характеризуемой положением пиксел€ нижнего левого угла буквы.
*/
bool ContentOfWindow::isPixelBelongsChar(POINT pixel, POINT pixelChar,CharInfo charInfo)
{
	bool result =  false;
	bool includeOnY = pixel.y < pixelChar.y && pixel.y >= pixelChar.y - charInfo.GetSize().y;
	bool includeOnX = pixel.x >= pixelChar.x && pixel.x < pixelChar.x + charInfo.GetSize().x;
	if (includeOnX && includeOnY)
	{
		result = true;
	}
	return result;
}

int ContentOfWindow::numberLineByIndex(int index)
{
	int result = 0;
	int currentStartLine = lines.at(0).startInText;
	for (int i = 0; i < (int)lines.size() && currentStartLine < index; i++)
	{
		result = currentStartLine;
		currentStartLine = lines.at(0).startInText;
	}
	return result;
}

/*
¬ыравнивает пиксель по верхнему левому углу воображаемого пр€моугольника, характеризуещего
букву, которой соответствует пиксель (которой соответствует каретка определ€ема€ этим пикселем). 
*/
POINT ContentOfWindow::normedByUpperCorner(POINT pixel)
{
	return pixelUpperCornerByIndex((indexByCaret(pixel)));
}

/*
Ћевый нижний угол буквы индексом index
*/
POINT ContentOfWindow::pixelLowerCornerByIndex(int index)
{
	POINT result = {0, charSize.y};
	if (!lines.empty())
	{
		result.y = lines.at(0).heigth;
		int textSize = text.size();
		int lineSize = lines.size();
		bool isDefine = false;
		for (int i = 0; i < lineSize && !isDefine; i++)
		{
			LineInfo line = lines.at(i);
			if (index >= line.startInText && index <= line.endInText + 1)
			{
				result.y = line.upperLeftCorner + line.heigth;
				//просто меньше, ибо размер самой буквы с индексом index не надо добавл€ть
				for (int j = line.startInText; j < index; j++)
				{
					if (j < textSize)
					{
						result.x += text.at(j).GetSize().x;
					}
					else
					{
						//or result.x = lines.at(lines.size() - 1).GetSize().x;
						result.x += charSize.x;
					}
				}
				isDefine = true;
				if ( (index - 1) < text.size() && text.at(index - 1).GetSymbol() == '\r' )
				{
					result.x = 0;
					if (i + 1 < lineSize)
					{
						result.y += lines.at(i + 1).heigth;
					}
					else
					{
						result.y += charSize.y;
					}
				}
			}
		}
	}
	return result;
}

POINT ContentOfWindow::pixelUpperCornerByIndex(int index)
{
	int textSize = (int)text.size();
	POINT result = pixelLowerCornerByIndex(index);
	if ( index < textSize && index > 0)
	{
		int number = numberLineByIndex(index);
		result.y -= text.at(index).GetSize().y;
	}
	else
	{
		if (index == 0 && textSize > index + 1)
		{
			result.y -= text.at(index + 1).GetSize().y;
		}
		else
		{
			result.y -= charSize.y;
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
		Gdiplus::Image* image = Gdiplus::Image::FromFile((WCHAR*)filename);
		if (GetLastError() == ERROR_SUCCESS)
		{
			images.push_back(image);
			addCharToText(SYMBOL_SIGN_PICTURES, image);
			drawText();
		}
    }
}

POINT ContentOfWindow::printCharOnDC(CharInfo symbol, POINT lowLeftAngle,  int indexLine)
{
	LineInfo lineInfo = lines.at(indexLine);
	wchar_t printedCh= symbol.GetSymbol();
	LPCWSTR printedChar = static_cast<LPCWSTR>(&printedCh);
	if (symbol.GetSymbol() != '\r')
	{
		if (symbol.GetImage() == NULL)
		{
			TextOut(hDC, lowLeftAngle.x, lowLeftAngle.y - lineInfo.maxHeigthChar, printedChar, 1);
		}
		else
		{
			POINT start = {lowLeftAngle.x, lowLeftAngle.y - symbol.GetSize().y};
			drawImage(symbol.GetImage(), start);
		}
		lowLeftAngle.x += symbol.GetSize().x;
	}
	else
	{
		indexLine++;
		lowLeftAngle.x = 0;
		if (indexLine < lines.size())
		{
			lineInfo = lines.at(indexLine);
			lowLeftAngle.y += lineInfo.heigth;
		}
		else
		{
			lowLeftAngle.y += lineInfo.maxHeigthChar;
		}
	}
	return lowLeftAngle;
}

void ContentOfWindow::processorVkLeft()
{
	if (caretIndex < text.size())
	{
		caretIndex--;
	}
}

void ContentOfWindow::processorWkRight()
{
	if (caretIndex >= 0)
	{
		caretIndex++;
	}
}

void ContentOfWindow::validateRectsForPaint()
{
	LineInfo lastLine = lines.at(lines.size() - 1);
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = clientSize.x;
	rect.bottom = lastLine.upperLeftCorner; 
	ValidateRect(hWnd, &rect);
	calculateEndTextPos();
	RECT lastLineRect;
	lastLineRect.left = 0;
	lastLineRect.top = lastLine.upperLeftCorner;
	lastLineRect.right = lastLine.lengthByX ;
	lastLineRect.bottom = lastLineRect.top + lastLine.heigth;
	ValidateRect(hWnd,&lastLineRect);
}

bool operator==(POINT a, POINT b)
{
	bool result = false;
	if (a.x == a.y && b.x == b.y)
	{
		result = true;
	}
	return result;
}