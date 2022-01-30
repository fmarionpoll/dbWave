// FormatHistogram.cpp : implementation file
//

#include "StdAfx.h"

#include "resource.h"
#include "DlgFormatHistogram.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgFormatHistogram::DlgFormatHistogram(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgFormatHistogram::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Ymax);
	DDX_Text(pDX, IDC_EDIT2, m_xfirst);
	DDX_Text(pDX, IDC_EDIT3, m_xlast);
}

BEGIN_MESSAGE_MAP(DlgFormatHistogram, CDialog)
	ON_BN_CLICKED(IDC_CHECK1, OnCheckbYmaxAuto)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonHistFill)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, OnHistBordercolor)
	ON_BN_CLICKED(IDC_BUTTON6, OnStimulusFillcolor)
	ON_BN_CLICKED(IDC_BUTTON7, OnStimulusBordercolor)
	ON_BN_CLICKED(IDC_BUTTON3, OnBackgroundcolor)
END_MESSAGE_MAP()

void DlgFormatHistogram::OnCheckbYmaxAuto()
{
	m_bYmaxAuto = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	if (!m_bYmaxAuto)
	{
		GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_EDIT1)->EnableWindow(TRUE);
	}
}

void DlgFormatHistogram::OnPaint()
{
	CRect rect; // rect variable
	CPaintDC dc(this); // device context for painting
	auto p_old_brush = static_cast<CBrush*>(dc.SelectStockObject(WHITE_BRUSH));
	auto pOldPen = static_cast<CPen*>(dc.SelectStockObject(NULL_PEN));

	// erase background
	CWnd* pFWnd = GetDlgItem(IDC_STATIC10);
	pFWnd->GetWindowRect(&rect); // get window rectangle
	ScreenToClient(&rect); // convert  coordinates
	dc.Rectangle(&rect);

	// display abcissa
	int binlen = rect.Width() / 8;
	int baseline = rect.Height() / 8;
	CRect rect0 = rect;
	rect0.DeflateRect(binlen, baseline);
	dc.FillSolidRect(&rect0, m_crChartArea);

	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(rect.left + binlen / 2, rect.bottom - baseline);
	dc.LineTo(rect.right - binlen / 2, rect.bottom - baseline);

	// display stim
	CBrush sbHist;
	sbHist.CreateSolidBrush(m_crStimFill);
	CPen spHist;
	spHist.CreatePen(PS_SOLID, 0, m_crStimBorder);
	dc.SelectObject(&sbHist);
	dc.SelectObject(&spHist);

	CRect rstim(rect.left + (3 * binlen + binlen / 2), rect.top + baseline,
	            rect.left + (5 * binlen + binlen / 2), rect.bottom - baseline);
	dc.Rectangle(&rstim); // 1

	// display histogram
	CBrush BHist;
	BHist.CreateSolidBrush(m_crHistFill);
	CPen PHist;
	PHist.CreatePen(PS_SOLID, 0, m_crHistBorder);
	dc.SelectObject(&BHist);
	dc.SelectObject(&PHist);

	CRect rhist(rect.left + binlen, rect.bottom - 2 * baseline,
	            rect.left + 2 * binlen, rect.bottom - baseline);
	dc.Rectangle(&rhist); // 1
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 3 * baseline;
	dc.Rectangle(&rhist); // 2
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 6 * baseline;
	dc.Rectangle(&rhist); // 3
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 4 * baseline;
	dc.Rectangle(&rhist); // 4
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 5 * baseline;
	dc.Rectangle(&rhist); // 5
	rhist.OffsetRect(binlen, 0);
	rhist.top = rect.bottom - 2 * baseline;
	dc.Rectangle(&rhist); // 6
}

void DlgFormatHistogram::OnHistBordercolor()
{
	CColorDialog dlg(m_crStimBorder, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal() != IDOK)
	{
		m_crHistBorder = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::OnStimulusFillcolor()
{
	CColorDialog dlg(m_crStimFill, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_crStimFill = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::OnStimulusBordercolor()
{
	CColorDialog dlg(m_crStimBorder, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_crStimBorder = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::OnButtonHistFill()
{
	CColorDialog dlg(m_crHistFill, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_crHistFill = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::OnBackgroundcolor()
{
	CColorDialog dlg(m_crChartArea, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_crChartArea = dlg.GetColor();
		Invalidate();
	}
}
