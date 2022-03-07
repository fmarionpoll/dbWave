#pragma once

#include "Intervals.h"
//#include "spikepar.h"	
#include "Spikelist.h"
#include "Spikeclas.h"
#include "OPTIONS_VIEWSPIKES.h"

class CSpikeDoc : public CDocument
{
	DECLARE_SERIAL(CSpikeDoc)

public:
	CSpikeDoc();
	~CSpikeDoc() override;

	// Attributes
public:
	WORD m_wVersion = 7;
	CTime m_detectiondate;
	CString m_comment = _T(""); 
	CString m_acqfile = _T("");
	CString m_newpath = _T("");
	CString m_acqcomment = _T("");
	CWaveFormat m_wformat {};
	CTime m_acqtime {}; 
	float m_acqrate = 1.f;
	long m_acqsize = 0;
	CSpikeClass m_spkclass;
	int m_currspklist = -1;
	CIntervals m_stimIntervals; 

protected:
	CArray<CSpikeList, CSpikeList> spikelist_array = {};

	// Operations
public:
	CString GetFileInfos();
	void InitSourceDoc(AcqDataDoc* p_document);

	CString GetSourceFilename() const { return m_acqfile; }
	CTime GetDate() const { return m_detectiondate; }
	CString GetComment() const { return m_comment; }
	CTime GetAcqTime() const { return m_acqtime; }
	float GetAcqDuration() const { return static_cast<float>(m_acqsize) / m_acqrate; }
	long GetAcqSize() const { return m_acqsize; }
	float GetAcqRate() const { return m_acqrate; }

	int GetSpkList_Size() const { return spikelist_array.GetSize(); }
	void SetSpkList_Size(int i) { return spikelist_array.SetSize(i); }
	CSpikeList* SetSpkList_AsCurrent(int ichan);
	CSpikeList* GetSpkList_Current();
	CSpikeList* GetSpkList_At(int ichan);
	int GetSpkList_CurrentIndex() const { return m_currspklist; }

	int AddSpkList()
	{
		spikelist_array.SetSize(GetSpkList_Size() + 1);
		return GetSpkList_Size();
	}

	void SetSourceFilename(CString cs) { m_acqfile = cs; }
	void SetDetectionDate(CTime t) { m_detectiondate = t; }
	void SetComment(CString cs) { m_comment = cs; }

	void SortStimArray();

public:
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
	void export_spk_psth(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass);
	void export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass);

	// Implementation
public:
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
	void readBeforeVersion6(CArchive& ar, WORD wwVersion);
	void readVersion6(CArchive& ar);
	void readVersion7(CArchive& ar);
	void set_file_extension_as_spk(CString& filename);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
