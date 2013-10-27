#include "StdAfx.h"
#include "resource.h"
#include "winMain.h"

bool ProcessingMenuMessages(WORD id)
{
	switch (id)
	{
	case ID_FILE_EXIT:
		PostQuitMessage(0);
		MessageBeep(MB_OK);  
		break;

	default:
		return false;
	}
	return true;
}