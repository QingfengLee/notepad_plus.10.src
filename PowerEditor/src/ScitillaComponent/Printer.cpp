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

#include "Printer.h"

void Printer::init(HINSTANCE hInst, HWND hwnd, bool showDialog, int startPos, int endPos)
{
	_startPos = startPos;
	_endPos = endPos;
	_pdlg.lStructSize = sizeof(PRINTDLG);
	_pdlg.hwndOwner = hwnd;
	_pdlg.hInstance = hInst;
	_pdlg.Flags = PD_USEDEVMODECOPIES | PD_ALLPAGES | PD_RETURNDC;
	_pdlg.nFromPage = 1;
	_pdlg.nToPage = 1;
	_pdlg.nMinPage = 1;
	_pdlg.nMaxPage = 0xffffU; // We do not know how many pages in the
							// document until the printer is selected and the paper size is known.
	_pdlg.nCopies = 1;
	_pdlg.hDC = 0;
	_pdlg.hDevMode = NULL;
	_pdlg.hDevNames = NULL;
	_pdlg.lCustData = NULL;
	_pdlg.lpfnPrintHook = NULL;
	_pdlg.lpfnSetupHook = NULL;
	_pdlg.lpPrintTemplateName = NULL;
	_pdlg.lpSetupTemplateName = NULL;
	_pdlg.hPrintTemplate = NULL;
	_pdlg.hSetupTemplate = NULL;
	
	// See if a range has been selected
	_pdlg.Flags |= (_startPos != _endPos)?PD_SELECTION:PD_NOSELECTION;

	if (!showDialog) 
	{
		// Don't display dialog box, just use the default printer and options
		_pdlg.Flags |= PD_RETURNDEFAULT;
	}

}

void Printer::doPrint(const ScintillaEditView & pSEView)
{
	if (!::PrintDlg(&_pdlg))
		return;

	POINT ptPage;
	POINT ptDpi;

	RECT rectMargins;
	RECT rectPhysMargins;

	// Get printer resolution
	ptDpi.x = GetDeviceCaps(_pdlg.hDC, LOGPIXELSX);    // dpi in X direction
	ptDpi.y = GetDeviceCaps(_pdlg.hDC, LOGPIXELSY);    // dpi in Y direction

	// Start by getting the physical page size (in device units).
	ptPage.x = GetDeviceCaps(_pdlg.hDC, PHYSICALWIDTH);   // device units
	ptPage.y = GetDeviceCaps(_pdlg.hDC, PHYSICALHEIGHT);  // device units

	// Get the dimensions of the unprintable
	// part of the page (in device units).
	rectPhysMargins.left = GetDeviceCaps(_pdlg.hDC, PHYSICALOFFSETX);
	rectPhysMargins.top = GetDeviceCaps(_pdlg.hDC, PHYSICALOFFSETY);

	// To get the right and lower unprintable area,
	// we take the entire width and height of the paper and
	// subtract everything else.
	rectPhysMargins.right = ptPage.x						// total paper width
	                        - GetDeviceCaps(_pdlg.hDC, HORZRES) // printable width
	                        - rectPhysMargins.left;				// left unprintable margin

	rectPhysMargins.bottom = ptPage.y						// total paper height
	                         - GetDeviceCaps(_pdlg.hDC, VERTRES)	// printable height
	                         - rectPhysMargins.top;				// right unprintable margin

	//TOMPORIREMENT
	rectMargins.left	= rectPhysMargins.left;
	rectMargins.top		= rectPhysMargins.top;
	rectMargins.right	= rectPhysMargins.right;
	rectMargins.bottom	= rectPhysMargins.bottom;

	// Convert device coordinates into logical coordinates
	DPtoLP(_pdlg.hDC, (LPPOINT)&rectMargins, 2);
	DPtoLP(_pdlg.hDC, (LPPOINT)&rectPhysMargins, 2);

	// Convert page size to logical units and we're done!
	DPtoLP(_pdlg.hDC, &ptPage, 1);

	TEXTMETRIC tm ;
	::GetTextMetrics(_pdlg.hDC, &tm);
	int printMarge = tm.tmHeight + tm.tmExternalLeading;
	printMarge = printMarge + printMarge / 2;

	DOCINFO docInfo;
	docInfo.cbSize = sizeof(DOCINFO);
	docInfo.lpszDocName = pSEView.getCurrentTitle();
	docInfo.lpszOutput = NULL;

	if (::StartDoc(_pdlg.hDC, &docInfo) < 0) 
	{
		MessageBox(NULL, "Can not start printer document.", 0, MB_OK);
		return;
	}
	
	int pageNum = 1;
	bool printPage;

	// By default, we will print all the document
	long lengthPrinted = 0;
	long lengthDoc = pSEView.getCurrentDocLen();
	long lengthDocMax = lengthDoc;

	// In the case that the print dialog was launched and that there's a range of selection
	// We print the range of selection
	if ((!(_pdlg.Flags & PD_RETURNDEFAULT)) && (_pdlg.Flags & PD_SELECTION))
	{
		if (_startPos > _endPos) 
		{
			lengthPrinted = _endPos;
			lengthDoc = _startPos;
		}
		else 
		{
			lengthPrinted = _startPos;
			lengthDoc = _endPos;
		}

		if (lengthPrinted < 0)
			lengthPrinted = 0;
		if (lengthDoc > lengthDocMax)
			lengthDoc = lengthDocMax;
	}

	RangeToFormat frPrint;
	frPrint.hdc = _pdlg.hDC;
	frPrint.hdcTarget = _pdlg.hDC;
	frPrint.rc.left = rectMargins.left - rectPhysMargins.left;
	frPrint.rc.top = rectMargins.top - rectPhysMargins.top;
	frPrint.rc.right = ptPage.x - rectMargins.right - rectPhysMargins.left;
	frPrint.rc.bottom = ptPage.y - rectMargins.bottom - rectPhysMargins.top;
	frPrint.rcPage.left = 0;
	frPrint.rcPage.top = 0;
	frPrint.rcPage.right = ptPage.x - rectPhysMargins.left - rectPhysMargins.right - 1;
	frPrint.rcPage.bottom = ptPage.y - rectPhysMargins.top - rectPhysMargins.bottom - 1;

	frPrint.rc.top += printMarge;
	frPrint.rc.bottom -= printMarge;
	frPrint.rc.left += printMarge;
	frPrint.rc.right -= printMarge;
	
	while (lengthPrinted < lengthDoc) 
	{
		printPage = (!(_pdlg.Flags & PD_PAGENUMS) ||
		             (pageNum >= _pdlg.nFromPage) && (pageNum <= _pdlg.nToPage));
		if (printPage) 
		{
			::StartPage(_pdlg.hDC);
		}
		
		frPrint.chrg.cpMin = lengthPrinted;
		frPrint.chrg.cpMax = lengthDoc;
		lengthPrinted = long(pSEView.execute(SCI_FORMATRANGE, printPage, reinterpret_cast<LPARAM>(&frPrint)));
		
		if (printPage) 
		{
			::EndPage(_pdlg.hDC);
		}
		pageNum++;

		if ((_pdlg.Flags & PD_PAGENUMS) && (pageNum > _pdlg.nToPage))
			break;
	}
	pSEView.execute(SCI_FORMATRANGE, FALSE, 0);
	::EndDoc(_pdlg.hDC);
	::DeleteDC(_pdlg.hDC);
}
