// Scintilla source code edit control
/** @file LexUser.cxx
 ** Lexer for User Define language.
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


inline bool isWordChar(const int ch)
{
	return isalnum(ch);
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

inline bool isWordStart(const int ch) 
{
	return isalnum(ch) ;
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

inline bool isOuverture(char ch)
{
	return ((ch == '[') || (ch == '(')  || (ch == '{')  || (ch == '<')  || (ch == '"'));
}

static void ColouriseUserLine(char *lineBuffer, unsigned int lengthLine, unsigned int startLine,
										unsigned int endPos, WordList *keywordlists[], Accessor &styler)
{
	char fermeture = '\0';

	WordList &booleanList		= *keywordlists[0];
	WordList &userDefine1		= *keywordlists[1];
	WordList &userDefine2		= *keywordlists[2];
	WordList &userDefine3		= *keywordlists[3];
	WordList &userDefine4		= *keywordlists[4];

	bool doCrochet    = booleanList[0][INDEX_CROCHET	] == 'Y';
    bool doParenthese = booleanList[0][INDEX_PARENTHESE ] == 'Y';
    bool doAccolade   = booleanList[0][INDEX_ACCOLADE	] == 'Y';
    bool doLosange    = booleanList[0][INDEX_LOSANGE	] == 'Y';
    bool doDblCote    = booleanList[0][INDEX_DBLCOTE	] == 'Y';
	
	unsigned int i = 0;
	unsigned int state = SCE_USER_DEFAULT;

	// Skip initial spaces
	while ((i < lengthLine) && isspacechar(lineBuffer[i]))
		i++;
	
	for ( ; i < lengthLine ; i++)
	{
		// Determine if the current state should terminate.
		switch (state)
		{
			case SCE_USER_IDENTIFIER :
				if (!isWordChar(lineBuffer[i]) )
				{
					char s[100];
					
					getCurrentStr(styler.GetStartSegment(), startLine + i - 1, styler, s, sizeof(s));
					
					if (userDefine1.InList(s) )				state = (SCE_USER_DEFINE_1);
					else if (userDefine2.InList(s))		state = (SCE_USER_DEFINE_2);
					else if (userDefine3.InList(s))		state = (SCE_USER_DEFINE_3);
					else if (userDefine4.InList(s))		state = (SCE_USER_DEFINE_4);
					
					styler.ColourTo(startLine + i -1 , state);					
					state = (SCE_USER_DEFAULT);
				}
				break;
				
			case SCE_USER_CROCHET :
			case SCE_USER_PARENTHESE :
			case SCE_USER_ACCOLADE :
			case SCE_USER_LOSANGE :
			case SCE_USER_DBLCOTE :
				if (lineBuffer[i] == fermeture)
				{
					styler.ColourTo(startLine + i, state);
					state = (SCE_USER_DEFAULT);
                    i++;
				}
				break;
				
			default:
			{}
		}
   
    	// Determine if a new state should be entered.
		if (state == SCE_USER_DEFAULT)
		{
			if (isWordStart(lineBuffer[i])) 
			{
				styler.ColourTo(startLine + i - 1, state);
				state = (SCE_USER_IDENTIFIER);
			}
			else if(isOuverture(lineBuffer[i])) 
			{
				styler.ColourTo(startLine + i - 1, state);
				switch (lineBuffer[i])
				{
					case '[' :
						if (doCrochet)
						{
							state = (SCE_USER_CROCHET);
							fermeture = ']';
						}
						break;
					
					case '(' :
						if (doParenthese)
						{
							state = (SCE_USER_PARENTHESE);	
							fermeture = ')';
						}
						break;
						
					case '{' :
						if (doAccolade)
						{
							state = (SCE_USER_ACCOLADE);
							fermeture = '}';
						}
						break;
					
					case '<' :
						if (doLosange)
						{
							state = (SCE_USER_LOSANGE);
							fermeture = '>';
						}
						break;
						
					case '\"' :
						if (doDblCote)
						{	
							state = (SCE_USER_DBLCOTE);
							fermeture = '\"';
						}
						break;

					default :
						state = (SCE_USER_DEFAULT);
				}
			}
		}
	}
}

static void ColouriseUserDoc(unsigned int startPos, int length, int initStyle,
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
			ColouriseUserLine(lineBuffer, linePos, startLine, i, keywordlists, styler);
			linePos = 0;
			startLine = i + 1;
		}
	}
    if (linePos > 0) // Last line does not have ending characters
		ColouriseUserLine(lineBuffer, linePos, startLine, startPos + length - 1, keywordlists, styler);
}

LexerModule lmUser(SCLEX_USER, ColouriseUserDoc, "user");
