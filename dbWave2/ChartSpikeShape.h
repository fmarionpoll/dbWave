#pragma once
#include "ChartSpike.h"


constexpr auto DISPLAY_TIME_INTERVAL = 0; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISPLAY_INDEX_RANGE = 1; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISPLAY_SELECTED_CLASS = 2; // mask: ON: display only selected spike/OFF=all classes

class ChartSpikeShape : public ChartSpike
{
public:
	ChartSpikeShape();
	~ChartSpikeShape() override;
	DECLARE_SERIAL(ChartSpikeShape)
	void Serialize(CArchive& ar) override;

protected:
	int m_color_selected_spike = RED_COLOR; // color selected spike (index / color table)
	BOOL m_bText = FALSE; // allow text default false
	int m_n_displayed_spikes{}; // number of spikes displayed
	CArray<CPoint, CPoint> polyline_points_; // points displayed with polyline

public:
	void draw_spike(const Spike* spike);

	void set_text_option(const BOOL b_text) { m_bText = b_text; }
	BOOL get_text_option() const { return m_bText; }

	float get_display_max_mv();
	float get_display_min_mv();
	float get_extent_m_v();
	float get_extent_ms();
	int get_hit_spike() const { return m_hit_spike; }

	void display_all_files(const BOOL b_on, CdbWaveDoc* p_document)
	{
		m_display_all_files = b_on;
		p_dbwave_doc = p_document;
	}

	// non-inline functions
	void move_vt_track(int i_track, int new_value);
	void select_spike(const dbSpike& spike_sel);
	int display_ex_data(short* p_data, int color = 9);
	void display_flagged_spikes(BOOL bHighLight);

	void plot_data_to_dc(CDC* p_dc) override;
	void ZoomData(CRect* rFrom, CRect* rDest) override;
	void Print(CDC* p_dc, const CRect* rect);

protected:
	void init_polypoint_x_axis();
	void fill_polypoint_y_axis(short* lp_source);
	void get_extents();
	void get_extents_current_spk_list();

	int hit_curve(CPoint point) override;
	int hit_curve_in_doc(CPoint point);

	void draw_spike_on_dc(const Spike* spike, CDC* p_dc);
	void draw_flagged_spikes(CDC* p_dc);
	void plot_array_to_dc(CDC* p_dc, short* p_array);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
