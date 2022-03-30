#include "StdAfx.h"
#include "GridDefaultCell.h"


IMPLEMENT_DYNCREATE(GridDefaultCell, GridCell)

GridDefaultCell::GridDefaultCell()
{
#ifdef _WIN32_WCE
	m_nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
#else
	m_nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
#endif
	m_crFgClr = CLR_DEFAULT;
	m_crBkClr = CLR_DEFAULT;
	m_Size = CSize(30, 10);
	m_dwStyle = 0;

#ifdef _WIN32_WCE
	LOGFONT lf;
	GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
	SetFont(&lf);
#else // not CE

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));

	SetFont(&(ncm.lfMessageFont));
#endif
}

GridDefaultCell::~GridDefaultCell()
{
	m_Font.DeleteObject();
}

void GridDefaultCell::SetFont(const LOGFONT* plf)
{
	ASSERT(plf);

	if (!plf) return;

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(plf);

	GridCell::SetFont(plf);

	// Get the font size and hence the default cell size
	CDC* p_dc = CDC::FromHandle(GetDC(nullptr));
	if (p_dc)
	{
		CFont* pOldFont = p_dc->SelectObject(&m_Font);

		SetMargin(p_dc->GetTextExtent(_T(" "), 1).cx);
		m_Size = p_dc->GetTextExtent(_T(" XXXXXXXXXXXX "), 14);
		m_Size.cy = (m_Size.cy * 3) / 2;

		p_dc->SelectObject(pOldFont);
		ReleaseDC(nullptr, p_dc->GetSafeHdc());
	}
	else
	{
		SetMargin(3);
		m_Size = CSize(40, 16);
	}
}

LOGFONT* GridDefaultCell::GetFont() const
{
	ASSERT(m_plfFont); // This is the default - it CAN'T be NULL!
	return m_plfFont;
}

CFont* GridDefaultCell::GetFontObject() const
{
	ASSERT(m_Font.GetSafeHandle());
	return (CFont*)&m_Font;
}
