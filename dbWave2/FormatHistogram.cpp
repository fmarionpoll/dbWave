// FormatHistogram.cpp : implementation file
//

#include "StdAfx.h"

#include "resource.h"
#include "FormatHistogram.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFormatHistogramDlg dialog


CFormatHistogramDlg::CFormatHistogramDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFormatHistogramDlg::IDD, pParent), m_bYmaxAuto(0), m_crHistFill(0), m_crHistBorder(0), m_crStimFill(0),
	  m_crStimBorder(0), m_crChartArea(0)
{
	m_Ymax = 0.0f;
	m_xfirst = 0.0f;
	m_xlast = 0.0f;
}


void CFormatHistogramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Ymax);
	DDX_Text(pDX, IDC_EDIT2, m_xfirst);
	DDX_Text(pDX, IDC_EDIT3, m_xlast);
}


BEGIN_MESSAGE_MAP(CFormatHistogramDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK1, OnCheckbYmaxAuto)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonHistFill)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, OnHistBordercolor)
	ON_BN_CLICKED(IDC_BUTTON6, OnStimulusFillcolor)
	ON_BN_CLICKED(IDC_BUTTON7, OnStimulusBordercolor)
	ON_BN_CLICKED(IDC_BUTTON3, OnBackgroundcolor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormatHistogramDlg message handlers

void CFormatHistogramDlg::OnCheckbYmaxAuto() 
{
	m_bYmaxAuto = ((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();
	if (!m_bYmaxAuto)
	{
		((CWnd*) GetDlgItem(IDC_EDIT1))->EnableWindow(FALSE);
	}
	else
	{
		((CWnd*) GetDlgItem(IDC_EDIT1))->EnableWindow(TRUE);
	}
}


void CFormatHistogramDlg::OnPaint() 
{
	CRect rect;							// rect variable
	CPaintDC dc(this); 					// device context for painting
	CBrush* pOldBrush = (CBrush*) dc.SelectStockObject(WHITE_BRUSH);
	CPen*   pOldPen = (CPen*) dc.SelectStockObject(NULL_PEN);

	// erase background
	CWnd* pFWnd = GetDlgItem(IDC_STATIC10);
	pFWnd->GetWindowRect(&rect);		// get window rectangle
	ScreenToClient(&rect);				// convert  coordinates
	dc.Rectangle(&rect);
	
	// display abcissa
	int binlen= rect.Width()/8;
	int baseline= rect.Height()/8;
	CRect rect0 = rect;
	rect0.DeflateRect(binlen, baseline);
	dc.FillSolidRect(&rect0, m_crChartArea);

	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(rect.left+binlen/2, rect.bottom - baseline);
	dc.LineTo(rect.right-binlen/2, rect.bottom - baseline);

	// display stim
	CBrush sbHist;
	sbHist.CreateSolidBrush(m_crStimFill);
	CPen spHist;
	spHist.CreatePen(PS_SOLID, 0, m_crStimBorder);
	dc.SelectObject(&sbHist);
	dc.SelectObject(&spHist);

	CRect rstim(rect.left+(3*binlen + binlen/2), rect.top + baseline, 
				rect.left+(5*binlen+ binlen/2), rect.bottom - baseline);
	dc.Rectangle(&rstim);		// 1	
	
	// display histogram
	CBrush BHist;
	BHist.CreateSolidBrush(m_crHistFill);
	CPen PHist;
	PHist.CreatePen(PS_SOLID, 0, m_crHistBorder);
	dc.SelectObject(&BHist);
	dc.SelectObject(&PHist);

	CRect rhist(rect.left+binlen, rect.bottom - 2*baseline, 
				rect.left+2*binlen, rect.bottom - baseline);
	dc.Rectangle(&rhist);		// 1
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 3*baseline;
	dc.Rectangle(&rhist);		// 2
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 6*baseline;
	dc.Rectangle(&rhist);		// 3
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 4*baseline;
	dc.Rectangle(&rhist);		// 4
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 5*baseline;
	dc.Rectangle(&rhist);		// 5
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 2*baseline;
	dc.Rectangle(&rhist);		// 6
}

void CFormatHistogramDlg::OnHistBordercolor() 
{
	CColorDialog dlg(m_crStimBorder, CC_RGBINIT, nullptr);
	if(IDOK == dlg.DoModal() != IDOK)
	{
		m_crHistBorder=dlg.GetColor();
		Invalidate();
	}
}

void CFormatHistogramDlg::OnStimulusFillcolor() 
{
	CColorDialog dlg(m_crStimFill, CC_RGBINIT, nullptr);
	if(IDOK == dlg.DoModal())
	{
		m_crStimFill=dlg.GetColor();
		Invalidate();
	}
}

void CFormatHistogramDlg::OnStimulusBordercolor() 
{
	CColorDialog dlg(m_crStimBorder, CC_RGBINIT, nullptr);
	if(IDOK == dlg.DoModal())
	{
		m_crStimBorder=dlg.GetColor();
		Invalidate();
	}
}

void CFormatHistogramDlg::OnButtonHistFill() 
{
	CColorDialog dlg(m_crHistFill, CC_RGBINIT, nullptr);
	if(IDOK == dlg.DoModal())
	{
		m_crHistFill=dlg.GetColor();
		Invalidate();
	}
}

void CFormatHistogramDlg::OnBackgroundcolor() 
{
	CColorDialog dlg(m_crChartArea, CC_RGBINIT, nullptr);
	if(IDOK == dlg.DoModal())
	{
		m_crChartArea=dlg.GetColor();
		Invalidate();
	}
}
