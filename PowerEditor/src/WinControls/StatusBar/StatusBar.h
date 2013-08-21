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

#ifndef STATUS_BAR_H
#define STATUS_BAR_H
#include "Window.h"

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

#include <commctrl.h>

class StatusBar : public Window
{
public :
	StatusBar() : Window() {};
	virtual ~StatusBar(){};

	//virtual bool init(HINSTANCE hInst, HWND hPere, ToolBarIcons *pToolBarIcons);
	virtual void init(HINSTANCE hInst, HWND hPere);

	virtual void destroy() {
		::DestroyWindow(_hSelf);
	};


	int getHeight() const {
		if (!::IsWindowVisible(_hSelf))
			return 0;
		return Window::getHeight();
	};

    void setText(const char *str) {
        ::SendMessage(_hSelf, WM_SETTEXT, 0, (LPARAM)str);
    };
private :

};

#endif // STATUS_BAR_H
