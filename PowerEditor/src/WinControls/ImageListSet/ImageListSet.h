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

#ifndef IMAGE_LIST_H
#define IMAGE_LIST_H

#include <windows.h>
#include <commctrl.h>
#include <vector>

const int nbMax = 30;
#define	IDI_SEPARATOR_ICON -1

class IconList
{
public :
	IconList() : _hImglst(NULL) {};

	void create(HINSTANCE hInst, int iconSize) {_iconSize = iconSize;
		_hInst = hInst;
		InitCommonControls(); 
		_hImglst = ImageList_Create(iconSize, iconSize, ILC_COLOR32 | ILC_MASK, 0, nbMax);
		if (!_hImglst)
			throw int(25);
	};

	void create(int iconSize, HINSTANCE hInst, int *iconIDArray, int iconIDArraySize) {
		create(hInst, iconSize);
		for (int i = 0 ; i < iconIDArraySize ; i++)
			addIcon(iconIDArray[i]);
	};

	void destroy() {
		ImageList_Destroy(_hImglst);
	};

	HIMAGELIST getHandle() const {return _hImglst;};

	void addIcon(int iconID) const {
		HICON hIcon = ::LoadIcon(_hInst, MAKEINTRESOURCE(iconID));
		//HBITMAP hBmp = (HBITMAP)::LoadImage(_hInst, MAKEINTRESOURCE(iconID), IMAGE_ICON, _iconSize, _iconSize, LR_LOADMAP3DCOLORS);
		if (!hIcon)
			throw int(26);
		ImageList_AddIcon(_hImglst, hIcon);
		//ImageList_AddMasked(_hImglst, (HBITMAP)hBmp, RGB(0, 0, 0));
		::DeleteObject(hIcon);
		//::DeleteObject(hBmp);
	};

private :
	HIMAGELIST _hImglst;
	HINSTANCE _hInst;
	int _iconSize;
};

typedef struct 
{	
	int _cmdID;

	int _defaultIcon;
	int _hotIcon;
	int _grayIcon;
}ToolBarButtonUnit;

typedef std::vector<ToolBarButtonUnit> ToolBarIconIDs;

typedef std::vector<IconList> IconListVector;

class IconLists
{
public :
	IconLists() {};
	HIMAGELIST getImageListHandle(int index) const {
		return _iconListVector[index].getHandle();
	};

protected :
	IconListVector _iconListVector;
};

const int HLIST_DEFAULT = 0;
const int HLIST_HOT = 1;
const int HLIST_DISABLE = 2;

class ToolBarIcons : public IconLists
{
public :
	ToolBarIcons() : _nbCmd(0) {};

	void init(ToolBarButtonUnit *buttonUnitArray, int arraySize);
	void create(HINSTANCE hInst, int iconSize);
	void destroy();

	HIMAGELIST getDefaultLst() const {
		return IconLists::getImageListHandle(HLIST_DEFAULT);
	};

	HIMAGELIST getHotLst() const {
		return IconLists::getImageListHandle(HLIST_HOT);
	};

	HIMAGELIST getDisableLst() const {
		return IconLists::getImageListHandle(HLIST_DISABLE);
	};

	int getNbCommand() const {return _nbCmd;};
	int getCommandAt(int index) const {return _cmdArray[index];};
	void resizeIcon(int size) {
		reInit(size);
	};

	void reInit(int size) {
		ImageList_SetIconSize(getDefaultLst(), size, size);
		ImageList_SetIconSize(getHotLst(), size, size);
		ImageList_SetIconSize(getDisableLst(), size, size);

		for (int i = 0 ; i < int(_tbiis.size()) ; i++)
		{
			//_cmdArray[_nbCmd++] = _tbiis[i]._cmdID;
			if (_tbiis[i]._defaultIcon != -1)
			{
				_iconListVector[HLIST_DEFAULT].addIcon(_tbiis[i]._defaultIcon);
				_iconListVector[HLIST_HOT].addIcon(_tbiis[i]._hotIcon);
				_iconListVector[HLIST_DISABLE].addIcon(_tbiis[i]._grayIcon);
			}
		}
	};
private :
	ToolBarIconIDs _tbiis;
	int _cmdArray[nbMax];
	int _nbCmd;
};

#endif //IMAGE_LIST_H
