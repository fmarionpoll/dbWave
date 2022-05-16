#include "StdAfx.h"
#include "ChartSpikeShape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_b_all_files is true: spike hit

IMPLEMENT_SERIAL(ChartSpikeShape, ChartSpike, 1)

BEGIN_MESSAGE_MAP(ChartSpikeShape, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeShape::ChartSpikeShape()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (spikeshape)");
}

ChartSpikeShape::~ChartSpikeShape()
= default;

void ChartSpikeShape::plot_spikes(CDC* p_dc)
{
	PlotDataToDC(p_dc);
}

void ChartSpikeShape::PlotDataToDC(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	// display data: trap error conditions
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	getExtents();
	prepareDC(p_dc);
	auto current_file = 0;
	auto n_files = 1;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->GetDB_NRecords();
		current_file = p_dbwave_doc->GetDB_CurrentRecordPosition();
	}

	for (auto i_file = 0; i_file < n_files; i_file++)
	{
		if (m_display_all_files)
		{
			p_dbwave_doc->set_db_current_record_position(i_file);
			p_dbwave_doc->open_current_spike_file();
		}
		p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();

		//test if data are there - if none, display message and exit
		if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		{
			if (!m_display_all_files)
			{
				p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT); 
				return;
			}
			continue;
		}

		// load resources and prepare context
		const auto spike_length = p_spike_list->get_spike_length();
		ASSERT(spike_length > 0);
		if (polypoints_.GetSize() != spike_length)
		{
			polypoints_.SetSize(spike_length, 2);
			initPolypointAbcissa();
		}

		// loop through all spikes of the list
		short* p_spike_data;
		auto i_last_spike = p_spike_list->get_spikes_count() - 1;
		auto i_first_spike = 0;
		if (m_range_mode == RANGE_INDEX)
		{
			i_last_spike = m_index_last_spike;
			i_first_spike = m_index_first_spike;
		}
		auto selected_pen_color = BLACK_COLOR;
		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
			selected_pen_color = SILVER_COLOR;
		const auto old_pen = p_dc->SelectObject(&m_penTable[selected_pen_color]);

		for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
		{
			Spike* spike = p_spike_list->get_spike(i_spike);

			// skip spike ?
			if (m_range_mode == RANGE_TIMEINTERVALS
				&& (spike->get_time() < m_lFirst || spike->get_time() > m_lLast))
				continue;

			// select pen according to class
			const auto spike_class = spike->get_class_id();
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
				if (spike_class != m_selected_class)
					continue;
				break;
			case PLOT_CLASSCOLORS:
				selected_pen_color = spike_class % NB_COLORS;
				p_dc->SelectObject(&m_penTable[selected_pen_color]);
				break;
			case PLOT_ONECLASS:
				if (spike_class == m_selected_class)
					continue;
			default:
				break;
			}

			// display data
			p_spike_data = spike->get_p_data();
			fillPolypointOrdinates(p_spike_data);
			p_dc->Polyline(&polypoints_[0], spike_length);
		}

		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
		{
			selected_pen_color = m_colorselected;
			if (m_plotmode == PLOT_ONECOLOR)
				selected_pen_color = m_selected_class % NB_COLORS;
			p_dc->SelectObject(&m_penTable[selected_pen_color]);
			for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
			{
				Spike* spike = p_spike_list->get_spike(i_spike);
				// skip spike ?
				if (m_range_mode == RANGE_TIMEINTERVALS
					&& (spike->get_time() < m_lFirst || spike->get_time() > m_lLast))
					continue;

				// skip spikes with the wrong class
				if (spike->get_class_id() != m_selected_class)
					continue;
				// display data
				p_spike_data = spike->get_p_data();
				fillPolypointOrdinates(p_spike_data);
				p_dc->Polyline(&polypoints_[0], spike_length);
			}
		}

		// display selected spike
		auto i_select = -1;
		if (m_selected_spike >= 0 && (IsSpikeWithinRange(m_selected_spike)))
			i_select = m_selected_spike;
		drawSelectedSpike(i_select, p_dc);

		if (p_spike_list->GetSpikeFlagArrayCount() > 0)
			drawFlaggedSpikes(p_dc);

		// display tags
		if (m_HZtags.GetNTags() > 0)
			DisplayHZtags(p_dc);

		if (m_VTtags.GetNTags() > 0)
			DisplayVTtags_Value(p_dc);

		// display text
		if (m_bText && m_plotmode == PLOT_ONECLASSONLY)
		{
			TCHAR num[10];
			wsprintf(num, _T("%i"), GetSelClass());
			p_dc->TextOut(1, 1, num);
		}

		// restore resource
		p_dc->SelectObject(old_pen);
	}

	// restore resources
	p_dc->RestoreDC(n_saved_dc);

	if (m_display_all_files)
	{
		p_dbwave_doc->set_db_current_record_position(current_file);
		p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();
	}
}

void ChartSpikeShape::drawSelectedSpike(int no_spike, CDC* p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();
	auto rect = m_displayRect;
	p_dc->DPtoLP(rect);
	p_dc->IntersectClipRect(&rect);

	if (no_spike >= 0)
	{
		getExtents();
		prepareDC(p_dc);

		p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2 + m_displayRect.top);
		p_dc->SetViewportExt(m_displayRect.Width(), -m_displayRect.Height());

		constexpr auto pen_size = 2;
		CPen new_pen(PS_SOLID, pen_size, m_colorTable[m_colorselectedspike]);
		auto* old_pen = p_dc->SelectObject(&new_pen);

		auto* p_data = p_spike_list->get_spike(no_spike)->get_p_data();
		fillPolypointOrdinates(p_data);
		p_dc->Polyline(&polypoints_[0], p_spike_list->get_spike_length());

		p_dc->SelectObject(old_pen);
	}
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::drawFlaggedSpikes(CDC* pDC0)
{
	ASSERT(pDC0 != NULL);
	const auto p_dc = pDC0;
	const auto n_saved_dc = p_dc->SaveDC();

	// change coordinate settings
	getExtents();
	prepareDC(p_dc);
	p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2);
	p_dc->SetViewportExt(m_displayRect.right, -m_displayRect.Height());

	constexpr auto pen_size = 1;
	CPen new_pen(PS_SOLID, pen_size, m_colorTable[m_colorselectedspike]);
	const auto old_pen = p_dc->SelectObject(&new_pen);

	// loop through all flagged spikes
	for (auto i = p_spike_list->GetSpikeFlagArrayCount() - 1; i >= 0; i--)
	{
		const auto no_spike = p_spike_list->GetSpikeFlagArrayAt(i);
		if (!IsSpikeWithinRange(no_spike))
			continue;
		fillPolypointOrdinates(p_spike_list->get_spike(no_spike)->get_p_data());
		p_dc->Polyline(&polypoints_[0], p_spike_list->get_spike_length());
	}

	p_dc->SelectObject(old_pen);
	pDC0->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::DisplayFlaggedSpikes(BOOL bHighLight)
{
	if (bHighLight)
		drawFlaggedSpikes(&m_PlotDC);
	Invalidate();
}

int ChartSpikeShape::DisplayExData(short* p_data, int color)
{
	// prepare array
	const auto spike_length = p_spike_list->get_spike_length();
	if (polypoints_.GetSize() != spike_length)
	{
		polypoints_.SetSize(spike_length, 2);
		initPolypointAbcissa();
	}

	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	prepareDC(&dc);
	CPen new_pen(PS_SOLID, 0, m_colorTable[color]);
	const auto old_pen = dc.SelectObject(&new_pen);
	fillPolypointOrdinates(p_data);
	dc.Polyline(&polypoints_[0], p_spike_list->get_spike_length());

	dc.SelectObject(old_pen);
	return color;
}

BOOL ChartSpikeShape::IsSpikeWithinRange(int spike_no) const
{
	if (spike_no > p_spike_list->get_spikes_count() - 1)
		return FALSE;
	if (m_range_mode == RANGE_TIMEINTERVALS
		&& (p_spike_list->get_spike(spike_no)->get_time() < m_lFirst || p_spike_list->get_spike(spike_no)->get_time() > m_lLast))
		return FALSE;
	if (m_range_mode == RANGE_INDEX
		&& (spike_no > m_index_last_spike || spike_no < m_index_first_spike))
		return FALSE;
	if (m_plotmode == PLOT_ONECLASSONLY
		&& (p_spike_list->get_spike(spike_no)->get_class_id() != m_selected_class))
		return FALSE;
	return TRUE;
}

int ChartSpikeShape::SelectSpikeShape(int spike_no)
{
	// erase plane
	const auto old_selected = m_selected_spike;
	m_selected_spike = spike_no;
	if (!m_bUseDIB)
	{
		CClientDC dc(this);
		drawSelectedSpike(m_selected_spike, &dc);
	}
	else
	{
		if (m_PlotDC.GetSafeHdc())
			drawSelectedSpike(m_selected_spike, &m_PlotDC);
	}
	Invalidate();
	return old_selected;
}

void ChartSpikeShape::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bLmouseDown)
	{
		postMyMessage(HINT_DROPPED, NULL);
		return;
	}

	switch (m_trackMode)
	{
	case TRACK_BASELINE:
		// curve was tracked
		{
			if (point.y != m_ptLast.y || point.x != m_ptLast.x)
				OnMouseMove(nFlags, point);
			m_trackMode = TRACK_OFF;
			releaseCursor();
			ChartSpike::OnLButtonUp(nFlags, point);
		}
		break;

	case TRACK_VTTAG:
		// vertical tag was tracked
		{
			// convert pix into data value and back again
			const auto val = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
			m_VTtags.SetTagVal(m_HCtrapped, val);
			point.x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			XorVTtag(point.x);
			ChartSpike::OnLButtonUp(nFlags, point);
			postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	default:
		{
			// none of those: zoom data or  offset display
			ChartSpike::OnLButtonUp(nFlags, point);
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
			constexpr auto jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (m_cursorType != CURSOR_ZOOM)
					postMyMessage(HINT_HITAREA, NULL);
				else
					zoomIn();
				return;
			}

			// perform action according to cursor type
			auto rect_in = m_displayRect;
			switch (m_cursorType)
			{
			case 0:
				rect_out = rect_in;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				postMyMessage(HINT_DEFINEDRECT, NULL);
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
		break;
	}
}

void ChartSpikeShape::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;
	// call base class to test for horizontal cursor or XORing rectangle
	if (m_VTtags.GetNTags() > 0)
	{
		for (auto i_tag = m_VTtags.GetNTags() - 1; i_tag >= 0; i_tag--) 
			m_VTtags.SetTagPix(i_tag, MulDiv(m_VTtags.GetValue(i_tag) - m_xWO, m_xVE, m_xWE) + m_xVO);
	}

	// track rectangle or VTtag?
	ChartSpike::OnLButtonDown(nFlags, point); 
	if (m_currCursorMode != 0 || m_HCtrapped >= 0)
		return; 

	// test if mouse hit one spike
	// if hit, then tell parent to select corresponding spike
	m_hit_spike = hitCurveInDoc(point);
	if (m_hit_spike >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF; 
		releaseCursor(); 
		if (nFlags & MK_SHIFT)
			postMyMessage(HINT_HITSPIKE_SHIFT, m_hit_spike); 

		else
			postMyMessage(HINT_HITSPIKE, m_hit_spike);
	}
}

void ChartSpikeShape::OnMouseMove(UINT nFlags, CPoint point)
{
	ChartSpike::OnMouseMove(nFlags, point);
}

// ZoomData(CRect* rFrom, CRect* rDest)
//
// max and min of rFrom should fit in rDest (same logical coordinates)
// then one can write the 4 equations:
// assume initial conditions WE1, WO1; destination: WE2, WO2
// LPmin = (rFrom.(top/left)      - VO) * WE1 / VE + WO1	(1)
// LPMax = (rFrom.(bottom, right) - VO) * WE1 / VE + WO1	(2)
// LPmin = (rDest.(top/left)      - VO) * WE2 / VE + WO2	(3)
// LPMax = (rDest.(bottom, right) - VO) * WE2 / VE + WO2	(4)
// from (1)-(2) = (3)-(4) one get WE2
// from (1)=(3)               get WO2

void ChartSpikeShape::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect(); // make sure that rect is not inverted
	rDest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_yWE; // save previous window extent
	m_yWE = MulDiv(m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
		- MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
		+ MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed
	const auto x_we = m_xWE; // save previous window extent
	m_xWE = MulDiv(m_xWE, rDest->Width(), rFrom->Width());
	m_xWO = m_xWO
		- MulDiv(rFrom->left - m_xVO, m_xWE, m_xVE)
		+ MulDiv(rDest->left - m_xVO, x_we, m_xVE);

	// display
	Invalidate();
	postMyMessage(HINT_CHANGEZOOM, 0);
}

void ChartSpikeShape::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selected_spike < 0 && p_spike_list->GetSpikeFlagArrayCount() < 1) || m_hit_spike < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		if (m_selected_spike >= 0)
		{
			postMyMessage(HINT_DBLCLKSEL, m_selected_spike);
		}
		else
		{
			const auto selected_spike = hitCurve(point);
			if (selected_spike > 0)
				postMyMessage(HINT_DBLCLKSEL, selected_spike);
		}
	}
}

int ChartSpikeShape::hitCurveInDoc(CPoint point)
{
	long n_files = 1;
	long current_file_index = 0;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->GetDB_NRecords();
		current_file_index = p_dbwave_doc->GetDB_CurrentRecordPosition();
	}

	int result = -1;
	for (long ifile = 0; ifile < n_files; ifile++)
	{
		if (m_display_all_files)
		{
			p_dbwave_doc->set_db_current_record_position(ifile);
			p_dbwave_doc->open_current_spike_file();
			p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();
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
		p_dbwave_doc->set_db_current_record_position(current_file_index);
		p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();
	}

	return result;
}

int ChartSpikeShape::hitCurve(const CPoint point)
{
	auto index_spike_hit = -1;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
	if (mouse_x < 0 || mouse_x > p_spike_list->get_spike_length())
		return index_spike_hit;
	const auto mouse_y = MulDiv(point.y - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto delta_y = MulDiv(3, m_yWE, m_yVE);

	// loop through all spikes
	auto index_last_spike = p_spike_list->get_spikes_count() - 1;
	auto index_first_spike = 0;
	if (m_range_mode == RANGE_INDEX)
	{
		index_last_spike = m_index_last_spike;
		index_first_spike = m_index_first_spike;
	}
	for (auto spike_index = index_last_spike; spike_index >= index_first_spike; spike_index--)
	{
		if (m_range_mode == RANGE_TIMEINTERVALS
			&& (p_spike_list->get_spike(spike_index)->get_time() < m_lFirst
				|| p_spike_list->get_spike(spike_index)->get_time() > m_lLast))
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
			&& p_spike_list->get_spike(spike_index)->get_class_id() != m_selected_class)
			continue;

		const auto val = p_spike_list->get_spike(spike_index)->get_value_at_offset(mouse_x);
		if (mouse_y + delta_y < val && mouse_y - delta_y > val)
		{
			index_spike_hit = spike_index;
			break;
		}
	}
	return index_spike_hit;
}

void ChartSpikeShape::getExtents()
{
	const auto current_file_index = p_dbwave_doc->GetDB_CurrentRecordPosition();
	auto file_first = current_file_index;
	auto file_last = current_file_index;
	if (m_display_all_files)
	{
		file_first = 0;
		file_last = p_dbwave_doc->GetDB_NRecords() - 1;
	}

	if (m_yWE == 1 || m_yWE == 0)
	{
		for (auto file_index = file_first; file_index <= file_last; file_index++)
		{
			if (file_index != current_file_index)
			{
				p_dbwave_doc->set_db_current_record_position(file_index);
				p_dbwave_doc->open_current_spike_file();
				p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();
			}
			if (p_spike_list != nullptr)
			{
				getExtentsCurrentSpkList();
				if (m_yWE != 0)
					break;
			}
		}
	}

	// exit 
	if (file_first != current_file_index || file_last != current_file_index)
	{
		p_dbwave_doc->set_db_current_record_position(current_file_index);
		if (p_dbwave_doc->open_current_spike_file() != nullptr)
			p_spike_list = p_dbwave_doc->m_pSpk->GetSpkList_Current();
	}
}

void ChartSpikeShape::getExtentsCurrentSpkList()
{
	if (m_yWE == 1 || m_yWE == 0)
	{
		short value_max, value_min;
		p_spike_list->GetTotalMaxMin(TRUE, &value_max, &value_min);
		m_yWE = MulDiv((value_max - value_min), 10, 9) + 1;
		m_yWO = value_max / 2 + value_min / 2;
	}

	if (m_xWE <= 1)
	{
		m_xWE = p_spike_list->get_spike_length();
		m_xWO = 0;
	}
}

void ChartSpikeShape::initPolypointAbcissa()
{
	const auto n_elements = polypoints_.GetSize();
	m_xWE = n_elements + 1;
	ASSERT(n_elements > 0);

	for (auto i = 0; i < n_elements; i++)
		polypoints_[i].x = i + 1;
}

void ChartSpikeShape::fillPolypointOrdinates(short* lpSource)
{
	auto n_elements = polypoints_.GetSize();
	if (n_elements == 0)
	{
		n_elements = p_spike_list->get_spike_length();
		ASSERT(n_elements > 0);
		polypoints_.SetSize(n_elements, 2);
		initPolypointAbcissa();
	}

	for (auto i = 0; i < n_elements; i++, lpSource++)
		polypoints_[i].y = *lpSource;
}

void ChartSpikeShape::Print(CDC* p_dc, CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		return;

	const auto old_y_vo = m_yVO;
	const auto old_y_ve = m_yVE;
	const auto old_x_extent = m_xWE;
	const auto old_x_origin = m_xWO;

	// size of the window
	m_yVO = rect->Height() / 2 + rect->top;
	m_yVE = -rect->Height();

	// check initial conditions
	if (m_yWE == 1) 
	{
		short value_max, value_min;
		p_spike_list->GetTotalMaxMin(TRUE, &value_max, &value_min);
		m_yWE = value_max - value_min + 1;
		m_yWO = (value_max + value_min) / 2;
	}

	m_xWO = rect->left;
	m_xWE = rect->Width() - 2;

	const auto spike_length = p_spike_list->get_spike_length();
	if (polypoints_.GetSize() != spike_length)
		polypoints_.SetSize(spike_length, 2);

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC();
	for (auto i = 0; i < spike_length; i++)
		polypoints_[i].x = rect->left + MulDiv(i, rect->Width(), spike_length);

	int selected_color = BLACK_COLOR;
	switch (m_plotmode)
	{
	//case PLOT_BLACK:			selpen = BLACK_COLOR; break;
	//case PLOT_ONECLASSONLY:	selpen = BLACK_COLOR; break;
	case PLOT_ONECLASS:
		selected_color = m_colorbackgr;
		break;
	case PLOT_ALLGREY:
		selected_color = m_colorbackgr;
		break;
	default:
		break;
	}

	const auto old_pen = p_dc->SelectObject(&m_penTable[selected_color]);
	auto spike_index_last = p_spike_list->get_spikes_count() - 1;
	auto spike_index_first = 0;
	if (m_range_mode == RANGE_INDEX)
	{
		spike_index_last = m_index_last_spike;
		spike_index_first = m_index_first_spike;
	}

	for (auto spike_index = spike_index_last; spike_index >= spike_index_first; spike_index--)
	{
		Spike* spike = p_spike_list->get_spike(spike_index);
		if (m_range_mode == RANGE_INDEX && (spike_index > m_index_last_spike || spike_index < m_index_first_spike))
			continue;
		if (m_range_mode == RANGE_TIMEINTERVALS)
		{
			if (spike->get_time() < m_lFirst)
				continue;
			if (spike->get_time() > m_lLast)
				continue;
		}

		const auto spike_class = p_spike_list->get_spike(spike_index)->get_class_id();
		if (m_plotmode == PLOT_ONECLASSONLY && spike_class != m_selected_class)
			continue;
		if (m_plotmode == PLOT_ONECLASS && spike_class == m_selected_class)
			continue;

		plotArraytoDC(p_dc, spike->get_p_data() );
	}

	// display selected class if requested by option
	if (m_plotmode == PLOT_ONECLASS)
	{
		p_dc->SelectObject(&m_penTable[m_colorselected]);
		for (auto spike_index = spike_index_last; spike_index >= spike_index_first; spike_index--)
		{
			Spike* spike = p_spike_list->get_spike(spike_index);
			if (m_range_mode == RANGE_TIMEINTERVALS)
			{
				const auto spike_time = spike->get_time();
				if (spike_time < m_lFirst || spike_time > m_lLast)
					continue;
			}
			if (spike->get_class_id() != m_selected_class)
				continue;
			plotArraytoDC(p_dc, spike->get_p_data());
		}
	}

	// display selected spike
	if (m_selected_spike >= 0 && IsSpikeWithinRange(m_selected_spike))
	{
		CPen new_pen(PS_SOLID, 0, m_colorTable[m_colorselectedspike]);
		p_dc->SelectObject(&new_pen);
		plotArraytoDC(p_dc, p_spike_list->get_spike(m_selected_spike)->get_p_data());
	}

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	m_xWE = old_x_extent; 
	m_xWO = old_x_origin;
	m_yVO = old_y_vo; 
	m_yVE = old_y_ve;
}

void ChartSpikeShape::plotArraytoDC(CDC* p_dc, short* pspk)
{
	const auto n_elements = polypoints_.GetSize();
	for (auto i = 0; i < n_elements; i++, pspk++)
	{
		auto y = *pspk;
		y = static_cast<short>(MulDiv(y - m_yWO, m_yVE, m_yWE) + m_yVO);
		polypoints_[i].y = y;
	}

	if (p_dc->m_hAttribDC == nullptr
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE))
		p_dc->Polyline(&polypoints_[0], n_elements);
	else
	{
		p_dc->MoveTo(polypoints_[0]);
		for (auto i = 0; i < n_elements; i++)
			p_dc->LineTo(polypoints_[i]);
	}
}

void ChartSpikeShape::MoveVTtrack(int i_track, int new_value)
{
	CPoint point;
	m_ptLast.x = MulDiv(m_VTtags.GetValue(i_track) - m_xWO, m_xVE, m_xWE) + m_xVO;
	m_VTtags.SetTagVal(i_track, new_value); 
	point.x = MulDiv(new_value - m_xWO, m_xVE, m_xWE) + m_xVO; 
	XorVTtag(point.x);
}

void ChartSpikeShape::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ChartSpike::Serialize(ar);
		polypoints_.Serialize(ar);

		ar << m_range_mode; // display range (time OR storage index)
		ar << m_lFirst; // time first (real time = index/sampling rate)
		ar << m_lLast; // time last
		ar << m_index_first_spike; // index first spike
		ar << m_index_last_spike; // index last spike
		ar << m_current_class; // selected class (different color) (-1 = display all)
		ar << m_selected_spike; // selected spike (display differently)
		ar << m_colorselectedspike; // color selected spike (index / color table)
		ar << m_hit_spike; // index spike
		ar << m_selected_class; // index class selected
		ar << m_bText; // allow text default false
		ar << m_selected_class; // dummy
	}
	else
	{
		ChartSpike::Serialize(ar);
		polypoints_.Serialize(ar);

		ar >> m_range_mode; // display range (time OR storage index)
		ar >> m_lFirst; // time first (real time = index/sampling rate)
		ar >> m_lLast; // time last
		ar >> m_index_first_spike; // index first spike
		ar >> m_index_last_spike; // index last spike
		ar >> m_current_class; // selected class (different color) (-1 = display all)
		ar >> m_selected_spike; // selected spike (display differently)
		ar >> m_colorselectedspike; // color selected spike (index / color table)
		ar >> m_hit_spike; // index spike
		ar >> m_selected_class; // index class selected
		ar >> m_bText; // allow text default false
		ar >> m_selected_class; // dummy
	}
}

float ChartSpikeShape::GetDisplayMaxMv()
{
	getExtents();
	return (p_spike_list->GetAcqVoltsperBin() * 1000.f * static_cast<float>(m_yWE - m_yWO - p_spike_list->GetAcqBinzero()));
}

float ChartSpikeShape::GetDisplayMinMv()
{
	if (p_spike_list == nullptr)
		return 1.f;
	getExtents();
	return (p_spike_list->GetAcqVoltsperBin() * 1000.f * static_cast<float>(m_yWO - m_yWE - p_spike_list->GetAcqBinzero()));
}

float ChartSpikeShape::GetExtent_mV()
{
	if (p_spike_list == nullptr)
		return 1.f;
	getExtents();
	return (p_spike_list->GetAcqVoltsperBin() * static_cast<float>(m_yWE) * 1000.f);
}

float ChartSpikeShape::GetExtent_ms()
{
	if (p_spike_list == nullptr)
		return 1.f;
	getExtents();
	return (static_cast<float>(1000.0 * m_xWE) / p_spike_list->GetAcqSampRate());
}


