#pragma once

// ChartSpikeHist build, stores and display a list of histograms
// each histogram has the same abscissa limits (same nb of bins also)
// they are intended to store and display histograms of parameters measured
// from spike classes and follow the same display conventions as CSpikFormButton
// and CSpikBarsButton.
// one histogram (DWORD array) is composed of
//		index 0 = "class" index (int)
//		index 1 to nbins = data

#include "ChartSpike.h"
#include "dbWaveDoc.h"

class ChartSpikeHist : public ChartSpike
{
	// Construction
public:
	ChartSpikeHist();
	~ChartSpikeHist() override;

protected:
	CArray<CDWordArray*, CDWordArray*> histogram_ptr_array; // array of DWord array containing histogram

	int m_binsize{}; // size of one bin
	int m_abcissaminval = 0; // minimum value from which histogram is built (abscissa)
	int m_abcissamaxval = 0; // maximum value (abscissa max)
	int m_nbins = 0; // n bins within histogram

	DWORD m_lmax{}; // value max
	int m_imax{}; // index max
	int m_ifirst{}; // index first interval with data
	int m_ilast{}; // index last interval with data

public:
	int GetBinsize() const { return m_binsize; }
	int GetBinMinval() const { return m_abcissaminval; }
	int GetBinMaxVal() const { return m_abcissamaxval; }
	int GetnBins() const { return m_nbins; }
	int GetHistMaxPos() const { return m_imax; }
	DWORD GetHistMax() const { return m_lmax; }

	void build_hist_from_document(CdbWaveDoc* p_document, BOOL b_all_files, long l_first, long l_last, int max, int min,
	                           int n_bins, BOOL b_new);

	void delete_histogram_data();
	LPTSTR export_ascii(LPTSTR lp); // export ascii data
	void move_vt_tag_to_val(int tag_index, int value);
	void move_hz_tag_to_val(int tag_index, int value);

	
	void zoom_data(CRect* prevRect, CRect* newRect) override;

protected:
	int hit_curve(CPoint point) override;
	void resize_and_clear_histograms(int n_bins, int max, int min);
	void get_histogram_limits(int i_hist);
	void get_extents();
	void plot_histogram(CDC* p_dc, const CDWordArray* p_dw, int color) const;

	void getClassArray(int i_class, CDWordArray*& p_dw);
	CDWordArray* init_class_array(int n_bins, int spike_class);
	void build_hist_from_spike_list(SpikeList* p_spk_list, long l_first, long l_last, int max, int min, int n_bins,
	                            BOOL b_new);

public:
	void plot_data_to_dc(CDC* p_dc) override;

protected:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) override;
	afx_msg void OnSize(UINT nType, int cx, int cy) override;

	DECLARE_MESSAGE_MAP()
};
