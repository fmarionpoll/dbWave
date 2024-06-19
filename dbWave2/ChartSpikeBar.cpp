#include "StdAfx.h"
#include "ChartSpikeBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_ballfiles is true: spike hit

IMPLEMENT_SERIAL(ChartSpikeBar, ChartSpike, 1)

BEGIN_MESSAGE_MAP(ChartSpikeBar, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeBar::ChartSpikeBar()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (spikebar)");
}

ChartSpikeBar::~ChartSpikeBar()
{
	if (p_envelope_ != nullptr)
	{
		p_envelope_->RemoveAll();
		delete p_envelope_;
	}
}

void ChartSpikeBar::PlotDataToDC(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// save context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->IntersectClipRect(&m_clientRect);

	long n_files = 1;
	long current_file = 0;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->db_get_n_records();
		current_file = p_dbwave_doc->db_get_current_record_position();
	}

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (m_display_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(i_file))
				p_spike_doc = p_dbwave_doc->open_current_spike_file();
		}
		p_spike_list = p_spike_doc->get_spike_list_current();

		// test presence of data
		if (p_spike_list == nullptr || p_spike_list->get_spikes_count() <= 0)
		{
			if (!m_display_all_files)
				p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT);
			continue;
		}

		// plot comment at the bottom
		if (m_bBottomComment)
			p_dc->DrawText(m_csBottomComment, m_csBottomComment.GetLength(), rect,DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

		// display data: trap error conditions
		if (m_yWE == 1)
		{
			short value_max, value_min;
			p_spike_list->get_total_max_min(TRUE, &value_max, &value_min);
			m_yWE = value_max - value_min;
			m_yWO = (value_max + value_min) / 2;
		}

		if (m_xWE == 1) // this is generally the case: && m_xWO == 0)
		{
			m_xWE = m_displayRect.right;
			m_xWO = m_displayRect.left;
		}

		display_bars(p_dc, &m_displayRect);

		const CIntervals* p_intervals = &(p_spike_doc->m_stimulus_intervals);

		if (p_intervals->n_items > 0)
			display_stimulus(p_dc, &m_displayRect);

		// display vertical cursors
		if (m_VTtags.get_tag_list_size() > 0)
		{
			// select pen and display mode
			const auto old_pen = p_dc->SelectObject(&m_blackDottedPen);
			const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

			// iterate through VT cursor list
			const int y1 = m_displayRect.bottom;
			for (auto j = m_VTtags.get_tag_list_size() - 1; j >= 0; j--)
			{
				constexpr auto y0 = 0;
				const auto lk = m_VTtags.get_tag_l_val(j); // get value
				if (lk < m_lFirst || lk > m_lLast)
					continue;
				const auto k = MulDiv(lk - m_lFirst, m_displayRect.Width() , m_lLast - m_lFirst + 1);
				p_dc->MoveTo(k, y0); // set initial pt
				p_dc->LineTo(k, y1); // VT line
			}
			p_dc->SetROP2(old_rop2); // restore old display mode
			p_dc->SelectObject(old_pen);
		}

		// temporary tag
		if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
		{
			const auto old_object = p_dc->SelectObject(&m_blackDottedPen);
			const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);
			p_dc->MoveTo(m_tempVTtag->m_pixel, m_displayRect.top + 2);
			p_dc->LineTo(m_tempVTtag->m_pixel, m_displayRect.bottom - 2);
			p_dc->SetROP2(old_ROP2);
			p_dc->SelectObject(old_object);
		}
	}

	p_dc->RestoreDC(n_saved_dc);

	if (m_display_all_files)
	{
		if (p_dbwave_doc->db_set_current_record_position(current_file))
			p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_p_spk->get_spike_list_current();
	}
}

void ChartSpikeBar::plot_single_spk_data_to_dc(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// save context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->IntersectClipRect(&m_clientRect);

	// test presence of data
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
	{
		p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT);
		p_dc->RestoreDC(n_saved_dc);
		return;
	}

	// plot comment at the bottom
	if (m_bBottomComment)
	{
		p_dc->DrawText(m_csBottomComment, m_csBottomComment.GetLength(), rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
	}

	// display data: trap error conditions
	if (m_yWE == 1)
	{
		short value_max, value_min;
		p_spike_list->get_total_max_min(TRUE, &value_max, &value_min);
		m_yWE = value_max - value_min;
		m_yWO = (value_max + value_min) / 2;
	}

	if (m_xWE == 1) // this is generally the case: && m_xWO == 0)
	{
		m_xWE = m_displayRect.right;
		m_xWO = m_displayRect.left;
	}

	display_bars(p_dc, &m_displayRect);

	if (p_spike_doc == nullptr)
		p_spike_doc = p_dbwave_doc->m_p_spk;
	const CIntervals* p_intervals = &(p_spike_doc->m_stimulus_intervals);

	if (p_intervals->n_items > 0)
		display_stimulus(p_dc, &m_displayRect);

	// display vertical cursors
	if (m_VTtags.get_tag_list_size() > 0)
	{
		// select pen and display mode
		const auto old_pen = p_dc->SelectObject(&m_blackDottedPen);
		const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);

		// iterate through VT cursor list
		const int y1 = m_displayRect.bottom;
		for (auto j = m_VTtags.get_tag_list_size() - 1; j >= 0; j--)
		{
			constexpr auto y0 = 0;
			const auto lk = m_VTtags.get_tag_l_val(j);
			if (lk < m_lFirst || lk > m_lLast)
				continue;
			const auto k = MulDiv(lk - m_lFirst, m_displayRect.Width(), m_lLast -m_lFirst + 1);
			p_dc->MoveTo(k, y0);
			p_dc->LineTo(k, y1);
		}
		p_dc->SetROP2(old_ROP2);
		p_dc->SelectObject(old_pen);
	}

	// temp tag
	if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
	{
		const auto old_pen = p_dc->SelectObject(&m_blackDottedPen);
		const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(m_tempVTtag->m_pixel, m_displayRect.top + 2);
		p_dc->LineTo(m_tempVTtag->m_pixel, m_displayRect.bottom - 2);
		p_dc->SetROP2(old_ROP2);
		p_dc->SelectObject(old_pen);
	}

	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeBar::display_stimulus(CDC* p_dc, const CRect* rect) const
{
	CPen blue_pen;
	blue_pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
	const auto old_pen = p_dc->SelectObject(&blue_pen);
	const int top = rect->bottom - m_bar_height + 2;
	const int bottom = rect->bottom - 3;
	const auto display_width = rect->Width();

	// search first stimulus transition within interval
	const auto ii_first = m_lFirst;
	const auto ii_last = m_lLast;
	const auto ii_length = ii_last - ii_first;
	auto i0 = 0;
	CIntervals* p_intervals = &(p_spike_doc->m_stimulus_intervals);

	while (i0 < p_intervals->GetSize()
		&& p_intervals->GetAt(i0) < ii_first)
		i0++; 

	auto state = bottom;				// use this variable to keep track of pulse broken by display limits
	const auto jj = (i0 / 2) * 2;	// keep index of the ON transition
	if (jj != i0)
		state = top;
	p_dc->MoveTo(rect->left, state);

	const auto n_stimuli = ((p_intervals->GetSize()) / 2) * 2;
	for (auto ii = jj; ii < n_stimuli; ii += 2)
	{
		// stimulus starts here
		int iix0 = p_intervals->GetAt(ii) - ii_first;
		if (iix0 >= ii_length) // first transition ON after last graph pt?
			break; 
		if (iix0 < 0) // first transition off graph?
			iix0 = 0; 

		iix0 = MulDiv(display_width, iix0, ii_length) + rect->left;
		p_dc->LineTo(iix0, state);	// draw line up to the first point of the pulse
		p_dc->LineTo(iix0, top);	// draw vertical line to top of pulse

		// stimulus ends here
		state = bottom; // after pulse, descend to bottom level
		int iix1 = p_intervals->GetAt(ii + 1) - ii_first;
		if (iix1 > ii_length) // last transition off graph?
		{
			iix1 = ii_length; // yes = clip
			state = top; // do not descend..
		}
		iix1 = MulDiv(display_width, iix1, ii_length) + rect->left + 1;
		p_dc->LineTo(iix1, top); // draw top of pulse
		p_dc->LineTo(iix1, state); // draw descent to bottom line
	}
	p_dc->LineTo(rect->left + display_width, state); // end of loop - draw the rest
	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::display_bars(CDC* p_dc, const CRect* rect)
{
	// prepare loop to display spikes
	auto* old_pen = static_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));
	const long rect_width = rect->Width();
	if (m_yWE == 1)
	{
		short value_max, value_min;
		p_spike_list->get_total_max_min(TRUE, &value_max, &value_min);
		m_yWE = value_max - value_min;
		m_yWO = (value_max + value_min) / 2;
	}
	const auto y_we = m_yWE;
	const auto y_wo = m_yWO;
	const auto y_vo = rect->Height() / 2 + rect->top;
	const auto y_ve = -rect->Height();

	// draw horizontal line
	const int baseline = MulDiv(p_spike_list->get_acq_bin_zero() - y_wo, y_ve, y_we) + y_vo;
	p_dc->MoveTo(rect->left, baseline);
	p_dc->LineTo(rect->right, baseline);

	// loop through all spikes of the list
	auto i_first = 0;
	auto i_last = p_spike_list->get_spikes_count() - 1;
	if (m_range_mode == RANGE_INDEX)
	{
		if (m_index_last_spike > i_last)
			m_index_last_spike = i_last;
		if (m_index_first_spike < 0)
			m_index_first_spike = 0;
		i_last = m_index_last_spike; // reduces the nb of spikes examined
		i_first = m_index_first_spike; // assuming an ordered list...
	}
	const auto len = (m_lLast - m_lFirst + 1);
	
	auto pen_color = BLACK_COLOR;
	for (auto i_spike = i_last; i_spike >= i_first; i_spike--)
	{
		const Spike* spike = p_spike_list->get_spike(i_spike);

		// skip spike if outside time range && option
		const auto l_spike_time = spike->get_time();
		if (m_range_mode == RANGE_TIMEINTERVALS
			&& (l_spike_time < m_lFirst || l_spike_time > m_lLast))
			continue;

		// select correct pen
		const auto spike_class = spike->get_class_id();
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
			if (spike_class != m_selected_class)
				continue;
			break;
		case PLOT_CLASSCOLORS:
			pen_color = spike_class % NB_COLORS;
			break;
		case PLOT_ONECLASS:
			if (spike_class != m_selected_class)
				pen_color = SILVER_COLOR;
			else
				pen_color = m_colorselected;
		default:
			break;
		}
		p_dc->SelectObject(&m_penTable[pen_color]);

		// and draw spike: compute abscissa & draw from max to min
		const auto llk = MulDiv((l_spike_time - m_lFirst), rect_width, len);
		const int abscissa = static_cast<int>(llk) + rect->left;
		short max, min;
		p_spike_list->get_spike(i_spike)->get_max_min(&max, &min);
		max = static_cast<short>(MulDiv(max - y_wo, y_ve, y_we) + y_vo);
		min = static_cast<short>(MulDiv(min - y_wo, y_ve, y_we) + y_vo);
		p_dc->MoveTo(abscissa, max);
		p_dc->LineTo(abscissa, min);

		if (spike_class >= 1)
		{
			TEXTMETRIC text_metric;
			if (p_dc->GetTextMetrics(&text_metric))
			{
				auto letter = _T('0');
				letter += spike_class;
				p_dc->TextOut(abscissa, min + (text_metric.tmHeight / 2), &letter, 1);
			}
		}
	}

	// display selected spike
	if (spike_selected_.spike_index >= 0)
	{
		const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
		display_spike(spike);
		highlight_spike(spike);
	}
	if (p_spike_list->get_spike_flag_array_count() > 0)
		display_flagged_spikes(TRUE);

	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::display_flagged_spikes(const BOOL b_high_light)
{
	if (p_spike_list->get_spike_flag_array_count() < 1)
		return;

	CClientDC dc(this);
	if (m_xWE == 1 || m_yWE == 1)
		return;
	m_xWE = m_displayRect.Width();
	m_xWO = m_displayRect.left;

	dc.IntersectClipRect(&m_clientRect);
	prepare_dc(&dc);

	// loop over the array of flagged spikes
	const boolean is_selected_spike_in_this_list = 
		(p_dbwave_doc->get_current_spike_file()->get_spike_list_current_index()
			== spike_selected_.spike_list_index);
	for (auto i = p_spike_list->get_spike_flag_array_count() - 1; i >= 0; i--)
	{
		constexpr auto pen_size = 0;
		const auto no_spike = p_spike_list->get_spike_flag_array_at(i);

		const Spike* spike = p_spike_list->get_spike(no_spike);
		const auto no_spike_class = spike->get_class_id();
		if (PLOT_ONECLASSONLY == m_plotmode && no_spike_class != m_selected_class)
			continue;

		auto color_index = RED_COLOR;
		if (!b_high_light)
		{
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
			case PLOT_ONECLASS:
				color_index = BLACK_COLOR;
				if (no_spike_class != m_selected_class)
					color_index = SILVER_COLOR;
				break;
			case PLOT_CLASSCOLORS:
				if (is_selected_spike_in_this_list && no_spike == spike_selected_.spike_index)
					highlight_spike(spike);
				color_index = no_spike_class % NB_COLORS;
				break;
			case PLOT_BLACK:
			default:
				color_index = BLACK_COLOR;
				break;
			}
		}

		CPen new_pen;
		new_pen.CreatePen(PS_SOLID, pen_size, m_colorTable[color_index]);
		const auto old_pen = dc.SelectObject(&new_pen);

		// display data
		const auto l_spike_time = spike->get_time();
		const auto len = m_lLast - m_lFirst + 1;
		const auto llk = MulDiv(l_spike_time - m_lFirst, m_xWE, len);
		const auto abscissa = static_cast<int>(llk) + m_xWO;
		short max, min;
		spike->get_max_min(&max, &min);

		dc.MoveTo(abscissa, max);
		dc.LineTo(abscissa, min);
		dc.SelectObject(old_pen);
	}
}

void ChartSpikeBar::display_spike(const Spike* spike)
{
	int color_index = BLACK_COLOR;
	const auto spike_class = spike->get_class_id();
	switch (m_plotmode)
	{
	case PLOT_ONECLASSONLY:
	case PLOT_ONECLASS:
		if (spike_class != m_selected_class)
			color_index = SILVER_COLOR;
		break;
	case PLOT_CLASSCOLORS:
		color_index = spike_class % NB_COLORS;
		break;
	case PLOT_BLACK:
	default:
		color_index = BLACK_COLOR;
		break;
	}

	draw_spike(spike, color_index);
}

void ChartSpikeBar::draw_spike(const Spike* spike, const int color_index)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	/*
	CClientDC dc(this);
	if (m_xWE == 1 || m_yWE == 1)
		return;
	m_xWE = m_displayRect.Width();
	m_xWO = m_displayRect.left;
	dc.IntersectClipRect(&m_clientRect);
	prepare_dc(&dc);
	*/
	CPen new_pen;
	constexpr auto pen_size = 0;
	new_pen.CreatePen(PS_SOLID, pen_size, m_colorTable[color_index]);
	const auto old_pen = dc.SelectObject(&new_pen);

	// display data
	const auto l_spike_time = spike->get_time();
	const auto len = m_lLast - m_lFirst + 1;
	const auto llk = MulDiv(l_spike_time - m_lFirst, m_xWE, len);
	const auto abscissa = static_cast<int>(llk) + m_xWO;
	short max, min;
	spike->get_max_min(&max, &min);

	dc.MoveTo(abscissa, max);
	dc.LineTo(abscissa, min);
	dc.SelectObject(old_pen);
}

void ChartSpikeBar::highlight_spike(const Spike* spike) 
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);

	const auto old_rop2 = dc.GetROP2();
	dc.SetROP2(R2_NOTXORPEN);

	const auto l_spike_time = spike->get_time();
	const auto len = m_lLast - m_lFirst + 1;
	const auto llk = MulDiv(l_spike_time - m_lFirst, m_xWE, len);
	const auto abscissa = static_cast<int>(llk) + m_xWO;

	const auto max = MulDiv(1 - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto min = MulDiv(m_displayRect.Height() - 2 - m_yVO, m_yWE, m_yVE) + m_yWO;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196, 2, 51));
	const auto old_pen = dc.SelectObject(&new_pen);
	auto* old_brush = dc.SelectStockObject(NULL_BRUSH);
	const CRect rect1(abscissa - 2, max, abscissa + 2, min);
	dc.Rectangle(&rect1);

	// restore resources
	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
	dc.SetROP2(old_rop2);
}

void ChartSpikeBar::select_spike(const Spike_selected& new_spike_selected)
{
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
		highlight_spike(spike);
		display_spike(spike);
	}

	spike_selected_ = new_spike_selected;
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
		display_spike(spike);
		highlight_spike(spike);
	}
}

// flag all spikes within a rectangle in screen coordinates
void ChartSpikeBar::select_spikes_within_rect(CRect* p_rect, const UINT n_flags) const
{
	// make sure that the rectangle is ok
	int i;
	if (p_rect->top > p_rect->bottom)
	{
		i = p_rect->top;
		p_rect->top = p_rect->bottom;
		p_rect->bottom = i;
	}
	if (p_rect->left > p_rect->right)
	{
		i = p_rect->left;
		p_rect->left = p_rect->right;
		p_rect->right = i;
	}

	const auto len = (m_lLast - m_lFirst + 1);
	const auto l_first = MulDiv(p_rect->left, len, m_displayRect.Width()) + m_lFirst;
	const auto l_last = MulDiv(p_rect->right, len, m_displayRect.Width()) + m_lFirst;
	const auto v_min = MulDiv(p_rect->bottom - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto v_max = MulDiv(p_rect->top - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto b_flag = (n_flags & MK_SHIFT) || (n_flags & MK_CONTROL);
	p_spike_list->select_spikes_within_bounds(v_min, v_max, l_first, l_last, b_flag);
}

void ChartSpikeBar::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (!m_bLmouseDown)
	{
		postMyMessage(HINT_DROPPED, NULL);
		return;
	}
	ChartSpike::OnLButtonUp(n_flags, point);

	CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
	constexpr auto jitter = 5;
	if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
	{
		if (m_cursorType != CURSOR_ZOOM)
			postMyMessage(HINT_HITAREA, NULL);
		else
			zoomIn();
		return; // exit: mouse movement was too small
	}

	// perform action according to cursor type
	auto rect_in = m_displayRect;
	switch (m_cursorType)
	{
	case 0:
		if (m_hit_spike < 0)
		{
			auto rect = GetDefinedRect();
			select_spikes_within_rect(&rect, n_flags);
			postMyMessage(HINT_SELECTSPIKES, NULL);
		}
		break;

	case CURSOR_ZOOM:
		ZoomData(&rect_in, &rect_out);
		m_ZoomFrom = rect_in;
		m_ZoomTo = rect_out;
		m_iUndoZoom = 1;
		postMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
		break;

	default:
		break;
	}
}

void ChartSpikeBar::OnLButtonDown(const UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;

	// detect bar hit: test if curve hit -- specific to SpikeBarButton
	if (m_currCursorMode == 0)
	{
		m_track_curve = FALSE;
		m_hit_spike = hitCurve(point);
		// tell parent spike selected
		if (m_hit_spike >= 0)
		{
			if (nFlags & MK_SHIFT)
				postMyMessage(HINT_HITSPIKE_SHIFT, m_hit_spike);
			else if (nFlags & MK_CONTROL)
				postMyMessage(HINT_HITSPIKE_CTRL, m_hit_spike);
			else
				postMyMessage(HINT_HITSPIKE, m_hit_spike);
			return;
		}
	}
	ChartSpike::OnLButtonDown(nFlags, point);
}

//---------------------------------------------------------------------------
// ZoomData()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=y_extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void ChartSpikeBar::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();
	rDest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_yWE;
	m_yWE = MulDiv(m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
		- MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
		+ MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed
	auto l_size = m_lLast - m_lFirst + 1;
	m_lFirst = m_lFirst + l_size * (rDest->left - rFrom->left) / rFrom->Width();
	if (m_lFirst < 0)
		m_lFirst = 0;
	l_size = l_size * rDest->Width() / rFrom->Width();
	m_lLast = m_lFirst + l_size - 1;

	postMyMessage(HINT_CHANGEHZLIMITS, NULL);
}

void ChartSpikeBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((spike_selected_.spike_index < 0 && p_spike_list->get_spike_flag_array_count() < 1) || m_hit_spike < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		if (spike_selected_.spike_index >= 0)
		{
			postMyMessage(HINT_DBLCLKSEL, spike_selected_.spike_index);
		}
		else
		{
			const auto i_selected_spike = hitCurve(point);
			if (i_selected_spike >= 0)
				postMyMessage(HINT_DBLCLKSEL, i_selected_spike);
		}
	}
}

int ChartSpikeBar::hit_curve_in_doc(const CPoint point)
{
	long n_files = 1;
	long current_file = 0;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->db_get_n_records();
		current_file = p_dbwave_doc->db_get_current_record_position();
	}

	int result = -1;
	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (m_display_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(i_file))
				p_dbwave_doc->open_current_spike_file();
			p_spike_list = p_dbwave_doc->m_p_spk->get_spike_list_current();
		}

		if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		{
			continue;
		}
		result = hitCurve(point);
		if (result >= 0)
			break;
	}

	if (m_display_all_files && result < 0)
	{
		if (p_dbwave_doc->db_set_current_record_position(current_file))
			p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_p_spk->get_spike_list_current();
	}

	return result;
}

int ChartSpikeBar::hitCurve(const CPoint point)
{
	auto hit_spike = -1;
	// for y coordinates, conversion is straightforward:
	const auto mouse_y = MulDiv(point.y - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto delta_y = MulDiv(3, m_yWE, m_yVE);
	/*
	// for x coordinates, the formula is in 2 steps:
	// 1) time -> relative time: ii_time = (spike_time-m_lFirst) [-offset]
	// 2) rel time -> logical coordinate(LC): LC = ii_time* m_xWE / len + m_xWO
	// formula used to display: time (long) --> abscissa (int)
	// 		long len =  (m_lLast - m_lFirst + 1);
	// 		int abscissa = (int) (((lSpikeTime - m_lFirst) * (long) m_xWE) / len) + m_xWO;
	// reverse formula: abscissa -> time
	//		long lSpikeTime  = (abscissa - m_xWO)*len/m_xWE + m_lFirst;
	// convert device coordinates into logical coordinates
	*/
	const auto mouse_x = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
	const auto delta_x = MulDiv(3, m_xWE, m_xVE);
	const auto len_data_displayed = (m_lLast - m_lFirst + 1);

	// find a spike which time of occurrence fits between l_X_max and l_X_min
	const auto x_max = m_lFirst + MulDiv(len_data_displayed, mouse_x + delta_x, m_xWE);
	const auto x_min = m_lFirst + MulDiv(len_data_displayed, mouse_x - delta_x, m_xWE);

	// loop through all spikes
	auto i_spike_first = 0;
	auto i_spike_last = p_spike_list->get_spikes_count() - 1;
	if (m_range_mode == RANGE_INDEX)
	{
		if (m_index_last_spike > i_spike_last) m_index_last_spike = i_spike_last;
		if (m_index_first_spike < 0) m_index_first_spike = 0;
		i_spike_last = m_index_last_spike;
		i_spike_first = m_index_first_spike;
	}

	for (auto i_spike = i_spike_last; i_spike >= i_spike_first; i_spike--)
	{
		const Spike* spike = p_spike_list->get_spike(i_spike);
		const auto l_spike_time = spike->get_time();
		if (l_spike_time < x_min || l_spike_time > x_max)
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
			&& spike->get_class_id() != m_selected_class)
			continue;

		short max, min;
		spike->get_max_min(&max, &min);
		if (mouse_y + delta_y < max && mouse_y - delta_y > min)
		{
			hit_spike = i_spike;
			break;
		}
	}

	return hit_spike;
}

void ChartSpikeBar::center_curve()
{
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() <= 0)
		return;
	short max, min;
	p_spike_list->get_total_max_min(TRUE, &max, &min);
	m_yWO = max / 2 + min / 2;
}

void ChartSpikeBar::max_gain()
{
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() <= 0)
		return;
	short max, min;
	p_spike_list->get_total_max_min(TRUE, &max, &min);
	m_yWE = MulDiv(max - min + 1, 10, 8);
}

void ChartSpikeBar::max_center()
{
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() <= 0)
		return;
	short max, min;
	p_spike_list->get_total_max_min(TRUE, &max, &min);
	
	m_yWE = MulDiv(max - min + 1, 10, 8);
	m_yWO = max / 2 + min / 2;
}

void ChartSpikeBar::Print(CDC* p_dc, const CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		return;

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC(); // save display context
	display_bars(p_dc, rect);

	if (p_dbwave_doc->m_p_spk->m_stimulus_intervals.n_items > 0)
		display_stimulus(p_dc, rect);

	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeBar::Serialize(CArchive& ar)
{
	ChartSpike::Serialize(ar);

	auto dummy = TRUE;
	int dummy_int = 1;
	if (ar.IsStoring())
	{
		ar << m_range_mode; // display range (time OR storage index)
		ar << m_lFirst; // time index of first pt displayed
		ar << m_lLast; // time index of last pt displayed
		ar << m_index_first_spike; // index first spike
		ar << m_index_last_spike; // index last spike
		ar << m_current_class; // current class in case of displaying classes
		ar << dummy_int;
		ar << m_hit_spike; // no of spike selected
		ar << m_selected_class; // index class selected
		ar << m_track_curve; // track curve ?
		ar << dummy;
		ar << m_selected_pen;
	}
	else
	{
		ar >> m_range_mode; // display range (time OR storage index)
		ar >> m_lFirst; // time index of first pt displayed
		ar >> m_lLast; // time index of last pt displayed
		ar >> m_index_first_spike; // index first spike
		ar >> m_index_last_spike; // index last spike
		ar >> m_current_class; // current class in case of displaying classes
		ar >> dummy_int; 
		ar >> m_hit_spike; // no of spike selected
		ar >> m_selected_class; // index class selected
		ar >> m_track_curve; // track curve ?
		ar >> dummy;
		ar >> m_selected_pen;
	}
}


