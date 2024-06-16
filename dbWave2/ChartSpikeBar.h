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
	BOOL display_acquisition_data = FALSE;			// FALSE=bars; TRUE=spikes
	int m_bar_height = 10;
	CDWordArray* p_envelope_ = nullptr; 


	// Helpers
public:
	void set_source_data(SpikeList* p_spk_list, CSpikeDoc* p_spike_document);

	int get_hit_spike() const { return m_hit_spike; }

	void display_spike(const Spike* spike, BOOL b_select);
	void display_flagged_spikes(BOOL bHighlight);
	void select_spike(const Spike_selected& new_spike_selected);

	void select_spikes_within_rect(CRect* pRect, UINT nFlags) const;
	BOOL is_spike_within_range(int no_spike);
	void center_curve();
	void max_gain();
	void max_center();

	void display_all_files(BOOL bON, CdbWaveDoc* p_document)
	{
		m_display_all_files = bON;
		p_dbwave_doc = p_document;
	}

	void Print(CDC* p_dc, const CRect* rect);
	void PlotDataToDC(CDC* p_dc) override;
	void plot_spikes(CDC* p_dc) override;
	void plot_single_spk_data_to_dc(CDC* p_dc);
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	int hitCurve(CPoint point) override;
	int hit_curve_in_doc(CPoint point);
	void display_bars(CDC* p_dc, const CRect* rect);
	void highlight_one_bar(const Spike* spike, CDC* p_dc) const;
	void display_stimulus(CDC* p_dc, const CRect* rect) const;

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
