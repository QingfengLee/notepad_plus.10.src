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

#include <shlwapi.h>
#include "Notepad_plus.h"
#include "SysMsg.h"
#include "FileDialog.h"
#include "resource.h"
#include "printer.h"
#include "Process.h"
#include "FileNamStringSpliter.h"

const char Notepad_plus::_className[32] = NOTEPAD_PP_CLASS_NAME;

void Notepad_plus::init(HINSTANCE hInst, HWND parent, const char *cmdLine)
{
	Window::init(hInst, parent);
    
	WNDCLASS MACOCS30EditorClass;

	MACOCS30EditorClass.style = 0;//CS_HREDRAW | CS_VREDRAW;
	MACOCS30EditorClass.lpfnWndProc = Notepad_plus_Proc;
	MACOCS30EditorClass.cbClsExtra = 0;
	MACOCS30EditorClass.cbWndExtra = 0;
	MACOCS30EditorClass.hInstance = _hInst;
	MACOCS30EditorClass.hIcon = ::LoadIcon(_hInst, MAKEINTRESOURCE(IDI_M30ICON));
	MACOCS30EditorClass.hCursor = NULL;
	MACOCS30EditorClass.hbrBackground = ::CreateSolidBrush(::GetSysColor(COLOR_MENU));
	MACOCS30EditorClass.lpszMenuName = MAKEINTRESOURCE(IDR_M30_MENU);
	MACOCS30EditorClass.lpszClassName = _className;

	if (!::RegisterClass(&MACOCS30EditorClass))
	{
		systemMessage("System Err");
		throw int(98);
	}

	_hSelf = ::CreateWindowEx(
					WS_EX_ACCEPTFILES/*WS_EX_CLIENTEDGE*/,\
					_className,\
					"MACOCS 30 IDE Demonstration",\
					WS_OVERLAPPEDWINDOW	| WS_CLIPCHILDREN,\
					CW_USEDEFAULT, CW_USEDEFAULT,\
					CW_USEDEFAULT, CW_USEDEFAULT,\
					_hParent,\
					NULL,\
					_hInst,\
					(LPVOID)this); // pass the ptr of this instantiated object
                                   // for retrive it in Notepad_plus_Proc from 
                                   // the CREATESTRUCT.lpCreateParams afterward.
  
	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(777);
	}
    
    if (cmdLine)
    {
        FileNamStringSpliter fnss(cmdLine);
        char *pFn = NULL;
        for (int i = 0 ; i < fnss.size() ; i++)
        {
            pFn = (char *)fnss.getFileName(i);
            doOpen((const char *)pFn);
        }
    }

	setTitle(_className);
	display();
	checkDocState();
}

bool Notepad_plus::doOpen(const char *fileName) 
{
	int i = - 1;
	if ( (i = _pDocTab->find(fileName)) != -1)
	{
		setTitleWith(_pDocTab->activate(i));
		_pEditView->getFocus();
		return false;
	}

    bool isNewDoc2Close = false;
	FILE *fp = fopen(fileName, "rb");
    
	if (fp)
	{
        if ((_pEditView->getNbDoc() == 1) 
            && (!strncmp(_pEditView->getCurrentTitle(), UNTITLED_STR, sizeof(UNTITLED_STR)-1))
            && (!_pEditView->isCurrentDocDirty()) && (_pEditView->getCurrentDocLen() == 0))
        {
            //_pDocTab->closeCurrentDoc();
            isNewDoc2Close = true;
        }
		setTitleWith(_pDocTab->newDoc(fileName));

		// It's VERY IMPORTANT to reset the view
		_pEditView->execute(SCI_CLEARALL);

		const int blockSize = 128 * 1024;
		char data[blockSize];

		int lenFile = int(fread(data, 1, sizeof(data), fp));
		while (lenFile > 0) {
			_pEditView->execute(SCI_ADDTEXT, lenFile, reinterpret_cast<LPARAM>(static_cast<char *>(data)));
			lenFile = int(fread(data, 1, sizeof(data), fp));
		}
		fclose(fp);

		//_pDocTab->activate(_pEditView->getCurrentDocIndex());
		_pEditView->getFocus();
		_pEditView->execute(SCI_SETSAVEPOINT);
		_pEditView->execute(EM_EMPTYUNDOBUFFER);

		// if file is read oly, we set the view read only
		_pEditView->execute(SCI_SETREADONLY, _pEditView->isCurrentBufReadOnly());
        if (isNewDoc2Close)
            _pDocTab->closeDocAt(0);
		return true;
	}
	else
	{
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		//strcat(msg, fullPath);
		strcat(msg, fileName);
		strcat(msg, "\".");
		::MessageBox(_hSelf, msg, "ERR", MB_OK);
		return false;
	}
	/*
	_pEditView->execute(SCI_SETUNDOCOLLECTION, 1);
	_pEditView->execute(SCI_GOTOPOS, 0);
	*/
}

void Notepad_plus::fileOpen()
{
	FileDialog fDlg(_hSelf, _hInst, true);
	
	//fDlg.setDefFileName("m30.scr");
	fDlg.setExtFilter("Pcom cmd file", "c");
	fDlg.setExtFilter("M30 Scr file", "cpp");
	fDlg.setExtFilter("Production file", "xml");

	fDlg.setExtFilter("Production file", "*");

	if (stringVector *pfns = fDlg.doOpenDlg())
	{
		int sz = int(pfns->size());
		for (int i = 0 ; i < sz ; i++)
			doOpen((pfns->at(i)).c_str());
	}
}

bool Notepad_plus::doSave(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	const int blockSize = 128 * 1024;
	if (fp)
	{
		char data[blockSize + 1];
		int lengthDoc = _pEditView->getCurrentDocLen();
		for (int i = 0; i < lengthDoc; i += blockSize)
		{
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize) 
				grabSize = blockSize;
			
			_pEditView->getText(data, i, i + grabSize);
			fwrite(data, grabSize, 1, fp);
		}
		fclose(fp);
		//checkIfReadOnlyFile()
		_pEditView->updateCurrentBufTimeStamp();
		_pEditView->execute(SCI_SETSAVEPOINT);
		return true;
	}
	return false;
}

bool Notepad_plus::fileSave()
{
	if (_pEditView->isCurrentDocDirty())
	{
		char str[10];
		// get the title of document
		const char *fn = _pEditView->getCurrentTitle();
		
		// copy the first characters (the the number char of UNTITLED_STR) 
		// frome the title to str
		strncpy(str, fn, strlen(UNTITLED_STR));
		str[strlen(UNTITLED_STR)] = '\0';

		// if UNTITLED_STR is the prefix of title, we are sure that 
		// this document doesn't exist so we call the SaveFileDialog; 
		// otherwise we just save it by using its full path name.
		if (!strcmp(str, UNTITLED_STR))
			return fileSaveAs();
		else
			return doSave(fn);
	}
	return false;
}

bool Notepad_plus::fileSaveAll() {

	int iCurrent = _pEditView->getCurrentDocIndex();

    if (_mainWindowStatus & TWO_VIEWS_MASK)
    {
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
        int iCur = _pEditView->getCurrentDocIndex();

	    for (int i = 0 ; i < _pEditView->getNbDoc() ; i++)
	    {
		    _pDocTab->activate(i);
		    fileSave();
	    }

        _pDocTab->activate(iCur);
        
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
    }
    
    for (int i = 0 ; i < _pEditView->getNbDoc() ; i++)
	{
		_pDocTab->activate(i);
		fileSave();
	}

	_pDocTab->activate(iCurrent);
	return true;
}

bool Notepad_plus::fileSaveAs()
{
	FileDialog fDlg(_hSelf, _hInst);
	fDlg.setExtFilter("All file", "*");

	if (char *pfn = fDlg.doSaveDlg())
	{
		int i = _pEditView->findDocIndexByName(pfn);
		if ((i == -1) || (i == _pEditView->getCurrentDocIndex()))
		{
			doSave(pfn);
			_pEditView->setCurrentTitle(pfn);
            _pEditView->setCurrentDocReadOnly(false);
            _pEditView->execute(SCI_SETREADONLY, false);
			_pDocTab->updateCurrentTabItem(PathFindFileName(pfn));
			setTitleWith(pfn);
			return true;
		}
		else
		{
			::MessageBox(_hSelf, "The file is already opened in the Editor.", "ERROR", MB_OK | MB_ICONSTOP);
			_pDocTab->activate(i);
			return false;
		}
        checkModifiedDocument();
	}
	else // cancel button is pressed
    {
        checkModifiedDocument();
		return false;
    }
}

void Notepad_plus::filePrint(bool showDialog)
{
	Printer printer;

	int startPos = int(_pEditView->execute(SCI_GETSELECTIONSTART));
	int endPos = int(_pEditView->execute(SCI_GETSELECTIONEND));
	
	printer.init(_hInst, _hSelf, showDialog, startPos, endPos);
	printer.doPrint(*_pEditView);
}

#define MENU 0x01
#define TOOLBAR 0x02

void Notepad_plus::enableCommand(int cmdID, bool doEnable, int which)
{
	if (which & MENU)
	{
		enableMenu(cmdID, doEnable);
	}
	if (which & TOOLBAR)
	{
		_toolBar.enable(cmdID, doEnable);
	}
}

void Notepad_plus::checkClipboard() 
{
	bool hasSelection = _pEditView->execute(SCI_GETSELECTIONSTART) != _pEditView->execute(SCI_GETSELECTIONEND);
	bool canPaste = bool(_pEditView->execute(SCI_CANPASTE));
	enableCommand(IDM_EDIT_CUT, hasSelection, MENU | TOOLBAR); 
	enableCommand(IDM_EDIT_COPY, hasSelection, MENU | TOOLBAR);
	enableCommand(IDM_EDIT_PASTE, canPaste, MENU | TOOLBAR);
}

void Notepad_plus::checkDocState()
{
	bool isCurrentDirty = _pEditView->isCurrentDocDirty();
	bool isSeveralDirty = !_pEditView->isAllDocsClean();
	enableCommand(IDM_FILE_SAVE, isCurrentDirty, MENU | TOOLBAR);
	enableCommand(IDM_FILE_SAVEALL, isSeveralDirty, MENU | TOOLBAR);
}

void Notepad_plus::checkUndoState()
{
	enableCommand(IDM_EDIT_UNDO, (bool)_pEditView->execute(SCI_CANUNDO), MENU | TOOLBAR);
	enableCommand(IDM_EDIT_REDO, (bool)_pEditView->execute(SCI_CANREDO), MENU | TOOLBAR);
}

void Notepad_plus::synchronise()
{
    Buffer & bufSrc = _pEditView->getCurrentBuffer();
    
    const char *fn = bufSrc.getFileName();

    int i = getNonCurrentDocTab()->find(fn);
    if (i != -1)
    {
        Buffer & bufDest = getNonCurrentEditView()->getBufferAt(i);
        bufDest.synchroniseWith(bufSrc);
        getNonCurrentDocTab()->updateTabItem(i);
    }
}

void Notepad_plus::setLangStatus(LangType langType)
{
    char *pStr;
	

    switch (langType)
    {
		case L_C:
			pStr = "c source file";
            break;

		case L_H:
			pStr = "c or c++ header file";
            break;

		case L_CPP:
			pStr = "c++ source file";
            break;

		case L_JAVA:
			pStr = "Java source file";
            break;

        case L_RC :
            pStr = "Windows Resource file";
            break;
        
        case L_MAKEFILE:
            pStr = "Makefile";
            break;

		case L_HTML:
            pStr = "Hyper Text Markup Language File";
            break;

        case L_XML:
            pStr = "eXtensible Markup Language File";
            break;

		case L_PHP:
            pStr = "php File";
            break;

        case L_NFO:
            pStr = "NFO File";
            break;

        case L_USER:
            pStr = "User Define File";
            break;

        case L_M30 :
            pStr = "Macocs Script File";
            break;
        
        case L_PCOM:
            pStr = "PCOM Script File";
            break;
        default:
            pStr = "Normal text File";
    }
    _statusBar.setText(pStr);
}

void Notepad_plus::notify(SCNotification *notification)
{
  switch (notification->nmhdr.code) 
  {
	case SCN_DOUBLECLICK :
		//MessageBox(NULL, "DBL click", "SCN_DOUBLECLICK", MB_OK);
      break;

    case SCN_SAVEPOINTREACHED:
      _pEditView->setCurrentDocState(false);
	  _pDocTab->updateCurrentTabItem();
	  checkDocState();
      synchronise();
      break;

    case SCN_SAVEPOINTLEFT:
      _pEditView->setCurrentDocState(true);
	  _pDocTab->updateCurrentTabItem();
	  checkDocState();
      synchronise();
      break;
    
    case  SCN_MODIFYATTEMPTRO :
      // on fout rien
      break;
	
	case SCN_KEY:
      //MessageBox(NULL, "Putain, sa mere!", "toto", MB_OK);
      break;

	case TCN_TABDROPPEDOUTSIDE:
	case TCN_TABDROPPED:
	{
        TabBar *sender = reinterpret_cast<TabBar *>(notification->nmhdr.idFrom);
        int destIndex = sender->getTabDraggedIndex();
		int scrIndex  = sender->getSrcTabIndex();

		// if the dragNdrop tab is not the current view tab,
		// we have to set it to the current view tab
		if (notification->nmhdr.hwndFrom != _pDocTab->getHSelf())
			switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

        _pEditView->sortBuffer(destIndex, scrIndex);
        _pEditView->activateDocAt(destIndex);

        if (notification->nmhdr.code == TCN_TABDROPPEDOUTSIDE)
        {
            POINT p = sender->getDraggingPoint();

			//It's the coordinate of screen, so we can call 
			//"WindowFromPoint" function without converting the point
            HWND hWin = ::WindowFromPoint(p);
			if (hWin == _pEditView->getHSelf()) // In the same view group
			{
				// If there's only one view...
				//if (!(_mainWindowStatus & TWO_VIEWS_MASK))
				{
					
					if (!_hTabPopupDropMenu)
					{
						_hTabPopupDropMenu = ::CreatePopupMenu();
						::InsertMenu(_hTabPopupDropMenu, 0, MF_BYPOSITION, IDC_DOC_GOTO_ANOTHER_VIEW, "Go to another View");
						::InsertMenu(_hTabPopupDropMenu, 1, MF_BYPOSITION, IDC_DOC_CLONE_TO_ANOTHER_VIEW, "Clone to another View");
					}
					::TrackPopupMenu(_hTabPopupDropMenu, TPM_LEFTALIGN, p.x, p.y, 0, _hSelf, NULL);
				}
			}
			else if ((hWin == getNonCurrentDocTab()->getHSelf()) || 
				     (hWin == getNonCurrentEditView()->getHSelf())) // In the another view group
			{
				if (::GetKeyState(VK_LCONTROL) & 0x80000000)
					docGotoAnotherEditView(MODE_CLONE);
				else
					docGotoAnotherEditView(MODE_TRANSFER);
			}
			//else on fout rien!!! // It's non view group
				//::MessageBox(NULL, "chez qqn d'autre", "", MB_OK);
        }
		break;
	}

	case TCN_SELCHANGE:
	{
        char *fullPath = NULL;

        if (notification->nmhdr.hwndFrom == _mainDocTab.getHSelf())
		{
			fullPath = _mainDocTab.clickedUpdate();
            switchEditViewTo(MAIN_VIEW);
			
		}
		else if (notification->nmhdr.hwndFrom == _subDocTab.getHSelf())
		{
			fullPath = _subDocTab.clickedUpdate();
            switchEditViewTo(SUB_VIEW);
		}
        checkDocState();
		break;
	}

    case NM_RCLICK :
    {        
		if (notification->nmhdr.hwndFrom == _mainDocTab.getHSelf())
		{
            switchEditViewTo(MAIN_VIEW);
		}
        else if (notification->nmhdr.hwndFrom == _subDocTab.getHSelf())
        {
            switchEditViewTo(SUB_VIEW);   
        }
		else // From tool bar
			break;
        
		POINT p, clientPoint;
        GetCursorPos(&p);
        clientPoint.x = p.x;
        clientPoint.y = p.y;
        
        if (!_hTabPopupMenu)
		{
			_hTabPopupMenu = ::CreatePopupMenu();
			::InsertMenu(_hTabPopupMenu, 0, MF_BYPOSITION, IDM_FILE_CLOSE, "Close");
			::InsertMenu(_hTabPopupMenu, 1, MF_BYPOSITION, IDM_FILE_SAVE, "Save");
            ::InsertMenu(_hTabPopupMenu, 2, MF_BYPOSITION, IDM_FILE_SAVEAS, "Save As...");
            ::InsertMenu(_hTabPopupMenu, 3, MF_BYPOSITION, IDM_FILE_PRINT, "Print");
            
            ::InsertMenu(_hTabPopupMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
            ::InsertMenu(_hTabPopupMenu, 5, MF_BYPOSITION, IDC_DOC_GOTO_ANOTHER_VIEW, "Go to another View");
			::InsertMenu(_hTabPopupMenu, 6, MF_BYPOSITION, IDC_DOC_CLONE_TO_ANOTHER_VIEW, "Clone to another View");
		}


		//verifyPopMenuItem();
        ScreenToClient(_pDocTab->getHSelf(), &clientPoint);
        ::SendMessage(_pDocTab->getHSelf(), WM_LBUTTONDOWN, 0, MAKELONG(clientPoint.x, clientPoint.y));
        ::TrackPopupMenu(_hTabPopupMenu, TPM_LEFTALIGN, p.x, p.y, 0, _hSelf, NULL);
        break;
    }

    case NM_RELEASEDCAPTURE :
    {
        if (notification->nmhdr.hwndFrom == _pDocTab->getHSelf())
		{
            //::MessageBox(NULL, "release click!", "NM_RELEASEDCAPTURE", MB_OK);
		}
        break;
    }

    case SCN_MARGINCLICK: 
    {
        if (notification->nmhdr.hwndFrom == _mainEditView.getHSelf())
            switchEditViewTo(MAIN_VIEW);
			
		else if (notification->nmhdr.hwndFrom == _subEditView.getHSelf())
            switchEditViewTo(SUB_VIEW);
        
        if (notification->margin == ScintillaEditView::_SC_MARGE_FOLDER) 
        {
            _pEditView->marginClick(notification->position, notification->modifiers);
        }
        else if (notification->margin == ScintillaEditView::_SC_MARGE_SYBOLE)
        {
            int lineClick = int(_pEditView->execute(SCI_LINEFROMPOSITION, notification->position));
            /*char str[10]; itoa(lineClick, str, 10);
            ::MessageBox(NULL, str, "SCN_MARGINCLICK", MB_OK);
            int mark = _pEditView->execute(SCI_MARKERGET, lineClick);
            if (!mark)
                _pEditView->execute(SCI_MARKERADD, lineClick, SC_MARKNUM_FOLDER);
            else
            {
                if (mark & SC_MARK_BOXPLUS)
                {
                    _pEditView->execute(SCI_MARKERDELETE, lineClick, SC_MARKNUM_FOLDER);
                    _pEditView->execute(SCI_MARKERADD, lineClick, SC_MARKNUM_FOLDEROPEN);
                }
                else //if (mark & SC_MARKNUM_FOLDEROPEN)
                {
                    _pEditView->execute(SCI_MARKERDELETE, lineClick, SC_MARKNUM_FOLDEROPEN);
                    _pEditView->execute(SCI_MARKERADD, lineClick, SC_MARKNUM_FOLDER);
                }
            }*/
        }
	}
	break;

	default :
		break;

  }
  //return TRUE;
}

void Notepad_plus::command(int id) {
	switch (id)
	{
		case IDM_FILE_NEW:
			fileNew();
			break;
		
		case IDM_FILE_OPEN:
			fileOpen();
			break;

		case IDM_FILE_CLOSE:
			fileClose();
			break;

		case IDM_FILE_CLOSEALL:
			fileCloseAll();
			break;

		case IDM_FILE_SAVE :
			fileSave();
			break;

		case IDM_FILE_SAVEALL :
			fileSaveAll();
			break;

		case IDM_FILE_SAVEAS :
			fileSaveAs();
			break;

		case IDC_BUTTON_PRINT :
			filePrint(false);
			break;

		case IDM_FILE_PRINT :
			filePrint(true);
			break;

		case IDM_FILE_EXIT:
			if (fileCloseAll())
				destroy();
			break;

		case IDM_EDIT_UNDO:
			_pEditView->execute(WM_UNDO);
			checkClipboard();
			checkUndoState();
			break;

		case IDM_EDIT_REDO:
			_pEditView->execute(SCI_REDO);
			checkClipboard();
			checkUndoState();
			break;

		case IDM_EDIT_CUT:
			_pEditView->execute(WM_CUT);
			break;

		case IDM_EDIT_COPY:
			_pEditView->execute(WM_COPY);
			checkClipboard();
			break;

		case IDM_EDIT_PASTE:
			_pEditView->execute(WM_PASTE);
			break;

		case IDM_EDIT_DELETE:
			_pEditView->execute(WM_CLEAR);
			break;

		case IDM_EDIT_FIND :
			_findReplaceDlg.doFindDlg();
			break;

		case IDM_EDIT_FINDNEXT :
			_findReplaceDlg.processFindNext();
			break;

		case IDM_EDIT_REPLACE :
			_findReplaceDlg.doReplaceDlg();
			break;

        case IDM_LANG_USER_DLG :
        {
		    bool isUDDlgVisible = false;
                
		    UserDefineDialog *udd = _pEditView->getUserDefineDlg();
		    if (!udd->isCreated())
		    {
			    _pEditView->doUserDefineDlg();
			    // the 1st time isUDDlgVisible should be false
		    }
			else
			{
				isUDDlgVisible = udd->isVisible();
				
				bool isUDDlgDocked = udd->isDocked();
				if ((isUDDlgDocked)&&(isUDDlgVisible))
				{
					::ShowWindow(_pMainSplitter->getHSelf(), SW_HIDE);

					if (_mainWindowStatus & TWO_VIEWS_MASK)
						_pMainWindow = &_subSplitter;
					else
						_pMainWindow = _pDocTab;

					RECT rc;
					getMainClientRect(rc);
					_pMainWindow->reSizeTo(rc);
					
					udd->display(false);
					_mainWindowStatus &= ~DOCK_MASK;
				}
				else if ((isUDDlgDocked)&&(!isUDDlgVisible))
				{
                    if (!_pMainSplitter)
                    {
                        _pMainSplitter = new SplitterContainer;
                        _pMainSplitter->init(_hInst, _hSelf);

                        Window *pWindow;
                        if (_mainWindowStatus & TWO_VIEWS_MASK)
                            pWindow = &_subSplitter;
                        else
                            pWindow = _pDocTab;

                        _pMainSplitter->create(pWindow, ScintillaEditView::getUserDefineDlg(), 8, RIGHT_FIX, 45); 
                    }

					_pMainWindow = _pMainSplitter;

					_pMainSplitter->setWin0((_mainWindowStatus & TWO_VIEWS_MASK)?(Window *)&_subSplitter:(Window *)_pDocTab);

					RECT rc;
					getMainClientRect(rc);
					_pMainWindow->reSizeTo(rc);
					_pMainWindow->display();

					_mainWindowStatus |= DOCK_MASK;
				}
				else if ((!isUDDlgDocked)&&(isUDDlgVisible))
				{
					udd->display(false);
				}
				else //((!isUDDlgDocked)&&(!isUDDlgVisible))
					udd->display();
			}
			checkMenuItem(IDM_LANG_USER_DLG, !isUDDlgVisible);
			_toolBar.setCheck(IDM_LANG_USER_DLG, !isUDDlgVisible);

            break;
        }

		case IDM_EDIT_SELECTALL:
			_pEditView->execute(SCI_SELECTALL);
			checkClipboard();
			break;

		case IDM_VIEW_TOOLBAR_HIDE:
		{
            int checkedID = getToolBarState();

            if (checkedID != IDM_VIEW_TOOLBAR_HIDE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.display(false);
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
            }

            changeCheckedItemFromTo(checkedID, IDM_VIEW_TOOLBAR_HIDE);
		}
		break;

		case IDM_VIEW_TOOLBAR_REDUCE:
		{
            int checkedID = getToolBarState();

            if (checkedID != IDM_VIEW_TOOLBAR_REDUCE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.reduce();
			    _toolBar.display();
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));

                changeCheckedItemFromTo(checkedID, IDM_VIEW_TOOLBAR_REDUCE);
            }
		}
		break;

		case IDM_VIEW_TOOLBAR_ENLARGE:
		{
            int checkedID = getToolBarState();
            if (checkedID != IDM_VIEW_TOOLBAR_ENLARGE)
            {
			    RECT rc;
			    getClientRect(rc);
			    _toolBar.enlarge();
			    _toolBar.display();
			    ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));

                changeCheckedItemFromTo(checkedID, IDM_VIEW_TOOLBAR_ENLARGE);
            }
			break;
		}
		
        case IDM_VIEW_LOCKTABBAR:
		{
			bool isDrag = TabBar::doDragNDropOrNot();
            TabBar::doDragNDrop(!isDrag);
			checkMenuItem(IDM_VIEW_LOCKTABBAR, isDrag);
            break;
		}

		case IDM_VIEW_DRAWTABBAR:
		{
			bool drawIt = TabBar::isOwnerDrawTab();
			TabBar::doOwnerDrawTab(!drawIt);
			checkMenuItem(IDM_VIEW_DRAWTABBAR, !drawIt);
			break;
		}

        case IDM_VIEW_STATUSBAR:
            RECT rc;
			getClientRect(rc);
            _statusBar.display((bool)checkStatusBar());
            ::SendMessage(_hSelf, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.bottom, rc.right));
            break;

        case IDM_VIEW_LINENUMBER:
        case IDM_VIEW_SYMBOLMARGIN:
        case IDM_VIEW_FOLDERMAGIN:
        {
            int margin;
            if (id == IDM_VIEW_LINENUMBER)
                margin = ScintillaEditView::_SC_MARGE_LINENUMBER;
            else if (id == IDM_VIEW_SYMBOLMARGIN)
                margin = ScintillaEditView::_SC_MARGE_SYBOLE;
            else 
                margin = ScintillaEditView::_SC_MARGE_FOLDER;

            if (_pEditView->hasMarginShowed(margin))
                _pEditView->showMargin(margin, false);
            else
                _pEditView->showMargin(margin);

            checkMenuItem(id, _pEditView->hasMarginShowed(margin));
            break;
        }

        case IDM_VIEW_FOLDERMAGIN_SIMPLE:
        case IDM_VIEW_FOLDERMAGIN_ARROW:
        case IDM_VIEW_FOLDERMAGIN_CIRCLE:
        case IDM_VIEW_FOLDERMAGIN_BOX:
        {
            int checkedID = getFolderMarginStyle();
            if (checkedID == id) return;
            folderStyle fStyle = (id == IDM_VIEW_FOLDERMAGIN_SIMPLE)?FOLDER_STYLE_SIMPLE:\
                ((id == IDM_VIEW_FOLDERMAGIN_ARROW)?FOLDER_STYLE_ARROW:\
                ((id == IDM_VIEW_FOLDERMAGIN_CIRCLE)?FOLDER_STYLE_CIRCLE:FOLDER_STYLE_BOX));
            _pEditView->setMakerStyle(fStyle);
            changeCheckedItemFromTo(checkedID, id);
            break;
        }
		case IDM_VIEW_ALL_CHARACTERS:
		{
			HMENU hMenu = ::GetMenu(_hSelf);
			bool isChecked = (::GetMenuState(hMenu, IDM_VIEW_ALL_CHARACTERS, MF_BYCOMMAND) == MF_CHECKED);
			_pEditView->showInvisibleChars(!isChecked);

			_toolBar.setCheck(IDM_VIEW_ALL_CHARACTERS, !_toolBar.getCheckState(IDM_VIEW_ALL_CHARACTERS));
			::CheckMenuItem(hMenu, IDM_VIEW_ALL_CHARACTERS, MF_BYCOMMAND | (isChecked?MF_UNCHECKED:MF_CHECKED));
			break;
		}
			
		case IDM_VIEW_INDENT_GUIDE:
		{
			_pEditView->showIndentGuideLine(!_pEditView->isShownIndentGuide());
            _toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());
			checkMenuItem(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());
			break;
		}

		case IDM_VIEW_ZOOMIN:
			{
			_pEditView->execute(SCI_ZOOMIN);
			break;
			}
		case IDM_VIEW_ZOOMOUT:
			_pEditView->execute(SCI_ZOOMOUT);
			break;


		case IDM_EXECUTE_CONVERTOR:
			launchConvertor();
			break;

		case IDM_EXECUTE_EDITVAR:
			launchEditVar();
			break;
/*
        case IDC_DOCK_USERDEFINE_DLG :
            break;

        case IDC_UNDOCK_USERDEFINE_DLG :
            break;
*/
        case IDC_DOC_GOTO_ANOTHER_VIEW:
            docGotoAnotherEditView(MODE_TRANSFER);
            break;

        case IDC_DOC_CLONE_TO_ANOTHER_VIEW:
            docGotoAnotherEditView(MODE_CLONE);
            break;

        case IDM_ABOUT:
            _aboutDlg.doDialog();
			break;

        case IDM_LANG_C	:
            setLanguage(L_C); 
            break;

        case IDM_LANG_CPP :
            setLanguage(L_CPP); 
            break;

        case IDM_LANG_JAVA :
            setLanguage(L_JAVA); 
            break;

        case IDM_LANG_HTML :
            setLanguage(L_HTML); 
            break;

        case IDM_LANG_XML :
            setLanguage(L_XML); 
            break;

        case IDM_LANG_PHP :
            setLanguage(L_PHP); 
            break;

        case IDM_LANG_USER :
            setLanguage(L_USER); 
            break;

        case IDM_LANG_TEXT :
            setLanguage(L_TXT); 
            break;

        case IDM_LANG_RC :
            setLanguage(L_RC); 
            break;

        case IDM_LANG_MAKEFILE :
            setLanguage(L_MAKEFILE); 
            break;
	}
}

void Notepad_plus::setTitleWith(const char *filePath)
{
	if (!filePath || !strcmp(filePath, ""))
		return;

	char str2concat[MAX_PATH]; 
	strcat(strcpy(str2concat, _className), " - ");
	strcat(str2concat, filePath);
	::SetWindowText(_hSelf, str2concat);
}

void Notepad_plus::dropFiles(HDROP hdrop) 
{
	if (hdrop)
	{
		// Determinate in which view the file(s) is (are) dropped
		POINT p;
		::DragQueryPoint(hdrop, &p);
		HWND hWin = ::ChildWindowFromPoint(_hSelf, p);
		if (hWin)
		{
			if ((_mainEditView.getHSelf() == hWin) || (_mainDocTab.getHSelf() == hWin))
				switchEditViewTo(MAIN_VIEW);
			else if ((_subEditView.getHSelf() == hWin) || (_subDocTab.getHSelf() == hWin))
				switchEditViewTo(SUB_VIEW);
		}

		int filesDropped = ::DragQueryFile(hdrop, 0xffffffff, NULL, 0);
		for (int i = 0 ; i < filesDropped ; ++i)
		{
			char pathDropped[MAX_PATH];
			::DragQueryFile(hdrop, i, pathDropped, sizeof(pathDropped));
			if (!doOpen(pathDropped))
			{
				break;
			}
            setLangStatus(_pEditView->getCurrentDocType());
		}
		::DragFinish(hdrop);
		// Put Notepad_plus to forefront
		// May not work for Win2k, but OK for lower versions
		// Note: how to drop a file to an iconic window?
		// Actually, it is the Send To command that generates a drop.
		if (::IsIconic(_hSelf))
		{
			::ShowWindow(_hSelf, SW_RESTORE);
		}
		::SetForegroundWindow(_hSelf);
	}
}

void Notepad_plus::checkModifiedDocument()
{
	const int NB_VIEW = 2;
	ScintillaEditView * pScintillaArray[NB_VIEW];
	DocTabView * pDocTabArray[NB_VIEW];

	// the oder (1.current view 2.non current view) is important
	// to synchronize with "hideCurrentView" function
	pScintillaArray[0] = _pEditView;
	pScintillaArray[1] = getNonCurrentEditView();

	pDocTabArray[0] = _pDocTab;
	pDocTabArray[1] = getNonCurrentDocTab();

	for (int j = 0 ; j < NB_VIEW ; j++)
	{
		for (int i = (pScintillaArray[j]->getNbDoc()-1) ; i >= 0  ; i--)
		{
			Buffer & docBuf = pScintillaArray[j]->getBufferAt(i);
			docFileStaus fStatus = docBuf.checkFileState();
			pDocTabArray[j]->updateTabItem(i);

			if (fStatus == MODIFIED_FROM_OUTSIDE)
			{
				if (doReloadOrNot(docBuf.getFileName()) == IDYES)
				{
					pDocTabArray[j]->activate(i);
					reload(docBuf.getFileName());
				}
				docBuf.updatTimeStamp();
			}
			else if (fStatus == FILE_DELETED)
			{
				if (doCloseOrNot(docBuf.getFileName()) == IDNO)
				{
					pDocTabArray[j]->activate(i);
					//_pDocTab->closeCurrentDoc();
					if ((pScintillaArray[j]->getNbDoc() == 1) && (_mainWindowStatus & TWO_VIEWS_MASK))
					{
						pDocTabArray[j]->closeCurrentDoc();
						hideCurrentView();
					}
					else
						pDocTabArray[j]->closeCurrentDoc();
				}
			}
	        
			bool isReadOnly = pScintillaArray[j]->isCurrentBufReadOnly();
			pScintillaArray[j]->execute(SCI_SETREADONLY, isReadOnly);
			//_pDocTab->updateCurrentTabItem();
		}
	}
}

void Notepad_plus::hideCurrentView()
{
	if (_mainWindowStatus & DOCK_MASK)
	{
		_pMainSplitter->setWin0(getNonCurrentDocTab());
		//_pMainWindow = _pMainSplitter;
	}
	else // otherwise the main window is the spltter container that we just created
		_pMainWindow = getNonCurrentDocTab();
	    
	_subSplitter.display(false);
	_pEditView->display(false);
	_pDocTab->display(false);

	// resize the main window
	RECT rc;
	getMainClientRect(rc);
	_pMainWindow->reSizeTo(rc);

	switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

	//setTitleWith(_pEditView->getCurrentTitle());

	_mainWindowStatus &= ~TWO_VIEWS_MASK;
}

bool Notepad_plus::fileClose()
{
	int res;
	bool isDirty = _pEditView->isCurrentDocDirty();
	if (isDirty)
	{
		if ((res = doSaveOrNot(_pEditView->getCurrentTitle())) == IDYES)
		{
			if (!fileSave()) // the cancel button of savdialog is pressed
				return false;
		}
		else if (res == IDCANCEL)
			return false;
		// else IDNO we continue
	}
	if ((_pEditView->getNbDoc() == 1) && (_mainWindowStatus & TWO_VIEWS_MASK))
	{
		_pDocTab->closeCurrentDoc();
		hideCurrentView();
		return true;
	}
	setTitleWith(_pDocTab->closeCurrentDoc());
	return true;
}

bool Notepad_plus::fileCloseAll()
{
    if (_mainWindowStatus & TWO_VIEWS_MASK)
    {
        while (_pEditView->getNbDoc() > 1)
		    if (!fileClose())
			    return false;

	    if (!fileClose())
			return false;
    }

	while (_pEditView->getNbDoc() > 1)
		if (!fileClose())
			return false;
	return fileClose();
}

void Notepad_plus::reload(const char *fileName)
{
	FILE *fp = fopen(fileName, "rb");
	if (fp)
	{
		//setTitleWith(_pDocTab->newDoc(fileName));

		// It's VERY IMPORTANT to reset the view
		_pEditView->execute(SCI_CLEARALL);

		const int blockSize = 128 * 1024;
		char data[blockSize];

		int lenFile = int(fread(data, 1, sizeof(data), fp));
		while (lenFile > 0) {
			_pEditView->execute(SCI_ADDTEXT, lenFile, reinterpret_cast<LPARAM>(static_cast<char *>(data)));
			lenFile = int(fread(data, 1, sizeof(data), fp));
		}
		fclose(fp);

		_pEditView->getFocus();
		_pEditView->execute(SCI_SETSAVEPOINT);
		_pEditView->execute(EM_EMPTYUNDOBUFFER);
//		return true;
	}
	else
	{
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		//strcat(msg, fullPath);
		strcat(msg, fileName);
		strcat(msg, "\".");
		::MessageBox(_hSelf, msg, "ERR", MB_OK);
//		return false;
	}
}

void Notepad_plus::getMainClientRect(RECT &rc) const
{
    Window::getClientRect(rc);
	rc.top += _toolBar.getHeight() + 2;
    rc.bottom -= _toolBar.getHeight() + 2 +_statusBar.getHeight();
}

void Notepad_plus::getStatusBarClientRect(RECT & rc) const
{
    RECT rectMain;
    
    getMainClientRect(rectMain);
    getClientRect(rc);
    rc.top = rectMain.top + rectMain.bottom;
    rc.bottom = rc.bottom - rc.top;
}

void Notepad_plus::dockUserDlg()
{
    if (!_pMainSplitter)
    {
        _pMainSplitter = new SplitterContainer;
        _pMainSplitter->init(_hInst, _hSelf);

        Window *pWindow;
        if (_mainWindowStatus & TWO_VIEWS_MASK)
            pWindow = &_subSplitter;
        else
            pWindow = _pDocTab;

        _pMainSplitter->create(pWindow, ScintillaEditView::getUserDefineDlg(), 8, RIGHT_FIX, 45); 
    }

    if (_mainWindowStatus & TWO_VIEWS_MASK)
        _pMainSplitter->setWin0(&_subSplitter);
    else 
        _pMainSplitter->setWin0(_pDocTab);

    RECT rc;
    
    getMainClientRect(rc);
    _pMainSplitter->reSizeTo(rc);
    _pMainSplitter->display();

    _mainWindowStatus |= DOCK_MASK;
    _pMainWindow = _pMainSplitter;
}

void Notepad_plus::undockUserDlg()
{
    // a cause de surchargement de "display"
    ::ShowWindow(_pMainSplitter->getHSelf(), SW_HIDE);

    if (_mainWindowStatus & TWO_VIEWS_MASK)
        _pMainWindow = &_subSplitter;
    else
        _pMainWindow = _pDocTab;
    
    RECT rc;
    getMainClientRect(rc);
    _pMainWindow->reSizeTo(rc);

    _mainWindowStatus &= ~DOCK_MASK;
    (ScintillaEditView::getUserDefineDlg())->display(); 
    //(_pEditView->getUserDefineDlg())->display();
}

void Notepad_plus::docGotoAnotherEditView(bool mode)
{
    if (!(_mainWindowStatus & TWO_VIEWS_MASK))
    {
        // if there's dock dialog, it means there's also a splitter container
        // we replace the right window by sub-spltter container that we just created
        if (_mainWindowStatus & DOCK_MASK)
        {
            _pMainSplitter->setWin0(&_subSplitter);
            _pMainWindow = _pMainSplitter;
        }
        else // otherwise the main window is the spltter container that we just created
            _pMainWindow = &_subSplitter;
        
        // resize the main window
        RECT rc;
		getMainClientRect(rc);
        _pMainWindow->reSizeTo(rc);

        getNonCurrentEditView()->display();
        getNonCurrentDocTab()->display();

        _pMainWindow->display();

        // update the main window status
        _mainWindowStatus |= TWO_VIEWS_MASK;
    }

    // Bon, define the source view and the dest view
    // source view
    DocTabView *pSrcDocTab;
    ScintillaEditView *pSrcEditView;
    if (getCurrentView() == MAIN_VIEW)
    {
        // make dest view
        switchEditViewTo(SUB_VIEW);

        // make source view
        pSrcDocTab = &_mainDocTab;
        pSrcEditView = &_mainEditView;

    }
    else
    {
        // make dest view : _pDocTab & _pEditView
        switchEditViewTo(MAIN_VIEW);

        // make source view
        pSrcDocTab = &_subDocTab;
        pSrcEditView = &_subEditView;
    }

    // Maintenant, we begin to manipulate the source and the dest:
    // 1. Save the current position of the source view to transfer
    pSrcEditView->saveCurrentPos();

    // 2. Retrieve the current buffer from the source
    Buffer & buf = pSrcEditView->getCurrentBuffer();

    // 3. See if the file to transfer exist in the dest view
    //    if so, we don't transfer the file(buffer) 
    //    but activate the opened document in the dest view then beat it
    int i;
    if ( (i = _pDocTab->find(buf.getFileName())) != -1)
	{
		setTitleWith(_pDocTab->activate(i));
		_pDocTab->getFocus();
		return;
	}

    // 4. Transfer the file (buffer) into the dest view
    bool isNewDoc2Close = false;

    if ((_pEditView->getNbDoc() == 1) 
        && (!strncmp(_pEditView->getCurrentTitle(), UNTITLED_STR, sizeof(UNTITLED_STR)-1))
        && (!_pEditView->isCurrentDocDirty()) && (_pEditView->getCurrentDocLen() == 0))
    {
        isNewDoc2Close = true;
    }

    setTitleWith(_pDocTab->newDoc(buf));
    _pDocTab->updateCurrentTabItem(NULL);
    
    if (isNewDoc2Close)
        _pDocTab->closeDocAt(0);

    // 5. If it's the clone mode, we keep the document to transfer
    //    in the source view (do nothing). If it's the transfer mode
    //    we remove the file (buffer) from the source view
    if (mode != MODE_CLONE)
    {
        // Make focus to the source view
        switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

        if (_pEditView->getNbDoc() == 1)
        {
            // close the current doc in the dest view
            _pDocTab->closeCurrentDoc();
			hideCurrentView();
        }
        else
        {
            // Make focus to the source view
            //switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);

            // close the current doc in the dest view
            _pDocTab->closeCurrentDoc();

            // return to state where the focus is on dest view
            switchEditViewTo((getCurrentView() == MAIN_VIEW)?SUB_VIEW:MAIN_VIEW);
        }
    }
}

ToolBarButtonUnit toolBarIcons[] = {
	{IDM_FILE_NEW,		IDI_NEW_OFF_ICON,		IDI_NEW_ON_ICON,		IDI_NEW_OFF_ICON},
	{IDM_FILE_OPEN,		IDI_OPEN_OFF_ICON,		IDI_OPEN_ON_ICON,		IDI_NEW_OFF_ICON},
	{IDM_FILE_SAVE,		IDI_SAVE_OFF_ICON,		IDI_SAVE_ON_ICON,		IDI_SAVE_DISABLE_ICON},
	{IDM_FILE_SAVEALL,	IDI_SAVEALL_OFF_ICON,	IDI_SAVEALL_ON_ICON,	IDI_SAVEALL_DISABLE_ICON},
	{IDM_FILE_CLOSE,	IDI_CLOSE_OFF_ICON,		IDI_CLOSE_ON_ICON,		IDI_CLOSE_OFF_ICON},
	{IDM_FILE_CLOSEALL,	IDI_CLOSEALL_OFF_ICON,	IDI_CLOSEALL_ON_ICON,	IDI_CLOSEALL_OFF_ICON},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_EDIT_CUT,		IDI_CUT_OFF_ICON,		IDI_CUT_ON_ICON,		IDI_CUT_DISABLE_ICON},
	{IDM_EDIT_COPY,		IDI_COPY_OFF_ICON,		IDI_COPY_ON_ICON,		IDI_COPY_DISABLE_ICON},
	{IDM_EDIT_PASTE,	IDI_PASTE_OFF_ICON,		IDI_PASTE_ON_ICON,		IDI_PASTE_DISABLE_ICON},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_EDIT_UNDO,		IDI_UNDO_OFF_ICON,		IDI_UNDO_ON_ICON,		IDI_UNDO_DISABLE_ICON},
	{IDM_EDIT_REDO,		IDI_REDO_OFF_ICON,		IDI_REDO_ON_ICON,		IDI_REDO_DISABLE_ICON},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDM_EDIT_FIND,		IDI_FIND_OFF_ICON,		IDI_FIND_ON_ICON,		IDI_FIND_OFF_ICON},
	{IDM_EDIT_REPLACE,  IDI_REPLACE_OFF_ICON,	IDI_REPLACE_ON_ICON,	IDI_REPLACE_OFF_ICON},
	 
	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

	{IDM_VIEW_ZOOMIN,	IDI_ZOOMIN_OFF_ICON,	IDI_ZOOMIN_ON_ICON,		IDI_ZOOMIN_OFF_ICON},
	{IDM_VIEW_ZOOMOUT,	IDI_ZOOMOUT_OFF_ICON,	IDI_ZOOMOUT_ON_ICON,	IDI_ZOOMOUT_OFF_ICON},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//

 {IDM_VIEW_ALL_CHARACTERS,  IDI_VIEW_ALL_CHAR_OFF_ICON,	IDI_VIEW_ALL_CHAR_ON_ICON,	IDI_VIEW_ALL_CHAR_OFF_ICON},
 {IDM_VIEW_INDENT_GUIDE,  IDI_VIEW_INDENT_OFF_ICON,	IDI_VIEW_INDENT_ON_ICON,	IDI_VIEW_INDENT_OFF_ICON},
 {IDM_LANG_USER_DLG,  IDI_VIEW_UD_DLG_OFF_ICON,	IDI_VIEW_UD_DLG_ON_ICON,	IDI_VIEW_UD_DLG_OFF_ICON},

	//-------------------------------------------------------------------------------------//
	{0,					IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON,		IDI_SEPARATOR_ICON},
	//-------------------------------------------------------------------------------------//
	 
	{IDC_BUTTON_PRINT,	IDI_PRINT_OFF_ICON,		IDI_PRINT_ON_ICON,		IDI_PRINT_OFF_ICON}
};    
					
int docTabIconIDs[] = {IDI_SAVED_ICON, IDI_UNSAVED_ICON, IDI_READONLY_ICON};

LRESULT Notepad_plus::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_CREATE:
		{
            //-- Tool Bar Section --//
            toolBarStatusType tbStatus = (NppParameters::getInstance())->getToolBarStatus();
            
            // TB_LARGE par default
            int iconSize = 32;
            bool willBeShown = true;
            int menuID = IDM_VIEW_TOOLBAR_ENLARGE;

            if (tbStatus == TB_HIDE)
            {
                willBeShown = false;
                menuID = IDM_VIEW_TOOLBAR_HIDE;
            }
            else if (tbStatus == TB_SMALL)
            {
                iconSize = 16;
                menuID = IDM_VIEW_TOOLBAR_REDUCE;
            }
			_toolBar.init(_hInst, hwnd, iconSize, toolBarIcons, sizeof(toolBarIcons)/sizeof(ToolBarButtonUnit));
            _toolBar.display(willBeShown);
            changeCheckedItemFromTo(0, menuID);

            _pDocTab = &_mainDocTab;
            _pEditView = &_mainEditView;

			_mainEditView.init(_hInst, hwnd);
            _subEditView.init(_hInst, hwnd);
			_mainEditView.display();
			
            //checkMarginMenu(IDM_VIEW_LINENUMBER, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_LINENUMBER));
            //checkMarginMenu(IDM_VIEW_SYMBOLMARGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_SYBOLE));
            //checkMarginMenu(IDM_VIEW_FOLDERMAGIN, _pEditView->hasMarginShowed(ScintillaEditView::_SC_MARGE_FOLDER));

           // changeCheckedItemFromTo(0, getFolderMaginStyleIDFrom(_pEditView->getFolderStyle()));
			
			_pEditView->showIndentGuideLine();
            //_toolBar.setCheck(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());
			//checkMenuItem(IDM_VIEW_INDENT_GUIDE, _pEditView->isShownIndentGuide());

			_docTabIconList.create(20, _hInst, docTabIconIDs, sizeof(docTabIconIDs)/sizeof(int));
			
            const char * str = _mainDocTab.init(_hInst, hwnd, &_mainEditView, &_docTabIconList);
			setTitleWith(str);
            _subDocTab.init(_hInst, hwnd, &_subEditView, &_docTabIconList);
			TabBar::doDragNDrop(true);
			_mainDocTab.display();

			checkMenuItem(IDM_VIEW_LOCKTABBAR, !TabBar::doDragNDropOrNot());
			checkMenuItem(IDM_VIEW_DRAWTABBAR, TabBar::isOwnerDrawTab());

            //--Splitter Section--//
            bool isVertical = ((NppParameters::getInstance())->getScintillaSplitterPos() == POS_VERTICAL);

            _subSplitter.init(_hInst, _hSelf);
            _subSplitter.create(&_mainDocTab, &_subDocTab, 8, DYNAMIC, 50, isVertical);

            //--Status Bar Section--//
            willBeShown = (NppParameters::getInstance())->willStatusBarBeShowed();
            _statusBar.init(_hInst, hwnd);
            _statusBar.display(willBeShown);
            checkMenuItem(IDM_VIEW_STATUSBAR, willBeShown);

            _findReplaceDlg.init(_hInst, hwnd, &_pEditView);
            _aboutDlg.init(_hInst, hwnd);

            _pMainWindow = &_mainDocTab;

            //--User Define Dialog Section--//
            int uddStatus = (NppParameters::getInstance())->getUserDefineDlgStatus();
		    UserDefineDialog *udd = _pEditView->getUserDefineDlg();
		    udd->create(IDD_USER_DEFINE_BOX);
			
		//_pEditView->doUserDefineDlg();

          switch (uddStatus)
            {
                case 0 :                        // hide & undocked
                    udd->display(false);
                    break;
                case UDD_SHOW :                 // show & undocked
		            udd->display(true);
                    break;
                case UDD_DOCKED : {              // hide & docked
                    //>doDialog();
                    //::SendMessage(udd->getHSelf(), WM_COMMAND, IDC_DOCK_BUTTON, 0);
                    udd->changeStyle();
                    //bool isV = udd->isVisible();
                    //::SendMessage(_hSelf, WM_COMMAND, IDM_LANG_USER_DLG, 0);
                    udd->display(false);
                    break;}
                case (UDD_SHOW | UDD_DOCKED) :    // show & docked
		            udd->doDialog();
		            ::SendMessage(udd->getHSelf(), WM_COMMAND, IDC_DOCK_BUTTON, 0);
                    break;
            }
			dynamicCheckMenuAndTB();
			return TRUE;
		}

		case WM_DRAWITEM :
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_TAB)
			{
				return ::SendMessage(dis->hwndItem, WM_DRAWITEM, wParam, lParam);
			}
		}

		case WM_DOCK_USERDEFINE_DLG:
		{
			dockUserDlg();
			return TRUE;
		}

        case WM_UNDOCK_USERDEFINE_DLG:
		{
            undockUserDlg();
			return TRUE;
		}

		case WM_CLOSE_USERDEFINE_DLG :
		{
			checkMenuItem(IDM_LANG_USER_DLG, false);
			_toolBar.setCheck(IDM_LANG_USER_DLG, false);
			return TRUE;
		}

		case WM_SIZE:
		{
			RECT rc;

			getMainClientRect(rc);
            _pMainWindow->reSizeTo(rc);

            getStatusBarClientRect(rc);
            _statusBar.reSizeTo(rc);
			return TRUE;
		}

		case WM_MOVE:
		{
			redraw();
			return TRUE;
		}
        case WM_COPYDATA :
        {
            COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT *)lParam;
            FileNamStringSpliter fnss((const char *)pCopyData->lpData);
            char *pFn = NULL;
            for (int i = 0 ; i < fnss.size() ; i++)
            {
                pFn = (char *)fnss.getFileName(i);
                doOpen((const char *)pFn);
            }
            return TRUE;
        }

		case WM_COMMAND:
            if (HIWORD(wParam) == SCEN_SETFOCUS)
            {
                switchEditViewTo((lParam == (LPARAM)_mainEditView.getHSelf())?MAIN_VIEW:SUB_VIEW);
            }
            else
                command(LOWORD(wParam));

			return TRUE;

		case WM_NOTIFY:
		{
			checkClipboard();
			checkUndoState();
			notify(reinterpret_cast<SCNotification *>(lParam));
			return FALSE;
		}

		case WM_ACTIVATEAPP :
		{
			if (LOWORD(wParam))
			{
				checkModifiedDocument();
				return FALSE;
			}
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
		}
/*
		case WM_KILLFOCUS :
		{
			if ((HWND)wParam == _hSelf)
			{
				_gotFocus = false;
				::MessageBox(NULL, "chui nik?!!!", "WM_KILLFOCUS", MB_OK);
				return TRUE;
			}
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
		}
*/
		case WM_DROPFILES:
		{
			dropFiles(reinterpret_cast<HDROP>(wParam));
			return TRUE;
		}

		case WM_CLOSE:
		{
			if (fileCloseAll())
				destroy();
			return TRUE;
		}

		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			break;
		}
/*
		case WM_KEYDOWN :
		{
			if (wParam == VK_LCONTROL)
				::SendMessage(_pDocTab->getHSelf(), WM_KEYDOWN, wParam, lParam);
			return TRUE;
		}
*/
		default:
		{
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
		}
	}
	return FALSE;
}

LRESULT CALLBACK Notepad_plus::Notepad_plus_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch(Message)
  {
    case WM_NCCREATE : // First message we get the ptr of instantiated object
                       // then stock it into GWL_USERDATA index in order to retrieve afterward
	{
		Notepad_plus *pM30ide = (Notepad_plus *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		pM30ide->_hSelf = hwnd;
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)pM30ide);

		return TRUE;
	}

    default :
    {
      return ((Notepad_plus *)::GetWindowLong(hwnd, GWL_USERDATA))->runProc(hwnd, Message, wParam, lParam);
    }
  }
}


//-----------------------------//
// Execute the executable file //
//-----------------------------//
void Notepad_plus::launchConvertor()
{
	Process CVTSR("D:\\PowerEditor\\PowerEditor\\visual.net\\Debug\\Cvtsr.exe", "D:\\PowerEditor\\PowerEditor\\visual.net\\Debug");
	CVTSR.run();
}

void Notepad_plus::launchEditVar()
{

	Process editvar("toto.exe", ".", CONSOLE_PROG);
	editvar.run();
	int code = editvar.getExitCode();
	char codeStr[256];
	sprintf(codeStr, "launchEditVar : %0.4X", code);
	//_itoa(code, codeStr, 16);
	::MessageBox(_hSelf, (char *)editvar.getStdout(), codeStr, MB_OK);
	if (editvar.hasStderr())
		::MessageBox(_hSelf, (char *)editvar.getStderr(), codeStr, MB_OK);

//	ProcessThread editvar("TEST", "toto.exe", ".", _hSelf);
//	editvar.run();
}
