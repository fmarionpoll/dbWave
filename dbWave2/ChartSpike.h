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
	int range_mode_ = RANGE_TIME_INTERVALS;	// display range
	long l_first_ = 0;						// time index of first pt displayed
	long l_last_ = 0;						// time index of last pt displayed
	int index_first_spike_ = 0;				// index first spike
	int index_last_spike_ = 0;				// index last spike
	int current_class_ = 0;					// current class in case of displaying classes
	db_spike spike_selected_ {};
	db_spike spike_hit_ {};

	int selected_pen_{};
	int selected_class_{};					// index class selected
	boolean track_curve_ = false;			// track curve ?
	boolean display_all_files_ = false;		// display data from all files in CdbWaveDoc*

	SpikeList* p_spike_list_ = nullptr;
	CSpikeDoc* p_spike_doc_ = nullptr;
	CdbWaveDoc* dbwave_doc_ = nullptr;

	int get_color_according_to_plot_mode(const Spike* spike, int plot_mode) const;

public:
	SpikeList* get_spike_list() { return p_spike_list_; }
	CSpikeDoc* get_spike_doc() const { return p_spike_doc_; }
	CdbWaveDoc* get_db_wave_doc() const { return dbwave_doc_; }

	db_spike get_hit_spike() const { return spike_hit_; }
	db_spike get_selected_spike() const { return spike_selected_; }
	boolean is_spike_within_range(const db_spike& spike_selected) const;
	db_spike hit_curve_in_doc(CPoint point);

	void set_range_mode(const int mode) { range_mode_ = mode; }
	int get_range_mode() const { return range_mode_; }

	void set_selected_class(const int selected_class) { selected_class_ = selected_class; }
	int get_selected_class() const { return selected_class_; }

	void set_plot_mode(int mode, int selected_class);
	int get_plot_mode() const { return plot_mode_; }

	void set_current_class(const int current_class) { current_class_ = current_class; }
	int get_current_class() const { return current_class_; }

	long get_time_first() const { return l_first_; }
	long get_time_last() const { return l_last_; }
	void set_time_intervals(const long l_first, const long l_last)
	{
		l_first_ = l_first;
		l_last_ = l_last;
	}

	void set_spike_list(SpikeList* p_spk_list) { p_spike_list_ = p_spk_list; }
	SpikeList* get_spike_list() const {return p_spike_list_; }

	void set_source_data(SpikeList* p_spk_list, CdbWaveDoc* p_document);
	void set_spk_indexes(const int first, const int last) { index_first_spike_ = first, index_last_spike_ = last; }

	void sub_item_draw(CDC& dc, CRect& rect);
	void sub_item_create(CWnd* parent_wnd, const CRect& rect, int i_id, int i_class, CdbWaveDoc* pdb_doc, SpikeList* spike_list);
};
