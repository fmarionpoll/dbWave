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

	int bin_size_ = 0; // size of one bin
	int abscissa_min_val_ = 0; // minimum value from which histogram is built (abscissa)
	int abscissa_max_val_ = 0; // maximum value (abscissa max)
	int n_bins_ = 0; // n bins within histogram

	DWORD m_lmax_ = 0; // value max
	int m_imax_{}; // index max
	int m_i_first_{}; // index first interval with data
	int m_i_last_{}; // index last interval with data

public:
	int get_bin_size() const { return bin_size_; }
	int get_bin_min_val() const { return abscissa_min_val_; }
	int get_bin_max_val() const { return abscissa_max_val_; }
	int get_n_bins() const { return n_bins_; }
	int get_hist_max_pos() const { return m_imax_; }
	DWORD get_hist_max() const { return m_lmax_; }

	void build_hist_from_document(CdbWaveDoc* p_doc, BOOL b_all_files, long l_first, long l_last, int max, int min,
	                           int n_bins, BOOL b_new);

	void remove_hist_data();
	LPTSTR export_ascii(LPTSTR lp); // export ascii data

	void move_vt_tag_to_val(int i_tag, int val);
	void move_hz_tag_to_val(int tag_index, int value);
	void zoom_data(CRect* prev_rect, CRect* new_rect) override;
	void plot_data_to_dc(CDC* p_dc) override;

	// implementation
protected:
	void get_histogram_limits(int histogram_index);
	void get_class_array(int i_class, CDWordArray*& p_dw);
	void get_extents();

	int hit_curve(CPoint point) override;
	void size_and_clear_histograms(int n_bins, int max, int min);
	CDWordArray* init_class_array(int n_bins, int spike_class);
	void build_hist_from_spike_list(SpikeList* p_spk_list, long l_first, long l_last, int max, int min, int n_bins,
	                            BOOL b_new);
	void plot_histogram(CDC* p_dc, const CDWordArray* p_dw, int color) const;

	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point);
	afx_msg void OnSize(UINT n_type, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
