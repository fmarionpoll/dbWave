#pragma once
#include "ChartSpike.h"


constexpr auto DISPLAY_TIMEINTERVAL = 0; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISPLAY_INDEXRANGE = 1; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISP_SELECTEDCLASS = 2; // mask: ON: display only selected spike/OFF=all classes

class ChartSpikeShapeWnd : public ChartSpike
{
	//Construction
public:
	ChartSpikeShapeWnd();
	~ChartSpikeShapeWnd() override;
	DECLARE_SERIAL(ChartSpikeShapeWnd)
	void Serialize(CArchive& ar) override;

	// parameters

protected:
	int m_colorselectedspike = RED_COLOR; // color selected spike (index / color table)
	BOOL m_bText = FALSE; // allow text default false
	int m_ndisplayedspikes{}; // number of spikes displayed

	CArray<CPoint, CPoint> polypoints_; // points displayed with polyline

	// Helpers
public:
	void SetTextOption(BOOL bText) { m_bText = bText; }
	BOOL GetTextOption() const { return m_bText; }

	float GetDisplayMaxMv();
	float GetDisplayMinMv();
	float GetExtent_mV();
	float GetExtent_ms();

	void SetSpkIndexes(int first, int last) { m_index_first_spike = first, m_index_last_spike = last; }
	int GetHitSpike() const { return m_hit_spike; }
	int GetSelectedSpike() const { return m_selected_spike; }

	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document)
	{
		m_display_all_files = bON;
		p_dbwave_doc = p_document;
	}

	// non-inline functions
	void MoveVTtrack(int i_track, int new_value);

	int SelectSpikeShape(int spikeno);
	BOOL IsSpikeWithinRange(int spike_no) const;

	void Print(CDC* p_dc, CRect* rect);
	int DisplayExData(short* p_data, int color = 9);
	void DisplayFlaggedSpikes(BOOL bHighLight);
	void PlotDataToDC(CDC* p_dc) override;
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	void initPolypointAbcissa();
	void fillPolypointOrdinates(short* lpSource);
	void getExtents();
	void getExtentsCurrentSpkList();

	int hitCurve(CPoint point) override;
	int hitCurveInDoc(CPoint point);
	void drawSelectedSpike(int no_spike, CDC* p_dc);
	void drawFlaggedSpikes(CDC* p_dc);
	void plotArraytoDC(CDC* p_dc, short* pspk);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
