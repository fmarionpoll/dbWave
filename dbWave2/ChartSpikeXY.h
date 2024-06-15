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
	long time_max_ = 0;
	long time_min_ = 0;
	int value_max_ = 0;
	int value_min_ = 0;

	// Helpers and public procedures
public:
	int get_selected_spike() const { return m_selected_spike; }
	int get_hit_spike() const { return m_hit_spike; }

	int select_spike(const int spike_no);
	void display_spike(const int spike_no, const BOOL b_select);
	BOOL is_spike_within_range(int spike_no) const;
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
	void display_spike(Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const;
	void draw_selected_spike(int no_spike, int color, CDC* p_dc) const;
	int hitCurve(CPoint point) override;
	int hit_curve_in_doc(CPoint point);
	BOOL is_spike_within_limits(int no_spike);
	void get_extents();
	void highlight_one_point(int no_spike, CDC* p_dc) const;
	void display_hz_tags(CDC* p_dc);
	void display_vt_tags(CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
