#pragma once

#include <string>
#include "AcqWaveFormat.h"
#include "AcqWaveChanArray.h"
#include "Taglist.h"

constexpr auto MOVAVG30 = 13;

class CWaveBuf : public CObject
{
	DECLARE_SERIAL(CWaveBuf)
	friend class AcqDataDoc;

	// Attributes
protected:
	short* get_pointer_to_raw_data_buffer() const;
	short* get_pointer_to_raw_data_element(int chan, int index) const;
	short* get_pointer_to_transformed_data_buffer() const;
	short* get_pointer_to_transformed_data_element(int index) const;

public:
	int get_wb_n_elements() const;
	int get_wb_n_channels() const;
	void set_wb_sample_rate(float sampling_rate);
	float get_wb_sample_rate() const;
	BOOL get_wb_volts_per_bin(int ch_index, float* volts_per_bin, int mode = 0) const;
	TagList* get_p_hz_tags() { return &hz_tags_; }
	TagList* get_p_vt_tags() { return &vt_tags_; }
	CWaveChanArray* get_p_wave_chan_array() { return &channels_array_; }
	CWaveFormat* get_p_wave_format() { return &wave_format_; }

	//operations
	CWaveBuf();
	~CWaveBuf() override;
	void Serialize(CArchive& ar) override;
	int create_buffer_with_n_channels(int i);

	// Transform Data
	//------------
	static CString wb_get_transforms_allowed(int i);
	static BOOL wb_is_transform_allowed(int i);
	static int	wb_get_n_transforms();
	BOOL		wb_init_transform_buffer();
	static int	wb_get_transform_span(int i);
	static int	wb_is_span_change_allowed(int i);
	static int	wb_set_transform_span(int i, int span);
	static int	wb_get_correction_factor(int i, float* correct);

	// Transformations
	//----------------
	void low_pass_differentiation(const short* lp_source, short* lp_dest, int cx) const;
	void copy(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_lanczo_2(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_diff_1(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_diff_2(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_diff_3(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_diff_10(const short* lp_source, short* lp_dest, int cx) const;

	void low_pass_lanczo_3(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_derivative_1f3(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_derivative_2f3(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_derivative_2f5(const short* lp_source, short* lp_dest, int cx) const;
	void low_pass_derivative_3f3(const short* lp_source, short* lp_dest, int cx) const;
	void moving_average_30(short* lp_source, short* lp_dest, int cx) const;
	void moving_median_30(short* lp_source, short* lp_dest, int cx);
	void moving_median_35(short* lp_source, short* lp_dest, int cx);
	void moving_median(short* lp_source, short* lp_dest, int cx, int n_span);
	void root_to_mean_square(short* lp_source, short* lp_dest, int cx) const;

private:
	void delete_buffers();

protected:
	BOOL create_w_buffer(int i_num_elements, int n_channels = 1);

	CWaveChanArray channels_array_{}; // array of structures with the channel description
	CWaveFormat wave_format_{}; // structure with data acquisition def & parameters
	TagList hz_tags_{}; // list of horizontal cursors
	TagList vt_tags_{}; // list of vertical tags


	static int transforms_count_;					// number of transformations allowed
	static std::string m_p_transforms_allowed_[];	// ASCII description of each transformation
	static int m_transform_buffer_span_[];			// size of sliding window necessary to filter data
	static float m_correction_factor_[];			// correction factor to transform binary data into voltage
	static int b_variable_span_[];				// flag to tell if the sliding window size can be changed

private:
	short* p_w_data_  {nullptr}; // Pointer to the origin of the primary data array
	short* p_w_data_transformed_  {nullptr}; // primary transform buffer
	BOOL is_transformed_  {FALSE};
	WORD w_version_  {0};
	int num_elements_  {0}; // n elements within buffer
	size_t dw_buffer_size_  {0};
	short* p_array_sorted_  {nullptr}; // array used by BMedian to store a sorted array of data
	short* p_array_circular_  {nullptr}; // array used by BMedian to store a sliding window array of data
	int p_array_size_  {0};
};
