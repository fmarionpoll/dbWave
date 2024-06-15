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
	void init_source_doc(AcqDataDoc* p_document);

	CString get_acq_filename() const { return m_acquisition_file_name; }
	void set_acq_filename(CString name) { m_acquisition_file_name = name; }
	CString get_comment() const { return m_comment; }
	CTime get_acq_time() const { return m_acquisition_time; }
	float get_acq_duration() const { return static_cast<float>(m_acquisition_size) / m_acquisition_rate; }
	long get_acq_size() const { return m_acquisition_size; }
	float get_acq_rate() const { return m_acquisition_rate; }

	int get_spk_list_size() const { return spike_list_array.GetSize(); }
	void set_spk_list_size(int i) { return spike_list_array.SetSize(i); }
	SpikeList* set_spk_list_as_current(int spike_list_index);
	SpikeList* get_spk_list_current();
	SpikeList* get_spk_list_at(int spike_list_index);
	int get_spk_list_current_index() const { return m_current_spike_list; }

	int add_spk_list()
	{
		spike_list_array.SetSize(get_spk_list_size() + 1);
		return get_spk_list_size();
	}

	void set_source_filename(CString file_name) { m_acquisition_file_name = file_name; }
	void set_detection_date(CTime time) { m_detection_date = time; }
	void set_comment(CString comment) { m_comment = comment; }

	void sort_stim_array();
	void export_spk_PSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, const CString& csFileComment); // 0
	void export_spk_amplitude_histogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist,
	                              const CString& csFileComment); // 4
	void export_spk_attributes_one_file(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS); // 1,2,3
	void export_spk_file_comment(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass, const CString& csFileComment);
	void export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment);
	void export_spk_average_wave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl, CString csFileComment);
	void export_table_title(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles);
	void export_table_col_headers_db(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void export_table_col_headers_data(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void export_spk_amplitude_histogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist, int ispklist, int iclass);
	void export_spk_average_wave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, int ispklist, int iclass);
	void export_spk_PSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass);
	void export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass);

	BOOL OnSaveDocument(LPCTSTR pszPathName) override;
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR pszPathName) override;
	void clear_data();
	long build_PSTH(OPTIONS_VIEWSPIKES* options_view_spikes, long* plSum0, int iclass);
	long build_ISI(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long build_autocorr(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long build_PSTHautocorr(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);

protected:
	void Serialize(CArchive& ar) override;
	void serialize_v7(CArchive& ar);
	void read_before_version6(CArchive& ar, WORD wwVersion);
	void read_version6(CArchive& ar);
	void set_file_extension_as_spk(CString& filename);
	void serialize_acquisition_parameters(CArchive& ar);
	void serialize_stimulus_intervals(CArchive& ar);
	void serialize_spike_list_arrays(CArchive& ar);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
