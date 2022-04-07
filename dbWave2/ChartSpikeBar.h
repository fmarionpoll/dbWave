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
	BOOL m_bDisplaySignal{};			// flag default:FALSE=bars; TRUE=spikes
	int m_barheight = 10;
	CDWordArray* p_envelope_ = nullptr; // data envelope (should be 4 times greater than size)


	// Helpers
public:
	void SetSourceData_spklist_dbwavedoc(SpikeList* p_spk_list, CdbWaveDoc* p_document)
	{
		p_dbwave_doc_ = p_document;
		p_spike_doc_ = p_dbwave_doc_->m_pSpk;
		p_spikelist_ = p_spk_list;
	}

	void SetSourceData_spklist_spikedoc(SpikeList* p_spk_list, CSpikeDoc* p_spike_document)
	{
		p_dbwave_doc_ = nullptr;
		p_spike_doc_ = p_spike_document;
		p_spikelist_ = p_spk_list;
	}

	void SetSpkList(SpikeList* p_spk_list) { p_spikelist_ = p_spk_list; }

	void SetSpkIndexes(int first, int last) { m_spkfirst = first, m_spklast = last; }

	int GetSelectedSpike() const { return m_selectedspike; }
	int GetHitSpike() const { return m_hitspk; }

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
		m_ballFiles = bON;
		p_dbwave_doc_ = p_document;
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
