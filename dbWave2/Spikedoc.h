#pragma once

#include "StimLevelArray.h"
//#include "spikepar.h"		// spike parameters options
#include "Spikelist.h"		// list of spikes
#include "Spikeclas.h"		// spike classes

class CSpikeDoc : public CDocument
{
	DECLARE_SERIAL(CSpikeDoc)

public:
	CSpikeDoc();
	virtual ~CSpikeDoc();
	
// Attributes
public:
	WORD				m_wVersion;	
	CTime 				m_detectiondate;	// date of creation
	CString 			m_comment;			// spike comments
	CString 			m_acqfile;			// data source file
	CString				m_newpath;
	CString				m_acqcomment;		// data acquisition comment	
	CWaveFormat			m_wformat;
	CTime				m_acqtime;			// data acquisition time
	float				m_acqrate;			// acquisition sampling rate
	long				m_acqsize;			// duration (nb of pts/chan)
	CSpikeClass			m_spkclass;			// list of templates
	int					m_currspklist;
	CIntervalsAndLevels	m_stimIntervals;	// stimulus array

protected:
	CArray<CSpikeList, CSpikeList> spikelist_array;

// Operations
public:
	CString		GetFileInfos();
	void		InitSourceDoc(CAcqDataDoc* p_document);

	CString		GetSourceFilename()			const {return m_acqfile;}
	CTime 		GetDate()					const {return m_detectiondate;}
	CString		GetComment()				const {return m_comment;}

	int			GetSpkListSize() const		{return spikelist_array.GetSize();}
	void		SetSpkListSize(int i)		{return spikelist_array.SetSize(i);}
	CSpikeList*	SetSpkListCurrent(int ichan);
	CSpikeList*	GetSpkListCurrent();
	int			GetSpkListCurrentIndex()	const {return m_currspklist;}
	int			AddSpkList() {spikelist_array.SetSize(GetSpkListSize()+1); return GetSpkListSize();}
	
	void		SetSourceFilename(CString cs)	{m_acqfile=cs;}
	void		SetDetectionDate(CTime t)		{m_detectiondate=t;}
	void		SetComment(CString cs)			{m_comment=cs;}

	CTime		GetAcqTime()		const {return m_acqtime;}
	float 		GetAcqDuration()	const {return m_acqsize/m_acqrate;}
	long		GetAcqSize()		const {return m_acqsize;}
	float		GetAcqRate()		const {return m_acqrate;}

	void		SortStimArray();
	
public: 	
	void ExportSpkPSTH				(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, const CString& csFileComment);	// 0
	void ExportSpkAmplitHistogram	(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist, const CString& csFileComment);	// 4
	void ExportSpkAttributesOneFile	(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);					// 1,2,3
	void ExportSpkFileComment		(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass, const CString& csFileComment);
	void ExportSpkLatencies			(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment);
	void ExportSpkAverageWave		(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl, CString csFileComment);
	void ExportTableTitle			(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles);
	void ExportTableColHeaders_db	(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void ExportTableColHeaders_data	(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS);
	void _ExportSpkAmplitHistogram	(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist, int ispklist, int iclass);
	void _ExportSpkAverageWave		(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, int ispklist, int iclass);
	void _ExportSpkPSTH				(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass);
	void _ExportSpkLatencies		(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass);
	

// Implementation
public:
	BOOL OnSaveDocument (LPCTSTR pszPathName);
	BOOL OnNewDocument();
	BOOL OnOpenDocument (LPCTSTR pszPathName);
	void ClearData();    
	long BuildPSTH			(OPTIONS_VIEWSPIKES* vdS, long* pSum0, int iclass);
	long BuildISI			(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long BuildAUTOCORR		(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);
	long BuildPSTHAUTOCORR	(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass);

protected:
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	void ReadBeforeVersion6(CArchive& ar, WORD wwVersion);
	void ReadVersion6(CArchive& ar);
	void ReadVersion7(CArchive& ar);

	// Generated message map functions
protected:
	
	DECLARE_MESSAGE_MAP()
};
