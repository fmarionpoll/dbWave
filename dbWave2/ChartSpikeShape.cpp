#include "StdAfx.h"
#include "ChartWnd.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"
#include "ChartSpikeShape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_ballfiles is true: spike hit

IMPLEMENT_SERIAL(ChartSpikeShapeWnd, ChartWnd, 1)

BEGIN_MESSAGE_MAP(ChartSpikeShapeWnd, ChartWnd)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeShapeWnd::ChartSpikeShapeWnd()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (spikeshape)");
}

ChartSpikeShapeWnd::~ChartSpikeShapeWnd()
= default;

void ChartSpikeShapeWnd::PlotDataToDC(CDC* p_dc)
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
	if (m_ballFiles)
	{
		n_files = p_dbwave_doc_->GetDB_NRecords();
		current_file = p_dbwave_doc_->GetDB_CurrentRecordPosition();
	}

	for (auto i_file = 0; i_file < n_files; i_file++)
	{
		if (m_ballFiles)
		{
			p_dbwave_doc_->SetDB_CurrentRecordPosition(i_file);
			p_dbwave_doc_->OpenCurrentSpikeFile();
		}
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();

		//test if data are there - if none, display message and exit
		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			if (!m_ballFiles)
			{
				p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT); 
				return;
			}
			continue;
		}

		// load resources and prepare context
		const auto spike_length = p_spikelist_->GetSpikeLength();
		ASSERT(spike_length > 0);
		if (polypoints_.GetSize() != spike_length)
		{
			polypoints_.SetSize(spike_length, 2);
			initPolypointAbcissa();
		}

		// loop through all spikes of the list
		short* p_spike_data;
		auto i_last_spike = p_spikelist_->GetTotalSpikes() - 1;
		auto i_first_spike = 0;
		if (m_rangemode == RANGE_INDEX)
		{
			i_last_spike = m_spklast;
			i_first_spike = m_spkfirst;
		}
		auto selected_pen_color = BLACK_COLOR;
		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
			selected_pen_color = SILVER_COLOR;
		const auto old_pen = p_dc->SelectObject(&m_penTable[selected_pen_color]);

		for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
		{
			Spike* spike = p_spikelist_->GetSpike(i_spike);

			// skip spike ?
			if (m_rangemode == RANGE_TIMEINTERVALS
				&& (spike->get_time() < m_lFirst || spike->get_time() > m_lLast))
				continue;

			// select pen according to class
			const auto spike_class = spike->get_class();
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
				if (spike_class != m_selclass)
					continue;
				break;
			case PLOT_CLASSCOLORS:
				selected_pen_color = spike_class % NB_COLORS;
				p_dc->SelectObject(&m_penTable[selected_pen_color]);
				break;
			case PLOT_ONECLASS:
				if (spike_class == m_selclass)
					continue;
			default:
				break;
			}

			// display data
			p_spike_data = spike->GetpData();
			fillPolypointOrdinates(p_spike_data);
			p_dc->Polyline(&polypoints_[0], spike_length);
		}

		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
		{
			selected_pen_color = m_colorselected;
			if (m_plotmode == PLOT_ONECOLOR)
				selected_pen_color = m_selclass % NB_COLORS;
			p_dc->SelectObject(&m_penTable[selected_pen_color]);
			for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
			{
				Spike* spike = p_spikelist_->GetSpike(i_spike);
				// skip spike ?
				if (m_rangemode == RANGE_TIMEINTERVALS
					&& (spike->get_time() < m_lFirst || spike->get_time() > m_lLast))
					continue;

				// skip spikes with the wrong class
				if (spike->get_class() != m_selclass)
					continue;
				// display data
				p_spike_data = spike->GetpData();
				fillPolypointOrdinates(p_spike_data);
				p_dc->Polyline(&polypoints_[0], spike_length);
			}
		}

		// display selected spike
		auto i_select = -1;
		if (m_selectedspike >= 0 && (IsSpikeWithinRange(m_selectedspike)))
			i_select = m_selectedspike;
		drawSelectedSpike(i_select, p_dc);

		if (p_spikelist_->GetSpikeFlagArrayCount() > 0)
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

	if (m_ballFiles)
	{
		p_dbwave_doc_->SetDB_CurrentRecordPosition(current_file);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();
	}
}

void ChartSpikeShapeWnd::drawSelectedSpike(int no_spike, CDC* p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();
	auto rect = m_displayRect;
	p_dc->DPtoLP(rect);
	p_dc->IntersectClipRect(&rect);

	if (no_spike >= 0)
	{
		// change coordinate settings
		getExtents();
		prepareDC(p_dc);

		p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2 + m_displayRect.top);
		p_dc->SetViewportExt(m_displayRect.Width(), -m_displayRect.Height());

		// prepare pen and select pen
		const auto pensize = 2;
		CPen new_pen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
		auto* poldpen = p_dc->SelectObject(&new_pen);

		// display data
		auto* lpspk = p_spikelist_->GetSpike(no_spike)->GetpData();
		fillPolypointOrdinates(lpspk);
		p_dc->Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());

		// restore resources
		p_dc->SelectObject(poldpen);
	}
	// restore ressources
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShapeWnd::drawFlaggedSpikes(CDC* pDC0)
{
	ASSERT(pDC0 != NULL);
	auto p_dc = pDC0;
	const auto n_saved_dc = p_dc->SaveDC();

	// change coordinate settings
	getExtents();
	prepareDC(p_dc);
	p_dc->SetViewportOrg(m_displayRect.left, m_displayRect.Height() / 2);
	p_dc->SetViewportExt(m_displayRect.right, -m_displayRect.Height());

	// prepare pen and select pen
	const auto pensize = 1;
	CPen new_pen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
	const auto oldpen = p_dc->SelectObject(&new_pen);

	// loop through all flagged spikes
	for (auto i = p_spikelist_->GetSpikeFlagArrayCount() - 1; i >= 0; i--)
	{
		const auto nospike = p_spikelist_->GetSpikeFlagArrayAt(i);
		// skip spike if not valid in this display
		if (!IsSpikeWithinRange(nospike))
			continue;
		//if (PLOT_ONECLASSONLY == m_plotmode && nospikeclass != m_selclass)
		//	continue;
		fillPolypointOrdinates(p_spikelist_->GetSpike(nospike)->GetpData());
		p_dc->Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());
	}

	// restore resources
	p_dc->SelectObject(oldpen);
	pDC0->RestoreDC(n_saved_dc);
}

void ChartSpikeShapeWnd::DisplayFlaggedSpikes(BOOL bHighLight)
{
	if (bHighLight)
		drawFlaggedSpikes(&m_PlotDC);
	Invalidate();
}

int ChartSpikeShapeWnd::DisplayExData(short* p_data, int color)
{
	// prepare array
	const auto nelements = p_spikelist_->GetSpikeLength();
	if (polypoints_.GetSize() != nelements)
	{
		polypoints_.SetSize(nelements, 2);
		initPolypointAbcissa();
	}

	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	prepareDC(&dc);
	CPen new_pen(PS_SOLID, 0, m_colorTable[color]);
	const auto oldpen = dc.SelectObject(&new_pen);
	fillPolypointOrdinates(p_data);
	dc.Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());

	dc.SelectObject(oldpen);
	return color;
}

BOOL ChartSpikeShapeWnd::IsSpikeWithinRange(int spikeno) const
{
	if (spikeno > p_spikelist_->GetTotalSpikes() - 1)
		return FALSE;
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (p_spikelist_->GetSpike(spikeno)->get_time() < m_lFirst || p_spikelist_->GetSpike(spikeno)->get_time() > m_lLast))
		return FALSE;
	if (m_rangemode == RANGE_INDEX
		&& (spikeno > m_spklast || spikeno < m_spkfirst))
		return FALSE;
	if (m_plotmode == PLOT_ONECLASSONLY
		&& (p_spikelist_->GetSpike(spikeno)->get_class() != m_selclass))
		return FALSE;
	return TRUE;
}

int ChartSpikeShapeWnd::SelectSpikeShape(int spikeno)
{
	// erase plane
	const auto oldselected = m_selectedspike;
	m_selectedspike = spikeno;
	if (!m_bUseDIB)
	{
		CClientDC dc(this);
		drawSelectedSpike(m_selectedspike, &dc);
	}
	else
	{
		if (m_PlotDC.GetSafeHdc())
			drawSelectedSpike(m_selectedspike, &m_PlotDC);
	}
	Invalidate();
	return oldselected;
}

void ChartSpikeShapeWnd::OnLButtonUp(UINT nFlags, CPoint point)
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
			ChartWnd::OnLButtonUp(nFlags, point);
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
			ChartWnd::OnLButtonUp(nFlags, point);
			postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	default:
		{
			// none of those: zoom data or  offset display
			ChartWnd::OnLButtonUp(nFlags, point);
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
			const auto jitter = 3;
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

void ChartSpikeShapeWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;
	// call base class to test for horiz cursor or XORing rectangle
	if (m_VTtags.GetNTags() > 0)
	{
		for (auto icur = m_VTtags.GetNTags() - 1; icur >= 0; icur--) // loop through all tags
			m_VTtags.SetTagPix(icur, MulDiv(m_VTtags.GetValue(icur) - m_xWO, m_xVE, m_xWE) + m_xVO);
	}

	// track rectangle or VTtag?
	ChartWnd::OnLButtonDown(nFlags, point); // capture cursor eventually
	if (m_currCursorMode != 0 || m_HCtrapped >= 0) // do nothing else if mode != 0
		return; // or any tag hit (VT, HZ) detected

	// test if mouse hit one spike
	// if hit, then tell parent to select corresp spike
	m_hitspk = hitCurveInDoc(point);
	if (m_hitspk >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF; // flag trackrect
		releaseCursor(); // release cursor capture
		if (nFlags & MK_SHIFT)
			postMyMessage(HINT_HITSPIKE_SHIFT, m_hitspk); // tell parent spike selected

		else
			postMyMessage(HINT_HITSPIKE, m_hitspk);
	}
}

void ChartSpikeShapeWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	ChartWnd::OnMouseMove(nFlags, point);
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

void ChartSpikeShapeWnd::ZoomData(CRect* rFrom, CRect* rDest)
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

void ChartSpikeShapeWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selectedspike < 0 && p_spikelist_->GetSpikeFlagArrayCount() < 1) || m_hitspk < 0)
		ChartWnd::OnLButtonDblClk(nFlags, point);
	else
	{
		if (m_selectedspike >= 0)
		{
			postMyMessage(HINT_DBLCLKSEL, m_selectedspike);
		}
		else
		{
			const auto iselectedspike = hitCurve(point);
			if (iselectedspike > 0)
				postMyMessage(HINT_DBLCLKSEL, iselectedspike);
		}
	}
}

int ChartSpikeShapeWnd::hitCurveInDoc(CPoint point)
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = p_dbwave_doc_->GetDB_NRecords();
		ncurrentfile = p_dbwave_doc_->GetDB_CurrentRecordPosition();
	}

	int result = -1;
	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			p_dbwave_doc_->SetDB_CurrentRecordPosition(ifile);
			p_dbwave_doc_->OpenCurrentSpikeFile();
			p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();
		}

		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			continue;
		}
		result = hitCurve(point);
		if (result >= 0)
			break;
	}

	if (m_ballFiles && result < 0)
	{
		p_dbwave_doc_->SetDB_CurrentRecordPosition(ncurrentfile);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();
	}

	return result;
}

int ChartSpikeShapeWnd::hitCurve(const CPoint point)
{
	auto hitspk = -1;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
	if (mouse_x < 0 || mouse_x > p_spikelist_->GetSpikeLength())
		return hitspk;
	const auto mouse_y = MulDiv(point.y - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto deltay = MulDiv(3, m_yWE, m_yVE);

	// loop through all spikes
	auto ilast = p_spikelist_->GetTotalSpikes() - 1;
	auto ifirst = 0;
	if (m_rangemode == RANGE_INDEX)
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}
	for (auto ispk = ilast; ispk >= ifirst; ispk--)
	{
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (p_spikelist_->GetSpike(ispk)->get_time() < m_lFirst
				|| p_spikelist_->GetSpike(ispk)->get_time() > m_lLast))
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
			&& p_spikelist_->GetSpike(ispk)->get_class() != m_selclass)
			continue;

		const auto val = p_spikelist_->GetSpike(ispk)->GetValueAtOffset(mouse_x);
		if (mouse_y + deltay < val && mouse_y - deltay > val)
		{
			hitspk = ispk;
			break;
		}
	}
	return hitspk;
}

void ChartSpikeShapeWnd::getExtents()
{
	const auto current_file_index = p_dbwave_doc_->GetDB_CurrentRecordPosition();
	auto file_first = current_file_index;
	auto file_last = current_file_index;
	if (m_ballFiles)
	{
		file_first = 0;
		file_last = p_dbwave_doc_->GetDB_NRecords() -1;
	}

	if (m_yWE == 1 || m_yWE == 0) // && m_yWO == 0)
	{
		for (auto file_index = file_first; file_index <= file_last; file_index++)
		{
			p_dbwave_doc_->SetDB_CurrentRecordPosition(file_index);
			p_dbwave_doc_->OpenCurrentSpikeFile();
			p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();
			if (p_spikelist_ != nullptr)
			{
				getExtentsCurrentSpkList();
				if (m_yWE != 0)
					break;
			}
		}
	}

	// exit
	p_dbwave_doc_->SetDB_CurrentRecordPosition(current_file_index);
	if (p_dbwave_doc_->OpenCurrentSpikeFile() != nullptr)
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkList_Current();
}

void ChartSpikeShapeWnd::getExtentsCurrentSpkList()
{
	if (m_yWE == 1 || m_yWE == 0)
	{
		int maxval, minval;
		p_spikelist_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = MulDiv((maxval - minval), 10, 9) + 1;
		m_yWO = maxval / 2 + minval / 2;
	}

	if (m_xWE <= 1)
	{
		m_xWE = p_spikelist_->GetSpikeLength();
		m_xWO = 0;
	}
}

void ChartSpikeShapeWnd::initPolypointAbcissa()
{
	const auto nelements = polypoints_.GetSize();
	m_xWE = nelements + 1;
	ASSERT(nelements > 0);

	for (auto i = 0; i < nelements; i++)
		polypoints_[i].x = i + 1;
}

void ChartSpikeShapeWnd::fillPolypointOrdinates(short* lpSource)
{
	auto nelements = polypoints_.GetSize();
	if (nelements == 0)
	{
		nelements = p_spikelist_->GetSpikeLength();
		ASSERT(nelements > 0);
		polypoints_.SetSize(nelements, 2);
		initPolypointAbcissa();
	}

	for (auto i = 0; i < nelements; i++, lpSource++)
		polypoints_[i].y = *lpSource;
}

void ChartSpikeShapeWnd::Print(CDC* p_dc, CRect* rect)
{
	// check if there are valid data to display
	if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		return;

	const auto old_y_vo = m_yVO;
	const auto old_y_ve = m_yVE;
	const auto old_xextent = m_xWE;
	const auto old_xorg = m_xWO;

	// size of the window
	m_yVO = rect->Height() / 2 + rect->top;
	m_yVE = -rect->Height();

	// check initial conditions
	if (m_yWE == 1) // && m_yWO == 0)
	{
		int maxval, minval;
		p_spikelist_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = maxval - minval + 1;
		m_yWO = (maxval + minval) / 2;
	}

	m_xWO = rect->left;
	m_xWE = rect->Width() - 2;

	const auto taillespk = p_spikelist_->GetSpikeLength();
	if (polypoints_.GetSize() != taillespk)
		polypoints_.SetSize(taillespk, 2);

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC();
	for (auto i = 0; i < taillespk; i++)
		polypoints_[i].x = rect->left + MulDiv(i, rect->Width(), taillespk);

	int selpen;
	switch (m_plotmode)
	{
	//case PLOT_BLACK:			selpen = BLACK_COLOR; break;
	//case PLOT_ONECLASSONLY:	selpen = BLACK_COLOR; break;
	case PLOT_ONECLASS:
		selpen = m_colorbackgr;
		break;
	case PLOT_ALLGREY:
		selpen = m_colorbackgr;
		break;
	default:
		selpen = BLACK_COLOR;
		break;
	}

	const auto old_pen = p_dc->SelectObject(&m_penTable[selpen]);
	auto ilast = p_spikelist_->GetTotalSpikes() - 1;
	auto ifirst = 0;
	if (m_rangemode == RANGE_INDEX)
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	for (auto ispk = ilast; ispk >= ifirst; ispk--)
	{
		if (m_rangemode == RANGE_INDEX && (ispk > m_spklast || ispk < m_spkfirst))
			continue;
		if (m_rangemode == RANGE_TIMEINTERVALS)
		{
			if (p_spikelist_->GetSpike(ispk)->get_time() < m_lFirst)
				continue;
			if (p_spikelist_->GetSpike(ispk)->get_time() > m_lLast)
				continue;
		}

		const auto spkcla = p_spikelist_->GetSpike(ispk)->get_class();
		if (m_plotmode == PLOT_ONECLASSONLY && spkcla != m_selclass)
			continue;
		if (m_plotmode == PLOT_ONECLASS && spkcla == m_selclass)
			continue;

		plotArraytoDC(p_dc, p_spikelist_->GetSpike(ispk)->GetpData());
	}

	// display selected class if requested by option
	if (m_plotmode == PLOT_ONECLASS)
	{
		p_dc->SelectObject(&m_penTable[m_colorselected]);
		for (auto ispk = ilast; ispk >= ifirst; ispk--)
		{
			if (m_rangemode == RANGE_TIMEINTERVALS)
			{
				const auto ltime = p_spikelist_->GetSpike(ispk)->get_time();
				if (ltime < m_lFirst || ltime > m_lLast)
					continue;
			}
			if (p_spikelist_->GetSpike(ispk)->get_class() != m_selclass)
				continue;
			plotArraytoDC(p_dc, p_spikelist_->GetSpike(ispk)->GetpData());
		}
	}

	// display selected spike
	if (m_selectedspike >= 0 && IsSpikeWithinRange(m_selectedspike))
	{
		CPen new_pen(PS_SOLID, 0, m_colorTable[m_colorselectedspike]);
		p_dc->SelectObject(&new_pen);
		plotArraytoDC(p_dc, p_spikelist_->GetSpike(m_selectedspike)->GetpData());
	}

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	m_xWE = old_xextent; 
	m_xWO = old_xorg;
	m_yVO = old_y_vo; 
	m_yVE = old_y_ve;
}

void ChartSpikeShapeWnd::plotArraytoDC(CDC* p_dc, short* pspk)
{
	const auto nelements = polypoints_.GetSize();
	for (auto i = 0; i < nelements; i++, pspk++)
	{
		auto y = *pspk;
		y = MulDiv(y - m_yWO, m_yVE, m_yWE) + m_yVO;
		polypoints_[i].y = y;
	}

	if (p_dc->m_hAttribDC == nullptr
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE))
		p_dc->Polyline(&polypoints_[0], nelements);
	else
	{
		p_dc->MoveTo(polypoints_[0]);
		for (auto i = 0; i < nelements; i++)
			p_dc->LineTo(polypoints_[i]);
	}
}

float ChartSpikeShapeWnd::GetDisplayMaxMv()
{
	getExtents();
	return (p_spikelist_->GetAcqVoltsperBin() * 1000.f * (m_yWE - m_yWO - p_spikelist_->GetAcqBinzero()));
}

float ChartSpikeShapeWnd::GetDisplayMinMv()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	getExtents();
	return (p_spikelist_->GetAcqVoltsperBin() * 1000.f * (m_yWO - m_yWE - p_spikelist_->GetAcqBinzero()));
}

float ChartSpikeShapeWnd::GetExtent_mV()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	getExtents();
	return (p_spikelist_->GetAcqVoltsperBin() * m_yWE * 1000.f);
}

float ChartSpikeShapeWnd::GetExtent_ms()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	getExtents();
	return (static_cast<float>(1000.0 * m_xWE) / p_spikelist_->GetAcqSampRate());
}

void ChartSpikeShapeWnd::MoveVTtrack(int itrack, int newval)
{
	CPoint point;
	m_ptLast.x = MulDiv(m_VTtags.GetValue(itrack) - m_xWO, m_xVE, m_xWE) + m_xVO;
	m_VTtags.SetTagVal(itrack, newval); // set new value
	point.x = MulDiv(newval - m_xWO, m_xVE, m_xWE) + m_xVO; // convert value into pixel
	XorVTtag(point.x); // xor line
}

void ChartSpikeShapeWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ChartWnd::Serialize(ar);
		polypoints_.Serialize(ar);

		ar << m_rangemode; // display range (time OR storage index)
		ar << m_lFirst; // time first (real time = index/sampling rate)
		ar << m_lLast; // time last
		ar << m_spkfirst; // index first spike
		ar << m_spklast; // index last spike
		ar << m_currentclass; // selected class (different color) (-1 = display all)
		ar << m_selectedspike; // selected spike (display differently)
		ar << m_colorselectedspike; // color selected spike (index / color table)
		ar << m_hitspk; // index spike
		ar << m_selclass; // index class selected
		ar << m_bText; // allow text default false
		ar << m_selclass; // dummy
	}
	else
	{
		ChartWnd::Serialize(ar);
		polypoints_.Serialize(ar);

		ar >> m_rangemode; // display range (time OR storage index)
		ar >> m_lFirst; // time first (real time = index/sampling rate)
		ar >> m_lLast; // time last
		ar >> m_spkfirst; // index first spike
		ar >> m_spklast; // index last spike
		ar >> m_currentclass; // selected class (different color) (-1 = display all)
		ar >> m_selectedspike; // selected spike (display differently)
		ar >> m_colorselectedspike; // color selected spike (index / color table)
		ar >> m_hitspk; // index spike
		ar >> m_selclass; // index class selected
		ar >> m_bText; // allow text default false
		ar >> m_selclass; // dummy
	}
}
