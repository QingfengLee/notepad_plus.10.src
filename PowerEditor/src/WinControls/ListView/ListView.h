//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef LIST_VIEW_H
#define LIST_VIEW_H

#include "Window.h"

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

//const int marge = 8;

#include <commctrl.h>

class ListView : public Window
{
public:
	ListView() : Window(){};
	virtual ~ListView() {
		if (_hSelf)
			::DestroyWindow(_hSelf);
	};

	virtual void destroy(){
		if (_hFont)
			DeleteObject(_hFont);

		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	};

	void init(HINSTANCE hInst, HWND hwnd, bool isVertical = false);

	virtual void reSizeTo(RECT & rc2Ajust);
	
	int insertAtEnd(const char *subTabName);

	void activateAt(int index) {
		//::SendMessage(_hSelf, TCM_SETCURSEL, index, 0);
	};

	void deletItemAt(int index) {
		::SendMessage(_hSelf, TCM_DELETEITEM, index, 0);
		_nbItem--;
	};

	void deletAllItem() {
		::SendMessage(_hSelf, TCM_DELETEALLITEMS, 0, 0);
		_nbItem = 0;
	};

	void setImageList(HIMAGELIST himl){
		_hasImgLst = true;
		::SendMessage(_hSelf, TCM_SETIMAGELIST, 0, (LPARAM)himl);
	};

protected:
	unsigned char _nbItem;
	bool _hasImgLst;
	HFONT _hFont;
};

#endif // LIST_VIEW_H
