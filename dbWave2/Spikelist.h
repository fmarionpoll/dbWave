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
	WORD GetVersion() const { return m_version; }
	BOOL IsClassListValid() const { return m_keep_only_valid_classes; }

	int get_classes_count() const;
	int get_class_id(const int i) const { return m_spike_class_descriptors.GetAt(i).get_class_id();}
	void set_class_id(const int i, const int id) { m_spike_class_descriptors.GetAt(i).set_class_id(id); }
	int add_class_id(const int id);

	int get_class_n_items(const int i) const { return m_spike_class_descriptors.GetAt(i).get_n_items(); }
	void set_class_n_items(const int i, int n_spikes) { m_spike_class_descriptors.GetAt(i).set_n_items(n_spikes); }

	int get_class_id_descriptor_index(int class_id);
	int get_class_id_n_items(const int class_id);
	int increment_class_n_items(int class_id);
	int decrement_class_n_items(int class_id);
	void change_spike_class_id(int spike_no, int class_id);

	Spike* get_spike(int no) { return m_spikes.GetAt(no); }
	int get_spikes_count() const { return m_spikes.GetCount(); }

	WORD GetAcqEncoding() const { return m_data_encoding_mode; }
	float GetAcqSampRate() const { return m_sampling_rate; }
	float GetAcqVoltsperBin() const { return m_volts_per_bin; }
	int GetAcqBinzero() const { return m_bin_zero; }

	void set_detection_parameters(SPKDETECTPARM* pSd) { m_detection_parameters = *pSd; }
	SPKDETECTPARM* get_detection_parameters() { return &m_detection_parameters; }

	int AddSpike(short* lpsource, int n_channels, long ii_time, int source_channel, int i_class, BOOL bCheck);

	int  get_spike_length() const { return m_spike_length; }
	void set_spike_length(int spike_length) { m_spike_length = spike_length; }

	int RemoveSpike(int spike_index);
	BOOL IsAnySpikeAround(long ii_time, int jitter, int& spike_index, int channel_index);

	void GetTotalMaxMin(BOOL b_recalculate, short* max, short* min);

	BOOL InitSpikeList(AcqDataDoc* pDataFile, SPKDETECTPARM* pFC);
	long UpdateClassList();
	void EraseData();
	void ChangeAllSpikeFromClassIDToNewClassID(int old_class_ID, int new_class_ID);

	void Measure_case0_AmplitudeMinToMax(int t1, int t2);
	void Measure_case1_AmplitudeAtT(int t);
	void Measure_case2_AmplitudeAtT2MinusAtT1(int t1, int t2);
	CSize Measure_Y1_MaxMin();

	BOOL SortSpikeWithY1(CSize from_class_ID_to_class_ID, CSize time_bounds, CSize limits);
	BOOL SortSpikeWithY1AndY2(CSize from_class_ID_to_class_ID, CSize time_bounds, CSize limits1, CSize limits2);

	int GetValidSpikeNumber(int spike_index) const;
	int GetNextSpike(int spike_index, int delta, BOOL b_keep_same_class) ;

public:
	int SetSpikeFlag(int spike_index, BOOL set_spike_flag);
	int ToggleSpikeFlag(int spike_index);
	void set_single_spike_flag(int spike_index);
	BOOL GetSpikeFlag(int spike_index);
	void RemoveAllSpikeFlags();
	void FlagRangeOfSpikes(long l_first, long l_last, BOOL bSet);
	void SelectSpikesWithinBounds(int v_min, int v_max, long l_first, long l_last, BOOL b_add);
	void GetRangeOfSpikeFlagged(long& l_first, long& l_last);
	BOOL GetSpikeFlagArrayAt(int i) const { return m_index_flagged_spikes.GetAt(i); }
	int GetSpikeFlagArrayCount() const { return m_index_flagged_spikes.GetCount(); }
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
