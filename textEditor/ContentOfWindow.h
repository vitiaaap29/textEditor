#include "StdAfx.h"
#include <string>
#include <math.h>

using namespace std;

class ContentOfWindow
{
private:
	HWND hWnd;
	wstring text;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	HFONT font;
	int lengthLine;
	HDC hDC;
	void addCharToText(WORD wParam);
	void printCharOnDC(int index);
	void validateRectsForPaint();
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void drawText();
	void processorWmChar(WORD wParam);
	void calculateCaretPos(LPARAM lParam);
	wstring Text(){return text;}
	void workWithCaret(WORD message);

	void setSizeAreaType(LPARAM param);
	void setFont(HFONT font){this->font = font;}
};


