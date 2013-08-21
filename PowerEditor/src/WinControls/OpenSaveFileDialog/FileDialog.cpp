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

#include "FileDialog.h"

FileDialog::FileDialog(HWND hwnd, HINSTANCE hInst, bool isMultiSel) 
	: _nbCharFileExt(0), _isMultiSel(isMultiSel)
{
	_fileName[0] = '\0';
 
	_ofn.lStructSize = sizeof(_ofn);     
	_ofn.hwndOwner = hwnd; 
	_ofn.hInstance = hInst;
	_ofn.lpstrFilter = _fileExt;
	_ofn.lpstrCustomFilter = (LPTSTR) NULL;
	_ofn.nMaxCustFilter = 0L;
	_ofn.nFilterIndex = 1L;
	_ofn.lpstrFile = _fileName;
	_ofn.nMaxFile = sizeof(_fileName);
	_ofn.lpstrFileTitle = NULL;
	_ofn.nMaxFileTitle = 0;
	_ofn.lpstrInitialDir = NULL;
	_ofn.lpstrTitle = NULL;
	_ofn.nFileOffset  = 0;
	_ofn.nFileExtension = 0;
	_ofn.lpstrDefExt = NULL;  // No default extension
	_ofn.lCustData = 0;
	_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
				OFN_EXPLORER | OFN_LONGNAMES | DS_CENTER | OFN_OVERWRITEPROMPT;

	if (_isMultiSel)
		_ofn.Flags |= OFN_ALLOWMULTISELECT; 
}

void FileDialog::setExtFilter(const char *extText, const char *ext)
{
  char constStr[] = " (*.";
  char tmp[50];
  int extTextLen = int(strlen(extText));
  memcpy(tmp, extText, extTextLen);
  char *pTmp = tmp + extTextLen;
  int constStrLen = int(strlen(constStr));
  memcpy(pTmp, constStr, constStrLen);
  pTmp += constStrLen;
  int extLen = int(strlen(ext));
  memcpy(pTmp, ext, extLen);
  pTmp += extLen;
  *pTmp = ')'; pTmp++;
  *pTmp = '\0'; pTmp++;
  *pTmp = '*'; pTmp++;
  *pTmp = '.'; pTmp++;
  memcpy(pTmp, ext, extLen);
  pTmp += extLen;
  *pTmp = '\0'; pTmp++;
  
  char *pFileExt = _fileExt + _nbCharFileExt;
  memcpy(pFileExt, tmp, pTmp-tmp);

  _nbCharFileExt += int(pTmp-tmp);
  _fileExt[_nbCharFileExt] = '\0';
}

stringVector * FileDialog::doOpenDlg() 
{
	if (::GetOpenFileName(&_ofn))
	{
		if (_isMultiSel)
		{
			char fn[MAX_PATH];
			char *pFn = _fileName + strlen(_fileName) + 1;
			if (!(*pFn))
				_fileNames.push_back(std::string(_fileName));
			else
				strcat(strcpy(fn, _fileName), "\\");
			int term = int(strlen(fn));
			while (*pFn)
			{
				fn[term] = '\0';
				strcat(fn, pFn);
				_fileNames.push_back(std::string(fn));
				pFn += strlen(pFn) + 1;
			}
		}
		else
		{
			_fileNames.push_back(std::string(_fileName));
		}
		return &_fileNames;
	}
	else
		return NULL;
}