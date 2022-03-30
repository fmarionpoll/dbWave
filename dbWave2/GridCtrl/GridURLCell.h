// GridURLCell.h: interface for the GridURLCell class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "GridCell.h"

using URLStruct = struct
{
	LPCTSTR szURLPrefix;
	size_t nLength;
};

class GridURLCell : public GridCell
{
	DECLARE_DYNCREATE(GridURLCell)

public:
	GridURLCell();
	~GridURLCell() override;

	BOOL Draw(CDC* p_dc, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE) override;
	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	virtual LPCTSTR GetTipText() { return nullptr; }
	void SetAutoLaunchUrl(BOOL bLaunch = TRUE) { m_bLaunchUrl = bLaunch; }
	BOOL GetAutoLaunchUrl() { return m_bLaunchUrl; }

protected:
	BOOL OnSetCursor() override;
	void OnClick(CPoint PointCellRelative) override;

	BOOL HasUrl(CString str);
	BOOL OverURL(CPoint& pt, CString& strURL);

protected:
#ifndef _WIN32_WCE
	static HCURSOR g_hLinkCursor; // Hyperlink mouse cursor
	HCURSOR GetHandCursor();
#endif
	static URLStruct g_szURIprefixes[];

protected:
	COLORREF m_clrUrl{};
	BOOL m_bLaunchUrl{};
	CRect m_Rect{};
};
