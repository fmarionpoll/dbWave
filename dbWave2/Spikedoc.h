#pragma once

#include "Intervals.h"
#include "Spikelist.h"
#include "options_view_spikes.h"

class CSpikeDoc : public CDocument
{
	DECLARE_SERIAL(CSpikeDoc)

	CSpikeDoc();
	~CSpikeDoc() override;

	CIntervals m_stimulus_intervals{};
	CWaveFormat m_wave_format{};

protected:
	CArray<SpikeList, SpikeList> spike_list_array_{};
	CTime detection_date_{};
	WORD w_version_  {7};
	CString comment_  {_T("")};
	CString new_path_  {_T("")};
	CString acquisition_file_name_  {_T("")};
	CString acquisition_comment_  {_T("")};
	CTime acquisition_time_{};
	float acquisition_rate_  {1.f};
	long acquisition_size_  {0};
	int current_spike_list_index_  {0};

	// Operations
public:
	CString 	get_file_infos();
	void 		init_source_doc(const AcqDataDoc* p_document);

	CString		get_acq_filename() const { return acquisition_file_name_; }
	void		set_acq_filename(const CString& name) { acquisition_file_name_ = name; }

	CString		get_comment() const { return comment_; }
	CTime		get_acq_time() const { return acquisition_time_; }
	float		get_acq_duration() const { return static_cast<float>(acquisition_size_) / acquisition_rate_; }
	long		get_acq_size() const { return acquisition_size_; }
	float		get_acq_rate() const { return acquisition_rate_; }

	int			get_spike_list_size() const { return spike_list_array_.GetSize(); }
	void		set_spike_list_size(const int i) { return spike_list_array_.SetSize(i); }
	SpikeList*	get_spike_list_at(int spike_list_index);
	int			get_index_current_spike_list() const { return current_spike_list_index_; }
	SpikeList*	set_index_current_spike_list(int spike_list_index);
	SpikeList*	get_spike_list_current();
	int 		add_spk_list()
	{
		spike_list_array_.SetSize(get_spike_list_size() + 1);
		return get_spike_list_size();
	}

	void set_source_filename(const CString& file_name) { acquisition_file_name_ = file_name; }
	void set_detection_date(const CTime time) { detection_date_ = time; }
	void set_comment(const CString& comment) { comment_ = comment; }

	void sort_stimulus_array();
	void export_spk_psth(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* pl_sum0, const CString& cs_file_comment); 
	void export_spk_amplitude_histogram(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* p_hist,
	                              const CString& cs_file_comment); // 4
	void export_spk_attributes_one_file(CSharedFile* shared_file, const options_view_spikes* options_view_spikes); // 1,2,3
	void export_spk_file_comment(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, int class_index, const CString& cs_file_comment);
	void export_spk_latencies(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, int n_intervals, const CString& cs_file_comment);
	void export_spk_average_wave(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, double* value, const CString& cs_file_comment);
	void export_table_title(CSharedFile* shared_file, options_view_spikes* options_view_spikes, int n_files);
	static void export_headers_descriptors(CSharedFile* shared_file, options_view_spikes* options_view_spikes);
	void export_headers_data(CSharedFile* shared_file, const options_view_spikes* options_view_spikes);
	void export_spk_amplitude_histogram(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* p_hist0, int spike_list_index, int class_index);
	void export_spk_average_wave(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, double* value, int spike_list_index, int class_index);
	void export_spk_psth(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* sum0, int spike_list_index, int class_index);
	void export_spk_latencies(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, int intervals_count, int spike_list_index, int class_index);

	BOOL OnSaveDocument(LPCTSTR psz_path_name) override;
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR psz_path_name) override;
	void clear_data();

	long build_psth(const options_view_spikes* options_view_spikes, long* pl_sum0, int class_index);
	long build_isi(const options_view_spikes* options_view_spikes, long* pl_sum0, int class_index);
	long build_autocorrelation(const options_view_spikes* options_view_spikes, long* sum0, int class_index);
	long build_psth_autocorrelation(const options_view_spikes* options_view_spikes, long* sum0, int class_index);

protected:
	void Serialize(CArchive& ar) override;
	void serialize_v7(CArchive& ar);
	void read_before_version6(CArchive& ar, WORD w_version);
	void read_version6(CArchive& ar);
	static void set_file_extension_as_spk(CString& file_name);
	void serialize_acquisition_parameters(CArchive& ar);
	void serialize_stimulus_intervals(CArchive& ar);
	void serialize_spike_list_arrays(CArchive& ar);

protected:
	DECLARE_MESSAGE_MAP()
};
