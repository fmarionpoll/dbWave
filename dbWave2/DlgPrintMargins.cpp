#include "StdAfx.h"
#include "resource.h"
#include "DlgPrintMargins.h"

#include "dbWave.h"
#include "DlgPrintDataOptions.h"
#include "DlgPrintDrawArea.h"
#include "DlgPrintPageMargins.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgPrintMargins::DlgPrintMargins(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgPrintMargins::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgPrintMargins, CDialog)
	ON_BN_CLICKED(IDC_COMMENTSOPTIONS, OnCommentsoptions)
	ON_BN_CLICKED(IDC_DRAWAREA, OnDrawarea)
	ON_BN_CLICKED(IDC_DRAWOPTIONS, OnDrawoptions)
	ON_BN_CLICKED(IDC_PAGEMARGINS, OnPagemargins)
	ON_BN_CLICKED(IDC_PRINTERSETUP, OnPrintersetup)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL DlgPrintMargins::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetPageSize();
	UpdateData(FALSE);
	SketchPrinterPage();
	return TRUE;
}

void DlgPrintMargins::OnOK()
{
	mdPM->b_changed = TRUE;
	CDialog::OnOK();
}

void DlgPrintMargins::OnCommentsoptions()
{
	CPrintDataCommentsDlg dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void DlgPrintMargins::OnDrawarea()
{
	DlgPrintDrawArea dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void DlgPrintMargins::OnDrawoptions()
{
	DlgPrintDataOptions dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void DlgPrintMargins::OnPagemargins()
{
	DlgPrintPageMargins dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void DlgPrintMargins::OnPrintersetup()
{
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // load browse parameters
	p_app->FilePrintSetup();
	GetPageSize();
	SketchPrinterPage();
}

void DlgPrintMargins::SketchPrinterPage()
{
	CClientDC dc(this);

	/*auto p_old_brush = (CBrush*) */
	dc.SelectStockObject(WHITE_BRUSH);
	/*auto pOldPen = (CPen*) */
	dc.SelectStockObject(BLACK_PEN);

	const auto p_f_wnd = GetDlgItem(IDC_RECT1);
	p_f_wnd->GetWindowRect(&m_rect);
	ScreenToClient(&m_rect);
	InvalidateRect(&m_rect, TRUE);
}

void DlgPrintMargins::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	/*CBrush* p_old_brush = (CBrush*) */
	dc.SelectStockObject(WHITE_BRUSH);
	const auto p_old_pen = static_cast<CPen*>(dc.SelectStockObject(BLACK_PEN));

	const auto page_background = RGB(192, 192, 192);
	const auto comment_area = page_background; //RGB(  0, 128, 128);

	// erase background
	const auto p_f_wnd = GetDlgItem(IDC_RECT1);
	p_f_wnd->GetWindowRect(&m_rect); // get window rectangle
	ScreenToClient(&m_rect); // convert  coordinates
	dc.FillSolidRect(&m_rect, page_background);

	CPoint center; // center of the drawing area
	center.x = (m_rect.right + m_rect.left) / 2;
	center.y = (m_rect.bottom + m_rect.top) / 2;
	const auto rectsize = min(m_rect.Width(), m_rect.Height()); // max size of the square
	const auto maxresol = max(mdPM->vertical_resolution, mdPM->horizontal_resolution); // max resolution

	// draw page area
	auto diff = MulDiv(mdPM->horizontal_resolution, rectsize, maxresol) / 2;
	m_pagerect.left = center.x - diff;
	m_pagerect.right = center.x + diff;
	diff = MulDiv(mdPM->vertical_resolution, rectsize, maxresol) / 2;
	m_pagerect.top = center.y - diff;
	m_pagerect.bottom = center.y + diff;
	dc.Rectangle(&m_pagerect);

	auto i = MulDiv(mdPM->left_page_margin, rectsize, maxresol); // vertical lines
	m_bars[0] = CRect(m_pagerect.left + i, m_rect.top, m_pagerect.left + i, m_rect.bottom);

	i = MulDiv(mdPM->right_page_margin, rectsize, maxresol);
	m_bars[2] = CRect(m_pagerect.right - i, m_rect.top, m_pagerect.right - i, m_rect.bottom);

	i = MulDiv(mdPM->top_page_margin, rectsize, maxresol);
	m_bars[1] = CRect(m_rect.left, m_pagerect.top + i, m_rect.right, m_pagerect.top + i);

	i = MulDiv(mdPM->bottom_page_margin, rectsize, maxresol);
	m_bars[3] = CRect(m_rect.left, m_pagerect.bottom - i, m_rect.right, m_pagerect.bottom - i);

	// draw drawing area
	if (mdPM->b_frame_rect)
		dc.SelectStockObject(BLACK_PEN);
	else
		dc.SelectStockObject(WHITE_PEN);

	const int rowmax = m_pagerect.bottom - i;
	CRect docrect;
	docrect.left = m_pagerect.left + MulDiv(mdPM->left_page_margin, rectsize, maxresol);
	docrect.right = docrect.left + MulDiv(mdPM->width_doc, rectsize, maxresol);

	CRect commtrect;
	commtrect.left = docrect.right + MulDiv(mdPM->text_separator, rectsize, maxresol);
	commtrect.right = m_pagerect.right - MulDiv(mdPM->right_page_margin, rectsize, maxresol);
	if (commtrect.right < commtrect.left)
		commtrect.right = commtrect.left + 10;

	// draw comments
	const auto rowheight = MulDiv(mdPM->height_doc, rectsize, maxresol);
	const auto rowsep = MulDiv(mdPM->height_separator, rectsize, maxresol);
	int toprow = m_pagerect.top + MulDiv(mdPM->top_page_margin, rectsize, maxresol);
	auto bottomrow = toprow + rowheight;

	m_bars[4] = CRect(docrect.right, m_rect.top, docrect.right, m_rect.bottom);
	m_bars[6] = CRect(commtrect.left, m_rect.top, commtrect.left, m_rect.bottom);
	m_bars[5] = CRect(m_rect.left, bottomrow, m_rect.right, bottomrow);
	m_bars[7] = CRect(m_rect.left, bottomrow + rowsep, m_rect.right, bottomrow + rowsep);

	while (bottomrow < rowmax)
	{
		docrect.top = toprow;
		docrect.bottom = bottomrow;
		dc.FillSolidRect(&docrect, comment_area);

		commtrect.top = docrect.top;
		commtrect.bottom = docrect.bottom;
		dc.FillSolidRect(&commtrect, comment_area);
		toprow = docrect.bottom + rowsep;
		bottomrow = toprow + rowheight;
	}

	// draw bars (resizable margins)
	for (auto& m_bar : m_bars)
		DrawBar(&m_bar, &dc);

	dc.SelectObject(p_old_pen);
}

void DlgPrintMargins::DrawBar(CRect* bar, CDC* pdc)
{
	CPen penbars(PS_DOT, 1, RGB(0, 0, 255));
	const auto p_old_pen = pdc->SelectObject(&penbars);

	pdc->MoveTo(bar->left, bar->top);
	pdc->LineTo(bar->right, bar->bottom);

	pdc->SelectObject(p_old_pen);
}

void DlgPrintMargins::GetPageSize()
{
	CPrintDialog dlg(FALSE);
	if (!AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd))
	{
		AfxMessageBox(_T("Printer functions not available:\nundefined printer"));
		CDialog::OnCancel();
		return;
	}

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC(); // to delete at the end -- see doc!
	ASSERT(h_dc != NULL);
	dc.Attach(h_dc);

	// Get the size of the page in pixels
	mdPM->horizontal_resolution = dc.GetDeviceCaps(HORZRES);
	mdPM->vertical_resolution = dc.GetDeviceCaps(VERTRES);

	CString csResolut;
	csResolut.Format(_T("%i"), mdPM->vertical_resolution);
	SetDlgItemText(IDC_PAGEHEIGHT, csResolut);
	csResolut.Format(_T("%i"), mdPM->horizontal_resolution);
	SetDlgItemText(IDC_PAGEWIDTH, csResolut);
}

void DlgPrintMargins::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rc_client;
	GetClientRect(&rc_client);
	const auto p_dc = GetDC();

	if (IsMouseOverAnyBar(&point) < 0)
	{
		ReleaseDC(p_dc);
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	SetCapture();
	m_bCaptured = TRUE;
	CDialog::OnLButtonDown(nFlags, point);
}

void DlgPrintMargins::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = FALSE;

		const auto rectsize = min(m_rect.Width(), m_rect.Height()); // max size of the square
		const auto maxresol = max(mdPM->vertical_resolution, mdPM->horizontal_resolution); // max resolution

		switch (m_icapturedBar)
		{
		case 0: // left page margin
			if (m_bars[0].left < m_pagerect.left)
			{
				m_bars[0].left = m_pagerect.left;
				m_bars[0].right = m_pagerect.left;
			}
			if (m_bars[0].left > m_pagerect.right)
			{
				m_bars[0].left = m_pagerect.right;
				m_bars[0].right = m_pagerect.right;
			}
			mdPM->left_page_margin = MulDiv(m_bars[0].right - m_pagerect.left, maxresol, rectsize);
			break;

		case 1: // top page margin
			if (m_bars[1].top < m_pagerect.top)
			{
				m_bars[1].top = m_pagerect.top;
				m_bars[1].bottom = m_pagerect.top;
			}
			if (m_bars[1].top > m_pagerect.bottom)
			{
				m_bars[1].top = m_pagerect.bottom;
				m_bars[1].bottom = m_pagerect.bottom;
			}
			mdPM->top_page_margin = MulDiv(m_bars[1].top - m_pagerect.top, maxresol, rectsize);
			break;

		case 2: // right page margin
			if (m_bars[2].left < m_pagerect.left)
			{
				m_bars[2].left = m_pagerect.left;
				m_bars[2].right = m_pagerect.left;
			}
			if (m_bars[2].left > m_pagerect.right)
			{
				m_bars[2].left = m_pagerect.right;
				m_bars[2].right = m_pagerect.right;
			}
			mdPM->right_page_margin = MulDiv((m_pagerect.right - m_bars[2].right), maxresol, rectsize);
			break;

		case 3: //bottom page margin
			if (m_bars[3].top < m_pagerect.top)
			{
				m_bars[3].top = m_pagerect.top;
				m_bars[3].bottom = m_pagerect.top;
			}
			if (m_bars[3].top > m_pagerect.bottom)
			{
				m_bars[3].top = m_pagerect.bottom;
				m_bars[3].bottom = m_pagerect.bottom;
			}
			mdPM->bottom_page_margin = MulDiv((m_pagerect.bottom - m_bars[3].top), maxresol, rectsize);
			break;

		case 4: // signal right
			if ((m_bars[4].right - m_bars[0].right) < 0)
			{
				m_bars[4].right = m_bars[0].left;
				m_bars[4].left = m_bars[0].left;
			}
			mdPM->width_doc = MulDiv((m_bars[4].right - m_bars[0].right), maxresol, rectsize);
			break;

		case 5: // signal band width
			if ((m_bars[5].top - m_bars[1].top) < 0)
			{
				m_bars[5].top = m_bars[1].top;
				m_bars[5].bottom = m_bars[1].top;
			}
			if ((m_bars[5].top - m_bars[3].top) > 0)
			{
				m_bars[5].top = m_bars[3].top;
				m_bars[5].bottom = m_bars[3].top;
			}
			mdPM->height_doc = MulDiv((m_bars[5].top - m_bars[1].top), maxresol, rectsize);
			break;

		case 6: // horizontal separator between signal and comment area
			if ((m_bars[6].left - m_bars[4].right) < 0)
			{
				m_bars[6].right = m_bars[4].right;
				m_bars[6].left = m_bars[4].right;
			}
			mdPM->text_separator = MulDiv((m_bars[6].left - m_bars[4].right), maxresol, rectsize);
			break;

		case 7: // separator beween consecutive bands
			if ((m_bars[7].top - m_bars[5].top) < 0)
			{
				m_bars[7].top = m_bars[5].top;
				m_bars[7].bottom = m_bars[5].top;
			}
			mdPM->height_separator = MulDiv((m_bars[7].top - m_bars[5].top), maxresol, rectsize);
			break;
		default:
			break;
		}
	}

	CDialog::OnLButtonUp(nFlags, point);
	Invalidate();
}

#define TRACKSIZE 3

int DlgPrintMargins::IsMouseOverAnyBar(CPoint* point)
{
	for (auto barindex = 0; barindex < 8; barindex++)
	{
		auto rect = m_bars[barindex];
		rect.InflateRect(TRACKSIZE, TRACKSIZE);
		if (rect.PtInRect(*point))
		{
			m_icapturedBar = barindex;
			return barindex;
		}
	}

	return -1;
}

void DlgPrintMargins::OnMouseMove(UINT nFlags, CPoint point)
{
	auto p_dc = GetDC();
	// check if point is not out of limits
	if (point.x < m_rect.left || point.x > m_rect.right
		|| point.y < m_rect.top || point.y > m_rect.bottom)
		return;

	// change mouse cursor if over one of the bars
	if (!m_bCaptured)
	{
		if (IsMouseOverAnyBar(&point) >= 0)
		{
			if ((m_icapturedBar % 2) > 0)
				SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL));
			else
				SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL));
		}
	}

	// change mouse cursor and replot if one bar is selected
	else
	{
		const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
		int cursor;
		// erase old bar
		DrawBar(&m_bars[m_icapturedBar], p_dc);
		// compute new bar position and display
		if ((m_icapturedBar % 2) > 0)
		{
			cursor = IDC_SPLITHORIZONTAL;
			m_bars[m_icapturedBar].top = point.y;
			m_bars[m_icapturedBar].bottom = point.y;
		}
		else
		{
			cursor = IDC_SPLITVERTICAL;
			m_bars[m_icapturedBar].left = point.x;
			m_bars[m_icapturedBar].right = point.x;
		}
		DrawBar(&m_bars[m_icapturedBar], p_dc);

		p_dc->SetROP2(nold_rop);
		SetCursor(AfxGetApp()->LoadCursor(cursor));
	}

	ReleaseDC(p_dc);
	CDialog::OnMouseMove(nFlags, point);
}
