#pragma once
#include "ChartSpike.h"


//constexpr auto DISPLAY_TIME_INTERVAL = 0; // mask: ON: display spikes within spk index range/OFF=within time interval;
//constexpr auto DISPLAY_INDEX_RANGE = 1; // mask: ON: display spikes within spk index range/OFF=within time interval;
//constexpr auto DISPLAY_SELECTED_CLASS = 2; // mask: ON: display only selected spike/OFF=all classes

class ChartSpikeShape : public ChartSpike
{
public:
	ChartSpikeShape();
	~ChartSpikeShape() override;
	DECLARE_SERIAL(ChartSpikeShape)
	void Serialize(CArchive& ar) override;

protected:
	int color_selected_spike_ = RED_COLOR; 
	BOOL b_text_ = FALSE;
	int n_displayed_spikes_{};

	CArray<CPoint, CPoint> polyline_points_;
	void init_polypoint_x_axis();
	void fill_polypoint_y_axis(short* lp_source);

	int saved_dc_;
	COLORREF saved_background_color_;

	void plot_data_to_dc_prepare_dc(CDC* p_dc);
	void message_no_spike(CDC* p_dc) const;
	boolean get_spike_file(int i_file);

	int hit_curve(CPoint point) override;

	void draw_spike_on_dc(const Spike* spike, CDC* p_dc);
	void draw_flagged_spikes(CDC* p_dc);
	void plot_array_to_dc(CDC* p_dc, short* p_array);

public:
	void draw_spike(const Spike* spike);

	void set_text_option(const BOOL b_text) { b_text_ = b_text; }
	BOOL get_text_option() const { return b_text_; }

	float get_display_max_mv();
	float get_display_min_mv();
	float get_extent_mv();
	float get_extent_ms();
	void get_extents_current_spk_list();
	void get_extents();

	void display_all_files(const BOOL b_on, CdbWaveDoc* p_document)
	{
		display_all_files_ = static_cast<boolean>(b_on);
		dbwave_doc_ = p_document;
	}

	void print(CDC* p_dc, const CRect* rect);
	void move_vt_track(int i_track, int new_value);
	void select_spike(const db_spike& spike_sel);
	int display_ex_data(short* p_data, int color = 9);
	void display_flagged_spikes(BOOL b_highlight);

	void plot_data_to_dc(CDC* p_dc) override;
	void display_spike_data(CDC* p_dc, const Spike* spike, const int spike_length);
	void zoom_data(CRect* r_from, CRect* r_dest) override;

protected:
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) override;

	DECLARE_MESSAGE_MAP()
};
