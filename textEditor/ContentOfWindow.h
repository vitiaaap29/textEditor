#include "StdAfx.h"
#include "resource.h"
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class ContentOfWindow
{
	struct CharInfo
	{
	private:
		HFONT* pfont;
		//POINT pixelPos;
		POINT size;
		Gdiplus::Image* image;
		wchar_t symbol;
	public:
		CharInfo();
		CharInfo(wchar_t symbol, HFONT* pfont, POINT size);
		wchar_t GetSymbol(){return symbol; }
		const POINT GetSize(){return size;}
		Gdiplus::Image* GetImage(){return image;}
		void SetImage(Gdiplus::Image* image);
	};

	struct LineInfo
	{
		int heigth;
		int maxHeigthChar;
		int upperLeftCorner;
		int lengthByX;
		int startInText;
		int endInText;
	public:
		LineInfo();
	};

private:
	static const WORD SYMBOL_SIGN_PICTURES = 0x265E;
	bool leftMouseButtonPressed;
	bool selectionFlag;
	bool waitingActionOnSelected;
	HWND hWnd;
	HDC hDC;
	vector<LineInfo> lines;
	vector<CharInfo> text;
	POINT caretPos;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	POINT startForSelection;
	int shiftCaretAfterDrawing;
	RECT clientRect;
	HFONT currentFont;
	vector<HFONT> fonts;
	ULONG_PTR gdiplusToken;
	vector<Gdiplus::Image*> images;

	void addCharToText(WORD wParam, Gdiplus::Image* image = NULL);
	bool belongsPixelToLineByY(POINT pixel, LineInfo line);
	bool belongsPixelToLineByX(POINT pixel, LineInfo line);
	void calculateCharSize();
	void calculateEndTextPos();
	bool caretIncludeSelectArea(POINT caretPos);
	bool deleteSelectedText();
	void drawImage(Gdiplus::Image* pImage, POINT start); //всё норм
	void ContentOfWindow::getLinesInfo(); //новый метод
	int indexByCaret(const POINT caretPos); //перписан, возможно потреуется доработка
	OPENFILENAME initializeStructOpenFilename(wchar_t *filename); //и так всё норм
	void initializeFonts(); //для Макса
	bool isPixelBelongsChar(POINT pixel, POINT pixelChar,CharInfo charInfo); //новый метод
	POINT lParamToPixel(LPARAM lParam); //переписано
	int numberLineByIndex(int index);
	POINT normedByUpperCorner(POINT pixel);
	void openImage(); //переписано
	POINT pixelLowerCornerByIndex(int index);
	POINT pixelUpperCornerByIndex(int index);
	POINT printCharOnDC(CharInfo symbol, POINT lowLeftAngle, int indexLine);
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
	void setFont(HFONT font){this->currentFont = font;}
	void setStartForSelection(LPARAM lParam);
	void leftMouseModePress(bool mode){this->leftMouseButtonPressed = mode;}
	void workWithCaret(WORD message);
};


