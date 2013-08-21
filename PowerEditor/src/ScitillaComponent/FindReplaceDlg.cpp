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

#include "FindReplaceDlg.h"
#include "ScintillaEditView.h"

BOOL CALLBACK FindReplaceDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDCANCEL : // Close
					display(false);
					//::AnimateWindow(_hSelf, 200, AW_HIDE|AW_SLIDE|AW_HOR_POSITIVE|AW_VER_POSITIVE);
					return TRUE;

				case IDOK : // Find Next
					processFindNext();
					return TRUE;

				case IDREPLACE :
					processReplace();
					return TRUE;

				case IDREPLACEALL :
					processReplaceAll();
					return TRUE;

				case IDWHOLEWORD :
					_isWholeWord = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDWHOLEWORD), BM_GETCHECK, 0, 0));
					return TRUE;

				case IDMATCHCASE :
					_isMatchCase = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDMATCHCASE), BM_GETCHECK, 0, 0));
					return TRUE;

				case IDREGEXP :
					_isRegExp = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDREGEXP), BM_GETCHECK, 0, 0));
					if (_isRegExp)
						_isWholeWord = false;
					::SendMessage(::GetDlgItem(_hSelf, IDWHOLEWORD), BM_SETCHECK, _isWholeWord?BST_CHECKED:BST_UNCHECKED, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDWHOLEWORD), (BOOL)!_isRegExp);

					::SendMessage(::GetDlgItem(_hSelf, IDDIRECTIONUP), BM_SETCHECK, BST_UNCHECKED, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDDIRECTIONUP), (BOOL)!_isRegExp);
					::SendMessage(::GetDlgItem(_hSelf, IDDIRECTIONDOWN), BM_SETCHECK, BST_CHECKED, 0);
					_whitchDirection = DIR_DOWN;
					return TRUE;

				case IDWRAP :
					_isWrapAround = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDWRAP), BM_GETCHECK, 0, 0));
					return TRUE;

				case IDDIRECTIONUP :
				case IDDIRECTIONDOWN :
					_whitchDirection = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDDIRECTIONDOWN), BM_GETCHECK, BST_CHECKED, 0));
					return TRUE;

				default :
					break;
			}
		}
	}
	return FALSE;
}

// return value :
// true  : the text2find is found
// false : the text2find is not found
bool FindReplaceDlg::processFindNext()
{
	if (!isCreated()) return false;

	getSearchTexts();
	
	int docLength = int((*_ppEditView)->execute(SCI_GETLENGTH));

	CharacterRange cr = (*_ppEditView)->getSelection();

	int startPosition = cr.cpMax;
	int endPosition = docLength;

	if (_whitchDirection == DIR_UP)
	{
		startPosition = cr.cpMin - 1;
		endPosition = 0;
	}

	int flags = (_isWholeWord ? SCFIND_WHOLEWORD : 0) |
	            (_isMatchCase ? SCFIND_MATCHCASE : 0) |
	            (_isRegExp ? SCFIND_REGEXP : 0);

	(*_ppEditView)->execute(SCI_SETTARGETSTART, startPosition);
	(*_ppEditView)->execute(SCI_SETTARGETEND, endPosition);
	(*_ppEditView)->execute(SCI_SETSEARCHFLAGS, flags);
	int posFind = int((*_ppEditView)->execute(SCI_SEARCHINTARGET, (WPARAM)strlen(_text2Find), (LPARAM)_text2Find));
	if (posFind == -1) //return;
	{
		if (_isWrapAround) 
		{
			if (_whitchDirection == DIR_DOWN)
			{
				startPosition = 0;
				endPosition = docLength;
			}
			else
			{
				startPosition = docLength;
				endPosition = 0;
			}
			(*_ppEditView)->execute(SCI_SETTARGETSTART, startPosition);
			(*_ppEditView)->execute(SCI_SETTARGETEND, endPosition);
			int posFind = int((*_ppEditView)->execute(SCI_SEARCHINTARGET, (WPARAM)strlen(_text2Find), (LPARAM)_text2Find));
			if (posFind == -1)
			{
				::MessageBox(_hParent, "Can't find the word", "Find", MB_OK);
				return false;
			}
			int start = int((*_ppEditView)->execute(SCI_GETTARGETSTART));
			int end = int((*_ppEditView)->execute(SCI_GETTARGETEND));
			(*_ppEditView)->execute(SCI_SETSEL, start, end);
		}
		else
		{
			::MessageBox(_hParent, "Can't find the word", "Find", MB_OK);
			return false;
		}
	}
	int start = int((*_ppEditView)->execute(SCI_GETTARGETSTART));
	int end = int((*_ppEditView)->execute(SCI_GETTARGETEND));
	(*_ppEditView)->execute(SCI_SETSEL, start, end);
	return true;
}

// return value :
// true  : the text is replaced, and find the next occurrence
// false : the text2find is not found, so the text is NOT replace
//      || the text is replaced, and do NOT find the next occurrence
bool FindReplaceDlg::processReplace()
{
	TextRange tr;
	char text[1024];
	tr.chrg = (*_ppEditView)->getSelection();
	tr.lpstrText = text;
	
	getReplaceTexts();

	(*_ppEditView)->execute(SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
	bool isSelectedMatch = !((_isMatchCase)?strcmp(_text2Find, tr.lpstrText):stricmp(_text2Find, tr.lpstrText));

	if (_isRegExp) isSelectedMatch = true;

	if (isSelectedMatch)
	{
		(*_ppEditView)->execute(SCI_REPLACESEL, 0, (LPARAM)_replaceText);
	}
	else // No match
	{
		if (processFindNext())
			(*_ppEditView)->execute(SCI_REPLACESEL, 0, (LPARAM)_replaceText);
		else
			return false;
	}
	return processFindNext();
}

void FindReplaceDlg::processReplaceAll()
{
	//bool tmp = _isWrapAround;
	//_isWrapAround = true;
	for ( ; processReplace() ; );
	//_isWrapAround = tmp;
}
