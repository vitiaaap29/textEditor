#include "ContentOfWindow.h"

/*—ей код попирает правило ѕаши Ћебедева: не использовать в именах больше четырЄх слов, ибо тогда
говном обращаетс€ код.

Ќа правило Ћебедева ложитс€, ибо есть правило —урокова: "»спользуйте столько слов, сколько требуетс€
дл€ полного понимани€ сути"*/

ContentOfWindow::CharInfo::CharInfo(wchar_t symbol, HFONT font, POINT size)
{
	this->symbol = symbol;
	this->font = font;
	this->size  = size;
	this->image = NULL;
}

void ContentOfWindow::CharInfo::SetImage(Gdiplus::Image* image, int numberImage)
{
	this->image = image;
	this->symbol = SYMBOL_SIGN_PICTURES;
	POINT imageSize = {image->GetWidth(), image->GetHeight()};
	this->size = imageSize;
	this->numberImage = numberImage;
}

bool ContentOfWindow::CharInfo::operator==(CharInfo chInfo)
{
	bool result = false;
	if (this->GetSymbol() == chInfo.GetSymbol() && this->GetSymbol() == chInfo.GetSymbol())
	{
		result = true;
	}
	return result;
}

bool ContentOfWindow::CharInfo::operator<(CharInfo chInfo)
{
	bool result = false;
	int difference = this->GetSymbol() -chInfo.GetSymbol();
	if ( difference < 0)
	{
		result = true;
	}
	return result;
}

//ContentOfWindow::SaverText::SaverText(int countSymbols, int countImages, vector<Gdiplus::Image*> images, vector<CharInfo> text)
//{
//	this->countSymbols = countSymbols;
//	this->countImages = countImages;
//	this->images = images;
//	this->text = text;
//}

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
		int oldLowLeftAngleY = lowLeftAngle.y;
		COLORREF color = GetBkColor(hDC);
		COLORREF highlightColor = RGB(0, 255, 255);
	
		int indexLine = 0;
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
		
			if (oldLowLeftAngleY != lowLeftAngle.y)
			{
				indexLine++;
			}
			oldLowLeftAngleY = lowLeftAngle.y;
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
					CharInfo pastedChar(fromClipboard.at(i), currentFont,charSize);
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
		save();
		break;
	case ID_OPEN_FILE:
		open();
		break;
	case ID_FONT:
		//тут вызов метода ћакса
		/*
		метод будет готовить все нужные структуры, вызывать диалог выбора шрифтов
		и ложить выбранный шрифт в currentFont; возможно надо сделать и изменение размера
		*/
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

void ContentOfWindow::recoveryImagesAddress()
{
	int textSize = text.size();
	for (int i = 0; i < textSize; i++)
	{
		if (text.at(i).GetSymbol() == SYMBOL_SIGN_PICTURES)
		{
			int indexImage = text.at(i).numberImage;
			text.at(i).SetImage(images.at(indexImage), indexImage);
		}
	}
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
		pCharInfo = new CharInfo(addedSymbol, currentFont, charSize);
	}
	else
	{
		POINT size = {0, charSize.y};
		pCharInfo = new CharInfo(addedSymbol, currentFont, size);
	}
	if (image == NULL)
	{
		ABC abc;
		int charWidth;
		int iAbc;
		if (GetCharABCWidths(hDC, addedSymbol,addedSymbol, &abc) )
		{
			charWidth = abc.abcA + abc.abcB + abc.abcC;
		}
		else if (GetCharWidth32W(hDC, addedSymbol, addedSymbol, &iAbc))
		{
			charWidth = iAbc;
		}
		pCharInfo->SetSizeX(charWidth);
	}
	else
	{
		pCharInfo->SetImageNumber(images.size() - 1);
		pCharInfo->SetImage(image, images.size() - 1);
	}
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

void ContentOfWindow::changeFontText(int pos1,int pos2, HFONT font)
{
	for (int i = pos1; i< pos2; i++)
	{
		text[i].font = font;
	}
}

int ContentOfWindow::ChangeFont()
{
	CHOOSEFONT cf;            // common dialog box structure
	static LOGFONT lf;        // logical font structure
	static DWORD rgbCurrent;  // current text color
	HFONT hfont, hfontPrev;
	DWORD rgbPrev;
	// Initialize CHOOSEFONT
	ZeroMemory(&cf, sizeof(cf));
	ZeroMemory(&lf, sizeof(lf));
	cf.lStructSize = sizeof (cf);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = rgbCurrent;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS;
	if (ChooseFont(&cf)==TRUE)
	{
		hfont = CreateFontIndirect(cf.lpLogFont);
		currentFont = hfont;
	}

	int startIndex = indexByCaret(startForSelection);
	if (startIndex != caretIndex)
	{
		changeFontText(min(startIndex,caretIndex), max(startIndex,caretIndex),currentFont);
	}
	InvalidateRect(hWnd, NULL, TRUE);
	return 1;
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

long ContentOfWindow::fileSize(FILE* f)
{
	long currentPos = ftell(f);
	fseek(f, 0, SEEK_END);
	int result = ftell(f);
	fseek(f, currentPos,SEEK_SET);
	return result;
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
			bool endWindow = false;
			if (i + 1 < (int)text.size())
			{
				endWindow = clientSize.x - pixelPos.x - text.at(i).GetSize().x < text.at(i+1).GetSize().x;
			}
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
				pixelPos.x += text.at(i).GetSize().x;
				line.lengthByX = pixelPos.x;
				lines.push_back(line);
				pixelPos.x = 0;

				line.startInText = i + 1;
				line.upperLeftCorner += line.heigth;
				line.lengthByX = 0;
				if ( i + 1 < (int)text.size())
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

OPENFILENAME ContentOfWindow::initializeStructOpenFilename(wchar_t *filename, wchar_t* filter)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(*filename) * 256;
	ofn.lpstrFilter = filter;L"Image\0*.bmp;*.jpg\0";
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
	LineInfo line = lines.at(result);
	for(;result < lines.size(); result++)
	{
		if (index >= line.startInText && index <= line.endInText)
		{
			if (result + 1 < lines.size())
			{
				line = lines.at(result + 1);
				if (index == line.startInText)
				{
					result++;
				}
			}
			break;
		}
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

void ContentOfWindow::open()
{
	wchar_t filename[256] = {0};
	OPENFILENAME ofn = initializeStructOpenFilename(filename, L"0*.linux\0\0");
	if(GetOpenFileName(&ofn))
    {
		setContentFromFile(filename);
		InvalidateRect(hWnd, NULL, true);
    }
}

void ContentOfWindow::openImage()
{
	wchar_t filename[256] = {0};
	OPENFILENAME ofn = initializeStructOpenFilename(filename, L"Image\0*.bmp;*.jpg\0\0");
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
		if (lowLeftAngle.x == lineInfo.lengthByX)
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

void ContentOfWindow::save()
{
	wchar_t filename[256] = {0};
	OPENFILENAME ofn = initializeStructOpenFilename(filename, L"Content\0*.linux\0\0");
	if(GetSaveFileName(&ofn))
    {
		SaverText saver;
		saver.countImages = images.size();
		saver.countSymbols = text.size();
		saver.sizeInBytes = 0;
		FILE *f = _wfopen(filename,L"wb");
		if (f != NULL)
		{
			wchar_t sign = SYMBOL_SIGN_PICTURES;
			fwrite(&sign, sizeof(sign), 1, f);
			fwrite(&saver.sizeInBytes, sizeof(saver.sizeInBytes), 1, f);
			fwrite(&(saver.countImages),sizeof(int),1,f);
			fwrite(&(saver.countSymbols), sizeof(int), 1, f);
			
			for (int i = 0; i < saver.countSymbols; i++)
			{
				fwrite(&text.at(i), sizeof(CharInfo), 1, f);
			}

			for (int i = 0; i < images.size(); i++)
			{
				POINT imageSize = {images.at(i)->GetWidth(), images.at(i)->GetHeight()};
				Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(imageSize.x, imageSize.y,PixelFormat32bppRGB);
				Graphics graphic(bitmap);
				//draw image on bitmap
				graphic.DrawImage(images.at(i),0,0,imageSize.x, imageSize.y);

				BitmapData bmpData;
				Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
				bitmap->LockBits(&rect,
					Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
					bitmap->GetPixelFormat(), &bmpData);
				int currentImageSize = bmpData.Height * abs(bmpData.Stride);

				//write size in bytes current image
				fwrite(&currentImageSize,sizeof(currentImageSize),1 ,f);
				//write width and heigth
				POINT bitmapSize = {bitmap->GetWidth(), bitmap->GetHeight()};
				fwrite(&bitmapSize, sizeof(imageSize), 1, f);

				byte* bufferImage = new byte[currentImageSize];
				//self bitmap to file
				fwrite(bmpData.Scan0, currentImageSize, 1, f);
				bitmap->UnlockBits(&bmpData);
				delete[] bufferImage;
			}
			saver.sizeInBytes = ftell(f);
			fseek(f, sizeof(sign),SEEK_SET);
			fwrite(&(saver.sizeInBytes),sizeof(saver.sizeInBytes), 1, f);
		}

		fclose(f);
    }
}

void ContentOfWindow::setContentFromFile(wchar_t* filename)
{
	SaverText opener;
	FILE *f = _wfopen(filename,L"r+b");
	if (f != NULL)
	{
		wchar_t sign;
		fread(&sign, sizeof(sign), 1, f);
		fread(&opener.sizeInBytes, sizeof(opener.sizeInBytes), 1, f);
		int realSize = fileSize(f);
		if (sign == SYMBOL_SIGN_PICTURES && realSize == opener.sizeInBytes)
		{
			images.clear();
			fread(&(opener.countImages),sizeof(int),1,f);
			fread(&(opener.countSymbols), sizeof(int), 1, f);
			text.clear();
			for (int i = 0; i < opener.countSymbols; i++)
			{
				CharInfo chInfo;
				fread(&chInfo, sizeof(chInfo), 1, f);
				text.push_back(chInfo);
			}
			caretIndex = 0;
			
			for (int i = 0; i < opener.countImages; i++)
			{
				int sizeImage;
				fread(&sizeImage, sizeof(sizeImage), 1, f);
				POINT size;
				fread(&size, sizeof(size), 1, f);
				byte *bytesImage = new byte[sizeImage];
				fread(bytesImage, sizeImage, 1, f);

				int stride = 4 * size.x;
				Bitmap *bitmap = new Bitmap(size.x, size.y, stride, PixelFormat32bppRGB, bytesImage);
				images.push_back(bitmap);
			}

			recoveryImagesAddress();
			getLinesInfo();
		}
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