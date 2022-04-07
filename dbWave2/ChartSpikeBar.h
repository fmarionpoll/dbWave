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
	void SetSourceData_spklist_spikedoc(SpikeList* p_spk_list, CSpikeDoc* p_spike_document);

	void SetSpkIndexes(int first, int last) { m_index_first_spike = first, m_index_last_spike = last; }

	int GetSelectedSpike() const { return m_selected_spike; }
	int GetHitSpike() const { return m_hit_spike; }

	void DisplaySpike(int no_spike, BOOL b_select);
	void DisplayFlaggedSpikes(BOOL bHighlight);
	int SelectSpike(int spike_no);

	void SelectSpikesWithinRect(CRect* pRect, UINT nFlags) const;
	BOOL IsSpikeWithinRange(int no_spike);
	void CenterCurve();
	void MaxGain();
	void MaxCenter();

	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document)
	{
		m_display_all_files = bON;
		p_dbwave_doc = p_document;
	}

	void Print(CDC* p_dc, CRect* rect);
	void PlotDataToDC(CDC* p_dc) override;
	void PlotSingleSpkDataToDC(CDC* p_dc);
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	int hitCurve(CPoint point) override;
	int hitCurveInDoc(CPoint point);
	void displayBars(CDC* p_dc, const CRect* rect);
	void highlightOneBar(int no_spike, CDC* p_dc) const;
	void displayStimulus(CDC* p_dc, const CRect* rect) const;

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
