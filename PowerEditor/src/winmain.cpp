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

#include "Notepad_plus.h"
#include "SysMsg.h"
#include <exception>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int nCmdShow)
{
    HWND hNotepad_plus = ::FindWindow(Notepad_plus::getClassName(), NULL);
    if (hNotepad_plus)
    {
        if (::IsIconic(hNotepad_plus))
            ::OpenIcon(hNotepad_plus);

        ::SetForegroundWindow(hNotepad_plus);
        if (lpszCmdLine[0])
        {
            COPYDATASTRUCT copyData;
            copyData.dwData = 0;//(ULONG_PTR);
            copyData.cbData = DWORD(strlen(lpszCmdLine) + 1);
            copyData.lpData = lpszCmdLine;
            ::SendMessage(hNotepad_plus, WM_COPYDATA, (WPARAM)hInstance, (LPARAM)&copyData);
        }
        return 0;
    }

	Notepad_plus notepad_plus_plus;
	MSG msg;
	msg.wParam = 0;

	try {
        char *pPathNames = NULL;
        if (lpszCmdLine[0])
        {
            pPathNames = lpszCmdLine;
        }
		notepad_plus_plus.init(hInstance, NULL, pPathNames);
		HACCEL hAccTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_M30_ACCELERATORS));
		MSG msg;
		msg.wParam = 0;
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			// if the message doesn't belong to the notepad_plus_plus's dialog
			if (!notepad_plus_plus.isDlgMsg(&msg))
			{
				if (::TranslateAccelerator(notepad_plus_plus.getHSelf(), hAccTable, &msg) == 0)
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	} catch(int i) {
		if (i == 106901)
			::MessageBox(NULL, "Scintilla.init is failled!", "106901", MB_OK);
		else {
			char str[50] = "God Damn Exception : ";
			char code[10];
			itoa(i, code, 10);
			::MessageBox(NULL, strcat(str, code), "int exception", MB_OK);
		}
	}
	
	catch(std::exception ex) {
		::MessageBox(NULL, ex.what(), "Exception", MB_OK);
	}
	catch(...) {
		systemMessage("System Err");
	}

	return (UINT)msg.wParam;
}

