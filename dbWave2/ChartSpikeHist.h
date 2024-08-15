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
	CArray<CDWordArray*, CDWordArray*> histogram_array_; 

	double abscissa_min_mv_ = 0.; 
	double abscissa_max_mv_ = 4.; 
	double abscissa_bin_mv_ = 0.1;
	int abscissa_n_bins_ = 0;

	DWORD l_max_{}; 
	int i_max_{}; 
	int i_first_{}; 
	int i_last_{}; 

	int hit_curve(CPoint point) override;
	void resize_histograms(double min_mv, double max_mv, double bin_mv);
	void get_histogram_limits(int i_hist);
	void get_class_array(int i_class, CDWordArray*& p_dw);
	CDWordArray* init_class_array(int n_bins, int spike_class);
	void build_hist_from_spike_list(SpikeList* p_spk_list, long l_first, long l_last, 
								double min_mv, double max_mv, double bin_mv);
	void plot_histogram(CDC* p_dc, const CDWordArray* p_dw, int color);

public:
	double	get_hist_bin_mv() const { return abscissa_bin_mv_; }
	double	get_hist_bin_min_mv() const { return abscissa_min_mv_; }
	double	get_hist_bin_max_mv() const { return abscissa_max_mv_; }
	int		get_hist_n_bins() const { return abscissa_n_bins_; }
	int		get_hist_max_bin_index() const { return i_max_; }
	DWORD	get_hist_max_value() const { return l_max_; }
	void	get_extents();

	void build_hist_from_document(CdbWaveDoc* p_document, BOOL b_all_files, long l_first, long l_last,
		double min_mv, double max_mv, double bin_mv);

	void clear_data();
	LPTSTR export_ascii(LPTSTR lp);
	void move_hz_tag_to_val(int tag_index, int value);

	void zoom_data(CRect* r_from, CRect* r_dest) override;
	void plot_data_to_dc(CDC* p_dc) override;;

	void move_vt_tag_to_value_mv(int tag_index, double value_mv);

	void set_vt_tag_value_mv(int tag_index, double value_mv);
	double get_vt_tag_value_mv(int tag_index);

	int convert_mv_to_abscissa(const double value_mv) const
	{
		return static_cast<int>((value_mv - abscissa_min_mv_)
			* static_cast<float>(x_we_)
			/ (abscissa_max_mv_ - abscissa_min_mv_)) + x_wo_;
	}

	double convert_abscissa_to_mv(const int abscissa) const
	{
		return (static_cast<double>(abscissa) - static_cast<double>(x_wo_))
			* (abscissa_max_mv_ - abscissa_min_mv_)
			/ static_cast<double>(x_we_) + abscissa_min_mv_;
	}

protected:
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point) override;
	afx_msg void OnSize(UINT n_type, int cx, int cy) override;

	DECLARE_MESSAGE_MAP()
};
