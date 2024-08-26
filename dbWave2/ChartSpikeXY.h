#pragma once
#include "ChartSpike.h"
#include "dbWaveDoc.h"


class ChartSpikeXY : public ChartSpike
{
	//Construction
public:
	ChartSpikeXY();
	~ChartSpikeXY() override;
	void plot_data_to_dc_prepare_dc(CDC* p_dc);

protected:
	int dot_width_ = 6;
	long x_max_ = 0;
	long x_min_ = 0;
	int y_max_ = 0;
	int y_min_ = 0;

	void display_spike_measure(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, long window_duration) const;
	void draw_spike_measure(const Spike* spike, int color_index);

	int hit_curve(CPoint point) override;
	boolean is_spike_within_limits(const Spike* spike) const;

	void get_extents();

	// Helpers and public procedures
public:
	void select_spike_measure(const db_spike& spike_selected);
	void display_spike_measure(const Spike* spike);
	void highlight_spike_measure(const Spike* spike);

	void move_hz_tag(int tag_index, int value);
	void move_vt_tag(int tag_index, int new_value);

	void display_all_files(const BOOL b_on, CdbWaveDoc* p_document)
	{
		b_display_all_files_ = static_cast<boolean>(b_on);
		dbwave_doc_ = p_document;
	}

	void plot_data_to_dc(CDC* p_dc) override;
	void zoom_data(CRect* rect_from, CRect* rect_dest) override;

protected:
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) override;

	DECLARE_MESSAGE_MAP()
};
