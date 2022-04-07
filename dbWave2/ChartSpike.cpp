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
