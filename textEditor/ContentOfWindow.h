#include "StdAfx.h"
#include <string>
#include <math.h>
#include <vector>

using namespace std;

class ContentOfWindow
{
private:
	HWND hWnd;
	wstring text;
	vector<int> vectorIndexesNewLines;
	vector<int>::iterator iteratorIndexes;
	bool autoMoveNextlineFlag;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	POINT currentPos;
	HFONT font;
	int lengthLine;
	HDC hDC;
	void addCharToText(WORD wParam);
	void printCharOnDC(int index);
	void validateRectsForPaint();
	void calculateCharSize();
	void calculateLengthLine();
	void calculateEndTextPos();
	int indexCharByLinesLength();
	void incrementIndexesNewLines(int start, int end);
	void autoNewLine();
public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void drawText();
	void processorWmChar(WORD wParam);
	void calulateCaretPosByCoordinates(LPARAM lParam);
	wstring Text(){return text;}
	void workWithCaret(WORD message);

	void setSizeAreaType(LPARAM param);
	void setFont(HFONT font){this->font = font;}
};


