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

#ifndef TOOL_BAR_H
#define TOOL_BAR_H
#include "Window.h"

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

#include <commctrl.h>
#include "ImageListSet.h"

const bool REDUCED = true;
const bool ENLARGED = false;

class ToolBar : public Window
{
public :
	ToolBar():Window(),/*_pToolBarIcons(NULL),*/ _pTBB(NULL){};
	virtual ~ToolBar(){};

	//virtual bool init(HINSTANCE hInst, HWND hPere, ToolBarIcons *pToolBarIcons);
	virtual bool init(HINSTANCE hInst, HWND hPere, int iconSize, ToolBarButtonUnit *buttonUnitArray, int arraySize);

	virtual void destroy() {
		delete [] _pTBB;
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
		_toolBarIcons.destroy();
	};
	void enable(int cmdID, bool doEnable) {
		::SendMessage(_hSelf, TB_ENABLEBUTTON, cmdID, (LPARAM)doEnable);
	};

	int getHeight() const {
		if (!::IsWindowVisible(_hSelf))
			return 0;
		return Window::getHeight();
	};

	void reduce() {
		if (_state == REDUCED)
			return;
		_toolBarIcons.resizeIcon(16);
		reset();
		Window::redraw();
		_state = REDUCED;
	};
	void enlarge() {
		if (_state == ENLARGED)
			return;
		_toolBarIcons.resizeIcon(32);
		reset();
		Window::redraw();
		_state = ENLARGED;
	};

	bool getCheckState(int ID2Check) const {
		return bool(::SendMessage(_hSelf, TB_GETSTATE, (WPARAM)ID2Check, 0) & TBSTATE_CHECKED);
	};

	void setCheck(int ID2Check, bool willBeChecked) const {
		::SendMessage(_hSelf, TB_CHECKBUTTON, (WPARAM)ID2Check, (LPARAM)MAKELONG(willBeChecked, 0));
	};



private :
	TBBUTTON *_pTBB;
	//ToolBarIcons *_pToolBarIcons;
	ToolBarIcons _toolBarIcons;
	bool _state;

	void setDefaultImageList() {
		::SendMessage(_hSelf, TB_SETIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons.getDefaultLst());
	};
	void setHotImageList() {
		::SendMessage(_hSelf, TB_SETHOTIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons.getHotLst());
	};
	void setDisableImageList() {
		::SendMessage(_hSelf, TB_SETDISABLEDIMAGELIST, (WPARAM)0, (LPARAM)_toolBarIcons.getDisableLst());
	};
/*
	void setButtonSizeByDefault() {
		::SendMessage(_hSelf, TB_SETBUTTONSIZE , (WPARAM)0, (LPARAM)MAKELONG (w, h));
	};
	

	void removeAllButton() {
		int nCount = SendMessage(_hSelf, TB_BUTTONCOUNT, 0, 0);
		for (int i = nCount ; i >= 0 ; i--)
			::SendMessage(_hSelf, TB_DELETEBUTTON, (WPARAM)i, (LPARAM)0);
	};
*/
	void setButtonSize(int w, int h) {
		::SendMessage(_hSelf, TB_SETBUTTONSIZE , (WPARAM)0, (LPARAM)MAKELONG (w, h));
	};
	
	void reset() {
		setDefaultImageList();
		setHotImageList();
		setDisableImageList();
		::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
	};
	
};

#endif // TOOL_BAR_H
