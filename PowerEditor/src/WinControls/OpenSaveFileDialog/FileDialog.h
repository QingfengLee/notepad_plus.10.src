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

#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

#include <windows.h>
#include <vector>
#include <string>

typedef std::vector<std::string> stringVector;

class FileDialog
{
public:
	FileDialog(HWND hwnd, HINSTANCE hInst, bool isMultiSel = false);
	void setExtFilter(const char *, const char *);
	void setDefFileName(const char *fn){strcpy(_fileName, fn);}

	char * doSaveDlg() {
		return ((::GetSaveFileName(&_ofn))?_fileName:NULL);
	};
	stringVector * doOpenDlg();

private:
	char _fileName[MAX_PATH];
	bool _isMultiSel;

	char _fileExt[MAX_PATH];
	int _nbCharFileExt;

	stringVector _fileNames;
	OPENFILENAME _ofn;
};

#endif //FILE_DIALOG_H
