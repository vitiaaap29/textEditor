#include "StdAfx.h"
#include "resource.h"
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class ContentOfWindow
{
	struct LineInfo
	{
		int heigth;
		int start;
	};

private:
	static const WORD SYMBOL_SIGN_PICTURES = 0x265E;
	bool leftMouseButtonPressed;
	bool selectionFlag;
	bool waitingActionOnSelected;
	vector<POINT> indexesNewLines;
	HWND hWnd;
	HDC hDC;
	wstring text;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	POINT startForSelection;
	POINT pixelPos;
	int shiftCaretAfterDrawing;
	RECT clientRect;
	HFONT font;
	int lengthLine;
	ULONG_PTR gdiplusToken;
	vector<Gdiplus::Image*> images;

	void addCharToText(WORD wParam);
	POINT calculateCaretPosByCoordinates(LPARAM lParam);
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
	bool caretIncludeSelectArea(POINT caretPos);
	OPENFILENAME initializeStructOpenFilename(wchar_t *filename);
	bool deleteSelectedText();
	void drawImage(Gdiplus::Image* pImage, POINT start);
	int heigthLine(int startIndex);
	int indexInTextByCaret(POINT caretPos);
	void openImage();
	POINT printCharOnDC(int indexCharInText, POINT currentPos, LineInfo lineInfo);
	void processorVkLeft();
	void processorWkRight();
	POINT realLinesSize(POINT caret);
	void validateRectsForPaint();
public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void CaretPosByCoordinates(LPARAM lParam);
	void drawText();
	bool leftMouseIsPress(){return leftMouseButtonPressed;};
	void mouseSelection(WPARAM wParam, LPARAM lParam);
	void processorArrows(WPARAM wParam);
	bool processorMenuMessages(WORD id);
	void processorWmChar(WORD wParam);
	void setSizeAreaType(LPARAM param);
	void setFont(HFONT font){this->font = font;}
	void setStartForSelection(LPARAM lParam);
	void leftMouseModePress(bool mode){this->leftMouseButtonPressed = mode;}
	void workWithCaret(WORD message);
};


