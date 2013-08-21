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

#ifndef SCINTILLA_EDIT_VIEW_H
#define SCINTILLA_EDIT_VIEW_H

#include <vector>
#include "Window.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "Buffer.h"
#include "colors.h"
#include "SysMsg.h"
#include "UserDefineDialog.h"

#define NB_WORD_LIST 4
#define WORD_LIST_LEN 256

typedef int (* SCINTILLA_FUNC) (void*, int, int, int);
typedef void * SCINTILLA_PTR;
typedef std::vector<Buffer> buf_vec_t;

#define DEF_SCINTILLA               1000
#define WM_DOCK_USERDEFINE_DLG      (WM_USER + DEF_SCINTILLA + 1)
#define WM_UNDOCK_USERDEFINE_DLG    (WM_USER + DEF_SCINTILLA + 2)
#define WM_CLOSE_USERDEFINE_DLG		(WM_USER + DEF_SCINTILLA + 3)

#define LINEDRAW_FONT  "LINEDRAW.TTF"
//const bool DOCK = true;
//const bool UNDOCK = false;
const int NB_FOLDER_STATE = 7;

enum folderStyle {FOLDER_TYPE, FOLDER_STYLE_SIMPLE, FOLDER_STYLE_ARROW, FOLDER_STYLE_CIRCLE, FOLDER_STYLE_BOX};

const char PHPKeyWords2[] = "and argv as argc break case cfunction class continue declare default do "
                            "die echo else elseif empty enddeclare endfor endforeach endif endswitch "
                            "endwhile e_all e_parse e_error e_warning eval exit extends false for "
                            "foreach function global http_cookie_vars http_get_vars http_post_vars "
                            "http_post_files http_env_vars http_server_vars if include include_once "
                            "list new not null old_function or parent php_os php_self php_version "
                            "print require require_once return static switch stdclass this true var "
                            "xor virtual while __file__ __line__ __sleep __wakeup";

class ScintillaEditView : public Window
{
public:
	ScintillaEditView()
		: Window(), _pScintillaFunc(NULL),_pScintillaPtr(NULL),
		  _currentIndex(0),_MSLineDrawFont(0), _folderStyle(FOLDER_STYLE_BOX)
	{
		++_refCount;

        for (int i = 0 ; i < NB_WORD_LIST ; i++)
            memset(_wordListArray[i], 0, WORD_LIST_LEN);
	};

	virtual ~ScintillaEditView()
	{
		--_refCount;

		if ((!_refCount)&&(_hLib))
		{
			::FreeLibrary(_hLib);
		}
        if (_MSLineDrawFont)
        {
            ::RemoveFontResource(LINEDRAW_FONT);
        }
	};
	virtual void destroy()
	{
		removeAllUnusedDocs();
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	};

	virtual void init(HINSTANCE hInst, HWND hPere);

	LRESULT execute(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const {
		return _pScintillaFunc(_pScintillaPtr, static_cast<int>(Msg), static_cast<int>(wParam), static_cast<int>(lParam));
	};
	
	void defineDocType(LangType typeDoc);

    void setCurrentDocType(LangType typeDoc) {
        if (_buffers[_currentIndex]._lang == typeDoc)
            return;
        _buffers[_currentIndex]._lang = typeDoc;
        defineDocType(typeDoc);
    };

	char * attatchDefaultDoc(int nb);

	int findDocIndexByName(const char *fn) const;
	char * activateDocAt(int index);
	char * createNewDoc(const char *fn);
	char * createNewDoc(int nbNew);
	int getCurrentDocIndex() const {return _currentIndex;};
	const char * getCurrentTitle() const {return _buffers[_currentIndex]._fullPathName;};
	int setCurrentTitle(const char *fn) {
		_buffers[_currentIndex].setFileName(fn);
		defineDocType(_buffers[_currentIndex]._lang);
		return _currentIndex;
	};
	int closeCurrentDoc(int & i2Activate);
    void closeDocAt(int i2Close);

	void removeAllUnusedDocs();
	void getText(char *dest, int start, int end);

	void setCurrentDocState(bool isDirty) {
		_buffers[_currentIndex]._isDirty = isDirty;
	};
	
	bool isCurrentDocDirty() const {
		return _buffers[_currentIndex]._isDirty;
	};

    void setCurrentDocReadOnly(bool isReadOnly) {
        _buffers[_currentIndex]._isReadOnly = isReadOnly;
    };
	
    bool isCurrentBufReadOnly() {
		return _buffers[_currentIndex]._isReadOnly;
	};

    void setCurrentDocLang(LangType lang) {
		_buffers[_currentIndex]._lang = lang;
	};

	bool isAllDocsClean() const {
		for (int i = 0 ; i < static_cast<int>(_buffers.size()) ; i++)
			if (_buffers[i]._isDirty)
				return false;
		return true;
	};

	int getNbDoc() const {
		return static_cast<int>(_buffers.size());
	};

	void saveCurrentPos()
	{
		int topLine = static_cast<int>(execute(SCI_GETFIRSTVISIBLELINE));
		_buffers[_currentIndex]._pos._fistVisibleLine = topLine;

		_buffers[_currentIndex]._pos._startPos = int(execute(SCI_GETSELECTIONSTART));
		_buffers[_currentIndex]._pos._endPos = int(execute(SCI_GETSELECTIONEND));
	};

	void restoreCurrentPos(const Position & prevPos)
	{		
		int scroll2Top = 0 - (int(execute(SCI_GETLINECOUNT)) + 1);
		execute(SCI_LINESCROLL, 0, scroll2Top);
		
		execute(SCI_LINESCROLL, 0, _buffers[_currentIndex]._pos._fistVisibleLine);
		execute(SCI_SETSELECTIONSTART, _buffers[_currentIndex]._pos._startPos);
		execute(SCI_SETSELECTIONEND, _buffers[_currentIndex]._pos._endPos);
	};

	Buffer & getBufferAt(int index) {
		if (index >= int(_buffers.size()))
			throw int(3615);
		return _buffers[index];
	};

	void updateCurrentBufTimeStamp() {
		_buffers[_currentIndex].updatTimeStamp();
	};

	int getLineStrAt(int nbLine, char *buf, bool activatReturn) {
		int nbChar = int(execute(SCI_LINELENGTH, nbLine));
		execute(SCI_GETLINE, nbLine, (LPARAM)buf);
		if (!activatReturn)
			// eliminate '\n' (0D 0A)
			nbChar -= 2;
		buf[nbChar] = '\0';
		return nbChar;
	};

	int getCurrentDocLen() const {
		return int(execute(SCI_GETLENGTH));
	};

	CharacterRange getSelection() const {
		CharacterRange crange;
		crange.cpMin = long(execute(SCI_GETSELECTIONSTART));
		crange.cpMax = long(execute(SCI_GETSELECTIONEND));
		return crange;
	};

    LangType getCurrentDocType() const {
        return _buffers[_currentIndex]._lang;
    };

    void doUserDefineDlg(bool willBeShown = true) {
        _userDefineDlg.doDialog(willBeShown);
    };

    void setKeyWord(int index, const char *words) {
        strcpy(_wordListArray[index], words);
    };

    static UserDefineDialog * getUserDefineDlg() {return &_userDefineDlg;};

    //void changeUserDefineDlgStyle() {_userDefineDlg.changeStyle();};

    void setCaretColorWidth(int color, int width = 1) const {
        execute(SCI_SETCARETFORE, color);
        execute(SCI_SETCARETWIDTH, width);
    };

    void setSelectionColor(int foreColor, int backColor) const {
        execute(SCI_SETSELFORE, 1, foreColor);
        execute(SCI_SETSELBACK, 1, backColor);
    };

    int addBuffer(Buffer & buffer) {
        _buffers.push_back(buffer);
        return (int(_buffers.size()) - 1);
    };

    Buffer & getCurrentBuffer() {
        return getBufferAt(_currentIndex);
    };

	void beSwitched() {
		_userDefineDlg.setScintilla(this);
	};

    void showMargin() {
        execute(SCI_SETMARGINWIDTHN, 0, 32);
    };

    //Marge memeber and method
    static const int _SC_MARGE_LINENUMBER;
    static const int _SC_MARGE_SYBOLE;
    static const int _SC_MARGE_FOLDER;

    void showMargin(int witchMarge, bool willBeShowed = true) {
        execute(SCI_SETMARGINWIDTHN, witchMarge, willBeShowed?((witchMarge == _SC_MARGE_LINENUMBER)?32:16):0);
    };

    bool hasMarginShowed(int witchMarge) {
        return (bool)execute(SCI_GETMARGINWIDTHN, witchMarge, 0);
    };
    
    void marginClick(int position, int modifiers);

    void setMakerStyle(folderStyle style) {
        if (_folderStyle == style)
            return;
        _folderStyle = style;
        for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
            defineMarker(_markersArray[FOLDER_TYPE][i], _markersArray[style][i], white, black);
    };

    folderStyle getFolderStyle() {return _folderStyle;};

	void showWSAndTab(bool willBeShowed = true) {
		execute(SCI_SETVIEWWS, willBeShowed?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE);
	};

	void showEOL(bool willBeShowed = true) {
		execute(SCI_SETVIEWEOL, willBeShowed);
	};

	void showInvisibleChars(bool willBeShowed = true) {
		showWSAndTab(willBeShowed);
		showEOL(willBeShowed);
	};

	bool isInvisibleCharsShown(bool willBeShowed = true) {
		return bool(execute(SCI_GETVIEWWS));
	};

	void showIndentGuideLine(bool willBeShowed = true) {
		execute(SCI_SETINDENTATIONGUIDES, (WPARAM)willBeShowed);
	};

	bool isShownIndentGuide() {
		return bool(execute(SCI_GETINDENTATIONGUIDES));
	};

    void sortBuffer(int destIndex, int scrIndex) {
		// Do nothing if there's no change of the position
		if (scrIndex == destIndex)
			return;

        Buffer buf2Insert = _buffers.at(scrIndex);
        _buffers.erase(_buffers.begin() + scrIndex);
        _buffers.insert(_buffers.begin() + destIndex, buf2Insert);
    };

private:
	static HINSTANCE _hLib;
	static int _refCount;
    static UserDefineDialog _userDefineDlg;

    static const int _markersArray[][NB_FOLDER_STATE];

    folderStyle _folderStyle;

	SCINTILLA_FUNC _pScintillaFunc;
	SCINTILLA_PTR  _pScintillaPtr;
    
	// the list of docs
	buf_vec_t _buffers;

    // For the file nfo
    int _MSLineDrawFont;

	// the current active buffer index of _buffers
	int _currentIndex;
    
    char _wordListArray[NB_WORD_LIST][WORD_LIST_LEN];

	void setStyle(int whichStyle, COLORREF fore, COLORREF back=white, int size=-1, const char *face=0) const;
	void setFont(int which, const char *fontName, bool isBold=false, bool isItalic=false) const;

    void setCppLexer(LangType type);
	void setM30Lexer();
	void setXmlLexer(LangType type);
	void setPcomLexer();
	void setUserLexer();
	void setEmbeddedJSLexer(COLORREF bkColor, int fontSize = 9, const char *font = "Comic Sans MS");
    void setPhpEmbeddedLexer(COLORREF bkColor = white, int fontSize =9, const char *font = 0);
	void setMakefileLexer();

    void defineMarker(int marker, int markerType, COLORREF fore, COLORREF back) {
	    execute(SCI_MARKERDEFINE, marker, markerType);
	    execute(SCI_MARKERSETFORE, marker, fore);
	    execute(SCI_MARKERSETBACK, marker, back);
    };

	void expand(int &line, bool doExpand, bool force = false, int visLevels = 0, int level = -1);
};

#endif //SCINTILLA_EDIT_VIEW_H
