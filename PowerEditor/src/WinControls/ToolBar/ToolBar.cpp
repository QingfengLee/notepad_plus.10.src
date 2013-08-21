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

#include "ToolBar.h"
#include "SysMsg.h"

bool ToolBar::init(HINSTANCE hInst, HWND hPere, int iconSize, ToolBarButtonUnit *buttonUnitArray, int arraySize)
{
	Window::init(hInst, hPere);
	//_pToolBarIcons = pToolBarIcons;
	_toolBarIcons.init(buttonUnitArray, arraySize);
	_toolBarIcons.create(_hInst, iconSize);
	
	_state = (iconSize < 32)?REDUCED:ENLARGED;

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);

	_hSelf = ::CreateWindowEx(
	               WS_EX_PALETTEWINDOW ,
	               TOOLBARCLASSNAME,
	               "",
	               WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
	               TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | CCS_TOP | BTNS_AUTOSIZE ,
	               0, 0,
	               0, 0,
	               _hParent,
				   NULL,
	               _hInst,
	               0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(9);
	}

//	long hCur = (long)::LoadCursor(hInst, MAKEINTRESOURCE(IDC_MY_CUR));
//	::SetClassLong(_hSelf, GCL_HCURSOR, hCur);

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
	// backward compatibility.
	::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	setDefaultImageList();
	setHotImageList();
	setDisableImageList();

	int nbElement = _toolBarIcons.getNbCommand();
	
	_pTBB = new TBBUTTON[nbElement];
	
	for (int i = 0, j = 0; i < nbElement ; i++)
	{
		int cmd = 0;
		
		if ((cmd = _toolBarIcons.getCommandAt(i)) != 0)
		{
			_pTBB[i].iBitmap = j++;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_BUTTON; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;
		}
		else
		{
			_pTBB[i].iBitmap = 0;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_SEP; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;
		}
	}

	setButtonSize(iconSize, iconSize);

	::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nbElement, (LPARAM)_pTBB); 
	//::SendMessage(_hSelf, TB_LOADIMAGES, IDB_VIEW_LARGE_COLOR, reinterpret_cast<LPARAM>(HINST_COMMCTRL));
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
	return true;
}

/*
bool ToolBar::init(HINSTANCE hInst, HWND hwnd, ToolBarIcons *pToolBarIcons)
{
	Window::init(hInst, hwnd);
	_pToolBarIcons = pToolBarIcons;

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);

	_hSelf = ::CreateWindowEx(
	               WS_EX_PALETTEWINDOW ,
	               TOOLBARCLASSNAME,
	               "",
	               WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
	               TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | CCS_TOP,
	               0, 0,
	               0, 0,
	               _hParent,
				   NULL,
	               _hInst,
	               0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(9);
	}

	long hCur = (long)::LoadCursor(hInst, MAKEINTRESOURCE(IDC_MY_CUR));
	::SetClassLong(_hSelf, GCL_HCURSOR, hCur);

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
	// backward compatibility. 
	::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	setDefaultImageList();
	setHotImageList();
	setDisableImageList();

	int nbElement = _pToolBarIcons->getNbCommand();
	
	_pTBB = new TBBUTTON[nbElement];
	
	for (int i = 0, j = 0; i < nbElement ; i++)
	{
		int cmd = 0;
		
		if ((cmd = _pToolBarIcons->getCommandAt(i)) != 0)
		{
			_pTBB[i].iBitmap = j++;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_BUTTON; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;
		}
		else
		{
			_pTBB[i].iBitmap = 0;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_SEP; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;
		}
	}

	setButtonSize(32, 32);

	::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nbElement, (LPARAM)_pTBB); 
	//::SendMessage(_hSelf, TB_LOADIMAGES, IDB_VIEW_LARGE_COLOR, reinterpret_cast<LPARAM>(HINST_COMMCTRL));
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
	return true;
}
*/