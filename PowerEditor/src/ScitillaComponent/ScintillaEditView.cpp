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

#include <windows.h>
#include <ShellAPI.h>
#include "ScintillaEditView.h"
#include "Buffer.h"

// initialize the static variable
HINSTANCE ScintillaEditView::_hLib = ::LoadLibrary("SciLexer.DLL");
int ScintillaEditView::_refCount = 0;
UserDefineDialog ScintillaEditView::_userDefineDlg;

const int ScintillaEditView::_SC_MARGE_LINENUMBER = 0;
const int ScintillaEditView::_SC_MARGE_SYBOLE = 1;
const int ScintillaEditView::_SC_MARGE_FOLDER = 2;

/*
SC_MARKNUM_*     | Arrow               Plus/minus           Circle tree                 Box tree 
-------------------------------------------------------------------------------------------------------------
FOLDEROPEN       | SC_MARK_ARROWDOWN   SC_MARK_MINUS     SC_MARK_CIRCLEMINUS            SC_MARK_BOXMINUS 
FOLDER           | SC_MARK_ARROW       SC_MARK_PLUS      SC_MARK_CIRCLEPLUS             SC_MARK_BOXPLUS 
FOLDERSUB        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_VLINE                  SC_MARK_VLINE 
FOLDERTAIL       | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_LCORNERCURVE           SC_MARK_LCORNER 
FOLDEREND        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEPLUSCONNECTED    SC_MARK_BOXPLUSCONNECTED 
FOLDEROPENMID    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEMINUSCONNECTED   SC_MARK_BOXMINUSCONNECTED 
FOLDERMIDTAIL    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_TCORNERCURVE           SC_MARK_TCORNER 
*/

const int ScintillaEditView::_markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND, SC_MARKNUM_FOLDEROPENMID, SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS, SC_MARK_PLUS, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN, SC_MARK_ARROW, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY, SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS, SC_MARK_CIRCLEPLUS, SC_MARK_VLINE, SC_MARK_LCORNERCURVE, SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS, SC_MARK_BOXPLUS, SC_MARK_VLINE, SC_MARK_LCORNER, SC_MARK_BOXPLUSCONNECTED, SC_MARK_BOXMINUSCONNECTED, SC_MARK_TCORNER}
};

//folderStyle ScintillaEditView::_folderStyle = FOLDER_STYLE_BOX;

void ScintillaEditView::init(HINSTANCE hInst, HWND hPere)
{
	if (!_hLib)
	{
		MessageBox( NULL, "can't not load the dynamic library", "SYS ERR : ", MB_OK | MB_ICONSTOP);
		throw int(106901);
	}

	Window::init(hInst, hPere);
   _hSelf = ::CreateWindowEx(
					WS_EX_CLIENTEDGE,\
					"Scintilla",\
					"Notepad++",\
					WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,\
					0, 0, 100, 100,\
					_hParent,\
					NULL,\
					_hInst,\
					NULL);

	if (!_hSelf)
	{
		systemMessage("System Err");
		throw int(106901);
	}

	_pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(_hSelf, SCI_GETDIRECTFUNCTION, 0, 0);
	_pScintillaPtr = (SCINTILLA_PTR)::SendMessage(_hSelf, SCI_GETDIRECTPOINTER, 0, 0);

    _userDefineDlg.init(_hInst, _hParent, this);

	if (!_pScintillaFunc || !_pScintillaPtr)
	{
		systemMessage("System Err");
		throw int(106901);
	}

    execute(SCI_SETMARGINMASKN, _SC_MARGE_FOLDER, SC_MASK_FOLDERS);
    
    execute(SCI_SETMARGINWIDTHN, _SC_MARGE_FOLDER, 16);

    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_FOLDER, true);
    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_SYBOLE, true);

    showMargin(_SC_MARGE_LINENUMBER);

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("1"));

	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));
	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
	execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETFOLDFLAGS, 16, 0);

    //setMakerStyle(FOLDER_STYLE_BOX);
	for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
        defineMarker(_markersArray[FOLDER_TYPE][i], _markersArray[_folderStyle][i], white, black);

};

void ScintillaEditView::setStyle(int whichStyle, COLORREF fore, COLORREF back, int size, const char *face) const
{
	execute(SCI_STYLESETFORE, whichStyle, fore);
	execute(SCI_STYLESETBACK, whichStyle, back);
	if (size >= 1)
		execute(SCI_STYLESETSIZE, whichStyle, size);
	if (face)
		execute(SCI_STYLESETFONT, whichStyle, reinterpret_cast<LPARAM>(face));
}

void ScintillaEditView::setFont(int which, const char *fontName, bool isBold, bool isItalic) const
{
	if ((!fontName)||(strcmp(fontName, "")))
		execute(SCI_STYLESETFONT, (WPARAM)which, (LPARAM)fontName);
	if (isBold)
		execute(SCI_STYLESETBOLD, (WPARAM)which, (LPARAM)isBold);
	if (isItalic)
		execute(SCI_STYLESETITALIC, (WPARAM)which, (LPARAM)isItalic);
}

// list MACOCS30 //
const char commandM30[] = "CALLFUNCTION CHECKSUM END ENTRANT GOTO LOCKREADER IF POWEROFF POWERON "
                          "SET SORTANT STATUSPERSO SWITCH UNLOCKREADER";

const char macroM30[]  = "ChangeCode CipherData CreateBinary CreateBinaryCipher CreateDirectoryV1 "
                         "CreateDirectoryV3 CreateFile CreateFilePlein00 CreateFilePleinFF CreateFileSMS "
                         "CreateFileVide CreateRecord CreateRecordCipher Delete DeleteFile DisableChv "
                         "EnableChv Envelope Fetch GenerateKey GetData GetResponse GetStatus Increase "
                         "Install InstallLoad Invalidate KeyCalculus LastLoad Load LockRecord "
                         "MultiCreateBinary MultiCreateRecord MultiLoad MultiUpdateBinary MultiUpdateRecord "
                         "PutData ReadBinary ReadDirectory ReadRecord Rehabilitate RsaInitKeyList RsaAddKeyToList "
                         "RsaSendRequest RsaGetPublicKeyVal RsaGetKeyModulusHash RsaGetAndCutPrivateKeyValue "
                         "RsaWritePrivateKey SetLock SetStatus SelectDF SelectEF Set Sleep StatusCard UpdateBinary "
                         "UpdateBinary2 UpdateRecord UnblockCode VerifyCode VerifyDisabled";

const char commandLabel[] = "ARG1 ARG2 CMD_ISO COMP CIBLE DATA DEST ELSE ETAT FREQ "
                            "IDFUNC LABEL OP PTS SECURITE SOURCE STATUS THEN";

const char macroLabel[] = "Blck Buf Cible CipherKey Cle ClearLen Code Compl CutLen "
                          "Data Dest Diversif DummyBuf ExponentVal FirstRec HiOffset "
                          "ID IndexInList KeyNumber Lg LgCre LgRec LgUp LoOffset "
                          "MaxLg Mode ModSel ModulusLen Nb NbRec NoRec Occur Offset Op "
                          "P1 P3 RBKey RBSize RecNum Source State Subset Target Tag1 Tag2 Type ULg";

void ScintillaEditView::setM30Lexer()
{
	execute(SCI_SETLEXER, SCLEX_M30);

	// 0 => SCE_M30_CMD
	execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(commandM30));
	// 1 => SCE_M30_MACRO
	execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(macroM30));
	// 2 => SCE_M30_CMD_LABEL
	execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(commandLabel));
	// 3 => SCE_M30_MACRO_LABEL
	execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(macroLabel));
	
	setStyle(SCE_M30_CMD, blue);
	setStyle(SCE_M30_MACRO, orange);
	setStyle(SCE_M30_CMD_LABEL, red);
	setStyle(SCE_M30_MACRO_LABEL, red);

	setFont(SCE_M30_NUMBER, "Courier New");
	setStyle(SCE_M30_NUMBER, cyan); //red by default
	setStyle(SCE_M30_COMMENTLINE, darkGreen); // blue by default
}

// list Pcom //
const char pcomNativeCmd[] = "DEFINE POWER_ON POWER_OFF STEP_ON STEP_OFF LIST_ON LIST_OFF SET_BUFFER INSERT EJECT "
							 "ERROR_BEEP_ON ERROR_BEEP_OFF END_BEEP_ON END_BEEP_ON PATH READER CALL EXECUTE EXIT "
							 "BEGIN_LOOP LOOP INCREASE_BUFFER DECREASE_BUFFER APPEND_BUFFER UNDEFINE ALLUNDEFINE "
							 "IFDEF ELSE ENDIF IFNDEF LOAD UNLOAD";

const char pcomExtCmd[] = "SET_DATA DISPLAY COMPARE SET_RSA_LEN SET_RSA_NUB SET_RSA_NBITS COMPUTE_RSA_KEYS "
						  "GET_RSA_PUBLIC_KEYS GET_RSA_SIGNATURE VERIFY_RSA_SIGNATURE SHA SET_PRF_SECRET "
						  "SET_PRF_LABEL_SEED PRF SET_PRF_SECRET SET_RSA_PUBLIC_KEYS SET_RADM1 SET_RADM2";

const char pcomDefaultBuf[] = "G H I J K L M N O Q R W";


void ScintillaEditView::setPcomLexer()
{
	execute(SCI_SETLEXER, SCLEX_PCOM);

	execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(pcomNativeCmd));
	execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(pcomExtCmd));
	execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(pcomDefaultBuf));
	execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(""));

    setStyle(STYLE_DEFAULT, black, white);
	setFont(SCE_PCOM_NUMBER, "Courier New");
	setStyle(SCE_PCOM_NUMBER, red); //red by default
	setStyle(SCE_PCOM_COMMENTLINE, darkGreen); // blue by default
	
	setFont(SCE_PCOM_NATIVE_CMD, NULL, true);
	setStyle(SCE_PCOM_NATIVE_CMD, darkBlue);
	setStyle(SCE_PCOM_EXT_CMD, blue);

	setFont(SCE_PCOM_DEFAULT_BUF, NULL, true, true);
	setStyle(SCE_PCOM_DEFAULT_BUF, purple);

	setStyle(SCE_PCOM_MACRO, orange);
	setStyle(SCE_PCOM_NCFORMAT_MACRO, grey);
}

const char htmlKeyWords[] = "a abbr acronym address applet area b base basefont "
							"bdo big blockquote body br button caption center "
							"cite code col colgroup dd del dfn dir div dl dt em "
							"fieldset font form frame frameset h1 h2 h3 h4 h5 h6 "
							"head hr html i iframe img input ins isindex kbd label "
							"legend li link map menu meta noframes noscript "
							"object ol optgroup option p param pre q s samp "
							"script select small span strike strong style sub sup "
							"table tbody td textarea tfoot th thead title tr tt u ul "
							"var xml xmlns"
							"accept-charset accept accesskey action align alink "
							"alt archive axis background bgcolor border "
							"cellpadding cellspacing char charoff charset checked cite "
							"class classid clear codebase codetype color cols colspan "
							"compact content coords "
							"data datafld dataformatas datapagesize datasrc datetime "
							"declare defer dir disabled enctype event "
							"face for frame frameborder "
							"headers height href hreflang hspace http-equiv "
							"id ismap label lang language leftmargin link longdesc "
							"marginwidth marginheight maxlength media method multiple "
							"name nohref noresize noshade nowrap "
							"object onblur onchange onclick ondblclick onfocus "
							"onkeydown onkeypress onkeyup onload onmousedown "
							"onmousemove onmouseover onmouseout onmouseup "
							"onreset onselect onsubmit onunload "
							"profile prompt readonly rel rev rows rowspan rules "
							"scheme scope selected shape size span src standby start style "
							"summary tabindex target text title topmargin type usemap "
							"valign value valuetype version vlink vspace width "
							"text password checkbox radio submit reset "
							"file hidden image public !doctype";

const char JSKeyWords[] = "if else for while function var";

const char PHPKeyWords[] = "and argv as argc break case cfunction class continue declare default do "
                            "die echo else elseif empty enddeclare endfor endforeach endif endswitch "
                            "endwhile e_all e_parse e_error e_warning eval exit extends false for "
                            "foreach function global http_cookie_vars http_get_vars http_post_vars "
                            "http_post_files http_env_vars http_server_vars if include include_once "
                            "list new not null old_function or parent php_os php_self php_version "
                            "print require require_once return static switch stdclass this true var "
                            "xor virtual while __file__ __line__ __sleep __wakeup";

void ScintillaEditView::setXmlLexer(LangType type)
{
	//It's XML by default
	int lexer = SCLEX_XML;
	COLORREF tagColor = blue;
	COLORREF attributColor = red;
	COLORREF tagUnknownColor = tagColor;
	COLORREF attributUnknownColor = attributColor;

	// pour synchroniser le lexer m30 
	// (de remettre les word lists vides)
	if (type == L_XML)
	{
		for (int i = 0 ; i < 4 ; i++)
			execute(SCI_SETKEYWORDS, i, reinterpret_cast<LPARAM>(""));
	}
	else if ((type == L_HTML) || (type == L_PHP))
	{
		execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(htmlKeyWords));
		
			//execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(""));
			//execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(""));
		lexer = SCLEX_HTML;

		tagUnknownColor = black;
		attributUnknownColor = black;
	}
	else
		return;

	execute(SCI_SETSTYLEBITS, 7, 0);
	execute(SCI_SETLEXER, lexer);
	
	setStyle(SCE_H_TAG, tagColor);
    setStyle(SCE_H_TAGEND, tagColor);
	setStyle(SCE_H_ATTRIBUTE, attributColor);
	setStyle(SCE_H_TAGUNKNOWN, tagUnknownColor);
	setStyle(SCE_H_ATTRIBUTEUNKNOWN, attributUnknownColor);
	
	setStyle(SCE_H_CDATA, orange);
	setStyle(SCE_H_COMMENT, darkGreen);

	setFont(SCE_H_SINGLESTRING, NULL, true);
	setStyle(SCE_H_SINGLESTRING, darkBlue);
	setFont(SCE_H_DOUBLESTRING, NULL, true);
	setStyle(SCE_H_DOUBLESTRING, darkBlue);

	setStyle(SCE_H_XMLSTART, red, yellow);
    setStyle(SCE_H_XMLEND, red, yellow);
    
    setStyle(SCE_H_SGML_DEFAULT, black, liteBlue);

	//execute(SCE_H_XMLSTART, SCE_HJ_DEFAULT, true);
    if ((type == L_HTML) || (type == L_PHP))
	{
		setEmbeddedJSLexer(extremeLiteBlue, 9, "Comic Sans MS");
        setPhpEmbeddedLexer(liteBerge, 9);
	}
}


void ScintillaEditView::setEmbeddedJSLexer(COLORREF bkColor, int fontSize, const char *font)
{
    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(JSKeyWords));

	setStyle(SCE_HJ_DEFAULT, black, bkColor, fontSize, font);
	setStyle(SCE_HJ_WORD, black, bkColor, fontSize, font);
	setStyle(SCE_HJ_KEYWORD, darkBlue, bkColor, fontSize, font);
	setFont(SCE_HJ_KEYWORD, NULL, true);
	setStyle(SCE_HJ_NUMBER, red, bkColor, fontSize, font);
	setStyle(SCE_HJ_DOUBLESTRING, grey, bkColor, fontSize, font);
	setStyle(SCE_HJ_SINGLESTRING, grey, bkColor, fontSize, font);
	setStyle(SCE_HJ_SYMBOLS, black, bkColor, fontSize, font);

	execute(SCI_STYLESETEOLFILLED, SCE_HJ_DEFAULT, true);
}

void ScintillaEditView::setPhpEmbeddedLexer(COLORREF bkColor, int fontSize, const char *font)
{
    execute(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(PHPKeyWords));
    setStyle(SCE_H_QUESTION, red, berge);
    
    setStyle(SCE_HPHP_DEFAULT, black, bkColor, fontSize, font);
	setStyle(SCE_HPHP_WORD, blue, bkColor, fontSize, font);
	setStyle(SCE_HPHP_COMMENT, darkGreen, bkColor, fontSize, font);
    setStyle(SCE_HPHP_COMMENTLINE, darkGreen, bkColor, fontSize, font);
	setStyle(SCE_HPHP_HSTRING, grey, bkColor, fontSize, font);

	setFont(SCE_HPHP_WORD, NULL, true);

	setStyle(SCE_HPHP_HSTRING, grey, bkColor, fontSize, font);
    setStyle(SCE_HPHP_SIMPLESTRING, grey, bkColor, fontSize, font);

    setStyle(SCE_HPHP_NUMBER, orange, bkColor, fontSize, font);
	
    setStyle(SCE_HPHP_VARIABLE, greenBlue, bkColor, fontSize, font);
	setStyle(SCE_HPHP_HSTRING_VARIABLE, greenBlue, bkColor, fontSize, font);
    
    setStyle(SCE_HPHP_OPERATOR, purple, bkColor, fontSize, font);

	execute(SCI_STYLESETEOLFILLED, SCE_HPHP_DEFAULT, true);
}



void ScintillaEditView::setUserLexer()
{
    execute(SCI_SETLEXER, SCLEX_USER);

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(_userDefineDlg.getBoolList()));
    for (int i = 0 ; i < _userDefineDlg.getNbWordList() ; i++)
    {
        const WordList & wl = _userDefineDlg.getWordList(i);
        execute(SCI_SETKEYWORDS, i + 1, reinterpret_cast<LPARAM>(wl._words));
        setStyle(SCE_USER_DEFINE_1+i, wl._color);
        setFont(SCE_USER_DEFINE_1+i, NULL, wl.isBold(), wl.isItalic());
    }
    
    for (int i = 0 ; i < _userDefineDlg.getNbBlock() ; i++)
    {
		setStyle(SCE_USER_CROCHET + i, black, _userDefineDlg.getBlockColor(i));
        //setFont(SCE_USER_DEBUT_FIN, NULL, true);
    }
}

const char cInstrWords[] = "if else switch case default break goto return for while do continue typedef sizeof NULL";
const char cppInstrWords[] = "new delete throw try catch namespace operator this const_cast static_cast dynamic_cast reinterpreter_cast true false null";
const char javaInstrWords[] = "new delete throw throws try catch finally this super extends implements import true false null";
const char rcInstrWords[] = "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON "
							"BEGIN BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS "
							"COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX DISCARDABLE "
							"EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE LISTBOX LTEXT "
							"MENU MENUEX MENUITEM MESSAGETABLE POPUP "
							"PUSHBUTTON RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 "
							"STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY ";

const char cTypeWords[] = "void struct union enum char short int long double float signed unsigned const static extern auto register volatile";
const char cppTypeWords[] = "bool class private protected public friend inline template virtual";
const char javaTypeWords[] = "byte boolean class interface native private protected public final abstract synchronized";

void ScintillaEditView::setCppLexer(LangType type)
{
	std::string cppInstrs;
	std::string cppTypes;

	switch (type)
	{
		case L_C:
		{
			cppInstrs = cInstrWords;
			cppTypes = cTypeWords;
			break;
		}

		case L_H:
		case L_CPP:
		{
			cppInstrs = cInstrWords;
			cppTypes = cTypeWords;

			cppInstrs += " ";
			cppInstrs += cppInstrWords;

			cppTypes += " ";
			cppTypes += cppTypeWords;
			break;
		}
		case L_JAVA:
		{
			cppInstrs = cInstrWords;
			cppTypes = cTypeWords;

			cppInstrs += " ";
			cppInstrs += javaInstrWords;

			cppTypes += " ";
			cppTypes += javaTypeWords;
			break;
		}
		case L_RC:
		{
			cppInstrs = rcInstrWords;
			cppTypes = ""/*rcTypeWords*/;
			break;
		}
		default :
			return;
	}

    execute(SCI_SETLEXER, SCLEX_CPP);
	execute(SCI_SETKEYWORDS, 0, (LPARAM) cppInstrs.c_str());
	execute(SCI_SETKEYWORDS, 1, (LPARAM) cppTypes.c_str());

    // Global default style.
    //setStyle(STYLE_DEFAULT, black, white, 10, "Verdana");
    //execute(SCI_STYLECLEARALL, 0, 0); // Copies to all other styles.

    // C Styles. 
    setStyle(SCE_C_DEFAULT, black, white/*, 10, "Verdana"*/); //0
    setStyle(SCE_C_COMMENT, darkGreen, white, 9, "Comic Sans MS"); //1
    setStyle(SCE_C_COMMENTLINE, darkGreen, white, 9, "Comic Sans MS"); //2
    setStyle(SCE_C_COMMENTDOC, darkGreen, white, 9, "Comic Sans MS"); //3
    setStyle(SCE_C_NUMBER, orange, white, 0, 0); //4
    setStyle(SCE_C_WORD, blue, white); //5
	setStyle(SCE_C_WORD2, purple, white); //5

    execute(SCI_STYLESETBOLD, SCE_C_WORD, 1); 
    setStyle(SCE_C_STRING, grey, white, 0, 0); //6
    setStyle(SCE_C_CHARACTER, grey, white, 0, 0); //7

	if (type != L_JAVA)
		setStyle(SCE_C_PREPROCESSOR, brown, white, 0, 0); //9

    setStyle(SCE_C_OPERATOR, darkBlue, white, 0, 0); //10
    execute(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1); 

    // setStyle(SCE_C_STRINGEOL, darkBlue, white, 0, 0); //12
    // setStyle(SCE_C_COMMENTLINEDOC, darkBlue, white, 0, 0); //15
    // setStyle(SCE_C_WORD2, darkBlue, white, 0, 0); //16

    return;
}

void ScintillaEditView::setMakefileLexer()
{
	execute(SCI_SETLEXER, SCLEX_MAKEFILE);
	setStyle(SCE_MAKE_COMMENT, darkGreen);
	setStyle(SCE_MAKE_TARGET, red);
	setStyle(SCE_MAKE_IDENTIFIER, blue);
	setStyle(SCE_MAKE_PREPROCESSOR, yellow);
//SCE_MAKE_DEFAULT 0

//SCE_MAKE_OPERATOR 4

//SCE_MAKE_IDEOL 9

}

void ScintillaEditView::defineDocType(LangType typeDoc)
{
	setStyle(STYLE_DEFAULT, black, white, 10, "Verdana");
    execute(SCI_STYLECLEARALL);

    int caretColor = black;
    int caretWidth = 1;
    int selectColorFore = yellow;
    int selectColorBack = grey;
	
	execute(SCI_SETSTYLEBITS, 5);

	switch (typeDoc)
	{
		case L_H :
		case L_C :
		case L_CPP :
		case L_JAVA :
		case L_RC :
            setCppLexer(typeDoc); break;

		case L_PHP:
		case L_HTML :
		case L_XML :
			setXmlLexer(typeDoc); break;

		case L_MAKEFILE:
			setMakefileLexer(); break;

		case L_M30 :
			setM30Lexer(); break;

		case L_PCOM :
			setPcomLexer(); break;

        case L_USER :
			setUserLexer(); break;

        case L_NFO :
			if (!_MSLineDrawFont)
				_MSLineDrawFont = ::AddFontResource(LINEDRAW_FONT);
			if (_MSLineDrawFont)
			{
				setStyle(STYLE_DEFAULT, liteGrey, black, 9, "MS LineDraw");
				execute(SCI_STYLECLEARALL);

				setStyle(STYLE_LINENUMBER, liteGrey, black, 9);

				caretColor = yellow;
				caretWidth = 3;
				selectColorFore = black;
				selectColorBack = white;
			}
			break;

		case L_TXT :
		default :
			execute(SCI_SETLEXER, SCLEX_NULL, 0); break;
	}
	//All the global styles should put here
	setStyle(STYLE_INDENTGUIDE, liteGrey);
	setStyle(STYLE_CONTROLCHAR, liteGrey, red);
	setStyle(STYLE_BRACELIGHT, blue, yellow);

    setCaretColorWidth(caretColor, caretWidth);
    setSelectionColor(selectColorFore, selectColorBack);

    execute(SCI_COLOURISE, 0, -1);
}

char * ScintillaEditView::attatchDefaultDoc(int nb)
{
	char title[10];
	char nb_str[4];
	strcat(strcpy(title, UNTITLED_STR), _itoa(nb, nb_str, 10));

	// get the doc pointer attached (by default) on the view Scintilla
	Document doc = execute(SCI_GETDOCPOINTER, 0, 0);

	// create the entry for our list
	_buffers.push_back(Buffer(doc, title));

	// set current index to 0
	_currentIndex = 0;
	//execute(SCI_SETSAVEPOINT);
	return _buffers[_currentIndex]._fullPathName;
}


int ScintillaEditView::findDocIndexByName(const char *fn) const
{
	int index = -1;
	for (int i = 0 ; i < int(_buffers.size()) ; i++)
	{
		if (!strcmp(_buffers[i]._fullPathName, fn))
		{
			index = i;
			break;
		}
	}
	return index;
}

//! \brief this method activates the doc and the corresponding sub tab
//! \brief return the index of previeus current doc
char * ScintillaEditView::activateDocAt(int index)
{
	// before activating another document, we get the current position
	// from the Scintilla view then save it to the current document
	saveCurrentPos();
	Position & prevDocPos = _buffers[_currentIndex]._pos;

	// increase current doc ref count to 2 
	execute(SCI_ADDREFDOCUMENT, 0, _buffers[_currentIndex]._doc);

	// change the doc, this operation will decrease 
	// the ref count of old current doc to 1
	execute(SCI_SETDOCPOINTER, 0, _buffers[index]._doc);
	
	// reset all for another doc
	//execute(SCI_CLEARDOCUMENTSTYLE);
    //bool isDocTypeDiff = (_buffers[_currentIndex]._lang != _buffers[index]._lang);
	_currentIndex = index;
	
    //if (isDocTypeDiff)
    defineDocType(_buffers[_currentIndex]._lang);

	restoreCurrentPos(prevDocPos);

    //execute(SCI_SETREADONLY, isCurrentBufReadOnly());
	
    return _buffers[_currentIndex]._fullPathName;
}

// this method creates a new doc ,and adds it into 
// the end of the doc list and a last sub tab, then activate it
// it returns the name of this created doc (that's the current doc also)
char * ScintillaEditView::createNewDoc(const char *fn)
{
	Document newDoc = execute(SCI_CREATEDOCUMENT);
	_buffers.push_back(Buffer(newDoc, fn));
	_buffers[_buffers.size()-1].checkIfReadOnlyFile();
	return activateDocAt(int(_buffers.size())-1);
}

char * ScintillaEditView::createNewDoc(int nbNew)
{
	char title[10];
	char nb[4];
	strcat(strcpy(title, UNTITLED_STR), _itoa(nbNew, nb, 10));
	return createNewDoc(title);
}

// return the index to close then (argument) the index to activate
int ScintillaEditView::closeCurrentDoc(int & i2Activate)
{
	int oldCurrent = _currentIndex;

	// if the file 2 delete is the last one
	if (_currentIndex == _buffers.size()-1)
    {
		// if current index is 0, ie. the current is the only one
		if (!_currentIndex)
		{
			_currentIndex = 0;
		}
		// the current is NOT the only one and it is the last one,
		// we set it to the index which precedes it
		else
			_currentIndex -= 1;
    }
	// else the next current index will be the same,
	// we do nothing

	// get the iterator and calculate its position with the old current index value
	buf_vec_t::iterator posIt = _buffers.begin() + oldCurrent;

	// erase the position given document from our list
	_buffers.erase(posIt);

	// set another document, so the ref count of old active document owned
	// by Scintilla view will be decreased to 0 by SCI_SETDOCPOINTER message
	execute(SCI_SETDOCPOINTER, 0, _buffers[_currentIndex]._doc);

	defineDocType(_buffers[_currentIndex]._lang);
	
	i2Activate = _currentIndex;
	
	return oldCurrent;
}

void ScintillaEditView::closeDocAt(int i2Close)
{
    execute(SCI_RELEASEDOCUMENT, 0, _buffers[i2Close]._doc);

	// get the iterator and calculate its position with the old current index value
	buf_vec_t::iterator posIt = _buffers.begin() + i2Close;

	// erase the position given document from our list
	_buffers.erase(posIt);

    _currentIndex -= (i2Close < _currentIndex)?1:0;
}

void ScintillaEditView::removeAllUnusedDocs()
{
	// unreference all docs  from list of Scintilla
	// by sending SCI_RELEASEDOCUMENT message
	for (int i = 0 ; i < int(_buffers.size()) ; i++)
		if (i != _currentIndex)
			execute(SCI_RELEASEDOCUMENT, 0, _buffers[i]._doc);
	
	// remove all docs except the current doc from list
	_buffers.clear();
}

void ScintillaEditView::getText(char *dest, int start, int end) 
{
	TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = dest;
	execute(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

void ScintillaEditView::marginClick(int position, int modifiers)
{
	int lineClick = int(execute(SCI_LINEFROMPOSITION, position, 0));
	int levelClick = int(execute(SCI_GETFOLDLEVEL, lineClick, 0));
	if (levelClick & SC_FOLDLEVELHEADERFLAG)
    {
		if (modifiers & SCMOD_SHIFT)
        {
			// Ensure all children visible
			execute(SCI_SETFOLDEXPANDED, lineClick, 1);
			expand(lineClick, true, true, 100, levelClick);
		}
        else if (modifiers & SCMOD_CTRL) 
        {
			if (execute(SCI_GETFOLDEXPANDED, lineClick, 0)) 
            {
				// Contract this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 0);
				expand(lineClick, false, true, 0, levelClick);
			} 
            else 
            {
				// Expand this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 1);
				expand(lineClick, true, true, 100, levelClick);
			}
		} 
        else 
        {
			// Toggle this line
			execute(SCI_TOGGLEFOLD, lineClick, 0);
		}
	}
}

void ScintillaEditView::expand(int &line, bool doExpand, bool force, int visLevels, int level)
{
	int lineMaxSubord = int(execute(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK));
	line++;
	while (line <= lineMaxSubord)
    {
		if (force) 
        {
			if (visLevels > 0)
				execute(SCI_SHOWLINES, line, line);
			else
				execute(SCI_HIDELINES, line, line);
		} 
        else 
        {
			if (doExpand)
				execute(SCI_SHOWLINES, line, line);
		}
		int levelLine = level;
		if (levelLine == -1)
			levelLine = int(execute(SCI_GETFOLDLEVEL, line, 0));
		if (levelLine & SC_FOLDLEVELHEADERFLAG)
        {
			if (force) 
            {
				if (visLevels > 1)
					execute(SCI_SETFOLDEXPANDED, line, 1);
				else
					execute(SCI_SETFOLDEXPANDED, line, 0);
				expand(line, doExpand, force, visLevels - 1);
			} 
            else
            {
				if (doExpand)
                {
					if (!execute(SCI_GETFOLDEXPANDED, line, 0))
						execute(SCI_SETFOLDEXPANDED, line, 1);

					expand(line, true, force, visLevels - 1);
				} 
                else 
                {
					expand(line, false, force, visLevels - 1);
				}
			}
		}
        else
        {
			line++;
		}
	}
}