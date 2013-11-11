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
	HWND hWnd;
	HDC hDC;
	wstring text;
	vector<int> vectorIndexesNewLines;
	bool autoMoveNextlineFlag;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	RECT clientRect;
	HFONT font;
	int lengthLine;

	void addCharToText(WORD wParam);
	void autoNewLine();
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
	void changeIndexesNewLines(int start, int additional);
	int indexInTextByCaret();
	POINT printCharOnDC(int indexCharInText, POINT currentPos);
	void validateRectsForPaint();
public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void calulateCaretPosByCoordinates(LPARAM lParam);
	void drawText();
	bool processorMenuMessages(WORD id);
	void processorWmChar(WORD wParam);
	void setSizeAreaType(LPARAM param);
	void setFont(HFONT font){this->font = font;}
	void workWithCaret(WORD message);
};


