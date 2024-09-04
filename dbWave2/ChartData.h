#pragma once

#include "Envelope.h"
#include "chanlistitem.h"
#include "ChartWnd.h"
#include "AcqDataDoc.h"
#include "Scale.h"

class ChartData : public ChartWnd
{
public:
	ChartData();
	~ChartData() override;
	DECLARE_SERIAL(ChartData)
	void	Serialize(CArchive& ar) override;

	BOOL	get_data_from_doc();
	BOOL	get_data_from_doc(long l_first);
	BOOL	get_data_from_doc(long l_first, long l_last);
	BOOL	get_smooth_data_from_doc(int i_option);
	BOOL	scroll_data_from_doc(const UINT n_sb_code);

	int		resize_channels(int n_pixels, long l_size); 
	BOOL	attach_data_file(AcqDataDoc* p_data_file);
	BOOL	is_defined() const { return (chan_list_item_ptr_array_.GetSize() > 0); } 
	AcqDataDoc* get_p_data_file() const { return m_p_data_file_; }

	// export representation of data to the clipboard
	BOOL	copy_as_text(int i_option, int i_unit, int n_abscissa);
	LPTSTR	get_ascii_line(LPTSTR lp_copy, int i_unit);
	LPTSTR	get_ascii_envelope(LPTSTR lp_copy, int i_unit);

	// Helper functions
	long	get_nx_pixels() const { return m_n_pixels_; } // number of pixels defined in this window
	long	get_data_first_index() const { return m_lx_first_; } 
	long	get_data_last_index() const { return m_lx_last_; } 
	long	get_data_size() const { return m_lx_size_; } 
	long	get_page_size() const { return m_lx_page_; } 
	long	get_line_size() const { return m_lx_line_; } 
	long	get_document_last() const { return m_lx_very_last_; } 
	long	get_data_offset_from_pixel(const int pix) const {
				return static_cast<long>(pix) * (m_lx_last_ - m_lx_first_ + 1) / static_cast<long>(display_rect_.right) + m_lx_first_; }

	void update_page_line_size(); // update page and line size parameters

	// Attributes
protected:
	AcqDataDoc*	m_p_data_file_ = nullptr; 
	CArray<CChanlistItem*, CChanlistItem*> chan_list_item_ptr_array_;
	CArray<CEnvelope*, CEnvelope*> envelope_ptr_array_; 
	CArray<CPoint, CPoint> m_poly_points_; 
	CHighLight m_highlighted_;
	Scale m_scale_;

	int m_n_pixels_ = 1; // nb pixels displayed horizontally
	int m_data_per_pixel_ = 1; // nb of data point per pixel
	long m_lx_very_last_ = 1; // end of document
	long m_lx_page_{}; // size of page increment / file index
	long m_lx_line_{}; // size of line increment / file index
	long m_lx_size_ = 1; // nb of data pts represented in a Envelope
	long m_lx_first_ = 0; // file index of 1rst pt in the Envelopes
	long m_lx_last_ = 1; // file index of last pt in the Envelopes
	float m_sampling_rate_{};
	BOOL m_b_track_curve_ = false; // track curve if hit

	int m_xor_n_elements_{}; // curve tracking parameters
	int m_hit_curve_{}; // ibid  which curve was hit if any
	int m_xor_x_ext_{}; // ibid  x extent
	int m_xor_y_ext_{}; // ibid  y extent
	int m_zero_{}; // ibid. zero

	BOOL m_b_ad_buffers_ = false; // flag when AD buffers are in displayed
	long m_l_ad_buffer_done_{}; // position of incoming data along m_lxSize

	BOOL m_b_print_hz_cursor_{}; // default=FALSE
	BOOL m_b_track_spike_ = false; // track spike with vertical cursor
	int m_track_len_ = 60; // length of waveform to track
	int m_track_offset_ = 20; // offset of waveform from cursor
	int m_track_channel_ = 0; // line_view channel

	// ChanList operations
public:
	int		get_channel_list_size() const { return chan_list_item_ptr_array_.GetSize(); }
	void	remove_all_channel_list_items();
	int		add_channel_list_item(int ns, int mode);
	int		remove_channel_list_item(int i_chan);
	
	int		set_channel_list_transform_mode(int i_chan, int i_mode);
	int		set_channel_list_source_channel(int i_channel, int acq_channel);
	void	set_channel_list_y(int i_chan, int acq_chan, int transform);
	void	set_channel_list_volts_extent(int i_chan, const float* p_value);
	void	set_channel_list_volts_zero(int i_chan, const float* p_value);

	CChanlistItem* get_channel_list_item(const int item) const { return chan_list_item_ptr_array_.GetAt(item); }

	float get_channel_list_volts_per_pixel(const int item) const {
		const CChanlistItem* channel_item = get_channel_list_item(item);
		return (static_cast<float>(channel_item->get_y_extent()) * channel_item->get_volts_per_bin() / static_cast<float>(-y_ve_));
	}

	float get_time_per_pixel() const{
		return static_cast<float>(get_data_size()) / m_p_data_file_->get_wave_format()->sampling_rate_per_channel
					/ static_cast<float>(get_rect_width());
	}

	int get_channel_list_bin_to_y_pixel(const int chan, const int bin) {
		return MulDiv(bin - chan_list_item_ptr_array_[chan]->get_y_zero(), y_ve_,
		              chan_list_item_ptr_array_[chan]->get_y_extent()) + y_vo_;
	}

	int get_channel_list_y_pixels_to_bin(const int chan, const int y_pixel_relative_to_top_client_window) {
		return chan_list_item_ptr_array_[chan]->get_y_zero() + 
				MulDiv(y_pixel_relative_to_top_client_window - y_vo_,
					chan_list_item_ptr_array_[chan]->get_y_extent(), 
					y_ve_);
	}

	options_scope_struct* get_scope_parameters() override;
	void	set_scope_parameters(options_scope_struct* p_struct) override;
	void	auto_zoom_chan(int channel_index) const;
	void	center_chan(int channel_index) const;
	void	max_gain_chan(int channel_index) const;
	void	split_channels() const;
	void	update_chan_list_from_doc();
	void	update_chan_list_max_span();
	void	update_gain_settings(int i_chan);
	void	set_highlight_data(const CDWordArray* p_intervals);
	void	set_highlight_data(const CHighLight& source);
	void	set_track_spike(BOOL b_track_spike, int track_len, int track_offset, int track_channel);
	void	move_hz_tag_to_val(int tag_index, int val_int);
	void	update_x_ruler();
	void	update_y_ruler();

	void	plot_data_to_dc(CDC* p_dc) override;
	void	zoom_data(CRect* previous_rect, CRect* new_rect) override;
	void	print(CDC* p_dc, const CRect* p_rect, BOOL b_center_line = FALSE);

	void	adjust_gain(boolean b_set_span_mv, float span_mv_value) const;
	void	load_data_within_window(boolean set_time_span, float t_first, float t_last);
	void	load_all_channels(int data_transform);

protected:
	void	highlight_data(CDC* p_dc, int chan);
	int		does_cursor_hit_curve(CPoint point);
	void	curve_xor();
	void	display_vt_tags_long_value(CDC* p_dc);
	void	display_hz_tags_for_channel(CDC* p_dc, int i_chan, const CChanlistItem* p_channel);

	afx_msg void OnLButtonDown(UINT n_flags, CPoint point) override;
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point) override;
	afx_msg void OnSize(UINT n_type, int cx, int cy) override;
	afx_msg void OnMouseMove(UINT n_flags, CPoint point) override;

	DECLARE_MESSAGE_MAP()
};
