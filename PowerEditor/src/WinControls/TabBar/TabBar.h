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

#ifndef TAB_BAR_H
#define TAB_BAR_H

#include "Window.h"

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

//Notification message
#define TCN_TABDROPPED (TCN_FIRST - 10)
#define TCN_TABDROPPEDOUTSIDE (TCN_FIRST - 11)

const int marge = 8;
const int nbCtrlMax = 10;

#include <commctrl.h>

class TabBar : public Window
{
public:
	TabBar() : Window(), _nbItem(0), _hasImgLst(false), _hFont(NULL),
        _tabBarDefaultProc(NULL), _isDragging(false) {};

	virtual ~TabBar() {
		if (_hSelf)
			::DestroyWindow(_hSelf);
	};

	virtual void destroy(){
		if (_hFont)
			DeleteObject(_hFont);

		::DestroyWindow(_hSelf);
		_hSelf = NULL;

		if (_ctrlID != -1)
		{
			_hwndArray[_ctrlID] = NULL;
			_nbCtrl--;
		}
	};

	void init(HINSTANCE hInst, HWND hwnd, bool isVertical = false);

	virtual void reSizeTo(RECT & rc2Ajust);
	
	int insertAtEnd(const char *subTabName);

	void activateAt(int index) {
		::SendMessage(_hSelf, TCM_SETCURSEL, index, 0);
		//::SendMessage(_hSelf, TCM_HIGHLIGHTITEM, index, MAKELONG(8, 0));
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
    
    int nbItem() const {
        return _nbItem;
    };

    static void doDragNDrop(bool justDoIt) {
        _doDragNDrop = justDoIt;
    };

    static bool doDragNDropOrNot() {
        return _doDragNDrop;
    };

	int getSrcTabIndex() const {
        return _nSrcTab;
    };

    int getTabDraggedIndex() const {
        return _nTabDragged;
    };

	POINT getDraggingPoint() const {
		return _draggingPoint;
	};

	static void doOwnerDrawTab(bool justDoIt) {
		if (_doOwnerDrawTab == justDoIt)
			return;

		for (int i = 0 ; i < _nbCtrl ; i++)
		{
			if (_hwndArray[i])
			{
				DWORD style = ::GetWindowLong(_hwndArray[i], GWL_STYLE);
				if (justDoIt)
					style |= TCS_OWNERDRAWFIXED;
				else
					style &= ~TCS_OWNERDRAWFIXED;
				::SetWindowLong(_hwndArray[i], GWL_STYLE, style);
			}
		}
		_doOwnerDrawTab = justDoIt;
	};

	static bool isOwnerDrawTab() {
        return _doOwnerDrawTab;
    };

protected:
	unsigned char _nbItem;
	bool _hasImgLst;
	HFONT _hFont;
	WNDPROC _tabBarDefaultProc;
	int _ctrlID;

    // it's the boss to decide if we do the drag N drop
    static bool _doDragNDrop;
	// drag N drop members
	bool _isDragging;
	bool _isDraggingInside;
    int _nSrcTab;
	int _nTabDragged;
	POINT _draggingPoint; // coordinate of Screen

	
	void exchangeItemData(POINT point);

	// it's the boss to decide if we do the ownerDraw style tab
	static bool _doOwnerDrawTab;
	static int _nbCtrl;
	static HWND _hwndArray[nbCtrlMax];

	void drawItem(DRAWITEMSTRUCT *pDrawItemStruct);
	void draggingCursor(POINT screenPoint);

	static LRESULT CALLBACK TabBar_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
		return ((TabBar *)(::GetWindowLong(hwnd, GWL_USERDATA)))->runProc(hwnd, Message, wParam, lParam);
	};

	LRESULT runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	int getRowCount() const {
		return int(::SendMessage(_hSelf, TCM_GETROWCOUNT, 0, 0));
	};
};

#endif // TAB_BAR_H
