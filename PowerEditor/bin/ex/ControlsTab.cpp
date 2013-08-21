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

#include "ControlsTab.h"

void ControlsTab::init(HINSTANCE hInst, HWND hwnd, bool isVertical,
                       Window *win0, const char *s0, 
                       Window *win1, const char *s1)
{
    _isVertical = isVertical;
	TabBar::init(hInst, hwnd, _isVertical);
	_pWin0 = win0;
	_pWin1 = win1;
	TabBar::insertAtEnd(s0);
	TabBar::insertAtEnd(s1);
	TabBar::activateAt(0);
	activateWindowAt(0);
}

void ControlsTab::reSizeTo(RECT & rc)
{
	TabBar::reSizeTo(rc);
	rc.left += marge;
	rc.top += marge;
	
	//-- We do those dirty things 
	//-- because it's a "vertical" tab control
    if (_isVertical)
    {
	    rc.right -= 40;
	    rc.bottom -= 20;
	    if (getRowCount() == 2)
	    {
		    rc.right -= 20;
	    }
    }
	//-- end of dirty things

	_pWin0->reSizeTo(rc);
	_pWin1->reSizeTo(rc);
};