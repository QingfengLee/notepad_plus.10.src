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

#include "TabBar.h"
#include "M30_IDE_commun.h"


void TabBar::init(HINSTANCE hInst, HWND parent, bool isVertical)
{
	Window::init(hInst, parent);
	int vertical = isVertical?(TCS_VERTICAL|TCS_MULTILINE):0;
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icce);
	
	_hSelf = ::CreateWindowEx(
				TCS_EX_FLATSEPARATORS ,
				WC_TABCONTROL,
				"Tab",
				WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
				TCS_FOCUSNEVER | TCS_TABS | TCS_FLATBUTTONS | WS_BORDER | vertical ,
				0, 0, 4, 4,
				_hParent,
				NULL,
				_hInst,
				0 );

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(69);
	}
	if (vertical)
	{
		_hFont = ::CreateFont( 14, 0, 0, 0,
			                   FW_NORMAL,
				               0, 0, 0, 0,
				               0, 0, 0, 0,
					           "Comic Sans MS");
		if (_hFont)
			::SendMessage(_hSelf, WM_SETFONT, reinterpret_cast<WPARAM>(_hFont), 0);
	}


	//::SetCursor(::LoadCursor(_hInst, IDC_ARROW));
}

int TabBar::insertAtEnd(const char *subTabName)
{
	TCITEM tie; 
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	int index = -1;

	if (_hasImgLst)
		index = 0;
	
	tie.iImage = index; 
	tie.pszText = (char *)subTabName; 
	
	return ::SendMessage(_hSelf, TCM_INSERTITEM, _nbItem++, reinterpret_cast<LPARAM>(&tie));
}

void TabBar::reSizeTo(RECT & rc2Ajust)
{
	//long style = ::GetWindowLong(_hSelf, GWL_STYLE);
	//long newStyle = style | WS_CLIPCHILDREN;
	//::SetWindowLong(_hSelf, GWL_STYLE, newStyle);

	::MoveWindow(_hSelf, rc2Ajust.left, rc2Ajust.top, rc2Ajust.right, rc2Ajust.bottom, TRUE);
	TabCtrl_AdjustRect(_hSelf, FALSE, &rc2Ajust);

	//::SetWindowLong(_hSelf, GWL_STYLE, style);
}

