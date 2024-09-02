#pragma once

#include "ChartSpike.h"
#include "dbWaveDoc.h"

class ChartSpikeBar : public ChartSpike
{
public:
	ChartSpikeBar();
	~ChartSpikeBar();

	void plot_data_to_dc_prepare_dc(CDC* p_dc);

	DECLARE_SERIAL(ChartSpikeBar)
	void Serialize(CArchive& archive);

protected:
	BOOL b_plot_spikes_ = FALSE; // FALSE=bars; TRUE=spike shapes
	int bar_height_ = 10;
	CDWordArray* p_envelope_ = nullptr; 

	// Helpers
public:
	void select_spike(const db_spike& new_spike_selected);
	void display_spike(const Spike* spike);
	void highlight_spike(const Spike* spike) ;

	void display_all_files(const BOOL b_on, CdbWaveDoc* p_document)
	{
		b_display_all_files_ = static_cast<boolean>(b_on);
		dbwave_doc_ = p_document;
	}

	void display_flagged_spikes(BOOL b_high_light);

	void select_spikes_within_rect(CRect* p_rect, UINT n_flags) const;
	void center_curve();
	void max_gain();
	void max_center();

	void print(CDC* p_dc, const CRect* rect);
	void plot_data_to_dc(CDC* p_dc) override;
	void display_temporary_tag(CDC* p_dc);
	void display_vt_tags_long_values(CDC* p_dc);
	
	void zoom_data(CRect* prev_rect, CRect* new_rect) override;

protected:
	int hit_curve(CPoint point) override;
	void display_bars(CDC* p_dc, const CRect* rect);
	void draw_spike(const Spike* spike, const COLORREF& color);
	void draw_spike(CDC* p_dc, const Spike* spike, const COLORREF& color) const;
	void display_stimulus(CDC* p_dc, const CRect* rect) const;

	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) ;
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) ;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) ;

	DECLARE_MESSAGE_MAP()
};
