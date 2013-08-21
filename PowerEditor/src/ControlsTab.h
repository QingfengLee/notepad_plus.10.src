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

#ifndef CONTROLS_TAB_H
#define CONTROLS_TAB_H

#include "TabBar.h"
#include "StaticDialog.h"
#include "SplitterContainer.h"

class ControlsTab : public TabBar
{
public :
	ControlsTab() : TabBar(){};
	~ControlsTab(){};
	void init(HINSTANCE hInst, HWND pere, bool isVertical, 
              Window *win0, const char *s0,
              Window *win1, const char *s1);

	void destroy() {
		TabBar::destroy();
	};
	
	virtual void reSizeTo(RECT & rc);
	
	void activateWindowAt(int index)
	{
		_pWin0->display(!(bool)index);
		_pWin1->display((bool)index);
	};

	void clickedUpdate()
	{
		int indexClicked = int(::SendMessage(_hSelf, TCM_GETCURSEL, 0, 0));
		activateWindowAt(indexClicked);
	};
	
private :
	Window *_pWin0;
	Window *_pWin1;
    bool _isVertical;
};



#endif //CONTROLS_TAB_H
