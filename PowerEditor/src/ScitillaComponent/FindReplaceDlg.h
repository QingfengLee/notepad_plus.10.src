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

#ifndef FIND_REPLACE_DLG_H
#define FIND_REPLACE_DLG_H

#include "StaticDialog.h"
#include "..\resource.h"

class ScintillaEditView;

typedef bool DIALOG_TYPE;
#define REPLACE true
#define FIND false

#define DIR_DOWN true
#define DIR_UP false

#define FIND_REPLACE_STR_MAX 256

class FindReplaceDlg : public StaticDialog
{
public :
	FindReplaceDlg() : StaticDialog() {};
	void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView) {
		Window::init(hInst, hPere);
		if (!ppEditView)
			throw int(9900);
		_ppEditView = ppEditView;
	};

	virtual void create(int dialogID) {
		StaticDialog::create(dialogID);
		_currentStatus = REPLACE;

		initOptionsFromDlg();

		POINT p;
		//get screen coordonnees (x,y)
		::GetWindowRect(::GetDlgItem(_hSelf, IDREPLACE), &_replaceCancelPos);
		// set point (x,y)
		p.x = _replaceCancelPos.left;
		p.y = _replaceCancelPos.top;
		// convert screen coordonnees to client coordonnees
		::ScreenToClient(_hSelf, &p);
		// get the width and height
		::GetClientRect(::GetDlgItem(_hSelf, IDREPLACE), &_replaceCancelPos);
		// fill out _replaceCancelPos
		_replaceCancelPos.left = p.x;
		_replaceCancelPos.top = p.y;
		
		::GetWindowRect(::GetDlgItem(_hSelf, IDCANCEL), &_findCancelPos);
		p.x = _findCancelPos.left;
		p.y = _findCancelPos.top;
		::ScreenToClient(_hSelf, &p);
		::GetClientRect(::GetDlgItem(_hSelf, IDCANCEL), &_findCancelPos);
		_findCancelPos.left = p.x;
		_findCancelPos.top = p.y;

	};
	
	void initOptionsFromDlg()
	{
		_isWholeWord = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDWHOLEWORD), BM_GETCHECK, 0, 0));
		_isMatchCase = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDMATCHCASE), BM_GETCHECK, 0, 0));
		_isRegExp = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDREGEXP), BM_GETCHECK, 0, 0));
		_isWrapAround = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDWRAP), BM_GETCHECK, 0, 0));
		
		// Set Direction : Down by default
		_whitchDirection = DIR_DOWN;
		::SendMessage(::GetDlgItem(_hSelf, IDDIRECTIONDOWN), BM_SETCHECK, BST_CHECKED, 0);
	};
/*
    void setScintilla(ScintillaEditView *pEditView) {
        _pEditView = pEditView;
    };

	bool isCreated() const {
		return (bool)_hSelf;
	};
*/

    void doFindDlg() {
		doDialog(FIND);
	};

	void doReplaceDlg() {
		doDialog(REPLACE);
	};

	void doDialog(DIALOG_TYPE witchType) {
		if (!isCreated())
			create(IDD_FIND_REPLACE_DLG);
		enableReplceFunc(witchType);
		display();
		//::AnimateWindow(_hSelf, 200, AW_ACTIVATE|AW_BLEND|AW_HOR_POSITIVE|AW_VER_POSITIVE);
	};
	bool processFindNext();
	bool processReplace();
	void processReplaceAll();

protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
	DIALOG_TYPE _currentStatus;

	char _text2Find[FIND_REPLACE_STR_MAX];
	char _replaceText[FIND_REPLACE_STR_MAX];

	bool _isWholeWord;
	bool _isMatchCase;
	bool _isRegExp;
	bool _isWrapAround;
	bool _whitchDirection;

	RECT _replaceCancelPos, _findCancelPos;

	ScintillaEditView **_ppEditView;

	void enableReplceFunc(bool isEnable) {
		int hideOrShow = isEnable?SW_SHOW:SW_HIDE;
		::ShowWindow(::GetDlgItem(_hSelf, ID_STATICTEXT_REPLACE),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACE),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEWITH),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEALL),hideOrShow);
		::ShowWindow(::GetDlgItem(_hSelf, IDREPLACEINSEL),hideOrShow);
		RECT *pRect = (!isEnable)?&_replaceCancelPos:&_findCancelPos;
		::MoveWindow(::GetDlgItem(_hSelf, IDCANCEL), pRect->left, pRect->top, pRect->right, pRect->bottom, TRUE);

		::SetWindowText(_hSelf, isEnable?"Replace":"Find");
		_currentStatus = isEnable;
	};

	void getSearchTexts() {
		getComboTextFrom(IDFINDWHAT);
	};

	void getReplaceTexts() {
		getComboTextFrom(IDREPLACEWITH);
	};

	void getComboTextFrom(int ID)
	{
		char *str;
		if (ID == IDFINDWHAT) str = _text2Find;
		else if (ID == IDREPLACEWITH) str = _replaceText;
		else return;

		::GetDlgItemText(_hSelf, ID, str, FIND_REPLACE_STR_MAX);
		if (_replaceText[0])
		{
			HWND handle = ::GetDlgItem(_hSelf, ID);
			if (CB_ERR == ::SendMessage(handle, CB_FINDSTRINGEXACT, -1, (LPARAM)str))
				::SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)str);
		}
	};
};

#endif //FIND_REPLACE_DLG_H
