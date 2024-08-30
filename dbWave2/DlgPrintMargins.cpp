#include "StdAfx.h"
#include "resource.h"
#include "DlgPrintMargins.h"

#include "ColorNames.h"
#include "dbWave.h"
#include "DlgPrintDataComments.h"
#include "DlgPrintDataOptions.h"
#include "DlgPrintDrawArea.h"
#include "DlgPrintPageMargins.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgPrintMargins::DlgPrintMargins(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgPrintMargins::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
}

BEGIN_MESSAGE_MAP(DlgPrintMargins, CDialog)
	ON_BN_CLICKED(IDC_COMMENTSOPTIONS, on_comments_options)
	ON_BN_CLICKED(IDC_DRAWAREA, on_draw_area)
	ON_BN_CLICKED(IDC_DRAWOPTIONS, on_draw_options)
	ON_BN_CLICKED(IDC_PAGEMARGINS, on_page_margins)
	ON_BN_CLICKED(IDC_PRINTERSETUP, on_printer_setup)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL DlgPrintMargins::OnInitDialog()
{
	CDialog::OnInitDialog();
	get_page_size();
	UpdateData(FALSE);
	sketch_printer_page();
	return TRUE;
}

void DlgPrintMargins::OnOK()
{
	options_view_data->b_changed = TRUE;
	CDialog::OnOK();
}

void DlgPrintMargins::on_comments_options()
{
	DlgPrintDataComments dlg;
	dlg.options_view_data = options_view_data;
	dlg.DoModal();
	sketch_printer_page();
}

void DlgPrintMargins::on_draw_area()
{
	DlgPrintDrawArea dlg;
	dlg.options_view_data = options_view_data;
	dlg.DoModal();
	sketch_printer_page();
}

void DlgPrintMargins::on_draw_options()
{
	DlgPrintDataOptions dlg;
	dlg.options_view_data = options_view_data;
	dlg.DoModal();
	sketch_printer_page();
}

void DlgPrintMargins::on_page_margins()
{
	DlgPrintPageMargins dlg;
	dlg.options_view_data = options_view_data;
	dlg.DoModal();
	sketch_printer_page();
}

void DlgPrintMargins::on_printer_setup()
{
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // load browse parameters
	p_app->FilePrintSetup();
	get_page_size();
	sketch_printer_page();
}

void DlgPrintMargins::sketch_printer_page()
{
	CClientDC dc(this);

	/*auto p_old_brush = (CBrush*) */
	dc.SelectStockObject(WHITE_BRUSH);
	/*auto pOldPen = (CPen*) */
	dc.SelectStockObject(BLACK_PEN);

	const auto p_f_wnd = GetDlgItem(IDC_RECT1);
	p_f_wnd->GetWindowRect(&m_rect_);
	ScreenToClient(&m_rect_);
	InvalidateRect(&m_rect_, TRUE);
}

void DlgPrintMargins::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	/*CBrush* p_old_brush = (CBrush*) */
	dc.SelectStockObject(WHITE_BRUSH);
	const auto p_old_pen = static_cast<CPen*>(dc.SelectStockObject(BLACK_PEN));

	constexpr auto page_background = col_silver;
	constexpr auto comment_area = page_background; //RGB(  0, 128, 128);

	// erase background
	const auto p_f_wnd = GetDlgItem(IDC_RECT1);
	p_f_wnd->GetWindowRect(&m_rect_); // get window rectangle
	ScreenToClient(&m_rect_); // convert  coordinates
	dc.FillSolidRect(&m_rect_, page_background);

	CPoint center; // center of the drawing area
	center.x = (m_rect_.right + m_rect_.left) / 2;
	center.y = (m_rect_.bottom + m_rect_.top) / 2;
	const auto rect_size = min(m_rect_.Width(), m_rect_.Height()); // max size of the square
	const auto max_resolution = max(options_view_data->vertical_resolution, options_view_data->horizontal_resolution); // max resolution

	// draw page area
	auto diff = MulDiv(options_view_data->horizontal_resolution, rect_size, max_resolution) / 2;
	m_page_rect_.left = center.x - diff;
	m_page_rect_.right = center.x + diff;
	diff = MulDiv(options_view_data->vertical_resolution, rect_size, max_resolution) / 2;
	m_page_rect_.top = center.y - diff;
	m_page_rect_.bottom = center.y + diff;
	dc.Rectangle(&m_page_rect_);

	auto i = MulDiv(options_view_data->left_page_margin, rect_size, max_resolution); // vertical lines
	m_bars_[0] = CRect(m_page_rect_.left + i, m_rect_.top, m_page_rect_.left + i, m_rect_.bottom);

	i = MulDiv(options_view_data->right_page_margin, rect_size, max_resolution);
	m_bars_[2] = CRect(m_page_rect_.right - i, m_rect_.top, m_page_rect_.right - i, m_rect_.bottom);

	i = MulDiv(options_view_data->top_page_margin, rect_size, max_resolution);
	m_bars_[1] = CRect(m_rect_.left, m_page_rect_.top + i, m_rect_.right, m_page_rect_.top + i);

	i = MulDiv(options_view_data->bottom_page_margin, rect_size, max_resolution);
	m_bars_[3] = CRect(m_rect_.left, m_page_rect_.bottom - i, m_rect_.right, m_page_rect_.bottom - i);

	// draw drawing area
	if (options_view_data->b_frame_rect)
		dc.SelectStockObject(BLACK_PEN);
	else
		dc.SelectStockObject(WHITE_PEN);

	const int row_max = m_page_rect_.bottom - i;
	CRect doc_rect;
	doc_rect.left = m_page_rect_.left + MulDiv(options_view_data->left_page_margin, rect_size, max_resolution);
	doc_rect.right = doc_rect.left + MulDiv(options_view_data->width_doc, rect_size, max_resolution);

	CRect comment_rect;
	comment_rect.left = doc_rect.right + MulDiv(options_view_data->text_separator, rect_size, max_resolution);
	comment_rect.right = m_page_rect_.right - MulDiv(options_view_data->right_page_margin, rect_size, max_resolution);
	if (comment_rect.right < comment_rect.left)
		comment_rect.right = comment_rect.left + 10;

	// draw comments
	const auto row_height = MulDiv(options_view_data->height_doc, rect_size, max_resolution);
	const auto row_sep = MulDiv(options_view_data->height_separator, rect_size, max_resolution);
	int top_row = m_page_rect_.top + MulDiv(options_view_data->top_page_margin, rect_size, max_resolution);
	auto bottom_row = top_row + row_height;

	m_bars_[4] = CRect(doc_rect.right, m_rect_.top, doc_rect.right, m_rect_.bottom);
	m_bars_[6] = CRect(comment_rect.left, m_rect_.top, comment_rect.left, m_rect_.bottom);
	m_bars_[5] = CRect(m_rect_.left, bottom_row, m_rect_.right, bottom_row);
	m_bars_[7] = CRect(m_rect_.left, bottom_row + row_sep, m_rect_.right, bottom_row + row_sep);

	while (bottom_row < row_max)
	{
		doc_rect.top = top_row;
		doc_rect.bottom = bottom_row;
		dc.FillSolidRect(&doc_rect, comment_area);

		comment_rect.top = doc_rect.top;
		comment_rect.bottom = doc_rect.bottom;
		dc.FillSolidRect(&comment_rect, comment_area);
		top_row = doc_rect.bottom + row_sep;
		bottom_row = top_row + row_height;
	}

	// draw bars (resizable margins)
	for (auto& m_bar : m_bars_)
		draw_bar(&m_bar, &dc);

	dc.SelectObject(p_old_pen);
}

void DlgPrintMargins::draw_bar(const CRect* bar, CDC* pdc)
{
	CPen pen_bars(PS_DOT, 1, col_blue);
	const auto p_old_pen = pdc->SelectObject(&pen_bars);

	pdc->MoveTo(bar->left, bar->top);
	pdc->LineTo(bar->right, bar->bottom);

	pdc->SelectObject(p_old_pen);
}

void DlgPrintMargins::get_page_size()
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
	options_view_data->horizontal_resolution = dc.GetDeviceCaps(HORZRES);
	options_view_data->vertical_resolution = dc.GetDeviceCaps(VERTRES);

	CString cs_resolution;
	cs_resolution.Format(_T("%i"), options_view_data->vertical_resolution);
	SetDlgItemText(IDC_PAGEHEIGHT, cs_resolution);
	cs_resolution.Format(_T("%i"), options_view_data->horizontal_resolution);
	SetDlgItemText(IDC_PAGEWIDTH, cs_resolution);
}

void DlgPrintMargins::OnLButtonDown(const UINT n_flags, CPoint point)
{
	CRect rc_client;
	GetClientRect(&rc_client);
	const auto p_dc = GetDC();

	if (is_mouse_over_any_bar(&point) < 0)
	{
		ReleaseDC(p_dc);
		CDialog::OnLButtonDown(n_flags, point);
		return;
	}

	SetCapture();
	m_b_captured_ = TRUE;
	CDialog::OnLButtonDown(n_flags, point);
}

void DlgPrintMargins::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (m_b_captured_)
	{
		ReleaseCapture();
		m_b_captured_ = FALSE;

		const auto rect_size = min(m_rect_.Width(), m_rect_.Height()); // max size of the square
		const auto max_resolution = max(options_view_data->vertical_resolution, options_view_data->horizontal_resolution); // max resolution

		switch (m_i_captured_bar_)
		{
		case 0: // left page margin
			if (m_bars_[0].left < m_page_rect_.left)
			{
				m_bars_[0].left = m_page_rect_.left;
				m_bars_[0].right = m_page_rect_.left;
			}
			if (m_bars_[0].left > m_page_rect_.right)
			{
				m_bars_[0].left = m_page_rect_.right;
				m_bars_[0].right = m_page_rect_.right;
			}
			options_view_data->left_page_margin = MulDiv(m_bars_[0].right - m_page_rect_.left, max_resolution, rect_size);
			break;

		case 1: // top page margin
			if (m_bars_[1].top < m_page_rect_.top)
			{
				m_bars_[1].top = m_page_rect_.top;
				m_bars_[1].bottom = m_page_rect_.top;
			}
			if (m_bars_[1].top > m_page_rect_.bottom)
			{
				m_bars_[1].top = m_page_rect_.bottom;
				m_bars_[1].bottom = m_page_rect_.bottom;
			}
			options_view_data->top_page_margin = MulDiv(m_bars_[1].top - m_page_rect_.top, max_resolution, rect_size);
			break;

		case 2: // right page margin
			if (m_bars_[2].left < m_page_rect_.left)
			{
				m_bars_[2].left = m_page_rect_.left;
				m_bars_[2].right = m_page_rect_.left;
			}
			if (m_bars_[2].left > m_page_rect_.right)
			{
				m_bars_[2].left = m_page_rect_.right;
				m_bars_[2].right = m_page_rect_.right;
			}
			options_view_data->right_page_margin = MulDiv((m_page_rect_.right - m_bars_[2].right), max_resolution, rect_size);
			break;

		case 3: //bottom page margin
			if (m_bars_[3].top < m_page_rect_.top)
			{
				m_bars_[3].top = m_page_rect_.top;
				m_bars_[3].bottom = m_page_rect_.top;
			}
			if (m_bars_[3].top > m_page_rect_.bottom)
			{
				m_bars_[3].top = m_page_rect_.bottom;
				m_bars_[3].bottom = m_page_rect_.bottom;
			}
			options_view_data->bottom_page_margin = MulDiv((m_page_rect_.bottom - m_bars_[3].top), max_resolution, rect_size);
			break;

		case 4: // signal right
			if ((m_bars_[4].right - m_bars_[0].right) < 0)
			{
				m_bars_[4].right = m_bars_[0].left;
				m_bars_[4].left = m_bars_[0].left;
			}
			options_view_data->width_doc = MulDiv((m_bars_[4].right - m_bars_[0].right), max_resolution, rect_size);
			break;

		case 5: // signal bandwidth
			if ((m_bars_[5].top - m_bars_[1].top) < 0)
			{
				m_bars_[5].top = m_bars_[1].top;
				m_bars_[5].bottom = m_bars_[1].top;
			}
			if ((m_bars_[5].top - m_bars_[3].top) > 0)
			{
				m_bars_[5].top = m_bars_[3].top;
				m_bars_[5].bottom = m_bars_[3].top;
			}
			options_view_data->height_doc = MulDiv((m_bars_[5].top - m_bars_[1].top), max_resolution, rect_size);
			break;

		case 6: // horizontal separator between signal and comment area
			if ((m_bars_[6].left - m_bars_[4].right) < 0)
			{
				m_bars_[6].right = m_bars_[4].right;
				m_bars_[6].left = m_bars_[4].right;
			}
			options_view_data->text_separator = MulDiv((m_bars_[6].left - m_bars_[4].right), max_resolution, rect_size);
			break;

		case 7: // separator between consecutive bands
			if ((m_bars_[7].top - m_bars_[5].top) < 0)
			{
				m_bars_[7].top = m_bars_[5].top;
				m_bars_[7].bottom = m_bars_[5].top;
			}
			options_view_data->height_separator = MulDiv((m_bars_[7].top - m_bars_[5].top), max_resolution, rect_size);
			break;
		default:
			break;
		}
	}

	CDialog::OnLButtonUp(n_flags, point);
	Invalidate();
}

#define TRACKSIZE 3

int DlgPrintMargins::is_mouse_over_any_bar(const CPoint* point)
{
	for (auto bar_index = 0; bar_index < 8; bar_index++)
	{
		auto rect = m_bars_[bar_index];
		rect.InflateRect(TRACKSIZE, TRACKSIZE);
		if (rect.PtInRect(*point))
		{
			m_i_captured_bar_ = bar_index;
			return bar_index;
		}
	}

	return -1;
}

void DlgPrintMargins::OnMouseMove(const UINT n_flags, const CPoint point)
{
	auto p_dc = GetDC();
	// check if point is not out of limits
	if (point.x < m_rect_.left || point.x > m_rect_.right
		|| point.y < m_rect_.top || point.y > m_rect_.bottom)
		return;

	// change mouse cursor if over one of the bars
	if (!m_b_captured_)
	{
		if (is_mouse_over_any_bar(&point) >= 0)
		{
			if ((m_i_captured_bar_ % 2) > 0)
				SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL));
			else
				SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL));
		}
	}

	// change mouse cursor and re-plot if one bar is selected
	else
	{
		const auto n_old_rop = p_dc->SetROP2(R2_NOTXORPEN);
		int cursor;
		// erase old bar
		draw_bar(&m_bars_[m_i_captured_bar_], p_dc);
		// compute new bar position and display
		if ((m_i_captured_bar_ % 2) > 0)
		{
			cursor = IDC_SPLITHORIZONTAL;
			m_bars_[m_i_captured_bar_].top = point.y;
			m_bars_[m_i_captured_bar_].bottom = point.y;
		}
		else
		{
			cursor = IDC_SPLITVERTICAL;
			m_bars_[m_i_captured_bar_].left = point.x;
			m_bars_[m_i_captured_bar_].right = point.x;
		}
		draw_bar(&m_bars_[m_i_captured_bar_], p_dc);

		p_dc->SetROP2(n_old_rop);
		SetCursor(AfxGetApp()->LoadCursor(cursor));
	}

	ReleaseDC(p_dc);
	CDialog::OnMouseMove(n_flags, point);
}
