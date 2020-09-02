// spikexyp.cpp Implementation File

#include "StdAfx.h"
#include "scopescr.h"
#include "Spikedoc.h"
#include "spikexyp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_ballfiles is true: display and spike hit

BEGIN_MESSAGE_MAP(CSpikeXYpWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

CSpikeXYpWnd::CSpikeXYpWnd()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (spikexp)");
	p_spikelist_ = nullptr;
}

CSpikeXYpWnd::~CSpikeXYpWnd()
= default;

void CSpikeXYpWnd::PlotDatatoDC(CDC * p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// save context
	const auto n_saved_dc = p_dc->SaveDC();
	const auto bkcolor = p_dc->GetBkColor();

	// display data: trap error conditions
	const auto windowduration = m_lLast - m_lFirst + 1;	// abcissa size
	GetExtents();								// get origin and extents for xy axes
	ASSERT(m_xWE != 1);
	p_dc->SetMapMode(MM_TEXT);

	// prepare brush & rectangles (small for all spikes, larger for spikes belonging to the selected class)
	auto selbrush = BLACK_COLOR;
	auto width = m_rwidth / 2;
	rect = CRect(-width, -width, width, width);
	width = m_rwidth * 2 / 3;
	const CRect rect1(-width, -width, width, width);
	CRect recti;

	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = p_dbwave_doc_->GetDB_NRecords();
		ncurrentfile = p_dbwave_doc_->GetDB_CurrentRecordPosition();
	}

	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			p_dbwave_doc_->SetDB_CurrentRecordPosition(ifile);
			p_dbwave_doc_->OpenCurrentSpikeFile();
		}
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();

		// test presence of data
		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			if (!m_ballFiles)
				p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT);
			continue;
		}

		auto ilast = p_spikelist_->GetTotalSpikes() - 1;
		auto ifirst = 0;
		if (m_rangemode == RANGE_INDEX)
		{
			if (m_spklast > ilast)
				m_spklast = ilast;
			if (m_spkfirst < 0)
				m_spkfirst = 0;
			ilast = m_spklast;
			ifirst = m_spkfirst;
		}

		// loop over all spikes
		for (auto ispk = ilast; ispk >= ifirst; ispk--)
		{
			// check that spike fits within time limits of the display
			const auto spike_element = p_spikelist_->GetSpikeElemt(ispk);
			const auto l_spike_time = spike_element->get_time();
			if (m_rangemode == RANGE_TIMEINTERVALS
				&& (l_spike_time < m_lFirst || l_spike_time > m_lLast))
				continue;

			// select correct brush
			const auto wspkcla = spike_element->get_class();
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
				if (wspkcla != m_selclass)
					continue;
				break;
			case PLOT_CLASSCOLORS:
				selbrush = wspkcla % NB_COLORS;
				break;
			case PLOT_ONECLASS:
				if (wspkcla != m_selclass)
					selbrush = SILVER_COLOR;
				else
					selbrush = m_colorselected;
			default:
				break;
			}
			// adjust rectangle size
			if (wspkcla == m_selclass)
				recti = rect1;
			else
				recti = rect;

			// draw point
			const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, windowduration) + m_xVO;
			const auto y1 = MulDiv(spike_element->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;
			recti.OffsetRect(x1, y1);
			p_dc->MoveTo(x1, y1);
			p_dc->FillSolidRect(&recti, m_colorTable[selbrush]);
		}
		p_dc->SetBkColor(bkcolor);		// restore background color

		// display spike selected
		if (m_selectedspike >= 0)
			HighlightOnePoint(m_selectedspike, p_dc);

		if (p_spikelist_->GetSpikeFlagArrayCount() > 0)
		{
			// loop over the array of flagged spikes
			for (auto i = p_spikelist_->GetSpikeFlagArrayCount() - 1; i >= 0; i--)
			{
				const auto nospike = p_spikelist_->GetSpikeFlagArrayAt(i);
				HighlightOnePoint(nospike, p_dc);
			}
		}

		//display HZ cursors
		if (GetNHZtags() > 0)
		{
			//DisplayHZtags(p_dc);
			// select pen and display mode
			const auto pold = p_dc->SelectObject(&m_blackDottedPen);
			const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);

			// iterate through HZ cursor list
			const auto x1 = m_displayRect.left;
			const auto x2 = m_displayRect.right;
			for (auto i = GetNHZtags() - 1; i >= 0; i--)
			{
				const auto k = GetHZtagVal(i);		// get val
				const auto y1 = MulDiv(k - m_yWO, m_yVE, m_yWE) + m_yVO;
				p_dc->MoveTo(x1, y1);		// set initial pt
				p_dc->LineTo(x2, y1);		// HZ line
			}
			p_dc->SelectObject(pold);
			p_dc->SetROP2(nold_rop);			// restore old display mode
		}

		// display VT cursors
		if (GetNVTtags() > 0)		// display vertical tags
		{
			// select pen and display mode
			const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
			const auto oldp = p_dc->SelectObject(&m_blackDottedPen);
			m_xWO = m_lFirst;
			m_xWE = m_lLast - m_lFirst + 1;

			// iterate through VT cursor list
			const auto y0 = m_displayRect.top;
			const auto y1 = m_displayRect.bottom;
			for (auto j = GetNVTtags() - 1; j >= 0; j--)
			{
				const auto val = GetVTtagVal(j);			// get val
				const auto pix_x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
				p_dc->MoveTo(pix_x, y0);			// set initial pt
				p_dc->LineTo(pix_x, y1);			// VT line
			}
			p_dc->SelectObject(oldp);
			p_dc->SetROP2(nold_rop);
		}
	}

	// restore resources
	p_dc->RestoreDC(n_saved_dc);
	// restore selection to initial file
	if (m_ballFiles)
	{
		p_dbwave_doc_->SetDB_CurrentRecordPosition(ncurrentfile);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
	}
}

BOOL CSpikeXYpWnd::IsSpikeWithinRange(int spikeno)
{
	if (p_spikelist_->GetTotalSpikes() < 1)
		return FALSE;

	const auto spike_element = p_spikelist_->GetSpikeElemt(spikeno);
	const auto iitime = spike_element->get_time();
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (iitime < m_lFirst || iitime > m_lLast))
		return FALSE;

	if (m_rangemode == RANGE_INDEX
		&& (spikeno > m_spklast || spikeno < m_spkfirst))
		return FALSE;

	if (m_plotmode == PLOT_ONECLASSONLY
		&& spike_element->get_class() != m_selclass)
		return FALSE;

	return TRUE;
}

void CSpikeXYpWnd::DisplaySpike(int spikeno, BOOL bselect)
{
	if (!IsSpikeWithinRange(spikeno))
		return;

	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	const auto spike_element = p_spikelist_->GetSpikeElemt(spikeno);
	const auto spike_class = spike_element->get_class();
	int color;
	if (!bselect)
	{
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
		case PLOT_ONECLASS:
			color = BLACK_COLOR;			// Black
			if (spike_class != m_selclass)
				color = SILVER_COLOR;		// LTgrey
			break;
		case PLOT_CLASSCOLORS:
			if (spikeno == m_selectedspike)
				HighlightOnePoint(spikeno, &dc);
			color = spike_class % 8;
			break;
		case PLOT_BLACK:
		default:
			color = BLACK_COLOR;	// Black
			break;
		}
	}
	else
	{
		color = RED_COLOR;
		if (m_plotmode == PLOT_CLASSCOLORS)
		{
			HighlightOnePoint(spikeno, &dc);
			color = spike_class % 8;
		}
	}

	// display spike
	DrawSelectedSpike(spikeno, color, &dc);
}

void CSpikeXYpWnd::HighlightOnePoint(int nospike, CDC * p_dc)
{
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	const auto spike_element = p_spikelist_->GetSpikeElemt(nospike);
	const auto l_spike_time = spike_element->get_time();
	const auto windowduration = m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, windowduration) + m_xVO;
	const auto y1 = MulDiv(spike_element->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196, 2, 51)); //RGB(255, 255, 255));
	auto* oldpen = (CPen*)p_dc->SelectObject(&new_pen);

	const auto width = m_rwidth * 2 / 3 + 2;
	CRect rect1(-width, -width, width, width);
	rect1.OffsetRect(x1 - 1, y1 - 1);
	p_dc->MoveTo(rect1.left, rect1.top);
	p_dc->LineTo(rect1.right, rect1.top);
	p_dc->LineTo(rect1.right, rect1.bottom);
	p_dc->LineTo(rect1.left, rect1.bottom);
	p_dc->LineTo(rect1.left, rect1.top);

	// restore resources
	p_dc->SelectObject(oldpen);
	p_dc->SetROP2(nold_rop);
}

void CSpikeXYpWnd::DrawSelectedSpike(int nospike, int color, CDC * p_dc)
{
	const auto spike_element = p_spikelist_->GetSpikeElemt(nospike);
	const auto l_spike_time = spike_element->get_time();
	const auto windowduration = m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, windowduration) + m_xVO;
	const auto y1 = MulDiv(spike_element->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;
	CRect rect(0, 0, m_rwidth, m_rwidth);
	rect.OffsetRect(x1 - m_rwidth / 2, y1 - m_rwidth / 2);

	const auto bkcolor = p_dc->GetBkColor();
	p_dc->MoveTo(x1, y1);
	p_dc->FillSolidRect(&rect, m_colorTable[color]);
	p_dc->SetBkColor(bkcolor);
}

void CSpikeXYpWnd::MoveHZtagtoVal(int i, int val)
{
	m_ptLast.y = MulDiv(GetHZtagVal(i) - m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto ypix = MulDiv(val - m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(ypix);
	SetHZtagVal(i, val);
}

void CSpikeXYpWnd::MoveVTtagtoVal(int i, int val)
{
	m_ptLast.x = MulDiv(GetVTtagVal(i) - m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto xpix = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(xpix);
	SetVTtagVal(i, val);
}

int	CSpikeXYpWnd::SelectSpike(int spikeno)
{
	// erase old selected spike (eventually)
	const auto oldselected = m_selectedspike;
	if (m_selectedspike >= 0) // && m_selectedspike != spikeno)
		DisplaySpike(m_selectedspike, FALSE);

	if (spikeno >= 0)
		DisplaySpike(spikeno, TRUE);
	m_selectedspike = spikeno;

	return oldselected;
}

void CSpikeXYpWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch (m_trackMode)
	{
	case TRACK_HZTAG:
	{
		// convert pix into data value
		const auto val = MulDiv(m_ptLast.y - m_yVO, m_yWE, m_yVE) + m_yWO;
		SetHZtagVal(m_HCtrapped, val);
		point.y = MulDiv(val - m_yWO, m_yVE, m_yWE) + m_yVO;
		XorHZtag(point.y);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEHZTAG, m_HCtrapped);
	}
	break;

	case TRACK_VTTAG:
	{
		// convert pix into data value
		const auto val = MulDiv(m_ptLast.x - m_xVO, m_xWE, m_xVE) + m_xWO;
		SetVTtagVal(m_HCtrapped, val);
		point.x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
		XorVTtag(point.x);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
	}
	break;

	default:
	{
		CScopeScreen::OnLButtonUp(nFlags, point);
		CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const auto jitter = 3;
		if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
		{
			if (m_cursorType != CURSOR_ZOOM)
				PostMyMessage(HINT_HITAREA, NULL);
			else
				ZoomIn();
			return;					// exit: mouse movement was too small
		}

		// perform action according to cursor type
		auto rect_in = m_displayRect;
		switch (m_cursorType)
		{
		case 0:
			rect_out = rect_in;
			rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
			ZoomData(&rect_in, &rect_out);
			break;
		case CURSOR_ZOOM: 	// zoom operation
			ZoomData(&rect_in, &rect_out);
			m_ZoomFrom = rect_in;
			m_ZoomTo = rect_out;
			m_iUndoZoom = 1;
			break;
		default:
			break;
		}
	}
	break;
	}
}

void CSpikeXYpWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// compute pixel position of tags
	if (GetNHZtags() > 0)
	{
		for (auto icur = GetNHZtags() - 1; icur >= 0; icur--)
			SetHZtagPix(icur, MulDiv(GetHZtagVal(icur) - m_yWO, m_yVE, m_yWE) + m_yVO);
	}
	if (GetNVTtags() > 0)
	{
		for (auto icur = GetNVTtags() - 1; icur >= 0; icur--)
		{
			const auto val = GetVTtagVal(icur);
			const auto pix = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			SetVTtagPix(icur, pix);
		}
	}

	// track rectangle or HZtag?
	CScopeScreen::OnLButtonDown(nFlags, point);
	if (m_currCursorMode != 0 || m_HCtrapped >= 0)// do nothing else if mode != 0
	{
		if (m_trackMode == TRACK_HZTAG || m_trackMode == TRACK_VTTAG)
			return;	 								// or any tag hit (VT, HZ) detected
	}

	// test if mouse hit a spike
	m_hitspk = DoesCursorHitCurveInDoc(point);
	if (m_hitspk >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF;		// flag trackrect
		ReleaseCursor();				// release cursor capture
		if (nFlags & MK_SHIFT)
			PostMyMessage(HINT_HITSPIKE_SHIFT, m_hitspk);

		else
			PostMyMessage(HINT_HITSPIKE, m_hitspk);
	}
}

//---------------------------------------------------------------------------
// ZoomData()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=yextent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void CSpikeXYpWnd::ZoomData(CRect * rFrom, CRect * rDest)
{
	rFrom->NormalizeRect();	// make sure that rect is not inverted
	rDest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_yWE;				// save previous window extent
	m_yWE = MulDiv(m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
		- MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
		+ MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed
	auto l_size = m_lLast - m_lFirst + 1;
	m_lFirst = m_lFirst + l_size * (rDest->left - rFrom->left) / rFrom->Width();
	l_size = l_size * rDest->Width() / rFrom->Width();
	m_lLast = m_lFirst + l_size - 1;
	// display
	Invalidate();
	PostMyMessage(HINT_CHANGEHZLIMITS, NULL);
}

void CSpikeXYpWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_selectedspike < 0 || m_hitspk < 0)
		CScopeScreen::OnLButtonDblClk(nFlags, point);
	else
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED), reinterpret_cast<LPARAM>(m_hWnd));
}

int CSpikeXYpWnd::DoesCursorHitCurveInDoc(CPoint point)
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
			p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
		}

		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			continue;
		}
		result = DoesCursorHitCurve(point);
		if (result >= 0)
			break;
	}

	if (m_ballFiles && result < 0)
	{
		p_dbwave_doc_->SetDB_CurrentRecordPosition(ncurrentfile);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
	}

	return result;
}

int CSpikeXYpWnd::DoesCursorHitCurve(CPoint point)
{
	// abcissa
	const auto taille = (m_lLast - m_lFirst + 1);
	time_max_ = m_lFirst + taille * (point.x + m_rwidth) / static_cast<long>(m_xVE);
	time_min_ = m_lFirst + taille * (point.x - m_rwidth) / static_cast<long>(m_xVE);
	// ordinates
	value_max_ = MulDiv(point.y - m_rwidth - m_yVO, m_yWE, m_yVE) + m_yWO;
	value_min_ = MulDiv(point.y + m_rwidth - m_yVO, m_yWE, m_yVE) + m_yWO;

	// first look at black spikes (foreground)
	int ispk;
	const auto upperbound = p_spikelist_->GetTotalSpikes() - 1;
	if (m_plotmode == PLOT_ONECLASS)
	{
		for (ispk = upperbound; ispk >= 0; ispk--)
		{
			// skip non selected class
			const auto spike_element = p_spikelist_->GetSpikeElemt(ispk);
			if (spike_element->get_class() != m_selclass)
				continue;
			if (is_spike_within_limits(ispk))
				return ispk;
		}
	}

	// then look through all other spikes
	for (ispk = upperbound; ispk >= 0; ispk--)
	{
		if (is_spike_within_limits(ispk))
			return ispk;
	}

	// none found
	return -1;
}

BOOL CSpikeXYpWnd::is_spike_within_limits(const int ispike)
{
	const auto spike_element = p_spikelist_->GetSpikeElemt(ispike);
	const auto l_spike_time = spike_element->get_time();
	if (l_spike_time < time_min_ || l_spike_time > time_max_)
		return false;

	const auto val = spike_element->get_y1();
	if (val < value_min_ || val > value_max_)
		return false;
	return true;
}

void CSpikeXYpWnd::GetExtents()
{
	if (m_yWE == 1) // && m_yWO == 0)
	{
		auto maxval = 4096;
		auto minval = 0;
		if (p_spikelist_ != nullptr)
		{
			const auto upperbound = p_spikelist_->GetTotalSpikes() - 1;
			maxval = p_spikelist_->GetSpikeElemt(upperbound)->get_y1();
			minval = maxval;
			for (auto i = upperbound; i >= 0; i--)
			{
				const auto val = p_spikelist_->GetSpikeElemt(i)->get_y1();
				if (val > maxval) maxval = val;
				if (val < minval) minval = val;
			}
		}
		m_yWE = maxval - minval + 2;
		m_yWO = (maxval + minval) / 2;
	}

	if (m_xVE == 1 && m_xVO == 0)
	{
		m_xVE = m_displayRect.Width();
		m_xVO = m_displayRect.left;
	}
	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_displayRect.Width();
		m_xWO = m_displayRect.left;
	}
}