#pragma once

#include "AcqDataDoc.h"
#include "AcqWaveChan.h"
#include "SpikeClassDescriptor.h"
#include "Spike.h"
#include "SPKDETECTPARM.h"



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

	int m_selected_spike = -1;
	int m_jitterSL = 2;
	int m_icenter1SL = 0;
	int m_icenter2SL = 60;
	int m_imaxmin1SL = 0;
	int m_imaxmin2SL = 59;

	// Description
protected:
	// (0) ---------------infos about file version and ID
	WORD m_version = 6;		// aug 2013 change spike element
	CString ID_string = _T("Awave Spike file v");

	// (1) ---------------infos about data acquisition and spike detection ------------------
	CWaveChan m_acquisition_channel{};	
	WORD m_data_encoding_mode = 0;
	long m_bin_zero = 4096; 
	float m_sampling_rate = 10000.f;
	float m_volts_per_bin = 0.001f; 
	SPKDETECTPARM m_detection_parameters{};
	CString m_spike_channel_description;

	// (2) -------------ordered spike list with class, time, etc-----------------------------
	BOOL m_extrema_valid = false;
	short m_minimum_over_all_spikes = 0; 
	short m_maximum_over_all_spikes = 0;
	int m_spike_length = 60;
	CArray<Spike*, Spike*> m_spikes;

	// (3) --------classes of spikes
	BOOL m_b_save_artefacts = false; 
	BOOL m_keep_only_valid_classes = false;
	int m_n_classes = 0;
	CArray<SpikeClassDescriptor, SpikeClassDescriptor> m_spike_class_descriptors{};

	//  (5) list of spikes flagged
	CArray<int, int> m_index_flagged_spikes;

	// Operations
public:
	WORD get_version() const { return m_version; }
	BOOL is_class_list_valid() const { return m_keep_only_valid_classes; }

	int get_classes_count() const {return  m_spike_class_descriptors.GetCount();}
	int get_class_id(const int i) const { return m_spike_class_descriptors.GetAt(i).get_class_id();}
	void set_class_id(const int i, const int id) { m_spike_class_descriptors.GetAt(i).set_class_id(id); }
	int add_class_id(const int id);

	int get_class_n_items(const int i) const { return m_spike_class_descriptors.GetAt(i).get_n_items(); }
	void set_class_n_items(const int i, int n_spikes) { m_spike_class_descriptors.GetAt(i).set_n_items(n_spikes); }

	int get_class_id_descriptor_index(int class_id);
	int get_class_id_n_items(int class_id);
	int increment_class_id_n_items(int class_id);
	int decrement_class_id_n_items(int class_id);

	void change_spike_class_id(int spike_no, int class_id);

	Spike* get_spike(const int index) { return m_spikes.GetSize() > 0 ? m_spikes.GetAt(index) : nullptr; }
	int get_spikes_count() const { return m_spikes.GetCount(); }

	WORD get_acq_encoding() const { return m_data_encoding_mode; }
	float get_acq_sampling_rate() const { return m_sampling_rate; }
	float get_acq_volts_per_bin() const { return m_volts_per_bin; }
	int get_acq_bin_zero() const { return m_bin_zero; }

	void set_detection_parameters(const SPKDETECTPARM* p_sd) { m_detection_parameters = *p_sd; }
	SPKDETECTPARM* get_detection_parameters() { return &m_detection_parameters; }

	int add_spike(short* source, int n_channels, long ii_time, int source_channel, int i_class, BOOL b_check);

	int  get_spike_length() const { return m_spike_length; }
	void set_spike_length(int spike_length) { m_spike_length = spike_length; }

	int remove_spike(int spike_index);
	BOOL is_any_spike_around(long ii_time, int jitter, int& spike_index, int channel_index);

	void get_total_max_min(BOOL b_recalculate, short* max, short* min);

	BOOL init_spike_list(AcqDataDoc* acq_data_doc, SPKDETECTPARM* spk_detect_parm);
	long update_class_list();
	void erase_data();
	void change_all_spike_from_class_id_to_new_class_id(int old_class_ID, int new_class_ID);

	void measure_case0_amplitude_min_to_max(int t1, int t2);
	void measure_case1_amplitude_at_t(int t);
	void measure_case2_amplitude_at_t2_minus_at_t1(int t1, int t2);
	CSize measure_y1_max_min();

	BOOL sort_spike_with_y1(CSize from_class_ID_to_class_ID, CSize time_bounds, CSize limits);
	BOOL sort_spike_with_y1_and_y2(CSize from_class_ID_to_class_ID, CSize time_bounds, CSize limits1, CSize limits2);

	int get_valid_spike_number(int spike_index) const;
	int get_next_spike(int spike_index, int delta, BOOL b_keep_same_class) ;

public:
	int set_spike_flag(int spike_index, BOOL set_spike_flag);
	int toggle_spike_flag(int spike_index);
	void set_single_spike_flag(int spike_index);
	BOOL get_spike_flag(int spike_index);
	void remove_all_spike_flags();
	void flag_range_of_spikes(long l_first, long l_last, BOOL bSet);
	void select_spikes_within_bounds(int v_min, int v_max, long l_first, long l_last, BOOL b_add);
	void get_range_of_spike_flagged(long& l_first, long& l_last);
	BOOL get_spike_flag_array_at(int i) const { return m_index_flagged_spikes.GetAt(i); }
	int get_spike_flag_array_count() const { return m_index_flagged_spikes.GetCount(); }
	void change_class_of_flagged_spikes(int new_class_id);

protected:
	void read_file_version1(CArchive& ar);
	void remove_artefacts();
	void read_file_version_before5(CArchive& ar, int version);
	void read_file_version5(CArchive& ar);

	void delete_arrays();
	void serialize_version7(CArchive& ar);
	void serialize_data_parameters(CArchive& ar);
	void serialize_spike_elements(CArchive& ar);
	void serialize_spike_data(CArchive& ar);
	void serialize_spike_class_descriptors(CArchive& ar);
	void serialize_additional_data(CArchive& ar);

	int get_index_first_spike(int index_start, boolean reject_artefacts);
	void get_total_max_min_read();
	void get_total_max_min_measure();
};
