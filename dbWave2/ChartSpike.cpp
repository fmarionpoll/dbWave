#include "StdAfx.h"
#include "ChartSpike.h"


ChartSpike::ChartSpike()
= default;

ChartSpike::~ChartSpike()
= default;

int ChartSpike::get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const
{
	const auto no_spike_class = spike->get_class_id();
	auto color = BLACK_COLOR;
	switch (plot_mode)
	{
	case PLOT_ONECLASSONLY:
	case PLOT_ONECLASS:
		if (no_spike_class != m_selected_class)
			color = SILVER_COLOR;
		break;
	case PLOT_CLASSCOLORS:
		color = no_spike_class % NB_COLORS;
		break;
	case PLOT_BLACK:
	default:
		color = BLACK_COLOR;
		break;
	}
	return color;
}

void ChartSpike::set_plot_mode(int mode, int selected_class)
{
	m_plotmode = mode;
	m_selected_class = selected_class;
}

void ChartSpike::set_source_data(SpikeList* p_spk_list, CdbWaveDoc* p_document)
{
	p_dbwave_doc = p_document;
	p_spike_doc = p_dbwave_doc->m_p_spk;
	p_spike_list = p_spk_list;
	spike_selected_.clear();
}

void ChartSpike::sub_item_draw(CDC& dc, CRect& rect)
{
	MoveWindow(rect, FALSE); 
	CDC dc_mem;
	CBitmap bmp_mem;
	dc_mem.CreateCompatibleDC(&dc);
	bmp_mem.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* p_old_bmp = dc_mem.SelectObject(&bmp_mem);

	// draw actual data
	SetDisplayAreaSize(rect.Width(), rect.Height());
	PlotDataToDC(&dc_mem); //plot_spikes(&dc_mem);

	// transfer data to DC and clean the memory DC
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dc_mem, 0, 0, SRCCOPY);
	dc_mem.SelectObject(p_old_bmp);
	dc_mem.DeleteDC(); 
}

void ChartSpike::sub_item_create(CWnd* parentWnd, const CRect& rect, int i_id, int i_class, CdbWaveDoc* pdbDoc, SpikeList* spike_list)
{
	Create(_T(""), WS_CHILD | WS_VISIBLE, rect, parentWnd, i_id);
	const auto n_spikes = spike_list->get_spikes_count();

	set_source_data(spike_list, pdbDoc);
	set_plot_mode(PLOT_ONECLASSONLY, i_class);
	set_range_mode(RANGE_INDEX);
	set_spk_indexes(0, n_spikes - 1);
	SetbDrawframe(TRUE);
	SetCursorMaxOnDblClick(m_cursorIndexMax);
}

boolean ChartSpike::is_spike_within_range(const Spike_selected& spike_selected) const
{
	if (m_range_mode == RANGE_INDEX
		&& (spike_selected.spike_index > m_index_last_spike || spike_selected.spike_index < m_index_first_spike))
		return false;

	const auto spike = p_dbwave_doc->get_spike(spike_selected);
	if (spike == nullptr)
		return false;

	const auto ii_time = spike->get_time();
	if (m_range_mode == RANGE_TIMEINTERVALS
		&& (ii_time < m_lFirst || ii_time > m_lLast))
		return false;

	if (m_plotmode == PLOT_ONECLASSONLY
		&& spike->get_class_id() != m_selected_class)
		return false;

	return true;
}
