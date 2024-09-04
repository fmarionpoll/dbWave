#pragma once

#include "AcqDataDoc.h"
#include "AcqWaveChan.h"
#include "SpikeClassProperties.h"
#include "Spike.h"
#include "options_detect_spikes.h"



// spikes from one data acquisition channel:
// - description of the original data acquisition set,
// - how spikes were initially detected
// - parameters associated to each spike
// - data describing the spikes
// only one data spike buffer (64Kb max) opened at a time
//
// this object has procedures to manage the list of spikes and to
// measure certain parameters on them
// as well as storing/reading them


class SpikeList : public CObject
{
public:
	void Serialize(CArchive& ar) override;
	DECLARE_SERIAL(SpikeList)

	SpikeList();
	~SpikeList() override;

	int m_selected_spike {-1};
	int m_jitter_sl {2};
	int m_i_center_1_sl {0};
	int m_i_center_2_sl {60};
	int shape_t1 {0};
	int shape_t2 {59};

	// Description
protected:
	// (0) infos about file version and ID
	WORD w_version_ {9};		// Aug 2024 save spike data into "spikes"
	CString id_string_ {_T("Awave Spike file v")};

	// (1)infos about data acquisition and spike detection
	CWaveChan wave_channel_{};	
	WORD data_encoding_mode_  {0};
	long bin_zero_ {4096}; 
	float sampling_rate_ {10000.f};
	float volts_per_bin_ {0.001f}; 
	options_detect_spikes options_detect_spk_ {};
	CString channel_description_ {};

	// (2) ordered spike list with class, time, etc
	BOOL extrema_valid_ {false};
	int minimum_over_all_spikes_ {0}; 
	int maximum_over_all_spikes_ {0};
	int min_y1_over_all_spikes_ {0};
	int max_y1_over_all_spikes_ {0};
	int spike_length_ {60};

	CArray<Spike*, Spike*> spikes_;

	// (3) classes of spikes
	BOOL b_save_artefacts_ {false}; 
	BOOL keep_only_valid_classes_ {false};
	int n_classes_ {0};

	CArray<SpikeClassProperties*, SpikeClassProperties*> class_properties_{};

	//  (5) list of spikes flagged
	CArray<int, int> flagged_spikes_ {};

	// Operations
public:
	BOOL is_class_list_valid() const { return keep_only_valid_classes_; }

	int get_classes_count() const {return  class_properties_.GetCount();}
	int get_class_id(const int i) const { return class_properties_.GetAt(i)->get_class_id();}
	void set_class_id(const int i, const int id) { class_properties_.GetAt(i)->set_class_id(id); }
	int add_class_id(int id);

	SpikeClassProperties* get_class_descriptor_from_id(int class_id);
	SpikeClassProperties* get_class_descriptor_from_index(const int index) {return class_properties_.GetAt(index);}
	CString get_class_description_from_id(int class_id);

	int get_class_id_index(int class_id);
	int increment_class_n_items(int class_id);
	int decrement_class_n_items(int class_id);
	void change_spike_class_id(int spike_no, int class_id);
	long update_class_list();
	void remove_spike_classes_with_no_spikes();
	void clear_spike_classes_spikes_count();

	Spike* get_spike(const int index) { return spikes_.GetSize() > 0 ? spikes_.GetAt(index) : nullptr; }
	int get_spikes_count() const { return spikes_.GetCount(); }

	WORD get_acq_encoding() const { return data_encoding_mode_; }
	float get_acq_sampling_rate() const { return sampling_rate_; }
	int get_acq_bin_zero() const { return bin_zero_; }
	float get_acq_volts_per_bin() const { return volts_per_bin_; }
	float convert_acquisition_point_to_mv (const int value) const { return volts_per_bin_ * 1000.f * static_cast<float>(value - bin_zero_); }
	float convert_difference_to_mv(const int value) const { return volts_per_bin_ * 1000.f * static_cast<float>(value); }

	void set_detection_parameters(const options_detect_spikes* p_sd) { options_detect_spk_ = *p_sd; }
	options_detect_spikes* get_detection_parameters() { return &options_detect_spk_; }

	int add_spike(short* source_data, int n_channels, long ii_time, int source_channel, int i_class, BOOL b_check);

	int  get_spike_length() const { return spike_length_; }
	void set_spike_length(const int spike_length) { spike_length_ = spike_length; }

	int remove_spike(int spike_index);
	BOOL is_any_spike_around(long ii_time, int jitter, int& spike_index, int channel_index);

	void get_total_max_min(BOOL b_recalculate, int* max, int* min);
	int get_total_max_min_of_y1(int* max, int* min);

	BOOL init_spike_list(const AcqDataDoc* acq_data_doc, const options_detect_spikes* spk_detect_parameters);

	void erase_data();
	void change_all_spike_from_class_id_to_new_class_id(int old_class_id, int new_class_id);

	void measure_amplitude_min_to_max(int t1, int t2);
	void measure_amplitude_vt(int t);
	void measure_amplitude_vt2_minus_vt1(int t1, int t2);
	CSize measure_y1_max_min();

	BOOL sort_spike_with_y1(CSize from_class_id_to_class_id, CSize time_bounds, CSize limits);
	BOOL sort_spike_with_y1_and_y2(CSize from_class_id_to_class_id, CSize time_bounds, CSize limits1, CSize limits2);

	int get_valid_spike_number(int spike_index) const;
	int get_next_spike(int spike_index, int delta, BOOL b_keep_same_class) ;

	int toggle_spike_flag(int spike_index);
	void set_spike_flag(int spike_index, boolean exclusive = true);
	boolean get_spike_flag(int spike_index) const;
	void clear_flagged_spikes();
	void flag_range_of_spikes(long l_first, long l_last, boolean b_set);
	void flag_spikes_within_bounds(int v_min, int v_max, long l_first, long l_last, boolean b_add);
	void get_range_of_spike_flagged(long& l_first, long& l_last);
	int get_spike_index_of_flag(const int i) const
	{
		return i <= flagged_spikes_.GetCount() ? flagged_spikes_.GetAt(i):-1;
	}
	int get_spike_flag_array_count() const {return flagged_spikes_.GetCount(); }
	void change_class_of_flagged_spikes(int new_class_id);

protected:
	void read_file_version_1(CArchive& ar);
	void remove_artefacts();
	void serialize_version_9(CArchive& ar);
	void serialize_version_8(CArchive& ar);
	void read_file_version_before_5(CArchive& ar, int version);
	void read_file_version_5(CArchive& ar);
	void read_class_descriptors_v5(CArchive& ar, int index);

	void delete_arrays();
	void serialize_version_7(CArchive& ar);
	void serialize_data_parameters(CArchive& ar);
	void serialize_spikes(CArchive& ar);
	void serialize_spike_data_short(CArchive& ar);
	
	void serialize_spike_class_properties(CArchive& ar);
	void serialize_additional_data(CArchive& ar);

	int get_index_first_spike(int index_start, boolean reject_artefacts);
	void get_total_max_min_read();
	void get_total_max_min_measure();
	int get_total_max_min_of_y1_measure();
};
