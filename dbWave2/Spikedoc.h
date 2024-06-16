#pragma once

#include "Intervals.h"
#include "Spikelist.h"
#include "SpikeClass.h"
#include "OPTIONS_VIEWSPIKES.h"

class CSpikeDoc : public CDocument
{
	DECLARE_SERIAL(CSpikeDoc)

	CSpikeDoc();
	~CSpikeDoc() override;

	SpikeClass m_spike_class{};
	int m_current_spike_list = -1;
	CIntervals m_stimulus_intervals{};
	CWaveFormat m_wave_format{};

protected:
	CArray<SpikeList, SpikeList> spike_list_array{};
	CTime m_detection_date{};
	WORD m_wVersion = 7;
	CString m_comment = _T("");
	CString m_acquisition_file_name = _T("");
	CString m_new_path = _T("");
	CString m_acquisition_comment = _T("");
	CTime m_acquisition_time{};
	float m_acquisition_rate = 1.f;
	long m_acquisition_size = 0;


	// Operations
public:
	CString get_file_infos();
	void init_source_doc(const AcqDataDoc* p_document);

	CString get_acq_filename() const { return m_acquisition_file_name; }
	void set_acq_filename(const CString& name) { m_acquisition_file_name = name; }
	CString get_comment() const { return m_comment; }
	CTime get_acq_time() const { return m_acquisition_time; }
	float get_acq_duration() const { return static_cast<float>(m_acquisition_size) / m_acquisition_rate; }
	long get_acq_size() const { return m_acquisition_size; }
	float get_acq_rate() const { return m_acquisition_rate; }

	int get_spike_list_size() const { return spike_list_array.GetSize(); }
	void set_spike_list_size(int i) { return spike_list_array.SetSize(i); }
	SpikeList* set_spike_list_as_current(int spike_list_index);
	SpikeList* get_spike_list_current();
	SpikeList* get_spike_list_at(int spike_list_index);
	int get_spike_list_current_index() const { return m_current_spike_list; }

	int add_spk_list()
	{
		spike_list_array.SetSize(get_spike_list_size() + 1);
		return get_spike_list_size();
	}

	void set_source_filename(const CString& file_name) { m_acquisition_file_name = file_name; }
	void set_detection_date(const CTime time) { m_detection_date = time; }
	void set_comment(const CString& comment) { m_comment = comment; }

	void sort_stimulus_array();
	void export_spk_psth(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, long* pl_sum0, const CString& cs_file_comment); 
	void export_spk_amplitude_histogram(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, long* p_hist,
	                              const CString& cs_file_comment); // 4
	void export_spk_attributes_one_file(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes); // 1,2,3
	void export_spk_file_comment(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes, int class_index, const CString& cs_file_comment);
	void export_spk_latencies(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, int n_intervals, const CString& cs_file_comment);
	void export_spk_average_wave(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, double* value, const CString& cs_file_comment);
	void export_table_title(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, int n_files);
	static void export_table_col_headers_db(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes);
	void export_table_col_headers_data(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes);
	void export_spk_amplitude_histogram(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes, long* p_hist0, int spike_list_index, int class_index);
	void export_spk_average_wave(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes, double* value, int spike_list_index, int class_index);
	void export_spk_psth(CSharedFile* shared_file, OPTIONS_VIEWSPIKES* options_view_spikes, long* sum0, int spike_list_index, int class_index);
	void export_spk_latencies(CSharedFile* shared_file, const OPTIONS_VIEWSPIKES* options_view_spikes, int intervals_count, int spike_list_index, int class_index);

	BOOL OnSaveDocument(LPCTSTR pszPathName) override;
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR pszPathName) override;
	void clear_data();
	long build_psth(const OPTIONS_VIEWSPIKES* options_view_spikes, long* pl_sum0, int class_index);
	long build_isi(const OPTIONS_VIEWSPIKES* options_view_spikes, long* pl_sum0, int class_index);
	long build_autocorrelation(const OPTIONS_VIEWSPIKES* options_view_spikes, long* sum0, int class_index);
	long build_psth_autocorrelation(const OPTIONS_VIEWSPIKES* options_view_spikes, long* sum0, int class_index);

protected:
	void Serialize(CArchive& ar) override;
	void serialize_v7(CArchive& ar);
	void read_before_version6(CArchive& ar, WORD wwVersion);
	void read_version6(CArchive& ar);
	static void set_file_extension_as_spk(CString& file_name);
	void serialize_acquisition_parameters(CArchive& ar);
	void serialize_stimulus_intervals(CArchive& ar);
	void serialize_spike_list_arrays(CArchive& ar);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
