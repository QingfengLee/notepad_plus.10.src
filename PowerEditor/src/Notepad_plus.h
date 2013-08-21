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

#ifndef NOTEPAD_PLUS_H
#define NOTEPAD_PLUS_H

#include "Window.h"
#include "ScintillaEditView.h"
#include "ToolBar.h"
#include "ImageListSet.h"
#include "DocTabView.h"

#include "ControlsTab.h"
#include "StaticDialog.h"
#include "SplitterContainer.h"
#include "WindowInterface.h"
#include "FindReplaceDlg.h"
#include "AboutDlg.h"
#include "UserDefineDialog.h"
#include "StatusBar.h"
#include "Parameters.h"

#define NOTEPAD_PP_CLASS_NAME	"Notepad++"

//#define WM_LOADFILEBYPATH WM_USER

const bool dirUp = true;
const bool dirDown = false;

const bool MODE_TRANSFER = true;
const bool MODE_CLONE = false;

const unsigned char DOCK_MASK = 1;
const unsigned char TWO_VIEWS_MASK = 2;

const int MAIN_VIEW = 0;
const int SUB_VIEW = 1;

class Notepad_plus : public Window
{
public:
	Notepad_plus():Window(), _mainWindowStatus(0), _pMainSplitter(NULL),
        _hTabPopupMenu(NULL), _hTabPopupDropMenu(NULL), _pEditView(NULL), _pDocTab(NULL){};

	void init(HINSTANCE, HWND, const char *);

	// ATTENTION : the order of the destruction is very important
	// because if the parent's window hadle is destroyed before
	// the destruction of its childrens' windows handle, 
	// its childrens' windows handle will be destroyed automatically!
	virtual ~Notepad_plus(){
        (NppParameters::getInstance())->destroyInstance();
	};

	void killAllChildren() {
		_toolBar.destroy();

        if (_pMainSplitter)
        {
            _pMainSplitter->destroy();
            delete _pMainSplitter;
        }

        _mainDocTab.destroy();
        _subDocTab.destroy();

		_mainEditView.destroy();
        _subEditView.destroy();

        _subSplitter.destroy();
        _statusBar.destroy();

		if (_findReplaceDlg.isCreated())
			_findReplaceDlg.destroy();

        if (_aboutDlg.isCreated())
			_aboutDlg.destroy();

        if (_hTabPopupMenu)
            DestroyMenu(_hTabPopupMenu);

        if (_hTabPopupDropMenu)
            DestroyMenu(_hTabPopupDropMenu);
	};

	virtual void destroy() {
		killAllChildren();
		if (_hSelf)
		{
			::DestroyWindow(_hSelf);
			_hSelf = NULL;
		}
	}

    static const char * getClassName() {
        return _className;
    };

	void setTitle(const char *title) const {
		::SetWindowText(_hSelf, _className);
	};
	
	void setTitleWith(const char *filePath);

	// For filtering the modeless Dialog message
	bool isDlgMsg(MSG *msg) const {
		if (_findReplaceDlg.isCreated())
			return bool(::IsDialogMessage(_findReplaceDlg.getHSelf(), msg));
		return false;
	};
    bool doOpen(const char *fileName);

private:
	static const char _className[32];
    Window *_pMainWindow;

    unsigned char _mainWindowStatus;

    DocTabView _mainDocTab;
    DocTabView _subDocTab;
    DocTabView *_pDocTab;

    ScintillaEditView _mainEditView;
    ScintillaEditView _subEditView;
    ScintillaEditView *_pEditView;

    SplitterContainer *_pMainSplitter;
    SplitterContainer _subSplitter;

    HMENU _hTabPopupMenu, _hTabPopupDropMenu;

	ToolBar	_toolBar;
	IconList _docTabIconList;
    StatusBar _statusBar;

	// Dialog
	FindReplaceDlg _findReplaceDlg;
    AboutDlg _aboutDlg;

	//M30ProjectFile _projectFile;

	static LRESULT CALLBACK Notepad_plus_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	LRESULT runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	void notify(SCNotification *notification);
	void command(int id);
	void fileNew(){
		setTitleWith(_pDocTab->newDoc(NULL));
	};

	void fileOpen();
	bool fileClose();
	bool fileCloseAll();

	void hideCurrentView();

	int doSaveOrNot(const char *fn) {
		char phrase[512] = "Do you wanna save file \"";
		strcat(strcat(phrase, fn), "\" ?");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL);
	};
	
	int doReloadOrNot(const char *fn) {
		char phrase[512] = "The file \"";
		strcat(strcat(phrase, fn), "\" is modified by another program. Do you wanna reload this file?");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
	};

	int doCloseOrNot(const char *fn) {
		char phrase[512] = "The file \"";
		strcat(strcat(phrase, fn), "\" doesn't exist anymore. Do you wanna keep this file in editor ? (Yes keep it, No remove it)");
		return ::MessageBox(_hSelf, phrase, "Save", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
	};
	
	bool fileSave();
	bool fileSaveAll();
	bool fileSaveAs();
	void filePrint(bool showDialog);
	bool doSave(const char *filename);
	void enableMenu(int cmdID, bool doEnable) {
		int flag = doEnable?MF_ENABLED | MF_BYCOMMAND:MF_DISABLED | MF_GRAYED | MF_BYCOMMAND;
		::EnableMenuItem(::GetMenu(_hSelf), cmdID, flag);
	}
	void enableCommand(int cmdID, bool doEnable, int which);
	void checkClipboard();
	void checkDocState();
	void checkUndoState();
	void dropFiles(HDROP hdrop);
	void checkModifiedDocument();
	void reload(const char *fileName);

    void docGotoAnotherEditView(bool mode);
    void dockUserDlg();
    void undockUserDlg();

    void getMainClientRect(RECT & rc) const;
    void getStatusBarClientRect(RECT & rc) const;

    void switchEditViewTo(int gid) {
        _pDocTab = (gid == MAIN_VIEW)?&_mainDocTab:&_subDocTab;
        _pEditView = (gid == MAIN_VIEW)?&_mainEditView:&_subEditView;
		_pEditView->beSwitched();
        _pEditView->getFocus();

        checkDocState();
        setTitleWith(_pEditView->getCurrentTitle());
        setLangStatus(_pEditView->getCurrentDocType());

		dynamicCheckMenuAndTB();
    };
	
	void dynamicCheckMenuAndTB() const {
		// Visibility of 3 margins
        checkMenuItem(IDM_VIEW_LINENUMBER, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_LINENUMBER));
        checkMenuItem(IDM_VIEW_SYMBOLMARGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_SYBOLE));
        checkMenuItem(IDM_VIEW_FOLDERMAGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_FOLDER));
		// Folder margin style
		changeCheckedItemFromTo(getFolderMarginStyle(), getFolderMaginStyleIDFrom(_pEditView->getFolderStyle()));

		// Visibility of invisible characters
		bool b = _pEditView->isInvisibleCharsShown();
		checkMenuItem(IDM_VIEW_ALL_CHARACTERS, b);
		_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, b);

		// Visibility of the indentation guide line 
		b = _pEditView->isShownIndentGuide();
		checkMenuItem(IDM_VIEW_INDENT_GUIDE, b);
		_toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, b);
	};

    int getCurrentView() const {
        return (_pEditView == &_mainEditView)?MAIN_VIEW:SUB_VIEW;
    };

    DocTabView * getNonCurrentDocTab() {
        return (_pDocTab == &_mainDocTab)?&_subDocTab:&_mainDocTab;
    };

    ScintillaEditView * getNonCurrentEditView() {
        return (_pEditView == &_mainEditView)?&_subEditView:&_mainEditView;
    };

    void synchronise();

    void setLangStatus(LangType langType);

    void setLanguage(LangType langType) {
        _pEditView->setCurrentDocType(langType);
        setLangStatus(langType);
    };

    int getToolBarState() const {
        HMENU hMenu = ::GetMenu(_hSelf);

        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_HIDE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_HIDE;
        
        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_REDUCE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_REDUCE;

        if (::GetMenuState(hMenu, IDM_VIEW_TOOLBAR_ENLARGE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_TOOLBAR_ENLARGE;

		return -1;
    };

    int getFolderMarginStyle() const {
        HMENU hMenu = ::GetMenu(_hSelf);

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_SIMPLE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_SIMPLE;
        
        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_ARROW, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_ARROW;

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_CIRCLE, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_CIRCLE;

        if (::GetMenuState(hMenu, IDM_VIEW_FOLDERMAGIN_BOX, MF_BYCOMMAND) == MF_CHECKED)
            return IDM_VIEW_FOLDERMAGIN_BOX;

		return 0;
    };

    void changeCheckedItemFromTo(int id2Uncheck, int id2Check) const {
		if (id2Uncheck == id2Check)
			return;
        HMENU hMenu = ::GetMenu(_hSelf);

        if (id2Uncheck)
            ::CheckMenuItem(hMenu, id2Uncheck, MF_BYCOMMAND | MF_UNCHECKED);
        if (id2Check)
            ::CheckMenuItem(hMenu, id2Check, MF_BYCOMMAND | MF_CHECKED);
    };

    int checkStatusBar() const {
        HMENU hMenu = ::GetMenu(_hSelf);
        int check = (::GetMenuState(hMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND) == MF_CHECKED)?MF_UNCHECKED:MF_CHECKED;
        ::CheckMenuItem(hMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND | check);
        return check;
    };

    int getFolderMaginStyleIDFrom(folderStyle fStyle) const {
        switch (fStyle)
        {
            case FOLDER_STYLE_SIMPLE : return IDM_VIEW_FOLDERMAGIN_SIMPLE;
            case FOLDER_STYLE_ARROW : return IDM_VIEW_FOLDERMAGIN_ARROW;
            case FOLDER_STYLE_CIRCLE : return IDM_VIEW_FOLDERMAGIN_CIRCLE;
            case FOLDER_STYLE_BOX : return IDM_VIEW_FOLDERMAGIN_BOX;
        }
        return 0;
    };

	void checkMenuItem(int itemID, bool willBeChecked) const {
		::CheckMenuItem(::GetMenu(_hSelf), itemID, MF_BYCOMMAND | (willBeChecked?MF_CHECKED:MF_UNCHECKED));
	};
    //-----------------------------//
	// Execute the executable file //
	//-----------------------------//
	void launchConvertor();
	void launchEditVar();
};

#endif //NOTEPAD_PLUS_H
