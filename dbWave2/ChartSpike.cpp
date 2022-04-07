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
