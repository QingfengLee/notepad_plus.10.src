// Scintilla source code edit control
/** @file LexM30.cxx
 ** Lexer for Macocs30 language.
 **
 ** Written by Don HO
 **/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <windows.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

// '(' et ',' sont à cause de label
// '/' et '=' à cause de nombre ie. [ULLA/0x36/0x15]
//char *noInclude = {'(', ',', '/', '='}

inline bool isSpecialChar(const int ch)
{
	return (ch == '_' || ch == '&' || ch == 'é' || ch == '°' ||
			   ch == '"' || ch == '\'' || ch == '~' || ch == '-' ||
			   ch == 'è' || ch == 'ç' || ch == 'à' || ch == '{' ||
			   ch == '|' || ch == '`' || ch == '^' || ch == '@' ||
			   ch == '}' || ch == '!' || ch == ':' || ch == '.' ||
			   ch == '?' ||  ch == '\\'|| ch == '+' || ch == '§' ||
			   ch == 'ù' || ch == '%' || ch == '*' || ch == 'µ' ||
			   ch == '¤' || ch == '£' || ch == '$' || ch == '¨'  );
}

inline bool isWordChar(const int ch)
{
	return (isalnum(ch) || isSpecialChar(ch) || ch == ' ');
}

inline bool isHexChar(const int ch)
{
	return ((ch >= '0') && (ch <= '9')) || 
	          ((ch >= 'A') && (ch <= 'F')) || 
	          ((ch >= 'a') && (ch <= 'f')) || 
	          (ch == ' ') || (ch == '\t');
}

inline bool AtEOL(Accessor &styler, unsigned int i)
{
	return (	styler[i] == '\n') ||
				((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

//l'ensemle qui fait absoluement pas partie : 
//    &é"'-è_çà°+}]@^\`|[{~;:!*ù$^£¨µ%§.?¤
//l'ensemle qui n'a pas encore traité : /=(,
inline bool isWordStart(const int ch) 
{
	return isalnum(ch) || isSpecialChar(ch) || 
			  ch == ')' || ch == '[' || ch == ']' || ch == '.' || 
	          ch == ';';
}

inline bool isOperator(char ch)
{
	if (isalnum(ch))
		return false;
	// '.' left out as it is used to make up numbers
	if (	ch == '/' || ch == '(' || ch == ')' || ch == '=' ||
		ch == '[' || ch == ']' || ch == ';' || ch == ',' )
		return true;
	return false;
}

inline void getCurrentStr(unsigned int start, unsigned int end, Accessor &styler, char *s, int len)
{
	unsigned int i = 0;
	while ((i < end - start + 1) && (i < (unsigned int)(len-1)))
	{
		s[i] = styler[start + i];
		i++;
	}
	s[i] = '\0';
}

static void ColouriseM30Line(char *lineBuffer, unsigned int lengthLine, unsigned int startLine,
										unsigned int endPos, WordList *keywordlists[], Accessor &styler)
{
	//MessageBox(NULL, lineBuffer, "toto", MB_OK);
	bool isID = true;	
	WordList &cmdList			= *keywordlists[0];
	WordList &macroList			= *keywordlists[1];
	WordList &labelCmdList		= *keywordlists[2];
	WordList &labelMacroList	= *keywordlists[3];
	WordList &userDefine1		= *keywordlists[4];
	WordList &userDefine2		= *keywordlists[5];

	unsigned int i = 0;
	int lastNonSpace = -1;
	unsigned int state = SCE_M30_DEFAULT;
	
	unsigned int statePre = SCE_M30_DEFAULT;
	int startWordPre = 0;
	
	bool bSpecial = false;
	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i])) {
		i++;
	}

	//int currentLine = styler.GetLine(startPos);
	
	for ( ; i < lengthLine ; i++)
	{/*
		if (lineBuffer[i] == '\0') {
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(startLine + i - 1);
			styler.SetLineState(currentLine, 0);
		}*/
		// Determine if the current state should terminate.
		switch (state)
		{
			case SCE_M30_NUMBER :
				if (!isHexChar(lineBuffer[i]))
				{
					styler.ColourTo(startLine + i - 1, state);
					state = SCE_M30_DEFAULT;
				}
				break;

			case SCE_M30_IDENTIFIER :
				if (!isWordChar(lineBuffer[i]) )
				{
					char s[100];
					
					getCurrentStr(styler.GetStartSegment(), startLine + i - 1, styler, s, sizeof(s));
					
					if (cmdList.InList(s))				state = (SCE_M30_CMD);
					else if (macroList.InList(s) )		state = (SCE_M30_MACRO);
					else if (labelCmdList.InList(s))		state = (SCE_M30_CMD_LABEL);
					else if (labelMacroList.InList(s))		state = (SCE_M30_MACRO_LABEL);
					else if (userDefine1.InList(s))		state = (SCE_M30_USER_DEFINE1);
					else if (userDefine2.InList(s))		state = (SCE_M30_USER_DEFINE2);
					
					styler.ColourTo(startLine + i -1 , state);					
					state = (SCE_M30_DEFAULT);
				}
				break;
				
			case SCE_M30_COMMENTLINE :
				if (lineBuffer[i+1] == '\0')
				{
					styler.ColourTo(startLine + i - 1, state);
					state = (SCE_M30_DEFAULT);
				}
				break;
				
			case SCE_M30_PREPROCESSOR :
				if (lineBuffer[i] == '\0')
				{
					styler.ColourTo(startLine + i - 1, state);
					state = (SCE_M30_DEFAULT);
				}
				break;
			/*
			case SCE_M30_DEFAULT :
				{
					if (statePre == SCE_M30_CMD_LABEL)
					{
						// Mettre début position coloration à startWordPre
						styler.ColourTo(startLine + i - 1, SCE_M30_MACRO);
						statePre = (SCE_M30_DEFAULT);
					}
				}
				break;
			*/
			default:
			{}
		}

		// Determine if a new state should be entered.
		if (state == SCE_M30_DEFAULT)
		{
			if ((lineBuffer[i] == '0') && (lineBuffer[i+1] == 'x'))
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_M30_NUMBER);
				i++;
			}
			else if (isWordStart(lineBuffer[i])) 
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_M30_IDENTIFIER);
			}
			else if (lineBuffer[i] == '#') 
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_M30_COMMENTLINE);
				i++;
			}
			else
			{
				state = (SCE_M30_DEFAULT);
			}
		}
	}
}

static void ColouriseM30Doc(	unsigned int startPos, int length, int initStyle,
												WordList *keywordlists[], Accessor &styler)
{
	char lineBuffer[1024];
	
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	unsigned int linePos = 0;
	unsigned int startLine = startPos;
	for (unsigned int i = startPos ;  i < startPos + length ; i++)
	{
		lineBuffer[linePos++] = styler[i];
		
		if (AtEOL(styler, i) || (linePos >= sizeof(lineBuffer) - 1))
		{
			// End of line (or of line buffer) met, colourise it
			lineBuffer[linePos] = '\0';
			ColouriseM30Line(lineBuffer, linePos, startLine, i, keywordlists, styler);
			linePos = 0;
			startLine = i + 1;
		}
	}

	if (linePos > 0)
	{	// Last line does not have ending characters
		ColouriseM30Line(lineBuffer, linePos, startLine, startPos + length - 1, keywordlists, styler);
	}

}

/*
inline bool IsAHexDigit(unsigned int ch) {
	return 	(((ch >= '0') && (ch <= '9')) || 
			 ((ch >= 'A') && (ch <= 'F')) || 
			 ((ch >= 'a') && (ch <= 'f')));
}



static void ColouriseM30Doc(unsigned int startPos, int length, int initStyle,
										WordList *keywordlists[], Accessor &styler)
{
	WordList &keywords	= *keywordlists[0];
	WordList &keywords2	= *keywordlists[1];
	WordList &keywords3	= *keywordlists[2];
	WordList &keywords4	= *keywordlists[3];
	WordList &keywords5	= *keywordlists[4];
	WordList &keywords6	= *keywordlists[5];

	int currentLine = styler.GetLine(startPos);
	
	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More() ; sc.Forward()) {
		if (sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(sc.currentPos);
			styler.SetLineState(currentLine, 0);
		}

		// Determine if the current state should terminate.
		switch (sc.state)
		{
			case SCE_M30_NUMBER :
				if (!isWordChar(sc.ch))
					sc.SetState(SCE_M30_DEFAULT);
				break;

			case SCE_M30_IDENTIFIER :
				if (!isWordChar(sc.ch) )
				{
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					if (keywords.InList(s))				sc.ChangeState(SCE_M30_WORD);
					else if (keywords2.InList(s))		sc.ChangeState(SCE_M30_WORD2);
					else if (keywords3.InList(s))		sc.ChangeState(SCE_M30_WORD3);
					else if (keywords4.InList(s))		sc.ChangeState(SCE_M30_WORD4);
					else if (keywords5.InList(s))		sc.ChangeState(SCE_M30_WORD5);
					else if (keywords6.InList(s))		sc.ChangeState(SCE_M30_WORD6);

					sc.SetState(SCE_M30_DEFAULT);
				}
				break;
				
			case SCE_M30_COMMENTLINE :
				if (sc.atLineEnd)
					sc.SetState(SCE_M30_DEFAULT);
				break;
				
			case SCE_M30_PREPROCESSOR :
				if (sc.atLineEnd)
					sc.SetState(SCE_M30_DEFAULT);
				break;
				
			default:
			{}
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_M30_DEFAULT) {
			if (IsADigit(sc.ch)) {
				sc.SetState(SCE_M30_NUMBER);
			} else if (IsAWordStart(sc.ch)) {
				sc.SetState(SCE_M30_IDENTIFIER);
			} else if (sc.Match('#')) {
				sc.SetState(SCE_M30_COMMENTLINE);
				sc.Forward();
			} else if (sc.atLineStart && sc.Match('$')) {
				sc.SetState(SCE_M30_PREPROCESSOR);	
			}
			else
				sc.SetState(SCE_M30_DEFAULT);
		}
	}
	sc.Complete();
}
*/
LexerModule lmM30(SCLEX_M30, ColouriseM30Doc, "m30");
