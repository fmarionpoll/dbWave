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
	CString GetFileInfos();
	void InitSourceDoc(AcqDataDoc* p_document);

	CString GetAcqFilename() const { return m_acquisition_file_name; }
	void SetAcqFilename(CString name) { m_acquisition_file_name = name; }
	CString GetComment() const { return m_comment; }
	CTime GetAcqTime() const { return m_acquisition_time; }
	float GetAcqDuration() const { return static_cast<float>(m_acquisition_size) / m_acquisition_rate; }
	long GetAcqSize() const { return m_acquisition_size; }
	float GetAcqRate() const { return m_acquisition_rate; }

	int GetSpkList_Size() const { return spike_list_array.GetSize(); }
	void SetSpkList_Size(int i) { return spike_list_array.SetSize(i); }
	SpikeList* SetSpkList_AsCurrent(int channel);
	SpikeList* GetSpkList_Current();
	SpikeList* GetSpkList_At(int channel);
	int GetSpkList_CurrentIndex() const { return m_current_spike_list; }

	int AddSpkList()
	{
		spike_list_array.SetSize(GetSpkList_Size() + 1);
		return GetSpkList_Size();
	}

	void SetSourceFilename(CString file_name) { m_acquisition_file_name = file_name; }
	void SetDetectionDate(CTime time) { m_detection_date = time; }
	void SetComment(CString comment) { m_comment = comment; }

	void SortStimArray();
	void ExportSpkPSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, const CString& csFileComment); // 0
	void ExportSpkAmplitHistogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist,
	                              const CString& csFileComment); // 4
	void ExportSpkAttributesOneFile(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS); // 1,2,3
	void ExportSpkFileComment(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass, const CString& csFileComment);
	void ExportSpkLatencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment);
	void ExportSpkAverageWave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl, CString csFileComment);
	void ExportTableTitle(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles);
	void ExportTableColHeaders_db(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void ExportTableColHeaders_data(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void export_spk_amplitude_histogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist, int ispklist, int iclass);
	void export_spk_average_wave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, int ispklist, int iclass);
	void export_spk_PSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass);
	void export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass);

	BOOL OnSaveDocument(LPCTSTR pszPathName) override;
	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR pszPathName) override;
	void ClearData();
	long BuildPSTH(OPTIONS_VIEWSPIKES* vdS, long* pSum0, int iclass);
	long BuildISI(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long BuildAUTOCORR(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long BuildPSTHAUTOCORR(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);

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
