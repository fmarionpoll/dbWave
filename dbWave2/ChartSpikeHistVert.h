#pragma once

// ChartSpikeHistVert build, stores and display a list of histograms
// each histogram has the same abscissa limits (same nb of bins also)
// they are intended to store and display histograms of parameters measured
// from spike classes and follow the same display conventions as CSpikFormButton
// and CSpikBarsButton.
// one histogram (DWORD array) is composed of
//		index 0 = "class" index (int)
//		index 1 to nbins = data

#include "ChartSpike.h"
#include "dbWaveDoc.h"

class ChartSpikeHistVert : public ChartSpike
{
	// Construction
public:
	ChartSpikeHistVert();
	~ChartSpikeHistVert() override;

protected:
	CArray<CDWordArray*, CDWordArray*> histogram_ptr_array; // array of DWord array containing histogram
	SpikeList* m_pSL = nullptr;

	int m_binsize = 0; // size of one bin
	int m_abscissaminval = 0; // minimum value from which histogram is built (abscissa)
	int m_abscissamaxval = 0; // maximum value (abscissa max)
	int m_nbins; // n bins within histogram

	DWORD m_lmax = 0; // value max
	int m_imax{}; // index max
	int m_ifirst{}; // index first interval with data
	int m_ilast{}; // index last interval with data

public:
	int GetBinsize() const { return m_binsize; }
	int GetBinMinval() const { return m_abscissaminval; }
	int GetBinMaxVal() const { return m_abscissamaxval; }
	int GetnBins() const { return m_nbins; }
	int GetHistMaxPos() const { return m_imax; }
	DWORD GetHistMax() const { return m_lmax; }

	void BuildHistFromDocument(CdbWaveDoc* p_document, BOOL ballFiles, long l_first, long l_last, int max, int min,
	                           int nbins, BOOL bNew);

	void RemoveHistData();
	LPTSTR ExportAscii(LPTSTR lp); // export ascii data
	void MoveVTtagtoVal(int i_tag, int val);
	void MoveHZtagtoVal(int i_tag, int val);

	void zoom_data(CRect* prevRect, CRect* newRect) override;
	void plot_data_to_dc(CDC* p_dc) override;

	// implementation
protected:
	int hit_curve(CPoint point) override;
	void reSize_And_Clear_Histograms(int nbins, int max, int min);
	void getHistogLimits(int ihist);
	void getClassArray(int iclass, CDWordArray*& pDW);
	CDWordArray* initClassArray(int nbins, int spike_class);
	void buildHistFromSpikeList(SpikeList* p_spk_list, long l_first, long l_last, int max, int min, int nbins,
	                            BOOL bNew);
	void getExtents();
	void plotHistogram(CDC* p_dc, const CDWordArray* p_dw, int color) const;

	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
