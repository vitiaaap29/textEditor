#include "StdAfx.h"
#include "resource.h"
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class ContentOfWindow
{
private:
	bool leftMouseButtonPressed;
	bool selectionFlag;
	bool waitingActionOnSelected;
	vector<int> indexesNewLines;
	HWND hWnd;
	HDC hDC;
	wstring text;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	POINT startForSelection;
	int shiftCaretAfterDrawing;
	RECT clientRect;
	HFONT font;
	int lengthLine;

	void addCharToText(WORD wParam);
	POINT calculateCaretPosByCoordinates(LPARAM lParam);
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
	bool caretIncludeSelectArea(POINT caretPos);
	OPENFILENAME initializeStructOpenFilename(wstring filter);
	bool deleteSelectedText();
	void drawImage(HBITMAP hBitmap, POINT start);
	int indexInTextByCaret(POINT caretPos);
	void openImage();
	POINT printCharOnDC(int indexCharInText, POINT currentPos);
	void processorVkLeft();
	void processorWkRight();
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


