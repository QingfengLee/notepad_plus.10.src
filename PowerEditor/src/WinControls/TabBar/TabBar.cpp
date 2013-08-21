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
#include "SysMsg.h"
//#include "color.h"

const COLORREF blue      	            = RGB(0,       0, 0xFF);
const COLORREF black     	            = RGB(0,       0,    0);
const COLORREF white     	            = RGB(0xFF, 0xFF, 0xFF);
const COLORREF grey      	            = RGB(128,   128,  128);

#define	IDC_DRAG_TAB     1404
#define	IDC_DRAG_INTERDIT_TAB 1405
#define	IDC_DRAG_PLUS_TAB 1406

bool TabBar::_doDragNDrop = false;
bool TabBar::_doOwnerDrawTab = true;
HWND TabBar::_hwndArray[nbCtrlMax] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int TabBar::_nbCtrl = 0;

void TabBar::init(HINSTANCE hInst, HWND parent, bool isVertical)
{
	Window::init(hInst, parent);
	int vertical = isVertical?(TCS_VERTICAL | TCS_MULTILINE | TCS_RIGHTJUSTIFY):0;
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icce);
	
	int style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |\
		        WS_BORDER |TCS_FOCUSNEVER | TCS_TABS | vertical;

	if (_doOwnerDrawTab)
		style |= TCS_OWNERDRAWFIXED;

	_hSelf = ::CreateWindowEx(
				TCS_EX_FLATSEPARATORS ,
				WC_TABCONTROL,
				"Tab",
				style,
				0, 0, 0, 0,
				_hParent,
				NULL,
				_hInst,
				0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(69);
	}
	
	if (!_hwndArray[_nbCtrl])
	{
		_hwndArray[_nbCtrl] = _hSelf;
		_ctrlID = _nbCtrl;
	}
	else 
	{
		int i = 0;
		bool found = false;
		for ( ; i < nbCtrlMax && !found ; i++)
			if (!_hwndArray[i])
				found = true;
		if (!found)
		{
			_ctrlID = -1;
			::MessageBox(NULL, "The nb of Tab Control is over its limit", "Tab Control err", MB_OK);
			destroy();
			throw int(96);
		}
		_hwndArray[i] = _hSelf;
		_ctrlID = i;
	}
	_nbCtrl++;

	//DWORD style = ::GetClassLong(_hSelf, GCL_STYLE);
	//::SetClassLong(_hSelf, GCL_STYLE, style ^ CS_HREDRAW ^ CS_VREDRAW);
    ::SetWindowLong(_hSelf, GWL_USERDATA, reinterpret_cast<LONG>(this));
	_tabBarDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLong(_hSelf, GWL_WNDPROC, reinterpret_cast<LONG>(TabBar_Proc)));	 


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

	if (_doOwnerDrawTab)
		::SendMessage(_hSelf, TCM_SETPADDING, 0, MAKELPARAM(6, 5));
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
	
	return int(::SendMessage(_hSelf, TCM_INSERTITEM, _nbItem++, reinterpret_cast<LPARAM>(&tie)));
}

void TabBar::reSizeTo(RECT & rc2Ajust)
{
	// Important to do that!
	// Otherwise, the window(s) it contains will take all the resouce of CPU
	// We don't need to resiz the contained windows if they are even invisible anyway!
	display(rc2Ajust.right > 10);

	Window::reSizeTo(rc2Ajust);
	TabCtrl_AdjustRect(_hSelf, FALSE, &rc2Ajust);
}

LRESULT TabBar::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_LBUTTONDOWN :
		{
            ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);

            if (_doDragNDrop)
            {
                _nSrcTab = _nTabDragged = ::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0);
        
                POINT point;
			    point.x = LOWORD(lParam);
			    point.y = HIWORD(lParam);
			    if(::DragDetect(hwnd, point)) 
			    {
				    // Yes, we're beginning to drag, so capture the mouse...
				    _isDragging = true;
				    ::SetCapture(hwnd);
				    return TRUE;
			    }
			    break;
            }
            else
                return TRUE;
		}

		case WM_MOUSEMOVE :
		{
			if (_isDragging)
			{
				POINT p;
 				p.x = LOWORD(lParam);
				p.y = HIWORD(lParam);
                exchangeItemData(p);

				// Get cursor position of "Screen"
				// For using the function "WindowFromPoint" afterward!!!
				::GetCursorPos(&_draggingPoint);
				draggingCursor(_draggingPoint);
			    return TRUE;
			}
			break;
		}

		case WM_LBUTTONUP :
		{
            if (_isDragging)
			{
				if(::GetCapture() == _hSelf)
					::ReleaseCapture();

				// Send a notification message to the parent with wParam = 0, lParam = 0
				// nmhdr.idFrom = this
				// destIndex = this->_nSrcTab
				// scrIndex  = this->_nTabDragged
				NMHDR nmhdr;
				nmhdr.hwndFrom = _hSelf;
				nmhdr.code = _isDraggingInside?TCN_TABDROPPED:TCN_TABDROPPEDOUTSIDE;
	            nmhdr.idFrom = reinterpret_cast<unsigned int>(this);

				::SendMessage(_hParent, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmhdr));
				return TRUE;				
			}
			break;
		}

		case WM_CAPTURECHANGED :
		{
			if (_isDragging)
			{
				_isDragging = false;
				return TRUE;
			}
			break;
		}

		case WM_DRAWITEM :
		{
			drawItem((DRAWITEMSTRUCT *)lParam);
			return TRUE;
		}

		case WM_KEYDOWN :
		{
			if (wParam == VK_LCONTROL)
				::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_PLUS_TAB)));
			return TRUE;
		}
	}
	return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
}

void TabBar::drawItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
	RECT rect = pDrawItemStruct->rcItem;

	int nTab = pDrawItemStruct->itemID;
	if (nTab < 0)
	{
		::MessageBox(NULL, "nTab < 0", "", MB_OK);
		//return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
	}
	bool isSelected = (nTab == ::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0));

	char label[64];
	TCITEM tci;
	tci.mask = TCIF_TEXT|TCIF_IMAGE;
	tci.pszText = label;     
	tci.cchTextMax = 63;

	if (!::SendMessage(_hSelf, TCM_GETITEM, nTab, reinterpret_cast<LPARAM>(&tci))) 
	{
		::MessageBox(NULL, "! TCM_GETITEM", "", MB_OK);
		//return ::CallWindowProc(_tabBarDefaultProc, hwnd, Message, wParam, lParam);
	}
	HDC hDC = pDrawItemStruct->hDC;
	
	int nSavedDC = ::SaveDC(hDC);

	// For some bizarre reason the rcItem you get extends above the actual
	// drawing area. We have to workaround this "feature".
	rect.top += ::GetSystemMetrics(SM_CYEDGE);

	::SetBkMode(hDC, TRANSPARENT);
	HBRUSH hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	::FillRect(hDC, &rect, hBrush);
	::DeleteObject((HGDIOBJ)hBrush);

	if (isSelected)
	{
		RECT barRect = rect;
		barRect.bottom = 6;

		hBrush = ::CreateSolidBrush(RGB(250, 170, 60));
		::FillRect(hDC, &barRect, hBrush);
		::DeleteObject((HGDIOBJ)hBrush);
	}
	// Draw image
	HIMAGELIST hImgLst = (HIMAGELIST)::SendMessage(_hSelf, TCM_GETIMAGELIST, 0, 0);

	if (hImgLst && tci.iImage >= 0)
	{
		SIZE size;
		::GetTextExtentPoint(hDC, "  ", 2, &size);
		rect.left += size.cx;		// Margin

		// Get height of image so we 
		IMAGEINFO info;
		
		ImageList_GetImageInfo(hImgLst, tci.iImage, &info);

		RECT & imageRect = info.rcImage;
		int yPos = (rect.top + (rect.bottom - rect.top)/2 + 1) - (imageRect.bottom - imageRect.top)/2;

		ImageList_Draw(hImgLst, tci.iImage, hDC, rect.left, yPos, isSelected?ILD_TRANSPARENT:ILD_SELECTED);
		rect.left += imageRect.right - imageRect.left;
	}

	if (isSelected) 
	{
		COLORREF _selectedColor = RGB(0, 0, 0);
		::SetTextColor(hDC, _selectedColor);
		
		//pDC->SelectObject(&m_SelFont);
		rect.top -= ::GetSystemMetrics(SM_CYEDGE);
		::DrawText(hDC, label, strlen(label), &rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	} 
	else 
	{
		COLORREF _unselectedColor = grey;
		::SetTextColor(hDC, _unselectedColor);
		//pDC->SelectObject(&m_UnselFont);
		::DrawText(hDC, label, strlen(label), &rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	}
	::RestoreDC(hDC, nSavedDC);
}



void TabBar::draggingCursor(POINT screenPoint)
{
	HWND hWin = ::WindowFromPoint(screenPoint);
	if (_hSelf == hWin)
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	else
	{
		char className[256];
		::GetClassName(hWin, className, 256);
		if ((!strcmp(className, "Scintilla")) || (!strcmp(className, WC_TABCONTROL)))
		{
			if (::GetKeyState(VK_LCONTROL) & 0x80000000)
				::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_PLUS_TAB)));
			else
				::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_TAB)));
		}
		else
			::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_INTERDIT_TAB)));
	}
}

void TabBar::exchangeItemData(POINT point)
{
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = point.x;
	hitinfo.pt.y = point.y;

	// Find the destination tab...
	unsigned int nTab = ::SendMessage(_hSelf, TCM_HITTEST, 0, (LPARAM)&hitinfo);

	// The position is over a tab.
	if (hitinfo.flags != TCHT_NOWHERE)
	{
		
		_isDraggingInside = true;

		if (nTab != _nTabDragged)
		{
			//1. set to focus
			::SendMessage(_hSelf, TCM_SETCURSEL, nTab, 0);

			//2. shift their data, and insert the source
			TCITEM itemData_nDraggedTab, itemData_shift;
			itemData_nDraggedTab.mask = itemData_shift.mask = TCIF_IMAGE | TCIF_TEXT;
			char str1[256];
			char str2[256];

			itemData_nDraggedTab.pszText = str1;
			itemData_nDraggedTab.cchTextMax = (sizeof(str1));

			itemData_shift.pszText = str2;
			itemData_shift.cchTextMax = (sizeof(str2));

			::SendMessage(_hSelf, TCM_GETITEM, _nTabDragged, reinterpret_cast<LPARAM>(&itemData_nDraggedTab));

			if (_nTabDragged > nTab)
			{
				for (int i = _nTabDragged ; i > nTab ; i--)
				{
					::SendMessage(_hSelf, TCM_GETITEM, i-1, reinterpret_cast<LPARAM>(&itemData_shift));
					::SendMessage(_hSelf, TCM_SETITEM, i, reinterpret_cast<LPARAM>(&itemData_shift));
				}
			}
			else
			{
				for (int i = _nTabDragged ; i < nTab ; i++)
				{
					::SendMessage(_hSelf, TCM_GETITEM, i+1, reinterpret_cast<LPARAM>(&itemData_shift));
					::SendMessage(_hSelf, TCM_SETITEM, i, reinterpret_cast<LPARAM>(&itemData_shift));
				}
			}
			//
			::SendMessage(_hSelf, TCM_SETITEM, nTab, reinterpret_cast<LPARAM>(&itemData_nDraggedTab));

			//3. update the current index
			_nTabDragged = nTab;
			
		}
	}
	else
	{
		//::SetCursor(::LoadCursor(_hInst, MAKEINTRESOURCE(IDC_DRAG_TAB)));
		_isDraggingInside = false;
	}
	
}
/*
bool Platform::IsKeyDown(int key) {
	return (::GetKeyState(key) & 0x80000000) != 0;
}*/