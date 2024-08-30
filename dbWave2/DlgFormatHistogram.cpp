// FormatHistogram.cpp : implementation file
//

#include "StdAfx.h"

#include "resource.h"
#include "DlgFormatHistogram.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgFormatHistogram::DlgFormatHistogram(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgFormatHistogram::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT1, m_y_max);
	DDX_Text(p_dx, IDC_EDIT2, m_x_first);
	DDX_Text(p_dx, IDC_EDIT3, m_x_last);
}

BEGIN_MESSAGE_MAP(DlgFormatHistogram, CDialog)
	ON_BN_CLICKED(IDC_CHECK1, on_check_max_auto)
	ON_BN_CLICKED(IDC_BUTTON1, on_button_hist_fill)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, on_hist_border_color)
	ON_BN_CLICKED(IDC_BUTTON6, on_stimulus_fill_color)
	ON_BN_CLICKED(IDC_BUTTON7, on_stimulus_border_color)
	ON_BN_CLICKED(IDC_BUTTON3, on_background_color)
END_MESSAGE_MAP()

void DlgFormatHistogram::on_check_max_auto()
{
	m_b_y_max_auto = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	if (!m_b_y_max_auto)
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
	CRect rect; 
	CPaintDC dc(this); 
	static_cast<CBrush*>(dc.SelectStockObject(WHITE_BRUSH));
	static_cast<CPen*>(dc.SelectStockObject(NULL_PEN));

	// erase background
	CWnd* pFWnd = GetDlgItem(IDC_STATIC10);
	pFWnd->GetWindowRect(&rect); // get window rectangle
	ScreenToClient(&rect); // convert  coordinates
	dc.Rectangle(&rect);

	// display abscissa
	const int bin_len = rect.Width() / 8;
	const int baseline = rect.Height() / 8;
	CRect rect0 = rect;
	rect0.DeflateRect(bin_len, baseline);
	dc.FillSolidRect(&rect0, m_cr_chart_area);

	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(rect.left + bin_len / 2, rect.bottom - baseline);
	dc.LineTo(rect.right - bin_len / 2, rect.bottom - baseline);

	// display stimulus
	CBrush sb_hist;
	sb_hist.CreateSolidBrush(m_cr_stimulus_fill);
	CPen spHist;
	spHist.CreatePen(PS_SOLID, 0, m_cr_stimulus_border);
	dc.SelectObject(&sb_hist);
	dc.SelectObject(&spHist);

	const CRect r_stimulus(rect.left + (3 * bin_len + bin_len / 2), rect.top + baseline,
	                  rect.left + (5 * bin_len + bin_len / 2), rect.bottom - baseline);
	dc.Rectangle(&r_stimulus); // 1

	// display histogram
	CBrush b_hist;
	b_hist.CreateSolidBrush(m_cr_hist_fill);
	CPen p_hist;
	p_hist.CreatePen(PS_SOLID, 0, m_cr_hist_border);
	dc.SelectObject(&b_hist);
	dc.SelectObject(&p_hist);

	CRect r_histogram(rect.left + bin_len, rect.bottom - 2 * baseline,
	            rect.left + 2 * bin_len, rect.bottom - baseline);
	dc.Rectangle(&r_histogram); // 1
	r_histogram.OffsetRect(bin_len, 0);
	r_histogram.top = rect.bottom - 3 * baseline;
	dc.Rectangle(&r_histogram); // 2
	r_histogram.OffsetRect(bin_len, 0);
	r_histogram.top = rect.bottom - 6 * baseline;
	dc.Rectangle(&r_histogram); // 3
	r_histogram.OffsetRect(bin_len, 0);
	r_histogram.top = rect.bottom - 4 * baseline;
	dc.Rectangle(&r_histogram); // 4
	r_histogram.OffsetRect(bin_len, 0);
	r_histogram.top = rect.bottom - 5 * baseline;
	dc.Rectangle(&r_histogram); // 5
	r_histogram.OffsetRect(bin_len, 0);
	r_histogram.top = rect.bottom - 2 * baseline;
	dc.Rectangle(&r_histogram); // 6
}

void DlgFormatHistogram::on_hist_border_color()
{
	CColorDialog dlg(m_cr_stimulus_border, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal() != IDOK)
	{
		m_cr_hist_border = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::on_stimulus_fill_color()
{
	CColorDialog dlg(m_cr_stimulus_fill, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_cr_stimulus_fill = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::on_stimulus_border_color()
{
	CColorDialog dlg(m_cr_stimulus_border, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_cr_stimulus_border = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::on_button_hist_fill()
{
	CColorDialog dlg(m_cr_hist_fill, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_cr_hist_fill = dlg.GetColor();
		Invalidate();
	}
}

void DlgFormatHistogram::on_background_color()
{
	CColorDialog dlg(m_cr_chart_area, CC_RGBINIT, nullptr);
	if (IDOK == dlg.DoModal())
	{
		m_cr_chart_area = dlg.GetColor();
		Invalidate();
	}
}
