#pragma once

#include "WaveBuf.h"
#include "SpikeBuffer.h"
#include "SpikeClassDescriptor.h"
#include "SpikeElement.h"
#include "spikepar.h"


// spikes from one data acquisition channel
// it contains a description of the original data acqusition set,
// how spikes were initially detected
// parameters associated to each spike and data describing the spikes
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
	int m_imaxmin2SL = 60;

	// Description
protected:
	// (0) ---------------infos about file version and ID
	WORD m_version = 6;		// aug 2013 change spike element
	CString ID_string = _T("Awave Spike file v");

	// (1) ---------------infos about data acquisition and spike detection ------------------
	CWaveChan m_acquisition_channel{};	
	WORD m_data_encoding_mode{};
	long m_bin_zero = 4096; 
	float m_sampling_rate = 10000.f;
	float m_volts_per_bin = 0.001f; 
	SPKDETECTPARM m_detection_parameters{};
	CString m_spike_channel_description;

	// (2) -------------ordered spike list with class, time, etc-----------------------------

	BOOL m_extrema_valid = false;
	int m_minimum_over_all_spikes = 0; 
	int m_maximum_over_all_spikes = 0; 
	CArray<SpikeElement*, SpikeElement*> m_spike_elements{}; 

	// (3) -------------unordered data buffers with spikes extracted from acq data-----------

	CSpikeBuffer m_spike_buffer{}; // spike data buffer

	// (4) miscellaneous

	BOOL m_b_save_artefacts = false; // save (yes/no) artefacts - default = FALSE
	BOOL m_only_valid_classes = false; // class list (array with classnb & nb spikes/class)
	int m_n_classes = 0;
	CArray<SpikeClassDescriptor, SpikeClassDescriptor> m_spike_class_descriptor_array{};
	// TODO enrich class to make use of indexes et al apparent into a class

	//  (5) list of spikes flagged

	CArray<int, int> m_spikes_flagged_;

	// Operations
public:
	WORD GetVersion() const { return m_version; }
	BOOL IsClassListValid() const { return m_only_valid_classes; }

	int GetNbclasses() const
	{
		if (m_only_valid_classes) 
			return m_n_classes;
		return -1;
	}

	int GetclassID(int i) const { return m_spike_class_descriptor_array.GetAt(i).id_number; }
	int GetclassNbspk(int i) const { return m_spike_class_descriptor_array.GetAt(i).n_items; }
	void SetclassNbspk(int i, int n_spikes) { m_spike_class_descriptor_array.GetAt(i).n_items = n_spikes; }

	int GetSpikeClass(int no) const { return m_spike_elements[no]->get_class(); }
	long GetSpikeTime(int no) const { return m_spike_elements[no]->get_time(); }
	int GetSpikeChan(int no) const { return m_spike_elements[no]->get_source_channel(); }
	void GetSpikeExtrema(int no, int* max, int* min) { m_spike_elements[no]->GetSpikeExtrema(max, min); }
	void GetSpikeMaxmin(int no, int* max, int* min, int* dmaxmin) { m_spike_elements[no]->GetSpikeMaxMin(max, min, dmaxmin); }
	int GetSpikeAmplitudeOffset(int no) const { return m_spike_elements[no]->get_amplitude_offset(); }
	int GetSpikeValAt(int no, int index) const { return *(GetpSpikeData(no) + index); }
	int GetSpikeLength() const { return m_spike_buffer.GetSpikeLength(); }
	int GetTotalSpikes() const { return m_spike_elements.GetCount(); }

	void SetSpikeClass(int no, const int nclass)
	{
		m_spike_elements[no]->set_class(nclass);
		m_only_valid_classes = FALSE;
	}

	void SetSpikeTime(int no, long ii_time) { m_spike_elements[no]->set_time(ii_time); }

	SpikeElement* GetSpikeElemt(int no) { return m_spike_elements.GetAt(no); }

	WORD GetAcqEncoding() const { return m_data_encoding_mode; }
	float GetAcqSampRate() const { return m_sampling_rate; }
	float GetAcqVoltsperBin() const { return m_volts_per_bin; }
	int GetSpikePretrig() const { return m_detection_parameters.prethreshold; }
	int GetSpikeRefractory() const { return m_detection_parameters.refractory; }
	int GetAcqBinzero() const { return m_bin_zero; }
	int GetdetectTransform() const { return m_detection_parameters.detectTransform; }
	int GetextractNpoints() const { return m_detection_parameters.extractNpoints; }
	int GetextractChan() const { return m_detection_parameters.extractChan; }
	int GetextractTransform() const { return m_detection_parameters.extractTransform; }
	int GetcompensateBaseline() const { return m_detection_parameters.compensateBaseline; }
	CString GetComment() const { return m_detection_parameters.comment; }
	int GetdetectWhat() const { return m_detection_parameters.detectWhat; }
	int GetdetectChan() const { return m_detection_parameters.detectChan; }
	int GetdetectThreshold() const { return m_detection_parameters.detectThreshold; }
	float GetdetectThresholdmV() const { return m_detection_parameters.detectThresholdmV; }

	void SetFlagSaveArtefacts(BOOL flag) { m_b_save_artefacts = flag; }
	void SetextractChan(int echan) { m_detection_parameters.extractChan = echan; }
	void SetdetectChan(int dchan) { m_detection_parameters.detectChan = dchan; }
	void SetextractTransform(int extractTransform) { m_detection_parameters.extractTransform = extractTransform; }

	void SetDetectParms(SPKDETECTPARM* pSd) { m_detection_parameters = *pSd; }
	SPKDETECTPARM* GetDetectParms() { return &m_detection_parameters; }

	int AddSpike(short* lpsource, int nchans, long ii_time, int sourcechan, int iclass, BOOL bCheck);
	BOOL TransferDataToSpikeBuffer(int no, short* lpsource, int nchans, BOOL badjust = FALSE);
	short* GetpSpikeData(int no) const { return m_spike_buffer.GetSpike(no); }
	int RemoveSpike(int spikeindex);
	BOOL IsAnySpikeAround(long ii_time, int jitter, int& spikeindex, int ichan);

	void MeasureSpikeMaxMin(int no, int* max, int* imax, int* min, int* imin);
	void MeasureSpikeMaxMinEx(int no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);
	void MeasureSpikeMaxThenMin(int no, int* max, int* imax, int* min, int* imin);
	void MeasureSpikeMaxThenMinEx(int no, int* max, int* imax, int* min, int* imin, int ifirst, int ilast);

	void GetTotalMaxMin(BOOL b_recalculate, int* max, int* min);
	void OffsetSpikeAmplitude(int no, int valfirst, int vallast, int center = 0);
	void CenterSpikeAmplitude(int spkindex, int ifirst, int ilast, WORD method = 0);
	BOOL InitSpikeList(AcqDataDoc* pDataFile, SPKDETECTPARM* pFC);
	long UpdateClassList();
	void EraseData();
	void ChangeSpikeClassID(int oldclaID, int newclaID);

	// measure parameters
	void Measure_case0_AmplitudeMinToMax(int t1, int t2);
	void Measure_case1_AmplitudeAtT(int t);
	void Measure_case2_AmplitudeAtT2MinusAtT1(int t1, int t2);

	CSize Measure_Y1_MaxMin();

	BOOL SortSpikeWithY1(CSize fromclass_toclass, CSize timewindow, CSize limits);
	BOOL SortSpikeWithY1AndY2(CSize fromclass_toclass, CSize timewindow, CSize limits1, CSize limits2);

	int GetValidSpikeNumber(int ispike);
	int GetNextSpike(int spikeno, int delta, BOOL bKeepSameClass);

public:
	int SetSpikeFlag(int spikeno, BOOL bFlag);
	int ToggleSpikeFlag(int spikeno);
	void SetSingleSpikeFlag(int spikeno);
	BOOL GetSpikeFlag(int spikeno);
	void RemoveAllSpikeFlags() { if (m_spikes_flagged_.GetCount() > 0) m_spikes_flagged_.RemoveAll(); }
	void FlagRangeOfSpikes(long l_first, long l_last, BOOL bSet);
	void SelectSpikeswithinRect(int vmin, int vmax, long l_first, long l_ast, BOOL bAdd);

	void GetRangeOfSpikeFlagged(long& l_first, long& l_last);
	BOOL GetSpikeFlagArrayAt(int i) const { return m_spikes_flagged_.GetAt(i); }
	int GetSpikeFlagArrayCount() const { return m_spikes_flagged_.GetCount(); }

protected:
	void read_file_version1(CArchive& ar);
	void remove_artefacts();
	void read_file_version_before5(CArchive& ar, int version);
	void read_file_version5(CArchive& ar);
	void read_file_version6(CArchive& ar);

	void write_file_version6(CArchive& ar);
	void delete_arrays();
	void serialize_data_parameters(CArchive& ar);
	void serialize_spike_elements(CArchive& ar);
	void serialize_spike_data(CArchive& ar);
	void serialize_spike_class_descriptors(CArchive& ar);
	void serialize_additional_data(CArchive& ar);
};
