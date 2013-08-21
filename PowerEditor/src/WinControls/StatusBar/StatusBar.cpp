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

#include "StatusBar.h"
#include "SysMsg.h"

void StatusBar::init(HINSTANCE hInst, HWND hPere)
{
	Window::init(hInst, hPere);
    InitCommonControls();

	_hSelf = ::CreateWindowEx(
	               0,
	               STATUSCLASSNAME,
	               "",
	               WS_CHILD /*| SBARS_SIZEGRIP*/,
	               0, 0, 0, 0,
	               _hParent,
				   NULL,
	               _hInst,
	               0);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(9);
	}
}
