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

#ifndef PARAMETERS_H
#define PARAMETERS_H

enum toolBarStatusType {TB_HIDE, TB_SMALL, TB_LARGE};

const bool POS_VERTICAL = true;
const bool POS_HORIZOTAL = false;

const int UDD_SHOW   = 1; // 0000 0001 
const int UDD_DOCKED = 2; // 0000 0010

// 0 : 0000 0000 hide & undocked
// 1 : 0000 0001 show & undocked
// 2 : 0000 0010 hide & docked
// 3 : 0000 0011 show & docked

struct ScintillaViewParams
{
	bool _lineNumberMarginShow;
	bool _bookMarkMarginShow;
	bool _folderMarginShow;
	int _folderStyle; //"simple", "arrow", "circle" and "box"
	bool _invisibleCharsShow;
	bool _indentGuideLineShow;
};            
          
class NppParameters 
{
public:
    static NppParameters * getInstance() {return _pSelf;};
    void destroyInstance(){delete _pSelf;};

    toolBarStatusType getToolBarStatus() {return _toolBarStatus;};
    bool willStatusBarBeShowed() {return _statusBarShow;};
    bool getScintillaSplitterPos() {return _splitterPos;};
    int getUserDefineDlgStatus() {return _userDefineDlgStatus;};

private:
    NppParameters() 
    {
        _toolBarStatus = TB_LARGE;
        _statusBarShow = true;
        _splitterPos = POS_HORIZOTAL;
        _userDefineDlgStatus = /*0; UDD_SHOW |*/ UDD_DOCKED;
    };
    static NppParameters *_pSelf;
	toolBarStatusType _toolBarStatus;		// small, large ou hide
	bool _statusBarShow;		// show ou hide
	bool _splitterPos;			// horizontal ou vertical
	int _userDefineDlgStatus;	// (hide||show) && (docked||undocked)


	ScintillaViewParams _svp[2];
};

#endif //PARAMETERS_H