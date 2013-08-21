// Scintilla source code edit control
/** @file LexPcom.cxx
 ** Lexer for Pcom language.
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


inline bool isSpecialChar(const int ch)
{
	return (ch == '_' || ch == '&' || ch == 'é' || ch == '°' ||
			   ch == '"' || ch == '\'' || ch == '~' || ch == '-' ||
			   ch == 'è' || ch == 'ç' || ch == 'à' || ch == '{' ||
			   ch == '|' || ch == '`' || ch == '^' || ch == '@' ||
			   ch == '}' || ch == '!' || ch == '#' || ch == '\\' ||
			   ch == '?' || ch == '.' || ch == '+' || ch == '§' ||
			   ch == 'ù' || ch == '%' || ch == 'µ' ||
			   ch == '¤' || ch == '£' || ch == '$' || ch == '¨' );
}

inline bool isWordChar(const int ch)
{
	return (isalnum(ch) || isSpecialChar(ch));
}

inline bool isHexChar(const int ch)
{
	return ((ch >= '0') && (ch <= '9')) || 
	          ((ch >= 'A') && (ch <= 'F')) || 
	          ((ch >= 'a') && (ch <= 'f')) || 
	          (ch == 'X') || (ch == 'x') ;
}

inline bool AtEOL(Accessor &styler, unsigned int i)
{
	return (	styler[i] == '\n') ||
				((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}


inline bool isWordStart(const int ch) 
{
	return isalnum(ch) || isSpecialChar(ch);
}

inline bool isOperator(char ch)
{
	if (isalnum(ch))
		return false;

	if (	ch == '\\' || ch == '(' || ch == ')' || ch == '=' ||
		    ch == '[' || ch == ']' || ch == ':' || ch == ';')
		return true;
	return false;
}

inline bool isNcFormatMacro(char *s)
{
	return ((s[0] == '#') && (strlen(s) == 3) && 
	           (isHexChar(s[1]) || (s[1] == 'n') || (s[1] == 'N')) && 
	           ((s[2] == 'c') || (s[1] == 'C')));
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

static void ColourisePcomLine(char *lineBuffer, unsigned int lengthLine, unsigned int startLine,
										unsigned int endPos, WordList *keywordlists[], Accessor &styler)
{
	//MessageBox(NULL, lineBuffer, "toto", MB_OK);
	bool isID = true;	
	WordList &cmdList			= *keywordlists[0];
	WordList &cmdExtList		= *keywordlists[1];
	WordList &defaultBuf		= *keywordlists[2];
	WordList &userDefine0		= *keywordlists[3];
	WordList &userDefine1		= *keywordlists[4];
	WordList &userDefine2		= *keywordlists[5];

	unsigned int i = 0;
	int lastNonSpace = -1;
	unsigned int state = SCE_PCOM_DEFAULT;
	
	int startWordPre = 0;
	
	bool hasParenthise = false;
	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i])) 
	{
		i++;
	}

	//int currentLine = styler.GetLine(startPos);
	
	for ( ; i < lengthLine ; i++)
	{
		/*
		if (lineBuffer[i] == '\0') 
		{
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(startLine + i - 1);
			styler.SetLineState(currentLine, 0);
		}*/
		// Determine if the current state should terminate.
		switch (state)
		{
			case SCE_PCOM_NUMBER :
				if (!isHexChar(lineBuffer[i]))
				{
					if (isWordChar(lineBuffer[i]) && (!isOperator(lineBuffer[i]))) state = SCE_PCOM_DEFAULT;
						
					styler.ColourTo(startLine + i - 1, state);
					state = SCE_PCOM_DEFAULT;
				}
				break;

			case SCE_PCOM_IDENTIFIER :
				if (!isWordChar(lineBuffer[i]) )
				{
					char s[100];
					getCurrentStr(styler.GetStartSegment(), startLine + i - 1, styler, s, sizeof(s));
					char *str = s + 1;
					if ((s[0] == '.') && (cmdList.InList(str)))				state = (SCE_PCOM_NATIVE_CMD);
					else if ((s[0] == '.') && (cmdExtList.InList(str)))		state = (SCE_PCOM_EXT_CMD);
					else if (defaultBuf.InList(s))									state = (SCE_PCOM_DEFAULT_BUF);
					else if (s[0] == '%')											state = (SCE_PCOM_MACRO);
					else if (isNcFormatMacro(s))									state = (SCE_PCOM_NCFORMAT_MACRO);

					styler.ColourTo(startLine + i -1 , state);					
					state = (SCE_PCOM_DEFAULT);
				}
				break;

			case SCE_PCOM_COMMENTLINE :
				if (lineBuffer[i+1] == '\0')
				{
					styler.ColourTo(startLine + i - 1, state);
					state = (SCE_PCOM_DEFAULT);
				}
				break;
			/*
			case SCE_M30_PREPROCESSOR :
				if (lineBuffer[i] == '\0')
				{
					styler.ColourTo(startLine + i - 1, state);
					state = (SCE_PCOM_DEFAULT);
				}
				break;
				*/

			default:
			{}
		}

		// Determine if a new state should be entered.
		if (state == SCE_PCOM_DEFAULT)
		{	
			if (lineBuffer[i] == '(')
				hasParenthise = true;
			if (lineBuffer[i] == ')')
				hasParenthise = false;
			
			if (isHexChar(lineBuffer[i]))
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_PCOM_NUMBER);
			}
			else if (isWordStart(lineBuffer[i]))
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_PCOM_IDENTIFIER);
			}

			else if ((lineBuffer[i] == '*') || ((lineBuffer[i] == ';') && !hasParenthise))
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_PCOM_COMMENTLINE);
				i++;
			}
			else
			{
				state = (SCE_PCOM_DEFAULT);
			}
		}
	}
}

static void ColourisePcomDoc(	unsigned int startPos, int length, int initStyle,
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
			ColourisePcomLine(lineBuffer, linePos, startLine, i, keywordlists, styler);
			linePos = 0;
			startLine = i + 1;
		}
	}
	if (linePos > 0) // Last line does not have ending characters
			ColourisePcomLine(lineBuffer, linePos, startLine, startPos + length - 1, keywordlists, styler);
}

LexerModule lmPcom(SCLEX_PCOM, ColourisePcomDoc, "pcom");
