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
	vector<int> indexesNewLines;
	HWND hWnd;
	HDC hDC;
	wstring text;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	RECT clientRect;
	HFONT font;
	int lengthLine;

	void addCharToText(WORD wParam);
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
	int indexInTextByCaret();
	int posXInline(int posY);
	POINT printCharOnDC(int indexCharInText, POINT currentPos);
	void validateRectsForPaint();
public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void calulateCaretPosByCoordinates(LPARAM lParam);
	void drawText();
	void processorArrows(WORD wParam);
	bool processorMenuMessages(WORD id);
	void processorWmChar(WORD wParam);
	void setSizeAreaType(LPARAM param);
	void setFont(HFONT font){this->font = font;}
	void workWithCaret(WORD message);
	void leftline(WORD lParam);
};


