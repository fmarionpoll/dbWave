// printmar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "printmar.h"
#include "printpag.h"
#include "printdat.h"
#include "printdra.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrintMarginsDlg dialog


CPrintMarginsDlg::CPrintMarginsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintMarginsDlg::IDD, pParent)
{
 	m_bCaptured = FALSE;
}

void CPrintMarginsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPrintMarginsDlg, CDialog)
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


/////////////////////////////////////////////////////////////////////////////
// CPrintMarginsDlg message handlers

BOOL CPrintMarginsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetPageSize();
	UpdateData(FALSE);
	SketchPrinterPage();
	return TRUE;
}

void CPrintMarginsDlg::OnOK()
{
	mdPM->bChanged = TRUE;
	CDialog::OnOK();
}

void CPrintMarginsDlg::OnCommentsoptions()
{
	CPrintDataCommentsDlg dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void CPrintMarginsDlg::OnDrawarea()
{
	CPrintDrawAreaDlg  dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void CPrintMarginsDlg::OnDrawoptions()
{
	CPrintDataOptionsDlg dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void CPrintMarginsDlg::OnPagemargins()
{
	CPrintPageMarginsDlg dlg;
	dlg.mdPM = mdPM;
	dlg.DoModal();
	SketchPrinterPage();
}

void CPrintMarginsDlg::OnPrintersetup()
{
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// load browse parameters	
	pApp->FilePrintSetup();
	GetPageSize();
	SketchPrinterPage();
}

void CPrintMarginsDlg::SketchPrinterPage()
{	
	CClientDC dc(this);

	CBrush* pOldBrush = (CBrush*) dc.SelectStockObject(WHITE_BRUSH);
	CPen*   pOldPen = (CPen*) dc.SelectStockObject(BLACK_PEN);

	CWnd* pFWnd = GetDlgItem(IDC_RECT1);
	pFWnd->GetWindowRect(&m_rect);
	ScreenToClient(&m_rect);
	InvalidateRect(&m_rect, TRUE);	
}

void CPrintMarginsDlg::OnPaint()
{
	CPaintDC dc(this); 					// device context for painting

	CBrush* pOldBrush = (CBrush*) dc.SelectStockObject(WHITE_BRUSH);
	CPen*   pOldPen = (CPen*) dc.SelectStockObject(BLACK_PEN);

	COLORREF pagebackground = RGB(192,192,192);
	COLORREF commentarea = pagebackground; //RGB(  0, 128, 128);

	// erase background
	CWnd* pFWnd = GetDlgItem(IDC_RECT1);
	pFWnd->GetWindowRect(&m_rect);		// get window rectangle
	ScreenToClient(&m_rect);			// convert  coordinates
	dc.FillSolidRect(&m_rect, pagebackground);

	CPoint center;						// center of the drawing area
	center.x = (m_rect.right+m_rect.left)/2;
	center.y = (m_rect.bottom+m_rect.top)/2;
	int rectsize = min(m_rect.Width(), m_rect.Height());	// max size of the square
	int maxresol = max(mdPM->vertRes, mdPM->horzRes);		// max resolution

	// draw page area
	int diff = MulDiv(mdPM->horzRes, rectsize, maxresol)/2;
	m_pagerect.left = center.x - diff;
	m_pagerect.right = center.x + diff;
	diff = MulDiv(mdPM->vertRes, rectsize, maxresol)/2;
	m_pagerect.top = center.y - diff;
	m_pagerect.bottom = center.y + diff;
	dc.Rectangle(&m_pagerect);

	int i = MulDiv(mdPM->leftPageMargin, rectsize, maxresol);   // vertical lines
	m_bars[0] = CRect(m_pagerect.left+i, m_rect.top, m_pagerect.left+i, m_rect.bottom);

	i = MulDiv(mdPM->rightPageMargin, rectsize, maxresol);
	m_bars[2] = CRect(m_pagerect.right-i, m_rect.top, m_pagerect.right-i, m_rect.bottom);
	
	i = MulDiv(mdPM->topPageMargin, rectsize, maxresol);
	m_bars[1] = CRect(m_rect.left, m_pagerect.top+i, m_rect.right, m_pagerect.top+i);
	
	i = MulDiv(mdPM->bottomPageMargin, rectsize, maxresol);
	m_bars[3] = CRect(m_rect.left, m_pagerect.bottom-i, m_rect.right, m_pagerect.bottom-i);

	// draw drawing area
	if (mdPM->bFrameRect)
		dc.SelectStockObject(BLACK_PEN);
	else
		dc.SelectStockObject(WHITE_PEN);

	int rowmax = m_pagerect.bottom-i;
	CRect docrect;
	docrect.left = m_pagerect.left+MulDiv(mdPM->leftPageMargin, rectsize, maxresol);
	docrect.right = docrect.left + MulDiv(mdPM->WidthDoc, rectsize, maxresol);

	CRect commtrect;
	commtrect.left = docrect.right + MulDiv(mdPM->textseparator, rectsize, maxresol);
	commtrect.right = m_pagerect.right-MulDiv(mdPM->rightPageMargin, rectsize, maxresol);
	if (commtrect.right < commtrect.left)
		commtrect.right = commtrect.left + 10;

	// draw comments
	int rowheight = MulDiv(mdPM->HeightDoc, rectsize, maxresol);
	int rowsep = MulDiv(mdPM->heightSeparator, rectsize, maxresol);
	int toprow = m_pagerect.top + MulDiv(mdPM->topPageMargin, rectsize, maxresol);
	int bottomrow = toprow + rowheight;

	m_bars[4] = CRect(docrect.right, m_rect.top, docrect.right, m_rect.bottom);
	m_bars[6] = CRect(commtrect.left, m_rect.top, commtrect.left, m_rect.bottom);
	m_bars[5] = CRect(m_rect.left, bottomrow, m_rect.right, bottomrow);
	m_bars[7] = CRect(m_rect.left, bottomrow+ rowsep, m_rect.right, bottomrow+ rowsep);
 
	while(bottomrow < rowmax)
	{
		docrect.top = toprow;
		docrect.bottom = bottomrow;
		dc.FillSolidRect(&docrect, commentarea);

		commtrect.top = docrect.top;
		commtrect.bottom = docrect.bottom;
		dc.FillSolidRect(&commtrect, commentarea);
		toprow = docrect.bottom + rowsep;
		bottomrow = toprow + rowheight;
	}    

	// draw bars (resizable margins)
	for (i=0; i< 8; i++)
		DrawBar (&m_bars[i], &dc); 

	dc.SelectObject(pOldPen);
}

void CPrintMarginsDlg::DrawBar(CRect* bar, CDC* pdc)
{
	CPen penbars(PS_DOT, 1, RGB(0,0,255));
	CPen*   pOldPen = (CPen*) pdc->SelectObject(&penbars);

	pdc->MoveTo(bar->left, bar->top);
	pdc->LineTo(bar->right, bar->bottom);

	pdc->SelectObject(pOldPen);
}

void CPrintMarginsDlg::GetPageSize()
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
	HDC hDC= dlg.CreatePrinterDC();     // to delete at the end -- see doc!	
	ASSERT(hDC != NULL);
	dc.Attach(hDC);

	// Get the size of the page in pixels
	mdPM->horzRes=dc.GetDeviceCaps(HORZRES);
	mdPM->vertRes=dc.GetDeviceCaps(VERTRES);

	CString csResolut;
	csResolut.Format(_T("%i"), mdPM->vertRes);
	SetDlgItemText(IDC_PAGEHEIGHT, csResolut);
	csResolut.Format(_T("%i"), mdPM->horzRes);
	SetDlgItemText(IDC_PAGEWIDTH, csResolut);	
}

void CPrintMarginsDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CDC* pDC = GetDC();	

	if (IsMouseOverAnyBar(&point) < 0)
	{
		ReleaseDC(pDC);
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	SetCapture();
	m_bCaptured = TRUE;

	CDialog::OnLButtonDown(nFlags, point);
}

void CPrintMarginsDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = FALSE;

		int rectsize = min(m_rect.Width(), m_rect.Height());	// max size of the square
		int maxresol = max(mdPM->vertRes, mdPM->horzRes);		// max resolution

		switch (m_icapturedBar)
		{
		case 0:	// left page margin
			if (m_bars[0].left < m_pagerect.left) 
			{
				m_bars[0].left= m_pagerect.left; 
				m_bars[0].right= m_pagerect.left;
			}
			if (m_bars[0].left >  m_pagerect.right)
			{ 
				m_bars[0].left= m_pagerect.right; 
				m_bars[0].right= m_pagerect.right;
			}
			mdPM->leftPageMargin = MulDiv(m_bars[0].right - m_pagerect.left, maxresol, rectsize);
			break;

		case 1:	// top page margin
			if (m_bars[1].top < m_pagerect.top) 
			{ 
				m_bars[1].top= m_pagerect.top; 
				m_bars[1].bottom= m_pagerect.top;
			}
			if (m_bars[1].top >  m_pagerect.bottom)
			{ 
				m_bars[1].top= m_pagerect.bottom; 
				m_bars[1].bottom = m_pagerect.bottom;
			}
			mdPM->topPageMargin = MulDiv(m_bars[1].top - m_pagerect.top, maxresol, rectsize);
			break;

		case 2:	// right page margin
			if (m_bars[2].left < m_pagerect.left) 
			{ 
				m_bars[2].left= m_pagerect.left; 
				m_bars[2].right= m_pagerect.left;
			}
			if (m_bars[2].left >  m_pagerect.right)
			{ 
				m_bars[2].left= m_pagerect.right; 
				m_bars[2].right= m_pagerect.right;
			}
			mdPM->rightPageMargin = MulDiv((m_pagerect.right - m_bars[2].right), maxresol, rectsize);
			break;

		case 3: //bottom page margin
			if (m_bars[3].top < m_pagerect.top) 
			{ 
				m_bars[3].top= m_pagerect.top; 
				m_bars[3].bottom= m_pagerect.top;
			}
			if (m_bars[3].top >  m_pagerect.bottom)
			{ 
				m_bars[3].top= m_pagerect.bottom; 
				m_bars[3].bottom = m_pagerect.bottom;
			}
			mdPM->bottomPageMargin = MulDiv((m_pagerect.bottom - m_bars[3].top), maxresol, rectsize);
			break;

		case 4:	// signal right
			if ((m_bars[4].right - m_bars[0].right) < 0)
			{
				m_bars[4].right = m_bars[0].left; 
				m_bars[4].left = m_bars[0].left;
			}
			mdPM->WidthDoc = MulDiv( (m_bars[4].right - m_bars[0].right), maxresol, rectsize);
			break;

		case 5:	// signal band width
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
			mdPM->HeightDoc = MulDiv((m_bars[5].top - m_bars[1].top), maxresol, rectsize);			    
			break;

		case 6:	// horizontal separator between signal and comment area
			if ((m_bars[6].left - m_bars[4].right) < 0)
			{
				m_bars[6].right = m_bars[4].right;
				m_bars[6].left = m_bars[4].right;
			}
			mdPM->textseparator = MulDiv((m_bars[6].left - m_bars[4].right), maxresol, rectsize);
			break;

		case 7: // separator beween consecutive bands
			if ((m_bars[7].top - m_bars[5].top) < 0)
			{
				m_bars[7].top = m_bars[5].top; 
				m_bars[7].bottom = m_bars[5].top;
			}
			mdPM->heightSeparator = MulDiv((m_bars[7].top - m_bars[5].top),  maxresol, rectsize);
			break;
		default:
			break;
		}
	}
	
	CDialog::OnLButtonUp(nFlags, point);
	Invalidate();
}

#define TRACKSIZE 3

int CPrintMarginsDlg::IsMouseOverAnyBar(CPoint* point)
{
	CRect rect;
	for (int barindex = 0; barindex < 8; barindex++)
	{
		rect = m_bars[barindex];
		rect.InflateRect (TRACKSIZE, TRACKSIZE);
		if (rect.PtInRect(*point))
		{
			m_icapturedBar = barindex;
			return barindex;
		}
	}

	return -1;
}

void CPrintMarginsDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDC* pDC = GetDC();
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
		// select display mode 
		int noldROP = pDC->SetROP2(R2_NOTXORPEN); 
		int cursor;
		// erase old bar
		DrawBar(&m_bars[m_icapturedBar], pDC);

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
		DrawBar(&m_bars[m_icapturedBar], pDC);

		// restore old display mode
		pDC->SetROP2(noldROP);

		// set mouse cursor
		SetCursor(AfxGetApp()->LoadCursor(cursor));
	}

	ReleaseDC(pDC);
	CDialog::OnMouseMove(nFlags, point);
}

