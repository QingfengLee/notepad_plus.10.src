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

#include "AboutDlg.h"

BOOL CALLBACK AboutDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
        case WM_INITDIALOG :
		{
            HWND licenceEditHandle = ::GetDlgItem(_hSelf, IDC_LICENCE_EDIT);
            ::SendMessage(licenceEditHandle, WM_SETTEXT, 0, (LPARAM)LICENCE_TXT);

            _emailLink.init(_hInst, _hSelf);
            _emailLink.create(::GetDlgItem(_hSelf, IDC_EMAIL_ADDR), "mailto:donho@altern.org");

            _pageLink.init(_hInst, _hSelf);
            _pageLink.create(::GetDlgItem(_hSelf, IDC_HOME_ADDR), "http://notepad-plus.sourceforge.net/");
			
			getClientRect(_rc);
			
			return TRUE;
		}

		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDOK : // Find Next
					display(false);
					return TRUE;
				
				default :
					break;
			}
		}
	}
	return FALSE;	
}

void AboutDlg::doDialog()
{
	if (!isCreated())
		create(IDD_ABOUTBOX);

    // Adjust the position of AboutBox
    RECT rc;
    ::GetClientRect(_hParent, &rc);
    POINT center;
    center.x = rc.left + (rc.right - rc.left)/2;
    center.y = rc.top + (rc.bottom - rc.top)/2;
    ::ClientToScreen(_hParent, &center);

    
    int x = center.x - _rc.right/2;
    int y = center.y - _rc.bottom/2;

    ::SetWindowPos(_hSelf, HWND_TOP, x, y, _rc.right, _rc.bottom, SWP_SHOWWINDOW);
    
};
