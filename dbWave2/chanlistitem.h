#pragma once

class CChanlistItem : public CObject
{
public:
	CChanlistItem();
	CChanlistItem(CEnvelope* pX, int iX, CEnvelope* pY, int iY);
	DECLARE_SERIAL(CChanlistItem)
	void Serialize(CArchive& ar) override;

	void init_display_parameters(WORD new_pen_width, WORD new_color, int new_zero, int new_y_extent, WORD new_draw_mode = 1);
public:
	CEnvelope* p_envelope_abscissa{ nullptr };
	CEnvelope* p_envelope_ordinates{ nullptr }; 
	CString dl_comment{}; 
	void get_envelope_array_indexes(int& x, int& y) const;
	void set_envelope_arrays(CEnvelope* px, int x, CEnvelope* py, int y);

protected:
	int dl_y_zero_{2048};
	int dl_y_extent_{4096}; 
	WORD dl_pen_width_{ 0 }; 
	WORD dl_color_{0}; 
	WORD dl_b_print_{1};

	float dl_data_volts_per_bin_ { 20.f / static_cast<float>(dl_y_extent_) }; 
	float dl_data_volt_span_{0}; 
	long dl_data_bin_zero_{2048}; // value of zero volts
	long dl_data_bin_span_{4096}; // nb of bins encoding values within envelope

	BOOL dl_b_hz_tags_print_{false}; // print HZ tags flag
	int dl_index_abscissa_{0};
	int dl_index_ordinates_{0};

	// helpers
public:
	int get_y_zero() const { return dl_y_zero_; }
	int get_y_extent() const { return dl_y_extent_; }
	WORD get_pen_width() const { return dl_pen_width_; }
	WORD get_color_index() const { return dl_color_; }
	WORD is_print_visible() const { return dl_b_print_; }
	float get_volts_per_bin() const { return dl_data_volts_per_bin_; }
	BOOL get_hz_tags_print_flag() const { return dl_b_hz_tags_print_; }
	void get_max_min(int* p_max, int* p_min) const { p_envelope_ordinates->get_envelope_max_min(p_max, p_min); }
	int get_bin_at(const int index) const { return p_envelope_ordinates->get_point_at(index); }
	CString get_comment() { return dl_comment; }
	int get_source_chan() const { return p_envelope_ordinates->get_source_chan(); }
	int get_transform_mode() const { return p_envelope_ordinates->get_source_mode(); }

	int get_data_bin_zero() const { return dl_data_bin_zero_; }
	int get_data_bin_span() const { return dl_data_bin_span_; }
	float get_data_volts_span() const { return dl_data_volt_span_; }

	void set_data_bin_format(const long bin_zero, const long bin_span)
	{
		dl_data_bin_zero_ = bin_zero;
		dl_data_bin_span_ = bin_span;
	}

	void set_data_volts_format(const float v_per_bin, const float volts_span)
	{
		dl_data_volts_per_bin_ = v_per_bin;
		dl_data_volt_span_ = volts_span;
	}

	void set_y_zero(const int zero) { dl_y_zero_ = zero; }
	void set_y_extent(const int y_extent) { dl_y_extent_ = y_extent; }
	void set_pen_width(const WORD pen_width) { dl_pen_width_ = pen_width; }
	void set_color(const WORD color) { dl_color_ = color; }
	void set_flag_print_visible(const WORD draw_mode) { dl_b_print_ = draw_mode; }
	void set_hz_tags_print_flag(const BOOL b_print) { dl_b_hz_tags_print_ = b_print; }
	void set_ordinates_source_data(const int chan, const int transform) const { p_envelope_ordinates->set_envelope_source_data(chan, transform); }

	float convert_data_bins_to_volts(const int bins) const { return static_cast<float>(bins) * dl_data_volts_per_bin_; }
	int convert_volts_to_data_bins(const float volts) const { return static_cast<int>(volts / dl_data_volts_per_bin_); }
};
