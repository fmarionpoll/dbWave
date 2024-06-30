#pragma once
#include "ChartSpike.h"
#include "dbWaveDoc.h"


class ChartSpikeXY : public ChartSpike
{
	//Construction
public:
	ChartSpikeXY();
	~ChartSpikeXY() override;

protected:
	int dot_width_ = 6;
	long x_max_ = 0;
	long x_min_ = 0;
	int y_max_ = 0;
	int y_min_ = 0;

	// Helpers and public procedures
public:
	void select_spike(const db_spike& spike_selected);
	void display_spike(const Spike* spike);
	void highlight_spike(const Spike* spike);

	void move_hz_tag(int index, int new_value);
	void move_vt_tag(int index, int new_value);

	void display_all_files(BOOL bON, CdbWaveDoc* p_document)
	{
		display_all_files_ = bON;
		dbwave_doc_ = p_document;
	}

	void plot_data_to_dc(CDC* p_dc) override;
	void zoom_data(CRect* rect_from, CRect* rect_dest) override;

protected:
	void display_spike(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const;
	void draw_spike(const Spike* spike, int color_index);

	int hit_curve(CPoint point) override;
	boolean is_spike_within_limits( const Spike* spike) const;

	void get_extents();
	void display_hz_tags(CDC* p_dc);
	void display_vt_tags(CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point) override;

	DECLARE_MESSAGE_MAP()
};
