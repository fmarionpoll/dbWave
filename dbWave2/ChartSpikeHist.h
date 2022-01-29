#pragma once

// CChartSpikeHist build, stores and display a list of histograms
// each histogram has the same abcissa limits (same nb of bins also)
// they are intended to store and display histograms of parameters measured
// from spike classes and follow the same display conventions as CSpikFormButton
// and CSpikBarsButton.
// one histogram (DWORD array) is composed of
//		index 0 = "class" index (int)
//		index 1 to nbins = data

#include "ChartWnd.h"
#include "dbWaveDoc.h"

class CChartSpikeHist : public ChartWnd
{
	// Construction
public:
	CChartSpikeHist();
	~CChartSpikeHist() override;

protected:
	CArray<CDWordArray*, CDWordArray*> histogram_ptr_array; // array of DWord array containing histogram
	CSpikeList* p_spikelist_;

	long m_lFirst = 0; // time index of first pt displayed
	long m_lLast = 0; // time index of last pt displayed
	int m_hitspk = -1; // no of spike selected
	int m_selclass = 0; // index class selected

	int m_binsize{}; // size of one bin
	int m_abcissaminval = 0; // minimum value from which histogram is built (abcissa)
	int m_abcissamaxval = 0; // maximum value (abcissa max)
	int m_nbins = 0; // n bins within histogram

	DWORD m_lmax{}; // value max
	int m_imax{}; // index max
	int m_ifirst{}; // index first interval with data
	int m_ilast{}; // index last interval with data

public:
	void SetPlotMode(int mode, int selclass)
	{
		m_plotmode = mode;
		m_selclass = selclass;
	}

	void SetTimeIntervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}

	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }

	int GetBinsize() const { return m_binsize; }
	int GetBinMinval() const { return m_abcissaminval; }
	int GetBinMaxVal() const { return m_abcissamaxval; }
	int GetnBins() const { return m_nbins; }
	int GetHistMaxPos() const { return m_imax; }
	DWORD GetHistMax() const { return m_lmax; }

	void BuildHistFromDocument(CdbWaveDoc* p_document, BOOL ballFiles, long l_first, long l_last, int max, int min,
	                           int nbins, BOOL bNew);

	void RemoveHistData();
	LPTSTR ExportAscii(LPTSTR lp); // export ascii data
	void MoveVTtagtoVal(int itag, int ival);
	void MoveHZtagtoVal(int itag, int ival);

	void SetSpkList(CSpikeList* p_spk_list) { p_spikelist_ = p_spk_list; }
	void ZoomData(CRect* prevRect, CRect* newRect) override;
	// implementation
protected:
	int hitCurve(CPoint point) override;
	void reSize_And_Clear_Histograms(int nbins, int max, int min);
	void getHistogLimits(int ihist);
	void getExtents();
	void plotHistogram(CDC* p_dc, CDWordArray* p_dw, int color);

	void getClassArray(int iclass, CDWordArray*& pDW);
	CDWordArray* initClassArray(int nbins, int spike_class);
	void buildHistFromSpikeList(CSpikeList* p_spk_list, long l_first, long l_last, int max, int min, int nbins,
	                            BOOL bNew);

public:
	void PlotDatatoDC(CDC* p_dc) override;

protected:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
