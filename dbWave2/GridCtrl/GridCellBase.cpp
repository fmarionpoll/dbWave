// GridCellBase.cpp : implementation file
//
// MFC Grid Control - Main grid cell base class
//
// Provides the implementation for the base cell type of the
// grid control. No data is stored (except for state) but default
// implementations of drawing, printingetc provided. MUST be derived
// from to be used.
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name and all copyright
// notices remains intact.
//
// An email letting me know how you are using it would be nice as well.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with GridCtrl v2.22+
//
// History:
// Ken Bertelson - 12 Apr 2000 - Split GridCell into GridCell and GridCellBase
// C Maunder     - 19 May 2000 - Fixed sort arrow drawing (Ivan Ilinov)
// C Maunder     - 29 Aug 2000 - operator= checks for NULL font before setting (Martin Richter)
// C Maunder     - 15 Oct 2000 - GetTextExtent fixed (Martin Richter)
// C Maunder     -  1 Jan 2001 - Added ValidateEdit
// Yogurt        - 13 Mar 2004 - GetCellExtent fixed
//
// NOTES: Each grid cell should take care of it's own drawing, though the Draw()
//        method takes an "erase background" paramter that is called if the grid
//        decides to draw the entire grid background in on hit. Certain ambient
//        properties such as the default font to use, and hints on how to draw
//        fixed cells should be fetched from the parent grid. The grid trusts the
//        cells will behave in a certain way, and the cells trust the grid will
//        supply accurate information.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCtrl.h"
#include "GridCellBase.h"

#include "../ColorNames.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(GridCellBase, CObject)

/////////////////////////////////////////////////////////////////////////////
// GridCellBase

GridCellBase::GridCellBase()
{
	m_nState = 0;
	Reset();
}

GridCellBase::~GridCellBase()
= default;

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Operations

void GridCellBase::Reset()
{
	m_nState = 0;
}

void GridCellBase::operator=(const GridCellBase& cell)
{
	if (this == &cell) return;

	SetGrid(cell.GetGrid()); // do first in case of dependencies

	SetText(cell.GetText());
	SetImage(cell.GetImage());
	SetData(cell.GetData());
	SetState(cell.GetState());
	SetFormat(cell.GetFormat());
	SetTextClr(cell.GetTextClr());
	SetBackClr(cell.GetBackClr());
	SetFont(cell.IsDefaultFont() ? nullptr : cell.GetFont());
	SetMargin(cell.GetMargin());
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Attributes

// Returns a pointer to a cell that holds default values for this particular type of cell
GridCellBase* GridCellBase::GetDefaultCell() const
{
	if (GetGrid())
		return GetGrid()->GetDefaultCell(IsFixedRow(), IsFixedCol());
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Operations

// EFW - Various changes to make it draw cells better when using alternate
// color schemes.  Also removed printing references as that's now done
// by PrintCell() and fixed the sort marker so that it doesn't draw out
// of bounds.
BOOL GridCellBase::Draw(CDC* p_dc, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /*=TRUE*/)
{
	// Note - all through this function we totally brutalise 'rect'. Do not
	// depend on it's value being that which was passed in.

	GridCtrl* pGrid = GetGrid();
	ASSERT(pGrid);

	if (!pGrid || !p_dc)
		return FALSE;

	if (rect.Width() <= 0 || rect.Height() <= 0) // prevents imagelist item from drawing even
		return FALSE; //  though cell is hidden

	//TRACE3("Drawing %scell %d, %d\n", IsFixed()? _T("Fixed ") : _T(""), nRow, nCol);

	const int nSavedDC = p_dc->SaveDC();
	p_dc->SetBkMode(TRANSPARENT);

	// Get the default cell implementation for this kind of cell. We use it if this cell
	// has anything marked as "default"
	const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
	if (!pDefaultCell)
		return FALSE;

	// Set up text and background colours
	COLORREF TextBkClr;

	COLORREF TextClr = (GetTextClr() == CLR_DEFAULT) ? pDefaultCell->GetTextClr() : GetTextClr();
	if (GetBackClr() == CLR_DEFAULT)
		TextBkClr = pDefaultCell->GetBackClr();
	else
	{
		bEraseBkgnd = TRUE;
		TextBkClr = GetBackClr();
	}

	// Draw cell background and highlighting (if necessary)
	if (IsFocused() || IsDropHighlighted())
	{
		// Always draw even in list mode so that we can tell where the
		// cursor is at.  Use the highlight colors though.
		if (GetState() & GVIS_SELECTED)
		{
			TextBkClr = GetSysColor(COLOR_HIGHLIGHT);
			TextClr = GetSysColor(COLOR_HIGHLIGHTTEXT);
			bEraseBkgnd = TRUE;
		}

		rect.right++;
		rect.bottom++; // FillRect doesn't draw RHS or bottom
		if (bEraseBkgnd)
		{
			TRY
				{
					CBrush brush(TextBkClr);
					p_dc->FillRect(rect, &brush);
				}
			CATCH(CResourceException, e)
				{
					//e->ReportError();
				}
			END_CATCH
		}

		// Don't adjust frame rect if no grid lines so that the
		// whole cell is enclosed.
		if (pGrid->GetGridLines() != GVL_NONE)
		{
			rect.right--;
			rect.bottom--;
		}

		if (pGrid->GetFrameFocusCell())
		{
			// Use same color as text to outline the cell so that it shows
			// up if the background is black.
			TRY
				{
					CBrush brush(TextClr);
					p_dc->FrameRect(rect, &brush);
				}
			CATCH(CResourceException, e)
				{
					//e->ReportError();
				}
			END_CATCH
		}
		p_dc->SetTextColor(TextClr);

		// Adjust rect after frame draw if no grid lines
		if (pGrid->GetGridLines() == GVL_NONE)
		{
			rect.right--;
			rect.bottom--;
		}

		//rect.DeflateRect(0,1,1,1);  - Removed by Yogurt
	}
	else if ((GetState() & GVIS_SELECTED))
	{
		rect.right++;
		rect.bottom++; // FillRect doesn't draw RHS or bottom
		p_dc->FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
		rect.right--;
		rect.bottom--;
		p_dc->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		if (bEraseBkgnd)
		{
			rect.right++;
			rect.bottom++; // FillRect doesn't draw RHS or bottom
			CBrush brush(TextBkClr);
			p_dc->FillRect(rect, &brush);
			rect.right--;
			rect.bottom--;
		}
		p_dc->SetTextColor(TextClr);
	}

	// Draw lines only when wanted
	if (IsFixed() && pGrid->GetGridLines() != GVL_NONE)
	{
		const GridCellID FocusCell = pGrid->GetFocusCell();

		// As above, always show current location even in list mode so
		// that we know where the cursor is at.
		const BOOL bHiliteFixed = pGrid->GetTrackFocusCell() && pGrid->IsValid(FocusCell) &&
			(FocusCell.row == nRow || FocusCell.col == nCol);

		// If this fixed cell is on the same row/col as the focus cell,
		// highlight it.
		if (bHiliteFixed)
		{
			rect.right++;
			rect.bottom++;
			p_dc->DrawEdge(rect, BDR_SUNKENINNER /*EDGE_RAISED*/, BF_RECT);
			rect.DeflateRect(1, 1);
		}
		else
		{
			CPen lightpen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT)),
			     darkpen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW)),
			     *pOldPen = p_dc->GetCurrentPen();

			p_dc->SelectObject(&lightpen);
			p_dc->MoveTo(rect.right, rect.top);
			p_dc->LineTo(rect.left, rect.top);
			p_dc->LineTo(rect.left, rect.bottom);

			p_dc->SelectObject(&darkpen);
			p_dc->MoveTo(rect.right, rect.top);
			p_dc->LineTo(rect.right, rect.bottom);
			p_dc->LineTo(rect.left, rect.bottom);

			p_dc->SelectObject(pOldPen);
			rect.DeflateRect(1, 1);
		}
	}

	// Draw Text and image
#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	if (!p_dc->m_bPrinting)
#endif
	{
		CFont* pFont = GetFontObject();
		ASSERT(pFont);
		if (pFont)
			p_dc->SelectObject(pFont);
	}

	//rect.DeflateRect(GetMargin(), 0); - changed by Yogurt
	rect.DeflateRect(GetMargin(), GetMargin());
	rect.right++;
	rect.bottom++;

	if (pGrid->GetImageList() && GetImage() >= 0)
	{
		IMAGEINFO Info;
		if (pGrid->GetImageList()->GetImageInfo(GetImage(), &Info))
		{
			//  would like to use a clipping region but seems to have issue
			//  working with CMemDC directly.  Instead, don't display image
			//  if any part of it cut-off
			//
			// CRgn rgn;
			// rgn.CreateRectRgnIndirect(rect);
			// p_dc->SelectClipRgn(&rgn);
			// rgn.DeleteObject();

			/*
			// removed by Yogurt
			int nImageWidth = Info.rcImage.right-Info.rcImage.left+1;
			int nImageHeight = Info.rcImage.bottom-Info.rcImage.top+1;
			if( nImageWidth + rect.left <= rect.right + (int)(2*GetMargin())
				&& nImageHeight + rect.top <= rect.bottom + (int)(2*GetMargin())  )
			{
				pGrid->GetImageList()->Draw(p_dc, GetImage(), rect.TopLeft(), ILD_NORMAL);
			}
			*/
			// Added by Yogurt
			const int nImageWidth = Info.rcImage.right - Info.rcImage.left;
			const int nImageHeight = Info.rcImage.bottom - Info.rcImage.top;
			if ((nImageWidth + rect.left <= rect.right) && (nImageHeight + rect.top <= rect.bottom))
				pGrid->GetImageList()->Draw(p_dc, GetImage(), rect.TopLeft(), ILD_NORMAL);

			//rect.left += nImageWidth+GetMargin();
		}
	}

	// Draw sort arrow
	if (pGrid->GetSortColumn() == nCol && nRow == 0)
	{
		CSize size = p_dc->GetTextExtent(_T("M"));
		const int nOffset = 2;

		// Base the size of the triangle on the smaller of the column
		// height or text height with a slight offset top and bottom.
		// Otherwise, it can get drawn outside the bounds of the cell.
		size.cy -= (nOffset * 2);

		if (size.cy >= rect.Height())
			size.cy = rect.Height() - (nOffset * 2);

		size.cx = size.cy; // Make the dimensions square

		// Kludge for vertical text
		const BOOL bVertical = (GetFont()->lfEscapement == 900);

		// Only draw if it'll fit!
		//if (size.cx + rect.left < rect.right + (int)(2*GetMargin())) - changed / Yogurt
		if (size.cx + rect.left < rect.right)
		{
			const int nTriangleBase = rect.bottom - nOffset - size.cy; // Triangle bottom right
			//int nTriangleBase = (rect.top + rect.bottom - size.cy)/2; // Triangle middle right
			//int nTriangleBase = rect.top + nOffset;                 // Triangle top right

			//int nTriangleLeft = rect.right - size.cx;                 // Triangle RHS
			//int nTriangleLeft = (rect.right + rect.left - size.cx)/2; // Triangle middle
			//int nTriangleLeft = rect.left;                            // Triangle LHS

			int nTriangleLeft;
			if (bVertical)
				nTriangleLeft = (rect.right + rect.left - size.cx) / 2; // Triangle middle
			else
				nTriangleLeft = rect.right - size.cx; // Triangle RHS

			CPen penShadow(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
			CPen penLight(PS_SOLID, 0, GetSysColor(COLOR_3DHILIGHT));
			if (pGrid->GetSortAscending())
			{
				// Draw triangle pointing upwards
				const auto pOldPen = p_dc->SelectObject(&penLight);
				p_dc->MoveTo(nTriangleLeft + 1, nTriangleBase + size.cy + 1);
				p_dc->LineTo(nTriangleLeft + (size.cx / 2) + 1, nTriangleBase + 1);
				p_dc->LineTo(nTriangleLeft + size.cx + 1, nTriangleBase + size.cy + 1);
				p_dc->LineTo(nTriangleLeft + 1, nTriangleBase + size.cy + 1);

				p_dc->SelectObject(&penShadow);
				p_dc->MoveTo(nTriangleLeft, nTriangleBase + size.cy);
				p_dc->LineTo(nTriangleLeft + (size.cx / 2), nTriangleBase);
				p_dc->LineTo(nTriangleLeft + size.cx, nTriangleBase + size.cy);
				p_dc->LineTo(nTriangleLeft, nTriangleBase + size.cy);
				p_dc->SelectObject(pOldPen);
			}
			else
			{
				// Draw triangle pointing downwards
				const auto pOldPen = p_dc->SelectObject(&penLight);
				p_dc->MoveTo(nTriangleLeft + 1, nTriangleBase + 1);
				p_dc->LineTo(nTriangleLeft + (size.cx / 2) + 1, nTriangleBase + size.cy + 1);
				p_dc->LineTo(nTriangleLeft + size.cx + 1, nTriangleBase + 1);
				p_dc->LineTo(nTriangleLeft + 1, nTriangleBase + 1);

				p_dc->SelectObject(&penShadow);
				p_dc->MoveTo(nTriangleLeft, nTriangleBase);
				p_dc->LineTo(nTriangleLeft + (size.cx / 2), nTriangleBase + size.cy);
				p_dc->LineTo(nTriangleLeft + size.cx, nTriangleBase);
				p_dc->LineTo(nTriangleLeft, nTriangleBase);
				p_dc->SelectObject(pOldPen);
			}

			if (!bVertical)
				rect.right -= size.cy;
		}
	}

	// We want to see '&' characters so use DT_NOPREFIX
	GetTextRect(rect);
	rect.right++;
	rect.bottom++;

	DrawText(p_dc->m_hDC, GetText(), -1, rect, GetFormat() | DT_NOPREFIX);

	p_dc->RestoreDC(nSavedDC);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Mouse and Cursor events

// Not yet implemented
void GridCellBase::OnMouseEnter()
{
	//TRACE0("Mouse entered cell\n");
}

void GridCellBase::OnMouseOver()
{
	//ATLTRACE2("Mouse over cell\n");
}

// Not Yet Implemented
void GridCellBase::OnMouseLeave()
{
	//ATLTRACE2("Mouse left cell\n");
}

void GridCellBase::OnClick(CPoint PointCellRelative)
{
	UNUSED_ALWAYS(PointCellRelative);
	//ATLTRACE2("Mouse Left btn up in cell at x=%i y=%i\n", PointCellRelative.x, PointCellRelative.y);
}

void GridCellBase::OnClickDown(CPoint PointCellRelative)
{
	UNUSED_ALWAYS(PointCellRelative);
	//ATLTRACE2("Mouse Left btn down in cell at x=%i y=%i\n", PointCellRelative.x, PointCellRelative.y);
}

void GridCellBase::OnRClick(CPoint PointCellRelative)
{
	UNUSED_ALWAYS(PointCellRelative);
	//ATLTRACE2("Mouse right-clicked in cell at x=%i y=%i\n", PointCellRelative.x, PointCellRelative.y);
}

void GridCellBase::OnDblClick(CPoint PointCellRelative)
{
	UNUSED_ALWAYS(PointCellRelative);
	//ATLTRACE2("Mouse double-clicked in cell at x=%i y=%i\n", PointCellRelative.x, PointCellRelative.y);
}

// Return TRUE if you set the cursor
BOOL GridCellBase::OnSetCursor()
{
#ifndef _WIN32_WCE_NO_CURSOR
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
#endif
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase editing

void GridCellBase::OnEndEdit()
{
	ASSERT(FALSE);
}

BOOL GridCellBase::ValidateEdit(LPCTSTR str)
{
	UNUSED_ALWAYS(str);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Sizing

BOOL GridCellBase::GetTextRect(LPRECT pRect) // i/o:  i=dims of cell rect; o=dims of text rect
{
	if (GetImage() >= 0)
	{
		IMAGEINFO Info;

		const GridCtrl* pGrid = GetGrid();
		const CImageList* pImageList = pGrid->GetImageList();

		if (pImageList && pImageList->GetImageInfo(GetImage(), &Info))
		{
			const int nImageWidth = Info.rcImage.right - Info.rcImage.left + 1;
			pRect->left += nImageWidth + GetMargin();
		}
	}

	return TRUE;
}

// By default this uses the selected font (which is a bigger font)
CSize GridCellBase::GetTextExtent(LPCTSTR sz_text, CDC* p_dc /*= NULL*/)
{
	GridCtrl* pGrid = GetGrid();
	ASSERT(pGrid);

	BOOL bReleaseDC = FALSE;
	if (p_dc == nullptr || sz_text == nullptr)
	{
		if (sz_text)
			p_dc = pGrid->GetDC();
		if (p_dc == nullptr || sz_text == nullptr)
		{
			const auto pDefCell = static_cast<GridDefaultCell*>(GetDefaultCell());
			ASSERT(pDefCell);
			CSize value(0, 0);
			if (pDefCell != nullptr)
				value = CSize(pDefCell->GetWidth(), pDefCell->GetHeight());
			return value;
		}
		bReleaseDC = TRUE;
	}

	CFont *pOldFont = nullptr,
	      *pFont = GetFontObject();
	if (pFont)
		pOldFont = p_dc->SelectObject(pFont);

	CSize size;
	const int nFormat = GetFormat();

	// If the cell is a multiline cell, then use the width of the cell
	// to get the height
	if ((nFormat & DT_WORDBREAK) && !(nFormat & DT_SINGLELINE))
	{
		CString str = sz_text;
		int nMaxWidth = 0;
		while (TRUE)
		{
			const int nPos = str.Find(_T('\n'));
			CString TempStr = (nPos < 0) ? str : str.Left(nPos);
			const int nTempWidth = p_dc->GetTextExtent(TempStr).cx;
			if (nTempWidth > nMaxWidth)
				nMaxWidth = nTempWidth;

			if (nPos < 0)
				break;
			str = str.Mid(nPos + 1); // Bug fix by Thomas Steinborn
		}

		CRect rect;
		rect.SetRect(0, 0, nMaxWidth + 1, 0);
		p_dc->DrawText(sz_text, -1, rect, nFormat | DT_CALCRECT);
		size = rect.Size();
	}
	else
		size = p_dc->GetTextExtent(sz_text, static_cast<int>(_tcslen(sz_text)));

	// Removed by Yogurt
	//TEXTMETRIC tm;
	//p_dc->GetTextMetrics(&tm);
	//size.cx += (tm.tmOverhang);

	if (pOldFont)
		p_dc->SelectObject(pOldFont);

	size += CSize(2 * GetMargin(), 2 * GetMargin());

	// Kludge for vertical text
	LOGFONT* pLF = GetFont();
	if (pLF->lfEscapement == 900 || pLF->lfEscapement == -900)
	{
		const int nTemp = size.cx;
		size.cx = size.cy;
		size.cy = nTemp;
		size += CSize(0, 4 * GetMargin());
	}

	if (bReleaseDC)
		pGrid->ReleaseDC(p_dc);

	return size;
}

CSize GridCellBase::GetCellExtent(CDC* p_dc)
{
	CSize size = GetTextExtent(GetText(), p_dc);
	CSize ImageSize(0, 0);

	const int nImage = GetImage();
	if (nImage >= 0)
	{
		const GridCtrl* pGrid = GetGrid();
		ASSERT(pGrid);
		IMAGEINFO Info;
		if (pGrid != nullptr && pGrid->GetImageList() && pGrid->GetImageList()->GetImageInfo(nImage, &Info))
		{
			ImageSize = CSize(Info.rcImage.right - Info.rcImage.left,
			                  Info.rcImage.bottom - Info.rcImage.top);
			if (size.cx > 2 * static_cast<int>(GetMargin()))
				ImageSize.cx += GetMargin();
			ImageSize.cy += 2 * static_cast<int>(GetMargin());
		}
	}
	size.cx += ImageSize.cx + 1;
	size.cy = max(size.cy, ImageSize.cy) + 1;
	if (IsFixed())
	{
		size.cx++;
		size.cy++;
	}
	return size;
}

// EFW - Added to print cells so that grids that use different colors are
// printed correctly.
BOOL GridCellBase::PrintCell(CDC* p_dc, int /*nRow*/, int /*nCol*/, CRect rect)
{
#if defined(_WIN32_WCE_NO_PRINTING) || defined(GRIDCONTROL_NO_PRINTING)
	return FALSE;
#else
	COLORREF crFG, crBG;
	GV_ITEM Item;

	GridCtrl* pGrid = GetGrid();
	if (!pGrid || !p_dc)
		return FALSE;

	if (rect.Width() <= 0
		|| rect.Height() <= 0) // prevents imagelist item from drawing even
		return FALSE; //  though cell is hidden

	const int nSavedDC = p_dc->SaveDC();

	p_dc->SetBkMode(TRANSPARENT);

	if (pGrid->GetShadedPrintOut())
	{
		// Get the default cell implementation for this kind of cell. We use it if this cell
		// has anything marked as "default"
		const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
		if (!pDefaultCell)
			return FALSE;

		// Use custom color if it doesn't match the default color and the
		// default grid background color.  If not, leave it alone.
		if (IsFixed())
			crBG = (GetBackClr() != CLR_DEFAULT) ? GetBackClr() : pDefaultCell->GetBackClr();
		else
			crBG = (GetBackClr() != CLR_DEFAULT && GetBackClr() != pDefaultCell->GetBackClr())
				       ? GetBackClr()
				       : CLR_DEFAULT;

		// Use custom color if the background is different or if it doesn't
		// match the default color and the default grid text color.
		if (IsFixed())
		{
			crFG = (GetBackClr() != CLR_DEFAULT) ? GetTextClr() : pDefaultCell->GetTextClr();
		}
		else
			crFG = (GetBackClr() != CLR_DEFAULT) ? GetTextClr() : pDefaultCell->GetTextClr();

		// If not printing on a color printer, adjust the foreground color
		// to a gray scale if the background color isn't used so that all
		// colors will be visible.  If not, some colors turn to solid black
		// or white when printed and may not show up.  This may be caused by
		// coarse dithering by the printer driver too (see image note below).
		if (p_dc->GetDeviceCaps(NUMCOLORS) == 2 && crBG == CLR_DEFAULT)
			crFG = RGB(GetRValue(crFG) * 0.30, GetGValue(crFG) * 0.59,
			           GetBValue(crFG) * 0.11);

		// Only erase the background if the color is not the default
		// grid background color.
		if (crBG != CLR_DEFAULT)
		{
			CBrush brush(crBG);
			rect.right++;
			rect.bottom++;
			p_dc->FillRect(rect, &brush);
			rect.right--;
			rect.bottom--;
		}
	}
	else
	{
		crBG = CLR_DEFAULT;
		crFG = col_black;
	}

	p_dc->SetTextColor(crFG);

	CFont* pFont = GetFontObject();
	if (pFont)
		p_dc->SelectObject(pFont);

	/*
	// ***************************************************
	// Disabled - if you need this functionality then you'll need to rewrite.
	// Create the appropriate font and select into DC.
	CFont Font;
	// Bold the fixed cells if not shading the print out.  Use italic
	// font it it is enabled.
	const LOGFONT* plfFont = GetFont();
	if(IsFixed() && !pGrid->GetShadedPrintOut())
	{
		Font.CreateFont(plfFont->lfHeight, 0, 0, 0, FW_BOLD, plfFont->lfItalic, 0, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
#ifndef _WIN32_WCE
			PROOF_QUALITY,
#else
			DEFAULT_QUALITY,
#endif
			VARIABLE_PITCH | FF_SWISS, plfFont->lfFaceName);
	}
	else
		Font.CreateFontIndirect(plfFont);

	p_dc->SelectObject(&Font);
	// ***************************************************
	*/

	// Draw lines only when wanted on fixed cells.  Normal cell grid lines
	// are handled in OnPrint.
	if (pGrid->GetGridLines() != GVL_NONE && IsFixed())
	{
		CPen lightpen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT)),
		     darkpen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW)),
		     *pOldPen = p_dc->GetCurrentPen();

		p_dc->SelectObject(&lightpen);
		p_dc->MoveTo(rect.right, rect.top);
		p_dc->LineTo(rect.left, rect.top);
		p_dc->LineTo(rect.left, rect.bottom);

		p_dc->SelectObject(&darkpen);
		p_dc->MoveTo(rect.right, rect.top);
		p_dc->LineTo(rect.right, rect.bottom);
		p_dc->LineTo(rect.left, rect.bottom);

		rect.DeflateRect(1, 1);
		p_dc->SelectObject(pOldPen);
	}

	rect.DeflateRect(GetMargin(), 0);

	if (pGrid->GetImageList() && GetImage() >= 0)
	{
		// NOTE: If your printed images look like fuzzy garbage, check the
		//       settings on your printer driver.  If it's using coarse
		//       dithering and/or vector graphics, they may print wrong.
		//       Changing to fine dithering and raster graphics makes them
		//       print properly.  My HP 4L had that problem.

		IMAGEINFO Info;
		if (pGrid->GetImageList()->GetImageInfo(GetImage(), &Info))
		{
			const int nImageWidth = Info.rcImage.right - Info.rcImage.left;
			pGrid->GetImageList()->Draw(p_dc, GetImage(), rect.TopLeft(), ILD_NORMAL);
			rect.left += nImageWidth + GetMargin();
		}
	}

	// Draw without clipping so as not to lose text when printed for real
	// DT_NOCLIP removed 01.01.01. Slower, but who cares - we are printing!
	DrawText(p_dc->m_hDC, GetText(), -1, rect,
	         GetFormat() | /*DT_NOCLIP | */ DT_NOPREFIX);

	p_dc->RestoreDC(nSavedDC);

	return TRUE;
#endif
}

/*****************************************************************************
Callable by derived classes, only
*****************************************************************************/
LRESULT GridCellBase::SendMessageToParent(int nRow, int nCol, int nMessage)
{
	const GridCtrl* pGrid = GetGrid();
	if (pGrid)
		return pGrid->SendMessageToParent(nRow, nCol, nMessage);
	return 0;
}
