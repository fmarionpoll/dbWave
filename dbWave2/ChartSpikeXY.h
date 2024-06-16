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
	int m_dot_width = 6;
	long x_max_ = 0;
	long x_min_ = 0;
	int y_max_ = 0;
	int y_min_ = 0;

	// Helpers and public procedures
public:
	int get_hit_spike() const { return m_hit_spike; }

	void select_spike(const Spike_selected& spike_selected);
	void display_spike(Spike_selected& spike_selected, const BOOL b_select);
	void move_hz_tag(int index, int new_value);
	void move_vt_tag(int index, int new_value);

	void display_all_files(BOOL bON, CdbWaveDoc* p_document)
	{
		m_display_all_files = bON;
		p_dbwave_doc = p_document;
	}

	void PlotDataToDC(CDC* p_dc) override;
	void ZoomData(CRect* rect_from, CRect* rect_dest) override;

protected:
	void display_spike(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const;
	void draw_selected_spike(Spike* spike, int color, CDC* p_dc) const;
	int hitCurve(CPoint point) override;
	int hit_curve_in_doc(CPoint point);
	boolean is_spike_within_limits( const Spike* spike) const;
	boolean is_spike_within_range(const Spike_selected& spike_selected) const;
	void get_extents();
	void highlight_one_point(Spike* spike, CDC* p_dc) const;
	void display_hz_tags(CDC* p_dc);
	void display_vt_tags(CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
