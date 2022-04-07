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
	void SetSpkIndexes(int first, int last) { m_index_first_spike = first, m_index_last_spike = last; }

	int GetSelectedSpike() const { return m_selected_spike; }
	int GetHitSpike() const { return m_hit_spike; }

	int SelectSpike(int spikeno);
	void DisplaySpike(int nospike, BOOL bselect);
	BOOL IsSpikeWithinRange(int spikeno);
	void MoveHZtagtoVal(int itag, int ival);
	void MoveVTtagtoVal(int itag, int ival);

	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document)
	{
		m_display_all_files = bON;
		p_dbwave_doc = p_document;
	}

	void PlotDataToDC(CDC* p_dc) override;
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	void drawSelectedSpike(int nospike, int color, CDC* p_dc);
	int hitCurve(CPoint point) override;
	int hitCurveInDoc(CPoint point);
	BOOL is_spike_within_limits(int ispike);
	void getExtents();
	void highlightOnePoint(int nospike, CDC* p_dc);
	void displayHZtags(CDC* p_dc);
	void displayVTtags(CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
