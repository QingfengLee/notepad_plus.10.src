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

#include "StaticDialog.h"
#include "SysMsg.h"

void StaticDialog::create(int dialogID) 
{
	_hSelf = ::CreateDialogParam(_hInst, MAKEINTRESOURCE(dialogID), _hParent,  (DLGPROC)dlgProc, (LPARAM)this);
	
	if (!_hSelf)
	{
		systemMessage("StaticDialog");
		throw int(666);
	}
	display();
}

BOOL CALLBACK StaticDialog::dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			StaticDialog *pStaticDlg = (StaticDialog *)(lParam);
			pStaticDlg->_hSelf = hwnd;
			::SetWindowLong(hwnd, GWL_USERDATA, (long)lParam);
            pStaticDlg->run_dlgProc(message, wParam, lParam);
			return TRUE;
		}

		default :
		{
			StaticDialog *pStaticDlg = reinterpret_cast<StaticDialog *>(::GetWindowLong(hwnd, GWL_USERDATA));
			if (!pStaticDlg)
				return FALSE;
			return pStaticDlg->run_dlgProc(message, wParam, lParam);
		}
	}
}

