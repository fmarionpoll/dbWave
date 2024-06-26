#pragma once
#include <tuple>

#include "ChartWnd.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "dbSpike.h"
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
	int m_index_first_spike = 0;			// index first spike
	int m_index_last_spike = 0;				// index last spike
	int m_current_class = 0;				// current class in case of displaying classes
	dbSpike spike_selected_;
	int m_selected_pen{};
	int m_hit_spike = -1;					// no of spike selected
	int m_selected_class{};					// index class selected
	BOOL m_track_curve = false;				// track curve ?
	BOOL m_display_all_files = false;		// display data from all files in CdbWaveDoc*

	SpikeList* p_spike_list = nullptr;
	CSpikeDoc* p_spike_doc = nullptr;
	CdbWaveDoc* p_dbwave_doc = nullptr;

	int get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const;

public:
	SpikeList* get_spike_list() { return p_spike_list; }
	CSpikeDoc* get_spike_doc() const { return p_spike_doc; }
	CdbWaveDoc* get_db_wave_doc() const { return p_dbwave_doc; }

	dbSpike* get_selected_spike() { return &spike_selected_; }
	boolean is_spike_within_range(const dbSpike& spike_selected) const;

	void set_range_mode(int mode) { m_range_mode = mode; }
	int get_range_mode() const { return m_range_mode; }

	void set_selected_class(int selected_class) { m_selected_class = selected_class; }
	int get_selected_class() const { return m_selected_class; }

	void set_plot_mode(int mode, int selected_class);
	int get_plot_mode() const { return plot_mode_; }

	void set_current_class(int current_class) { m_current_class = current_class; }
	int get_current_class() const { return m_current_class; }

	long get_time_first() const { return m_lFirst; }
	long get_time_last() const { return m_lLast; }
	void set_time_intervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}

	void set_spike_list(SpikeList* p_spk_list) { p_spike_list = p_spk_list; }
	SpikeList* get_spike_list() const {return p_spike_list; }

	void set_source_data(SpikeList* p_spk_list, CdbWaveDoc* p_document);
	void set_spk_indexes(int first, int last) { m_index_first_spike = first, m_index_last_spike = last; }

	void sub_item_draw(CDC& dc, CRect& rect);
	void sub_item_create(CWnd* parentWnd, const CRect& rect, int i_id, int i_class, CdbWaveDoc* pdbDoc, SpikeList* spike_list);
};
