#pragma once

// ChartSpikeHist build, stores and display a list of histograms
// each histogram has the same abscissa limits (same nb of bins also)
// they are intended to store and display histograms of parameters measured
// from spike classes and follow the same display conventions as CSpikeFormButton
// and CSpikeBarsButton.
// one histogram (DWORD array) is composed of
//		index 0 = "class" index (int)
//		index 1 to n_bins = data

#include "ChartSpike.h"
#include "dbWaveDoc.h"

class ChartSpikeHist : public ChartSpike
{
	// Construction
public:
	ChartSpikeHist();
	~ChartSpikeHist() override;

protected:
	CArray<CDWordArray*, CDWordArray*> histogram_ptr_array_; // array of DWord array containing histogram

	int bin_size_{}; // size of one bin
	int abscissa_min_value_ = 0; // minimum value from which histogram is built (abscissa)
	int abscissa_max_value_ = 0; // maximum value (abscissa max)
	int n_bins_ = 0; // n bins within histogram

	DWORD l_max_{}; // value max
	int i_max_{}; // index max
	int i_first_{}; // index first interval with data
	int i_last_{}; // index last interval with data

public:
	int get_bin_size() const { return bin_size_; }
	int get_bin_min_value() const { return abscissa_min_value_; }
	int get_bin_max_value() const { return abscissa_max_value_; }
	int get_n_bins() const { return n_bins_; }
	int get_hist_max_bin_index() const { return i_max_; }
	DWORD get_hist_max_value() const { return l_max_; }

	void build_hist_from_document(CdbWaveDoc* p_document, BOOL b_all_files, long l_first, long l_last, int max, int min,
	                           int n_bins, BOOL b_new);
	void delete_histogram_data();
	LPTSTR export_ascii(LPTSTR lp); // export ascii data
	void move_vt_tag_to_val(int tag_index, int value);
	void move_hz_tag_to_val(int tag_index, int value);

	void zoom_data(CRect* r_from, CRect* r_dest) override;

protected:
	int hit_curve(CPoint point) override;
	void resize_and_clear_histograms(int n_bins, int max, int min);
	void get_histogram_limits(int i_hist);
	void get_extents();
	void plot_histogram(CDC* p_dc, const CDWordArray* p_dw, int color) const;

	void get_class_array(int i_class, CDWordArray*& p_dw);
	CDWordArray* init_class_array(int n_bins, int spike_class);
	void build_hist_from_spike_list(SpikeList* p_spk_list, long l_first, long l_last, int max, int min, int n_bins,
	                            BOOL b_new);
public:
	void plot_data_to_dc(CDC* p_dc) override;

protected:
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) override;
	afx_msg void OnSize(UINT n_type, int cx, int cy) override;

	DECLARE_MESSAGE_MAP()
};
