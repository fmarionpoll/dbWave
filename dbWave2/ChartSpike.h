#pragma once
#include "ChartWnd.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "Spikelist.h"

class ChartSpike : public ChartWnd
{
public:
	ChartSpike();
	~ChartSpike() override;

protected:
	int m_range_mode = RANGE_TIMEINTERVALS;	// display range
	long m_lFirst = 0;						// time index of first pt displayed
	long m_lLast = 0;						// time index of last pt displayed
	int m_index_first_spike = 0;						// index first spike
	int m_index_last_spike = 0;						// index last spike
	int m_current_class = 0;					// current class in case of displaying classes
	int m_selected_spike = -1;				// selected spike (display differently)
	int m_selected_pen{};
	int m_hit_spike = -1;						// no of spike selected
	int m_selected_class{};						// index class selected
	BOOL m_track_curve = false;				// track curve ?
	BOOL m_display_all_files = false;				// display data from all files in CdbWaveDoc*

	SpikeList* p_spike_list = nullptr;
	CSpikeDoc* p_spike_doc = nullptr;
	CdbWaveDoc* p_dbwave_doc = nullptr;

	int get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const;

public:
	void SetRangeMode(int mode) { m_range_mode = mode; }
	int GetRangeMode() const { return m_range_mode; }

	void SetSelClass(int selected_class) { m_selected_class = selected_class; }
	int GetSelClass() const { return m_selected_class; }

	void SetPlotMode(int mode, int selected_class)
	{
		m_plotmode = mode;
		m_selected_class = selected_class;
	}
	int GetPlotMode() const { return m_plotmode; }

	void SetCurrentClass(int current_class) { m_current_class = current_class; }
	int GetCurrentClass() const { return m_current_class; }

	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }
	void SetTimeIntervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}
};
