#include "StdAfx.h"
#include "resource.h"
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>

using namespace std;
using namespace Gdiplus;

static int wheelDelta = 0;

class ContentOfWindow
{
	struct CharInfo
	{
	private:
		int numberFont;
		POINT size;
		Gdiplus::Image* image;
		wchar_t symbol;
	public:
		int belowBaseLine;
		int leading;
		int numberImage;
		ABC abc;
		HFONT font;
		CharInfo(){}
		CharInfo(wchar_t symbol, HFONT font, POINT size);
		wchar_t GetSymbol(){return symbol; }
		const POINT GetSize(){return size;}
		Gdiplus::Image* GetImage(){return image;}
		int GetNumberImage(){return numberImage;}
		int GetNumberFont(){return numberFont;}
		void SetImage(Gdiplus::Image* image, int numberImage);
		void SetSizeX(int x){size.x = x;}
		void SetImageNumber(int imageNumber){numberImage = imageNumber;}
		void SetNumberFont(int number){numberFont = number;}
		bool operator==(CharInfo chInfo);
		bool operator<(CharInfo chInfo);
	};

	struct LineInfo
	{
		int heigth;
		int maxHeigthChar;
		int upperLeftCorner;
		int baseLineY;
		int lengthByX;
		int startInText;
		int endInText;
	public:
		LineInfo();
	};

	struct SaverText
	{
		int countSymbols;
		int countImages;
		int sizeInBytes;
	};

public:
	ContentOfWindow(HWND hWnd);
	~ContentOfWindow(void);
	void CaretPosByCoordinates(LPARAM lParam);
	void changeScalling(WPARAM wparam);
	void drawText();
	void invertSelectionFlag();
	void mouseSelection(WPARAM wParam, LPARAM lParam);
	void processorArrows(WPARAM wParam);
	bool processorMenuMessages(WORD id);
	void processorWmChar(WORD wParam);
	void setSizeAreaType(LPARAM param);
	void setStartForSelection(LPARAM lParam);
	void workWithCaret(WORD message);

private:
	static const WORD SYMBOL_SIGN_PICTURES = 0x265E;
	double scale;
	int belowCharBaseLine;
	bool leftMouseButtonPressed;
	bool selectionFlag;
	bool canWorkWithSelected;
	bool changeFontFlag;
	HWND hWnd;
	HDC hDC;
	vector<LineInfo> lines;
	vector<CharInfo> text;
	int caretIndex;
	POINT clientSize;
	POINT charSize;
	POINT endTextPos;
	int startSelectionIndex;
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
	void decreaseScale();
	void increaseScale();
	bool indexIncludeSelectArea(int indexCaret);
	void changeFont();
	void changeFontText(int pos1,int pos2, HFONT font);
	bool deleteSelectedText();
	void drawCaret(int indexCaretInTExt);
	void drawImage(Gdiplus::Image* pImage, POINT start);
	long fileSize(FILE* f);
	void getLinesInfo();
	int indexByCaret(const POINT caretPos);
	OPENFILENAME initializeStructOpenFilename(wchar_t *filename, wchar_t* filter);
	bool isPixelBelongsChar(POINT pixel, POINT pixelChar,CharInfo charInfo);
	bool isEndWindow(POINT pixel, int indexInText);
	POINT lParamToPixel(LPARAM lParam);
	int numberLineByIndex(int index);
	POINT normedByUpperCorner(POINT pixel);
	void open();
	void openImage(); 
	POINT pixelLowerCornerByIndex(int index);
	POINT pixelUpperCornerByIndex(int index);
	POINT printCharOnDC(CharInfo symbol, POINT lowLeftAngle, int indexLine);
	void processorVkLeft();
	void processorWkRight();
	void recoveryImagesAddress();
	void save();
	void scallingFont();
	bool setContentFromFile(wchar_t* filename);
	void validateRectsForPaint();
	void updateCaret(int index);
	void updateCaretSize();
};

inline void ContentOfWindow::invertSelectionFlag()
{
	selectionFlag = !selectionFlag;
	if (selectionFlag == true)
	{
		canWorkWithSelected = true;
	}
}
