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

#ifndef BUFFER_H
#define BUFFER_H

#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Scintilla.h"
const char UNTITLED_STR[] = "new ";
typedef sptr_t Document;
enum LangType {L_PHP ,L_H, L_C, L_CPP, L_JAVA, L_RC, L_HTML, L_XML, L_MAKEFILE, L_M30, L_PCOM, L_NFO, L_USER, L_TXT};
enum docFileStaus{NEW_DOC, FILE_DELETED, NO_PROBLEM, MODIFIED_FROM_OUTSIDE};

struct Position
{ 
	int _fistVisibleLine;
	int _startPos;
	int _endPos;
};

class Buffer
{
friend class ScintillaEditView;
public :
	Buffer(Document doc, const char *fileName)
		: _doc(doc), _isDirty(false), _isReadOnly(false)  
	{
		_pos._fistVisibleLine = 0;
		_pos._startPos = 0;
		_pos._endPos = 0;
		setFileName(fileName);
	};

    Buffer(const Buffer & buf) : _isDirty(buf._isDirty),  _doc(buf._doc), _lang(buf._lang),
        _timeStamp(buf._timeStamp), _isReadOnly(buf._isReadOnly), _pos(buf._pos)
    {
        strcpy(_fullPathName, buf._fullPathName);
    };

    Buffer & operator=(const Buffer & buf)
    {
        if (this != &buf)
        {
            this->_isDirty = buf._isDirty;
            this->_doc = buf._doc;
            this->_lang = buf._lang;
            this->_timeStamp = buf._timeStamp;
            this->_isReadOnly = buf._isReadOnly;
            this->_pos = buf._pos;
            
            strcpy(this->_fullPathName, buf._fullPathName);
        }
        return *this;
    }


	// this method 1. copies the file name
	//             2. determinates the language from the ext of file name
	//             3. gets the last modified time
	void setFileName(const char *fn) 
	{
		strcpy(_fullPathName, fn);
        if (PathFileExists(_fullPathName))
		{
			// for _lang
			char *ext = PathFindExtension(_fullPathName);

			if (!_stricmp(ext, ".h"))
				_lang = L_H;

			else if (!_stricmp(ext, ".c"))
				_lang = L_C;

            else if ((!_stricmp(ext, ".cpp"))||(!_stricmp(ext, ".cxx"))||
					(!_stricmp(ext, ".h")))
                _lang = L_CPP;

			else if (!_stricmp(ext, ".java"))
				_lang = L_JAVA;

			else if (!_stricmp(ext, ".rc"))
				_lang = L_RC;

			else if ((!_stricmp(ext, ".html"))||(!_stricmp(ext, ".htm")))
				_lang = L_HTML;

            else if (!_stricmp(ext, ".xml")) 
				_lang = L_XML;

			else if ((!_stricmp(ext, ".php")) || (!_stricmp(ext, ".phtml"))) 
				_lang = L_PHP;

            else if ((!_stricmp(ext, ".scr"))||(!_stricmp(ext, ".res"))||
                    (!_stricmp(ext, ".re2")))
                _lang = L_M30;

			else if (!_stricmp(ext, ".cmd"))
				_lang = L_PCOM;

            else if (!_stricmp(ext, ".nfo"))
                _lang = L_NFO;

			else
			{
				char *fileName = PathFindFileName(_fullPathName);

				if (!_stricmp(fileName, "makefile"))
					_lang = L_MAKEFILE;
				else
					_lang = L_TXT;
			}
			// for _timeStamp
			updatTimeStamp();
		}
		else
		{
			_lang = L_USER;
			_timeStamp = 0;
		}
	};

	const char * getFileName() {return _fullPathName;};
	void setLang(LangType lang) {_lang = lang;};

	void updatTimeStamp() {
		struct _stat buf;
		_timeStamp = (_stat(_fullPathName, &buf)==0)?buf.st_mtime:0;
	};

	docFileStaus checkFileState() {
		char str[10];
		strncpy(str, _fullPathName, strlen(UNTITLED_STR));
		str[strlen(UNTITLED_STR)] = '\0';
		if (!strcmp(str, UNTITLED_STR))
		{
			_isReadOnly = false;
			return NEW_DOC;
		}
        if (!PathFileExists(_fullPathName))
		{
			_isReadOnly = false;
			return FILE_DELETED;
		}
		struct _stat buf;
		if (!_stat(_fullPathName, &buf))
		{
			_isReadOnly = (bool)(!(buf.st_mode & _S_IWRITE));

			if (_timeStamp != buf.st_mtime)
				return MODIFIED_FROM_OUTSIDE;
		}
		return NO_PROBLEM;
	};

	// to use this method with open and save
	void checkIfReadOnlyFile() {
		struct _stat buf;
		if (!_stat(_fullPathName, &buf))
		{
			_isReadOnly = (bool)(!(buf.st_mode & _S_IWRITE));
		}
	};

    bool isDirty() const {
        return _isDirty;
    };

    bool isReadOnly() const {
        return _isReadOnly;
    };
    
    time_t getTimeStamp() const {
        return _timeStamp;
    };

    void synchroniseWith(const Buffer & buf) {
        _isDirty = buf.isDirty();
        _timeStamp = buf.getTimeStamp();
    };

private :
	char _fullPathName[MAX_PATH];
	bool _isDirty;
	Document _doc;
	LangType _lang;
	time_t _timeStamp; // O if it's a new doc
	bool _isReadOnly;
	Position _pos;
};

#endif //BUFFER_H
