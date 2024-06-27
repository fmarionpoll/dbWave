#pragma once

#include "ChartSpike.h"
#include "dbWaveDoc.h"

class ChartSpikeBar : public ChartSpike
{
public:
	ChartSpikeBar();
	~ChartSpikeBar() override;
	DECLARE_SERIAL(ChartSpikeBar)
	void Serialize(CArchive& archive) override;

protected:
	BOOL display_acquisition_data_ = FALSE;			// FALSE=bars; TRUE=spikes
	int bar_height_ = 10;
	CDWordArray* p_envelope_ = nullptr; 

	// Helpers
public:
	void select_spike(const db_spike& new_spike_selected);
	void display_spike(const Spike* spike);
	void highlight_spike(const Spike* spike) ;

	void display_all_files(const BOOL b_on, CdbWaveDoc* p_document)
	{
		display_all_files_ = b_on;
		dbwave_doc_ = p_document;
	}

	void display_flagged_spikes(BOOL b_high_light);

	void select_spikes_within_rect(CRect* p_rect, UINT n_flags) const;
	void center_curve();
	void max_gain();
	void max_center();

	void print(CDC* p_dc, const CRect* rect);
	void plot_data_to_dc(CDC* p_dc) override;
	void plot_single_spk_data_to_dc(CDC* p_dc);
	void zoom_data(CRect* prev_rect, CRect* new_rect) override;

protected:
	int hit_curve(CPoint point) override;
	void display_bars(CDC* p_dc, const CRect* rect);

	void draw_spike(const Spike* spike, int color_index);
	void display_stimulus(CDC* p_dc, const CRect* rect) const;

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
