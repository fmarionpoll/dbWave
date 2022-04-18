#include "StdAfx.h"
#include "ChartSpike.h"


ChartSpike::ChartSpike()
= default;

ChartSpike::~ChartSpike()
= default;

int ChartSpike::get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const
{
	const auto no_spike_class = spike->get_class();
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
	p_spike_doc = p_dbwave_doc->m_pSpk;
	p_spike_list = p_spk_list;
	m_selected_spike = -1;
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
	plot_spikes(&dc_mem);

	// transfer data to DC and clean the memory DC
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dc_mem, 0, 0, SRCCOPY);
	dc_mem.SelectObject(p_old_bmp);
	dc_mem.DeleteDC(); 
}

void ChartSpike::plot_spikes(CDC* p_dc)
{
	
}


void ChartSpike::sub_item_create(CWnd* parentWnd, const CRect& rect, int i_id, int i_class, CdbWaveDoc* pdbDoc, SpikeList* spike_list)
{
	Create(_T(""), WS_CHILD | WS_VISIBLE, rect, parentWnd, i_id);
	const auto n_spikes = spike_list->get_spikes_count();

	set_source_data(spike_list, pdbDoc);
	set_plot_mode(PLOT_ONECLASSONLY, i_class);
	SetRangeMode(RANGE_INDEX);
	SetSpkIndexes(0, n_spikes - 1);
	SetbDrawframe(TRUE);
	SetCursorMaxOnDblClick(m_cursorIndexMax);
}
