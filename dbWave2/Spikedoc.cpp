// spikedoc.cpp : implementation file
//
// derived from CDocument

#include "stdafx.h"
#include "acqdatad.h"
#include "spikedoc.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc

IMPLEMENT_SERIAL(CSpikeDoc, CDocument, 1 /* schema number*/ )

CSpikeDoc::CSpikeDoc()
{
	m_wVersion = 7;
	m_detectiondate = CTime::GetCurrentTime();
	m_comment = "";
	m_acqfile = "";
	m_acqtime = m_detectiondate;
	m_acqrate = 1.f;
	m_acqsize = 0;	
	m_newpath="";
	m_spklistArray.SetSize(1);
	m_currspklist = -1; //0;
}

void CSpikeDoc::ClearData()
{
	m_spkclass.EraseData();	
	if (m_stim.nitems > 0)
	{
		m_stim.iisti.RemoveAll();
		m_stim.nitems=0;
	}
	m_spklistArray.SetSize(1);
	m_spklistArray[0].EraseData();
}

BOOL CSpikeDoc::OnNewDocument()
{
	ClearData();
	// this crashes and is replaced by the 3 lines below
	//if (!CDocument::OnNewDocument())
	//	return FALSE;

	// what follows is the code of CDocument::OnNewDocument() without the last line
	DeleteContents();
	m_strPathName.Empty();      // no path name yet
	SetModifiedFlag(FALSE);     // make clean
	//OnDocumentEvent(onAfterNewDocument);		// this induces an error - no template is declared
	// for cspikedoc and the program crashes

	return TRUE;
}

CSpikeDoc::~CSpikeDoc()
{
	ClearData();
}


BEGIN_MESSAGE_MAP(CSpikeDoc, CDocument)
	
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc serialization

void CSpikeDoc::Serialize(CArchive& ar)
{
// ---------------------  spike file --------------------- 
	// header
	// spikes
	// classes
	// stimulus
	// comments...
// ---------------------  spike file --------------------- 

	if (ar.IsStoring())
	{
		ar << m_wVersion;					// W1
		ar << m_detectiondate;				// W2
		ar << m_comment;					// W3
		ar << m_acqfile;					// W4
		ar << m_acqcomment;					// W5
		ar << m_acqtime;					// W6
		ar << m_acqrate;					// W7
		ar << m_acqsize;					// W8

		int nitems = 1;
		ar << nitems;						// W9
		m_wformat.Serialize(ar);			// W10

		nitems = 1; 
		ar << nitems;						// W11
		m_stim.Serialize(ar);
	
		nitems = m_spklistArray.GetSize();	// added Aug 23, 2005
		ar << nitems;
		for (int i=0; i< nitems; i++)
			m_spklistArray[i].Serialize(ar);
		m_spkclass.Serialize(ar);
	}
	else
	{
		WORD wwVersion;
		ar >> wwVersion;					// R1
		if (wwVersion == 7)
			ReadVersion7(ar);
		else if (wwVersion == 6)
			ReadVersion6(ar);
		else if (wwVersion < 6)
			ReadBeforeVersion6(ar, wwVersion);
		else
		{
			ASSERT(FALSE);
			ATLTRACE2(_T("Version not recognized: %i\n"), wwVersion);
		}
	}
}
void CSpikeDoc::SortStimArray()
{
	int nsti = m_stim.iisti.GetSize();
	if (nsti == 0 || (m_stim.iisti[nsti-1] > m_stim.iisti[0]))
		return;

	// bubble sort from bottom to top
	for (int j=0; j< nsti; j++)
	{
		int imin = m_stim.iisti[j];
		for (int k=j+1; k<nsti; k++)
		{
			if (m_stim.iisti[k] < imin)
			{	// if new min, exchange positions
				imin = m_stim.iisti[k];
				m_stim.iisti[k] = m_stim.iisti[j];
				m_stim.iisti[j] = imin;
			}
		}
	}
}

void CSpikeDoc::ReadBeforeVersion6(CArchive& ar, WORD wwVersion)
{
	if (wwVersion >= 2 && wwVersion < 5)
	{
		ar >> m_wformat.csStimulus >> m_wformat.csConcentration;
		ar >> m_wformat.csSensillum;
	}
	ar >> m_detectiondate >> m_comment;			// R2-3
	ar >> m_acqfile >> m_acqcomment >> m_acqtime; // R4-6
	ar >> m_acqrate >> m_acqsize;				// R7-8
	
	if (wwVersion >= 3 && wwVersion <= 5)
	{
			ar >> m_stim.nitems;	// R9 - load number of items
			m_stim.iisti.Serialize(ar);	// R10 - read data from file
			SortStimArray();
	}

	if (wwVersion >= 4)
	{
		int nitems;						// presumably 1
		ar >> nitems;					// R11
		ASSERT(nitems == 1);
		m_wformat.Serialize(ar);		// R12
		ar >> nitems;					// R13 -  normally (-1): end of list
		if (nitems > 0)
		{
			int isize;
			ar >> isize; 
			ASSERT(isize == 1);
			for (int i=0; i< isize; i++)
				m_stim.Serialize(ar);
			SortStimArray();
		}
	}
	// read stimulus and spike classes
	m_spklistArray.RemoveAll();
	m_spklistArray.SetSize(1);
	m_spklistArray[0].Serialize(ar);
	m_spkclass.Serialize(ar);
}

void CSpikeDoc::ReadVersion6(CArchive& ar)
{
	ar >> m_detectiondate;		// W2
	ar >> m_comment;			// W3
	ar >> m_acqfile;			// W4
	ar >> m_acqcomment;			// W5
	ar >> m_acqtime;			// W6
	ar >> m_acqrate;			// W7
	ar >> m_acqsize;			// W8

	// version 4
	int nitems;
	ar >> nitems;				// W9
	ASSERT(nitems == 1);
	m_wformat.Serialize(ar);

	ar >> nitems;
	if (nitems > 0)
	{
		int isize;
		ar >> isize; 
		ASSERT(isize == 1);
		for (int i=0; i< isize; i++)
			m_stim.Serialize(ar);
	}

	// read stimulus and spike classes
	m_spklistArray.RemoveAll();
	m_spklistArray.SetSize(1);
	m_spklistArray[0].Serialize(ar);
	SortStimArray();
	m_spkclass.Serialize(ar);
}

void CSpikeDoc::ReadVersion7(CArchive& ar)
{
	ar >> m_detectiondate;		// W2
	ar >> m_comment;			// W3
	ar >> m_acqfile;			// W4
	ar >> m_acqcomment;			// W5
	ar >> m_acqtime;			// W6
	ar >> m_acqrate;			// W7
	ar >> m_acqsize;			// W8

	int nitems;	
	ar >> nitems;				// W9
	ASSERT(nitems == 1);
	m_wformat.Serialize(ar);	

	int isize;
	ar >> isize; 
	ASSERT(isize <= 1);
	for (int i=0; i< isize; i++)
		m_stim.Serialize(ar);
	SortStimArray();

	m_spklistArray.RemoveAll();
	ar >> isize;
	m_spklistArray.SetSize(isize);
	for (int i=0; i< isize; i++)
		m_spklistArray[i].Serialize(ar);
	m_spkclass.Serialize(ar);
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc commands
/**************************************************************************/


/**************************************************************************
 function:   OnSaveDocument
 purpose:	 save document
 parameters: char* pszPathName
 returns:	 FALSE if save failed 
 comments:
 **************************************************************************/

BOOL CSpikeDoc::OnSaveDocument (LPCTSTR pszPathName)
{
	// check that path name has ".spk"
	CString docname = pszPathName;	// copy name into CString object
	int i = docname.ReverseFind('.');		// find extension separator
	if (i > 0)
		docname = docname.Left(i);	// clip name to remove extension
	docname += ".spk";				// add "fresh" extension (spk)
		
	CFileStatus rStatus;			// file status: time creation, ...
	BOOL bFlagExist = CFile::GetStatus(docname, rStatus);

	if (bFlagExist && (rStatus.m_attribute & CFile::readOnly))
	{
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, docname);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
		case IDCANCEL:
			return FALSE;			// don't continue

		case IDYES:
			{
			// If so, either Save or Update, as appropriate DoSave
			CDocTemplate* pTemplate = GetDocTemplate();
			ASSERT(pTemplate != NULL);
			BOOL bReplace = TRUE;
			if (!m_newpath.IsEmpty())
			{
				int i = docname.ReverseFind('\\')+1;
				if (i != -1)
					docname.Mid(i);
				docname = m_newpath + docname;
			}

			if (!AfxGetApp()->DoPromptFileName(docname,
				bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
				OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
				return FALSE;       // don't even attempt to save
			int i = docname.ReverseFind('\\')+1;	// find last occurence of antislash	
			if (i != -1) 
				m_newpath = docname.Left(i);
			}
			break;

		case IDNO:			
			return FALSE;			// don't continue
			break;

		default:
			ASSERT(FALSE);
			break;
		}
	}

	CFile f;
	CFileException e;
	try
	{
		f.Open(docname,CFile::modeCreate|CFile::modeWrite, &e);
		CArchive ar(&f, CArchive::store);
		Serialize(ar);		//serialize object			
		ar.Flush();
		ar.Close();
		f.Close();
		SetModifiedFlag(FALSE);
	}
	catch(CFileException* pe)
	{
		f.Abort();
		ReportSaveLoadException(docname, pe, FALSE, AFX_IDP_FAILED_TO_SAVE_DOC);
		pe->Delete();
		return FALSE;
	}
	SetModifiedFlag(FALSE);	// mark the document as clean
	return TRUE;
}

/**************************************************************************
 function:   OnOpenDocument
 purpose:	 load document
 parameters: char* pszPathName
 returns:	 FALSE if load failed 
 comments:
 **************************************************************************/

BOOL CSpikeDoc::OnOpenDocument (LPCTSTR pszPathName)
{
	ClearData();
	CString filename = pszPathName;	
	
	// check if file can be opened - exit if it can't and return an empty object
	CFileStatus rStatus;
	if (!CFile::GetStatus(pszPathName, rStatus)) 
		return FALSE;
	
	CFile f;
	CFileException fe;    // trap exceptions
	BOOL bRead = TRUE;
	if (f.Open(filename, CFile::modeRead |CFile::shareDenyNone, &fe))
	{
		CArchive ar(&f, CArchive::load);
		m_currspklist = 0;
		try 
		{
			Serialize(ar);
			ar.Flush();
			ar.Close();
			f.Close();
			SetModifiedFlag(FALSE);	
			// update nb of classes
			for (int i=0; i< m_spklistArray.GetSize(); i++)
			{
				if (!m_spklistArray[i].IsClassListValid())	// if class list not valid:
				{
					m_spklistArray[i].UpdateClassList();	// rebuild list of classes
					SetModifiedFlag();						// and set modified flag
				}
			}
		}
		catch (CArchiveException * e)
		{
			e->Delete ();
			bRead = FALSE;
		}
	}
	else
	{		
		ReportSaveLoadException (filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		bRead = FALSE;
	}	
	return bRead;
}

void CSpikeDoc::InitSourceDoc(CAcqDataDoc* pDoc)
{
	// load parameters from file	
	CWaveFormat* pwaveFormat = pDoc->GetpWaveFormat();
	m_acqtime =			pwaveFormat->acqtime;
	m_acqsize =			pDoc->GetDOCchanLength();
	m_acqrate =			pwaveFormat->chrate;
	m_acqcomment =		pwaveFormat->csComment;
	m_wformat =			*pwaveFormat;	
}

////////////////////////////////////////////////////////////////////////////////

/**************************************************************************
 function:   GetFileInfos
 purpose:	 export infos on current file
 parameters: 
 returns:
 comments:
 **************************************************************************/
CString CSpikeDoc::GetFileInfos()
{
	TCHAR sz[64];
	LPTSTR psz = sz;
	TCHAR* psepTAB = _T("\t");
	TCHAR* psepRC = _T("\r\n");
	CString csOut;
	int ifile = 0;
	csOut = GetPathName( );
	csOut.MakeUpper();
	csOut += psepTAB+ m_detectiondate.Format(_T("%c"));	// date of creation
	
	// PB to convert time into date
	// m_date.Format("%c");// date of creation
	// BUG TIMECORE.CPP -- ptmTemp struct is not created when time_m is negative
	// struct tm* ptmTemp = localtime(&m_time
	csOut += psepRC + m_comment;			// spike file comment
	csOut += psepRC;						// next line

	// ------------ SOURCE DATA -------------------	
	csOut += _T("*** SOURCE DATA ***\r\n");
	csOut += m_acqfile		+ psepRC;		// data source file
	csOut += m_acqcomment	+ psepRC;		// data acquisition comment	
	csOut += m_acqtime.Format(_T("%#d-%B-%Y"))+ psepRC; // data acquisition time removed:"%c"
	
	// ------------ SPIKE LIST ---------------------
	csOut += _T("*** SPIKE LIST ***\r\n");	
	// test if empty spike list
	long lnspikes=0;
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];
	if (pspklist->GetTotalSpikes() > 0)
		lnspikes = pspklist->GetTotalSpikes();		
	wsprintf( psz,_T("n spikes = %li\r\n"), lnspikes);
	csOut += sz;

	// ----------- SPIKE CLASS ---------------------	
	// ----------- stimulus ------------------------
	// ----------- comments ------------------------
	return csOut;
}


// post stimulus histogram, ISI, autocorrelation
// vdS->exportdatatype == 0
// each line represents 1 recording
// we start with a header, then each file is scanned and exported
// if nintervals < 0: export all spikes // feb 23, 2009
void CSpikeDoc::_ExportSpkLatencies (CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment, int ispklist, int iclass)
{
	CString csDummy;
	CSpikeList* pspklist= &m_spklistArray[ispklist];
	int totalspikes = pspklist->GetTotalSpikes();

	// export time of nintervals first spikes
	int nspikes = nintervals;
	if (nspikes > totalspikes || nspikes < 0)
		nspikes = totalspikes;
	float rate = pspklist->GetAcqSampRate();

	for (int j = 0; j<nspikes; j++)
	{
			// skip classes not requested
		int cla = pspklist->GetSpikeClass(j);
		if (cla <0 /*&& !vdS->bartefacts*/)	
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)	
			continue;

		// export time
		float tval = ((float)pspklist->GetSpikeTime(j))/rate;
		csDummy.Format(_T("\t%lf"), tval);
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::ExportSpkLatencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment)
{
	CString csDummy;
	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0=0;
	int class1=0;
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	if ((vdS->spikeclassoption ==-1 
		|| vdS->spikeclassoption==1) 
		 && !pspklist->IsClassListValid()) 
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption ==-1) 
	{
		// search item index with correct class ID
		int i=pspklist->GetNbclasses()-1;
		for (i; i>= 0; i--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption==1)
		class1=pspklist->GetNbclasses()-1;
	
	// loop header..................		
	for (int kclass = class0; kclass<= class1; kclass++)
	{
		// check if class is Ok
		int nbspk_for_thisclass = pspklist->GetclassNbspk(kclass);
		if ((FALSE == vdS->bexportzero)&&(nbspk_for_thisclass == 0))
			continue;
		int iclass = pspklist->GetclassID(kclass);

		// ................................COMMENTS
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkLatencies (pSF, vdS, nintervals, csFileComment, m_currspklist, iclass);	
	}
}

void CSpikeDoc::_ExportSpkPSTH (CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, CString csFileComment, int ispklist, int iclass)
{
	CString csDummy;
	CSpikeList* pspklist= &m_spklistArray[ispklist];
	// compute how much bins we need and the boundaries
	int nbins;
	float tspan;
	float tbin;
	float tstart;
	int ilen=0;
	switch (vdS->exportdatatype)
	{	
	case EXPORT_PSTH:		// PSTH
		nbins = vdS->nbins;
		tspan = vdS->timeend - vdS->timestart;
		tbin = vdS->timebin; //tspan / nbins;
		tstart = vdS->timestart;
		break;
	case EXPORT_ISI:		// ISI
		nbins = vdS->nbinsISI;
		tspan = vdS->binISI * nbins;
		tbin = vdS->binISI;
		tstart = 0;
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		nbins = vdS->nbinsISI;
		tspan = vdS->binISI * nbins;
		tbin = vdS->binISI;
		tstart = -tspan/2.f;
		break;
	default:
		break;
	}

	// ................................DATA
	// spike list -- prepare parameters		
	long* plSum = plSum0;
	long N;
	for (int i = 0; i<nbins; i++, plSum++)
		*plSum = 0;

	// export number of spikes / interval
	switch (vdS->exportdatatype)
	{	
	case EXPORT_PSTH:		// PSTH
		N=BuildPSTH(vdS, plSum0, iclass);
		break;
	case EXPORT_ISI:		// ISI
		N=BuildISI(vdS, plSum0, iclass);
		csDummy.Format(_T("\t%li"), N); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		N=BuildAUTOCORR(vdS, plSum0, iclass);
		csDummy.Format(_T("\t%li"), N); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		break;
	default:
		break;
	}

	// export nb spikes / bin: print nb
	for (int i = 0; i<nbins; i++)
	{
		if ((vdS->bexportzero == TRUE) || (*(plSum0+i) >= 0))
			csDummy.Format(_T("\t%li"), *(plSum0+i));
		else
			csDummy = _T("\t");
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	}

	// export stimulus occurence time(s) that fit(s) into the time interval requested
	if (vdS->exportdatatype == EXPORT_PSTH && m_stim.iisti.GetSize() > 0)
	{			
		float samprate = pspklist->GetAcqSampRate();
		ASSERT(samprate != 0.f);
		int istim0 = 0;
		int istim1 = m_stim.iisti.GetSize() -1;
		int iioffset0 = 0;
		if (!vdS->babsolutetime)
			iioffset0= m_stim.iisti.GetAt(vdS->istimulusindex);
		long iistart = (long) (vdS->timestart*samprate) +iioffset0;
		long iiend = (long) (vdS->timeend*samprate) +iioffset0;
		int bUP = -1;
		BOOL bstarted = FALSE;

		for (int istim = istim0; istim <= istim1; istim++)
		{
			int iistim = m_stim.iisti.GetAt(istim);
			bUP *= -1;
			if (iistim < iistart)
				continue;
			if (iistim > iiend)
				break;
				
			float tval = ((float)iistim-iioffset0)/samprate;
			if (bUP < 0)
				csDummy.Format(_T("\t(-)%lf"), tval); 
			else
				csDummy.Format(_T("\t(+)%lf"), tval); 
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::ExportSpkPSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, CString csFileComment)
{	
	CString csDummy;
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0=0;
	int class1=0;
	if ((vdS->spikeclassoption == -1			// 1 class selected
		|| vdS->spikeclassoption== 1)			// all classes selected but displayed on different lines
		 && !pspklist->IsClassListValid()) 
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption ==-1)				// only 1 class selected
	{
		// search item index with correct class ID
		int i=pspklist->GetNbclasses()-1;
		for (i; i>= 0; i--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
		{
			ExportSpkFileComment(pSF, vdS, vdS->classnb, csFileComment);
			// dummy export if requested export nb spikes / bin: print nb
			switch (vdS->exportdatatype)
			{	
			case EXPORT_ISI:		// ISI
			case EXPORT_AUTOCORR:	// Autocorr
				csDummy.Format(_T("\t0")); 
				pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
				break;
			default:
				break;
			}
			return;
		}		
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption==1)
		class1=pspklist->GetNbclasses()-1;

	// loop header..................		
	for (int kclass = class0; kclass<= class1; kclass++)
	{
		// check if class is Ok
		int nbspk_for_thisclass = pspklist->GetclassNbspk(kclass);
		int iclass = pspklist->GetclassID(kclass);
		// export the comments
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		// test if we should continue
		if ((FALSE == vdS->bexportzero) && (nbspk_for_thisclass == 0))
			continue;
		/// export the measures
		_ExportSpkPSTH (pSF, vdS, plSum0, csFileComment, m_currspklist, iclass);
	}
}

// export histograms of the amplitude of the spikes found in each file of the file series
// export type = 4
// same tb as PSTH: each line represents one record, 1 class
void CSpikeDoc::_ExportSpkAmplitHistogram (CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, CString csFileComment, int ispklist, int iclass)
{
	// ................................DATA
	CSpikeList* pspklist= &m_spklistArray[ispklist];
	CString csDummy;
	// temp parameters for amplitudes histogram
	int histampl_min = (int) (vdS->histampl_vmin / pspklist->GetAcqVoltsperBin());
	int histampl_max = (int) (vdS->histampl_vmax / pspklist->GetAcqVoltsperBin());
	int histampl_binsize = (histampl_max - histampl_min) / vdS->histampl_nbins;
	long* pHist;
	int nbins = 1;
	// update offset
	int iioffset0 = 0;
	if (!vdS->babsolutetime && m_stim.nitems > 0) 
		iioffset0= m_stim.iisti.GetAt(vdS->istimulusindex);

	// clear histogram area
	ASSERT(vdS->exportdatatype == EXPORT_HISTAMPL);
	pHist = pHist0;
	for (int k=0; k<vdS->histampl_nbins+2; k++, pHist++)
		*pHist = 0;

	long ySum=0;
	long ySum2=0;
	long yN=0;

	float tspan = vdS->timeend - vdS->timestart;
	float tbin = tspan / nbins;
	float tfirst = vdS->timestart;
	float rate = pspklist->GetAcqSampRate();
	if (rate == 0.0)
		rate = 1.0f;
	long iitime1 = (long) (tfirst*rate);

	for (int i = 0; i<nbins; i++)
	{
		tfirst = vdS->timestart + tbin*i;
		long iitime0 = iitime1;
		iitime1 = (long) ((tfirst + tbin)*rate);

		// export number of spikes / interval
		// ------- count total spk/interval (given bspkclass&bartefacts)
		long iitotal = 0;
		for (int j=0; j<pspklist->GetTotalSpikes(); j++)
		{
			// skip intervals not requested
			long iitime = pspklist->GetSpikeTime(j) - iioffset0;
			if (iitime < iitime0) 
				continue;
			if (iitime >= iitime1) 
				break;
			// skip classes not requested
			int cla = pspklist->GetSpikeClass(j);
			if (cla <0 /*&& !vdS->bartefacts*/)	
				continue;
			if (vdS->spikeclassoption != 0 && cla != iclass)	
				continue;
			// get value, compute statistics
			int max, min;
			pspklist->GetSpikeExtrema(j, &max, &min);
			int val = max-min;
			ySum += val;
			ySum2 += val*val;
			yN ++;
			// build histogram: compute index and increment bin content
			int index = 0;
			if (val >= histampl_min && val <= histampl_max)						
				index = (val - histampl_min) / histampl_binsize + 1;
			else
				index = vdS->histampl_nbins+1;
			val = *(pHist0+index) + 1;
			*(pHist0+index)= val;				
		}

		// 4) export histogram
		if (yN > 0)
		{
			float VperBin = pspklist->GetAcqVoltsperBin()*1000.f;
			csDummy.Format(_T("\t%.3lf\t%.3lf\t%i"), 
				(ySum/yN)*VperBin,
				(ySum2)*VperBin*VperBin,
				yN);
		}
		else
			csDummy= _T("\t\t\t0"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		// actual measures
		pHist = pHist0;
		for (int k=0; k<vdS->histampl_nbins+2; k++, pHist++)
		{
			if ((vdS->bexportzero == TRUE) || (*pHist >= 0))
				csDummy.Format(_T("\t%i"), *pHist);
			else
				csDummy = _T("\t");
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::ExportSpkAmplitHistogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, CString csFileComment)
{
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0=0;
	int class1=0;
	if ((vdS->spikeclassoption ==-1 
		|| vdS->spikeclassoption==1) 
		 && !pspklist->IsClassListValid()) 
		pspklist->UpdateClassList();

	// just one class selected: see if there is at least 1 spike with this class
	if (vdS->spikeclassoption ==-1) 
	{
		// search item index with correct class ID
		int i=pspklist->GetNbclasses()-1;
		for (i; i>= 0; i--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
			return ExportSpkFileComment(pSF, vdS, vdS->classnb, csFileComment);
		// set class boundaries to the index of the class having the proper ID
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption==1)
		class1 = pspklist->GetNbclasses()-1;

	// loop over all classes..................		
	for (int kclass = class0; kclass<= class1; kclass++)
	{
		int iclass = pspklist->GetclassID(kclass);
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkAmplitHistogram(pSF, vdS, pHist0, csFileComment, m_currspklist, iclass);
	}
}


// option 1: spike intervals
// option 2: spike extrema (max and min)
// option 3: export spike amplitudes (max - min)
// one file at a time, organized in columns

void CSpikeDoc::ExportSpkAttributesOneFile(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, CString csFileComment)
{
	CString csDummy;
	int iioffset0 = 0;
	if (!vdS->babsolutetime && m_stim.nitems > 0) 
	{
		iioffset0= m_stim.iisti.GetAt(vdS->istimulusindex);
	}


	// ................................DATA
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	csDummy = _T("\r\ntime(s)\tclass"); 
	float rate = pspklist->GetAcqSampRate();
	if (rate == 0.0)
		rate = 1.0f;
	
	switch (vdS->exportdatatype)
	{
	case EXPORT_EXTREMA:
		csDummy += _T("\tmax(mV)\tmin(mV)"); 
		break;
	case EXPORT_AMPLIT:
		{
			csDummy += _T("\tamplitude(mV)\ttime(ms)"); 
			float ratems = rate/1000.f;
			float xfirst = ((float) pspklist->m_imaxmin1SL)/ ratems;
			float xlast = ((float) pspklist->m_imaxmin2SL)/ ratems;
			CString cs;
			cs.Format(_T(" [%.3f to %.3f]"), xfirst, xlast);
			csDummy += cs;
		}
		break;
	}
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

	// spike list -- prepare parameters		
	float tspan = vdS->timeend - vdS->timestart;
	float tfirst = vdS->timestart;
	long iitime1 = (long) (tfirst*rate);

	tfirst = vdS->timestart + tspan;
	long iitime0 = iitime1;
	iitime1 = (long) ((tfirst + tspan)*rate);
	int max, min;

	// export number of spikes / interval
	// ------- count total spk/interval (given bspkclass&bartefacts)
	float tval;
	float VperBin = pspklist->GetAcqVoltsperBin()*1000.f;
	int	binzero = pspklist->GetAcqBinzero();
	int imax, imin;
	int ifirst = pspklist->m_imaxmin1SL;
	int ilast = pspklist->m_imaxmin2SL;
	float ratems = rate/1000.f;

	// export data: loop through all spikes
	for (int j=0; j<pspklist->GetTotalSpikes(); j++)
	{
		// skip intervals not requested
		long iitime = pspklist->GetSpikeTime(j) - iioffset0;
		if (iitime < iitime0) 
			continue;
		if (iitime >= iitime1) 
			break;
		// skip classes not requested
		int cla = pspklist->GetSpikeClass(j);
		if (cla <0 /*&& !vdS->bartefacts*/)	
			continue;

		// export time and spike time
		tval = ((float)iitime)/rate;
		csDummy.Format(_T("\r\n%lf\t%i"), tval, cla); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

		switch (vdS->exportdatatype)
		{
		// 2) export time interval (absolute time)
		case EXPORT_INTERV:
			break;					
		// 3) export extrema
		case EXPORT_EXTREMA:
			pspklist->GetSpikeExtrema(j, &max, &min);
			csDummy.Format(_T("\t%.3lf\t%.3lf"), (max-binzero)*VperBin, (min-binzero)*VperBin);
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			break;
		// 4) export max - min
		case EXPORT_AMPLIT:	
			pspklist->MeasureSpikeMaxThenMinEx(j, &max, &imax, &min, &imin, ifirst, ilast);
			csDummy.Format(_T("\t%.3lf\t%.3lf"), ((float)(max-min))*VperBin, ((float)(imin-imax))/ratems); 
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			break;
		case EXPORT_SPIKEPOINTS:
			for (int index = 0; index< pspklist->GetSpikeLength(); index++)
			{
				int val = pspklist->GetSpikeValAt(j, index);
				csDummy.Format(_T("\t%.3lf"), ((float)val)*VperBin);
				pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			}
			break;
		}
	}
}

// Describe the type of data exported
void CSpikeDoc::ExportTableTitle(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles)
{
	CString csDummy;

	switch (vdS->exportdatatype)
	{	
	case EXPORT_PSTH:		csDummy = _T("Histogram nb spikes/bin");	break;
	case EXPORT_ISI:		csDummy = _T("Intervals between spikes histogram");	break;
	case EXPORT_AUTOCORR:	csDummy = _T("Autocorrelation histogram");	break;
	case EXPORT_LATENCY:	csDummy = _T("Latency first 10 spikes");break;
	case EXPORT_AVERAGE:	csDummy = _T("Spike average waveform"); break;
	case EXPORT_INTERV:		csDummy = _T("Spike time intervals"); nfiles = 1; break;
	case EXPORT_EXTREMA:	csDummy = _T("Spike amplitude extrema"); nfiles = 1; break;
	case EXPORT_AMPLIT:		csDummy = _T("Spike amplitudes"); nfiles = 1; break;
	case EXPORT_HISTAMPL:	csDummy = _T("Spike amplitude histograms"); break;
	case EXPORT_SPIKEPOINTS: csDummy= _T("Spike data points"); break;
	default: vdS->exportdatatype = 0; csDummy = _T("Histogram nb spikes/bin"); break;
	}
	CTime t= CTime::GetCurrentTime();				// current date & time
	CString csDate;									// first string to receive date and time
	csDate.Format(_T("  created on %d-%d-%d at %d:%d"), t.GetDay(), t.GetMonth(),t.GetYear(), t.GetHour(), t.GetMinute());
	csDummy += csDate;
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	
	switch (vdS->exportdatatype)
	{	
	case EXPORT_PSTH:
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
	case EXPORT_EXTREMA:	
		csDummy.Format(_T("\r\nnbins= %i\tbin size (s):\t%.3f\tt start (s):\t%.3f\tt end (s):\t%.3f\r\nn files = %i\r\n"), 
			vdS->nbins, 
			(float) ((vdS->timeend - vdS->timestart) / vdS->nbins),
			vdS->timestart,
			vdS->timeend,
			nfiles); 
		break;
	case EXPORT_AVERAGE:
		csDummy.Format(_T("\r\ndata:\tN\txi=mean amplitude (mV)\tSum square of amplitudes\r\nn points:\t%i\r\n"), 
			GetSpkListCurrent()->GetSpikeLength());
		break;
	case EXPORT_HISTAMPL:
	case EXPORT_LATENCY:
	default:
		csDummy.Format(_T("\r\n\r\n\r\n"));
		break;
	}
	pSF->Write(csDummy, csDummy.GetLength()*sizeof(TCHAR));
}

// Row of column headers for the database parameters
// returns number of columns used
void CSpikeDoc::ExportTableColHeaders_db(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString csDummy;
	int ncolumns;

	// these infos are stored in the database - what we do here, is equivalent of doing a query
	csDummy.Format(_T("\r\nID\tFilename")); 
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	ncolumns = 2;

	csDummy.Empty();
	if (vdS->bacqdate) 	
	{	// ................     acq date, acq time
		csDummy += _T("\tacq_date\tacq_time"); 
		ncolumns += 2;
	}
	if (vdS->bacqcomments)	
	{	// ................     acq comments, stim, conc, type
		csDummy +=_T( "\tExpt\tinsectID\tssID\tinsect\tstrain\tsex\tlocation\toperator\tmore");
		ncolumns += 9;
		csDummy += _T("\tstim1\tconc1\trepeat1\tstim2\tconc2\trepeat2\ttype\ttag"); 
		ncolumns += 8;
	}

	// this is specific to the spike file (not stored in the database)
	if (vdS->bspkcomments)	
	{	// ................     spike comments
		csDummy+= _T("\tspk_comment"); 
		ncolumns += 1;
	}
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

	// spike detection chan
	if (vdS->btotalspikes)	
	{	// ................     total spikes
		csDummy = _T("\ttotal_spikes\tnb_classes\tduration(s)"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		ncolumns += 3;
	}

	// export spike detect chan, channel and class
	csDummy = _T("\tdetectchan\tchan\tclass"); 
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	ncolumns += 3;

	vdS->ncommentcolumns = ncolumns;
}

// Row (continuation) of column headers for the measures
void CSpikeDoc::ExportTableColHeaders_data(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString csDummy;
	// header of the data
	int nbins=0;
	float tspan;
	float tbin;
	float tstart;
	switch (vdS->exportdatatype)
	{	
	case EXPORT_PSTH:		// PSTH
		nbins = vdS->nbins;
		tspan = vdS->timeend - vdS->timestart;
		tbin = tspan / nbins;
		tstart = vdS->timestart;
		break;
	case EXPORT_ISI:		// ISI
		nbins = vdS->nbinsISI;
		tspan = vdS->binISI * nbins;
		tbin = vdS->binISI;
		tstart = 0;
		csDummy = _T("\tN"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		nbins = vdS->nbinsISI;
		tspan = vdS->binISI * nbins;
		tbin = vdS->binISI;
		tstart = -tspan/2.f;
		csDummy = _T("\tN"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_HISTAMPL: 
		nbins = vdS->histampl_nbins;
		tstart = vdS->histampl_vmin;
		tbin = ((vdS->histampl_vmax - vdS->histampl_vmin)*1000.f)/ nbins;
		nbins +=2;
		tstart -=tbin;
		csDummy = _T("\tmean\tsum2\tNelmts"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AVERAGE:
		{
			int npoints = GetSpkListCurrent()->GetSpikeLength();
			csDummy = _T("\tN");
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			CString cs;
			csDummy.Empty();
			for (int i = 0; i< npoints; i++)
			{
				cs.Format(_T("\tx%i"), i);
				csDummy += cs;
			}
			for (int i = 0; i< npoints; i++)
			{
				cs.Format(_T("\tSx2_%i"), i);
				csDummy+= cs;
			}
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
		break;
	
	default:
		break;
	}

	// ................     loop to scan all time intervals
	csDummy.Empty();
	if (vdS->exportdatatype == EXPORT_LATENCY)
	{
		nbins = 10;
		for (int i = 0; i<nbins; i++)
		{
			CString cs;
			cs.Format(_T("\t%i"), i); 
			csDummy += cs;
		}
	}
	else
	{
		for (int i = 0; i<nbins; i++)
		{
			CString cs;
			cs.Format(_T("\tb_%.3f"), (float) (tstart + tbin*i)); 
			csDummy += cs;
		}
	}
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
}

// output descriptors of each record
// TODO: output descriptors from the database and not from the spike files ??
void CSpikeDoc::ExportSpkFileComment(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass, CString csFileComment)
{
	CString csDummy;
	CString csTemp;
	CString csTab = _T("\t");

	// spike file
	int ilen = 0;
	if (csFileComment.IsEmpty())
	{
		csDummy = _T("\r\n");
		csDummy += GetPathName(); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

		// source data file items
		if (vdS->bacqdate)				// source data time and date
		{
			csDummy.Format(_T("\t%s"), (LPCTSTR) m_acqtime.Format(_T("%#d %m %Y\t%X")));
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
		// source data comments
		if (vdS->bacqcomments)
		{
			csDummy = csTab +m_acqcomment;
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			csDummy.Format(_T("\t%i"), m_wformat.insectID);
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

			csDummy.Format(_T("\t%i"), m_wformat.sensillumID);
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

			csDummy =  csTab + m_wformat.csInsectname;
			csDummy += csTab + m_wformat.csStrain;
			csDummy += csTab + m_wformat.csSex;
			csDummy += csTab + m_wformat.csLocation;
			csDummy += csTab + m_wformat.csOperator;
			csDummy += csTab + m_wformat.csMoreComment;

			csDummy += csTab + m_wformat.csStimulus;
			csDummy += csTab + m_wformat.csConcentration;
			csTemp.Format(_T("%i"), m_wformat.repeat);
			csDummy += csTab + csTemp;
			csDummy += csTab + m_wformat.csStimulus2;
			csDummy += csTab + m_wformat.csConcentration2;
			csTemp.Format(_T("%i"), m_wformat.repeat2);
			csDummy += csTab + csTemp;
			csDummy += csTab + m_wformat.csSensillum;
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		pSF->Write(csFileComment, csFileComment.GetLength() * sizeof(TCHAR));
	}

	// spike file additional comments
	if (vdS->bspkcomments)
	{
		pSF->Write(csTab, csTab.GetLength() * sizeof(TCHAR));
		pSF->Write(m_comment, m_comment.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];
		
	if (vdS->btotalspikes)
	{
		csDummy.Format(_T("\t%i"), pspklist->GetTotalSpikes());
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		csDummy.Format(_T("\t%i"), pspklist->GetNbclasses());
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		float tduration = (float) m_acqsize / (float) m_acqrate;
		csDummy.Format(_T("\t%.3f"), tduration);
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	}
	// spike list item, spike class
	csDummy.Format(_T("\t%i \t%s \t%i"), vdS->ichan, (LPCTSTR) pspklist->GetComment(), iclass);
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
}

// compute poststimulus histogram
// in:
//	OPTION_VIEWSPIKES
//		nbins		number of bins
//		timestart	first valid spike occurence time (s) 
//		timeend		last valid spike occurence (s)
//		spikeclassoption	0=all spike class, -1:only one, 1=exclude artefacts
//		classnb		if !spikeclassoption, compute only from spikes with this class nb
//		babsolutetime	false=correct time with first stimulus time
//		timebin		bin duration (in s)
//	long* plSum0 array of longs to receive histogram bins
//	iclass		class selected
// out:
//	*plSum0		
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::BuildPSTH(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	// ajust parameters
	long N = 0;
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	int nspikes = pspklist->GetTotalSpikes();
	if (nspikes <= 0)
		return 0;
	float rate = pspklist->GetAcqSampRate();
	ASSERT(rate != 0.f);

	// check validity of istimulusindex
	int istimulusindex = vdS->istimulusindex;
	if (istimulusindex > m_stim.iisti.GetSize()-1)
		istimulusindex = m_stim.iisti.GetSize() -1;
	if (istimulusindex < 0)
		istimulusindex = 0;
	int istim0 = istimulusindex;
	int istim1 = istim0+1;
	int increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stim.iisti.GetSize();
		increment = vdS->nstipercycle;
		if (m_stim.npercycle > 1 && increment > m_stim.npercycle)
			increment = m_stim.npercycle;
		increment *= 2;
	}

	for (int istim = istim0; istim <istim1; istim+=increment, N++)
	{
		int iioffset0 = 0;
		if (!vdS->babsolutetime)
		{
			if (m_stim.nitems > 0) 
				iioffset0= m_stim.iisti.GetAt(istim);
			else
				iioffset0 = (long) -(vdS->timestart * rate);
		}
		long iitime_start = (long) (vdS->timestart * rate) + iioffset0;
		long iitime_end = (long) (vdS->timeend * rate) + iioffset0;
		long iitime_len = iitime_end - iitime_start;
		long iibinsize = (long) (vdS->timebin * rate); //(iitime_end - iitime_start) / vdS->nbins;
		if (iibinsize <= 0)
			iibinsize = 1;
		ASSERT(iibinsize > 0);
		
		// check file size and position pointer at the first spike within the bin
		int i0=0;
		while ((i0 <nspikes) && (pspklist->GetSpikeTime(i0) < iitime_start)) 
			i0++;

		for (int j=i0; j<nspikes; j++)
		{
			// skip intervals not requested
			int iitime = pspklist->GetSpikeTime(j);
			if (iitime >= iitime_end)
				break;

			// skip classes not requested (artefact or wrong class)
			if (pspklist->GetSpikeClass(j) <0 /*&& !vdS->bartefacts*/)
				continue;
			if (vdS->spikeclassoption != 0 && pspklist->GetSpikeClass(j) != iclass)	
				continue;

			int ii = (iitime - iitime_start) / iibinsize;
			if (ii >= vdS->nbins)
				ii = vdS->nbins;
			(*(plSum0 + ii))++;
		}
	}
	return N;
}

// compute interspike intervals histogram
// in:
//	OPTION_VIEWSPIKES
//		nbinsISI	number of bins
//		timestart	first valid spike occurence time (s) 
//		timeend		last valid spike occurence (s)
//		spikeclassoption	0=all spike class, -1:only one, 1=exclude artefacts
//		classnb		if !spikeclassoption, compute only from spikes with this class nb
//		babsolutetime	false=correct time with first stimulus time
//		binISI		bin duration (in s)

//	long* plSum0 array of longs to receive histogram bins
//	iclass		class selected
// out:
//	*plSum0		
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::BuildISI(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	float samprate = pspklist->GetAcqSampRate();		// sampling rate
	long N = 0;
	ASSERT(samprate != 0.0f);	// converting iitime into secs and back needs <> 0!
	int nspikes = pspklist->GetTotalSpikes();	// this is total nb of spikes within file
	float binsize = vdS->binISI;				// bin size (in secs)
	long iibinsize = int(binsize * samprate);
	ASSERT(iibinsize > 0);
	
	int istim0 = vdS->istimulusindex;
	int istim1 = istim0+1;
	int increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stim.iisti.GetSize();
		increment = vdS->nstipercycle;
		if (m_stim.npercycle > 1 && increment > m_stim.npercycle)
			increment = m_stim.npercycle;
		increment *= 2;
	}
	
	for (int istim = istim0; istim <istim1; istim+= increment, N++)
	{

		long iistart = (long) (vdS->timestart*samprate);	// boundaries
		long iiend = (long) (vdS->timeend*samprate);
		if (!vdS->babsolutetime && m_stim.nitems > 0)	// adjust boundaries if ref is made to
		{										// a stimulus
				iistart += m_stim.iisti.GetAt(istim);
				iiend += m_stim.iisti.GetAt(istim);
		}

		// find first spike within interval requested
		// assume all spikes are ordered with respect to their occurence time
		long iitime0;
		int j= 0;
		for (j=0; j< nspikes; j++)
		{
			iitime0 = pspklist->GetSpikeTime(j);
			if (iitime0 <= iiend && iitime0 >= iistart)
				break;	// found!
		}
		if (nspikes <= 0 || j >= nspikes)
			return 0;

		// build histogram
		for (int i = j+1; i<nspikes; i++)
		{
			long iitime = pspklist->GetSpikeTime(i);
			if (iitime > iiend)
				break;
			ASSERT (iitime > iitime0);
			if (iitime < iitime0)
			{ 
				iitime0 = iitime; 
				continue;
			}
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;
			int ii = (int)((iitime-iitime0) / iibinsize);
			if (ii <= (int) vdS->nbinsISI)
			{
				(*(plSum0 + ii))++;
				N++;
			}
			iitime0 = iitime;
		}
	}
	return N;
}

// compute autocorrelation
// in:
//	OPTION_VIEWSPIKES
//		nbinsISI	number of bins
//		timestart	first valid spike occurence time (s) 
//		timeend		last valid spike occurence (s)
//		spikeclassoption	0=all spike class, -1:only one, 1=exclude artefacts
//		classnb		if !spikeclassoption, compute only from spikes with this class nb
//		babsolutetime	false=correct time with first stimulus time
//		binISI		bin duration (in s)
//	long* plSum0 array of longs to receive histogram bins
//	iclass		class selected
// out:
//	*plSum0
//	number of spikes used as time ref for autocorrelation
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::BuildAUTOCORR(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	long N=0;			// number of pivot spikes used to build autocorrelation
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	int nspikes = pspklist->GetTotalSpikes();		// number of spikes in that file
	if (nspikes <= 0)			// exit if no spikes
		return N;

	float samprate = pspklist->GetAcqSampRate();	// sampling rate
	ASSERT(samprate != 0.f);
	int istim0 = vdS->istimulusindex;
	int istim1 = istim0+1;
	int increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stim.iisti.GetSize();
		increment = vdS->nstipercycle;
		if (m_stim.npercycle > 1 && increment > m_stim.npercycle)
			increment = m_stim.npercycle;
		increment *= 2;
	}

	for (int istim = istim0; istim <istim1; istim+= increment, N++)
	{
		int iioffset0 = 0;								// initial offset
		if (!vdS->babsolutetime)						// if stimulus locking
		{												// get time of reference stim
			if (m_stim.nitems > 0) 
				iioffset0= m_stim.iisti.GetAt(istim);
			else
				iioffset0 = (long) -(vdS->timestart * samprate);
		}
		// adjust time limits to cope with stimulus locking
		long iistart = (long) (vdS->timestart*samprate) +iioffset0;
		long iiend = (long) (vdS->timeend*samprate) +iioffset0;

		// histogram parameters
		float x = vdS->binISI * vdS->nbinsISI * samprate;
		long iispan = (long) x;
		if (x - float(iispan) > 0.5f)
			iispan++;
		long iibinsize = iispan / vdS->nbinsISI;
		ASSERT(iibinsize > 0);
		iispan /= 2;

		// search with the first spike falling into that window
		int i0=0;
		while ((i0 < nspikes) && (pspklist->GetSpikeTime(i0) < iistart))
			i0++;

		// build histogram external loop search 'pivot spikes'
		for (int i = i0; i<nspikes; i++)
		{
			long iitime0 = pspklist->GetSpikeTime(i);	// get spike time
			if (iitime0 > iiend)						// stop loop if out of range
				break;
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;								// discard if class not requested
			N++;										// update nb of pivot spikes
			// search backwards first spike that is ok
			int i1=i;
			int ifirst = iitime0 - iispan;
			while (i1 > 0 && pspklist->GetSpikeTime(i1) > ifirst)
				i1--;

			// internal loop: build autoc
			for (int j = i1; j<nspikes; j++)
			{
				if (j== i)								// discard spikes w. same time
					continue;
				long iitime = pspklist->GetSpikeTime(j) -iitime0;
				if (abs(iitime) >= iispan)
					continue;
				if (vdS->spikeclassoption && pspklist->GetSpikeClass(j) != iclass)
					continue;

				int tiitime = ((iitime + iispan)/ iibinsize);
				if (tiitime >= (int) vdS->nbinsISI)
					continue;
				ASSERT(tiitime >= 0);
				ASSERT(tiitime < vdS->nbinsISI);
				(*(plSum0+tiitime))++;
			}
		}
	}
	return N;
}


// compute peristimulus - autocorrelation
// in:
//	OPTION_VIEWSPIKES
//		timebin		PSTH bin duration (s)
//		nbinsISI	number of bins
//		timestart	first valid spike occurence time (s) 
//		timeend		last valid spike occurence (s)
//		spikeclassoption	0=all spike class, -1:only one, 1=exclude artefacts
//		classnb		if !spikeclassoption, compute only from spikes with this class nb
//		babsolutetime	false=correct time with first stimulus time
//		binISI		bin duration (in s)
//	long* plSum0 array of longs to receive histogram bins
//	iclass		class selected
// out:
//	*plSum0  structure:  [row=nbinsISI; col=nbins]
//	number of spikes used as time ref for autocorrelation
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::BuildPSTHAUTOCORR(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	long N=0;					// number of 'pivot spikes'
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	int nspikes = pspklist->GetTotalSpikes();
	if (nspikes <= 0)			// return if no spikes in that file
		return N;
	
	// lock PSTH to stimulus if requested
	float samprate = pspklist->GetAcqSampRate();
	ASSERT(samprate != 0.f);

	int istim0 = vdS->istimulusindex;
	int istim1 = istim0+1;
	int increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stim.iisti.GetSize();
		increment = vdS->nstipercycle;
		if (m_stim.npercycle > 1 && increment > m_stim.npercycle)
			increment = m_stim.npercycle;
		increment *= 2;
	}

	for (int istim = istim0; istim <istim1; istim+=increment, N++)
	{
		int iioffset0 = 0;
		if (!vdS->babsolutetime)
		{
			if (m_stim.nitems > 0) 
				iioffset0= m_stim.iisti.GetAt(istim);
			else
				iioffset0 = (long) -(vdS->timestart * samprate);
		}
		long iistart = (long) (vdS->timestart*samprate) +iioffset0;
		long iiend = (long) (vdS->timeend*samprate) +iioffset0;
		long iilength = iiend - iistart;

		// histogran parameters
		float x = vdS->binISI * vdS->nbinsISI * samprate;
		long iispan = (long) x;
		if (x - float(iispan) > 0.5f)
			iispan++;
		long iiautocorrbinsize = iispan / vdS->nbinsISI;
		iispan /= 2;
		int iidummy = vdS->nbins * vdS->nbinsISI;

		// build histogram
		int i0 = 0;								// search first spike within interval
		while (pspklist->GetSpikeTime(i0) < iistart) 
		{
			i0++;								// loop until found
			if (i0 >= nspikes)
			{
				return 0L;
				break;
			}
		}

		// external loop: pivot spikes
		for (int i = i0; i < nspikes; i++)		// loop over spikes A
		{
			// find an appropriate spike
			long iitime0 = pspklist->GetSpikeTime(i);
			if (iitime0 >= iiend)				// exit loop
				break;
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;
			N++;								// update nb of pivot spikes
			// compute base index (where to store autocorrelation for this pivot spike)
			int iPSTH = ((iitime0 -iistart)* vdS->nbins ) / iilength;
			ASSERT (iPSTH >= 0);
			ASSERT (iPSTH < vdS->nbins);
			iPSTH *= vdS->nbinsISI;

			// get nb of counts around that spike
			// loop backwards to find first spike OK
			int ifirst = iitime0 - iispan;		// temp value
			int i1 = i;
			while (i1 > 0 && pspklist->GetSpikeTime(i1) > ifirst)
				i1--;
			// internal loop build histogram
			for (int j = i1; j<nspikes; j++)
			{
				if (j== i)
					continue;
				
				long iitime =pspklist->GetSpikeTime(j) -iitime0;
				if (iitime >= iispan)
					break;
				if (iitime < -iispan)
					continue;

				if (vdS->spikeclassoption && pspklist->GetSpikeClass(j) != iclass)
					continue;

				int tiitime = (iitime +iispan) / iiautocorrbinsize;
				ASSERT(tiitime >= 0);
				ASSERT(tiitime < vdS->nbinsISI);
				if (tiitime > (int) vdS->nbinsISI)
					continue;
				int ii = iPSTH + tiitime;
				ASSERT(ii <= iidummy);
				(*(plSum0 +ii))++;
			}
		}
	}
	return N;
}

void CSpikeDoc::_ExportSpkAverageWave (CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, CString csFileComment, int ispklist, int iclass)
{
	double* pDoubl;
	CString csDummy;
	CSpikeList* pspklist= &m_spklistArray[ispklist];
	// update offset
	int iioffset0 = 0;
	if (!vdS->babsolutetime && m_stim.nitems > 0) 
		iioffset0= m_stim.iisti.GetAt(vdS->istimulusindex);
	// prepare parameters
	float rate = pspklist->GetAcqSampRate();
	long iitime_start = (long) (vdS->timestart * rate) + iioffset0;
	long iitime_end = (long) (vdS->timeend * rate) + iioffset0;
	int nspikes = pspklist->GetTotalSpikes();

	// clear histogram area if histogram of amplitudes
	ASSERT(vdS->exportdatatype == EXPORT_AVERAGE);
	pDoubl = pDoubl0+1;
	int spklen = int(*pDoubl0)	;	// spk length + nb items 
	int i0 = spklen*2 + 2;
	for (int k=1; k<i0; k++, pDoubl++)
		*pDoubl = 0;

	// check file size and position pointer at the first spike within the bin
	i0=0;
	while ((i0 <nspikes) && (pspklist->GetSpikeTime(i0) < iitime_start)) 
		i0++;

	// ------- count total spk/interval (given bspkclass&bartefacts)
	short binzero = pspklist->GetAcqBinzero();
	for (int j=i0; j<nspikes; j++)
	{
		// skip intervals not requested
		long iitime = pspklist->GetSpikeTime(j);
		if (iitime >= iitime_end)
			break;
		// skip classes not requested
		int cla = pspklist->GetSpikeClass(j);
		if (cla <0 /*&& !vdS->bartefacts*/)	
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)	
			continue;
		// get value, compute statistics
		short* pSpik = pspklist->GetpSpikeData(j);
		double* pN = pDoubl0 +1;
		double* pSUM = pDoubl0+2;
		double* pSUM2 = pDoubl0+2 +spklen;
		for (int k = 0; k < spklen; k++, pSUM++, pSUM2++, pSpik++)
		{
			double x = double (*pSpik - binzero);
			*pSUM += x;
			*pSUM2 += (x*x);				
		}
		(*pN)++;
	}

	// 4) export histogram
	double* pN = pDoubl0 +1;
	double* pSUM = pDoubl0+2;
	double* pSUM2 = pDoubl0+2 +spklen;			
	if (*pN > 0)
	{
		double VperBin = pspklist->GetAcqVoltsperBin()*1000.f;
		double VperBin2 = VperBin * VperBin;
		double yN = *pN;
		csDummy.Format(_T("\t%i"), int(yN)); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		int k=0;
		for (k = 0; k < spklen; k++, pSUM++)
		{
			csDummy.Format(_T("\t%.3lf"), ((*pSUM)/yN)*VperBin); 
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
		for (k = 0; k < spklen; k++, pSUM2++)
		{			
			csDummy.Format(_T("\t%.3lf"), (*pSUM2)*VperBin2); 
			pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		csDummy = _T("\t\t\t0"); 
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::ExportSpkAverageWave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, CString csFileComment)
{
	CSpikeList* pspklist= &m_spklistArray[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0=0;
	int class1=0;
	if ((vdS->spikeclassoption ==-1 
		|| vdS->spikeclassoption==1) 
		 && !pspklist->IsClassListValid()) 
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption ==-1) 
	{
		// search item index with correct class ID
		int i=pspklist->GetNbclasses()-1;
		for (i; i>= 0; i--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;

		// if none is found, export descriptor and exit
		if (i < 0)
		{
			ExportSpkFileComment(pSF, vdS, vdS->classnb, csFileComment);
			return;
		}
		class0 = i;
		class1 = i;

	}
	else if (vdS->spikeclassoption==1)
		class1=pspklist->GetNbclasses()-1;


	// loop header..................		
	for (int kclass = class0; kclass<= class1; kclass++)
	{
		// ................................COMMENTS
		int iclass = pspklist->GetclassID(kclass);
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkAverageWave (pSF, vdS, pDoubl0, csFileComment, m_currspklist, iclass);
	}
}

CSpikeList*	CSpikeDoc::SetSpkListCurrent (int ichan)
{
	CSpikeList* pspklist= NULL;
	if (ichan < 0)
		ichan = 0;

	// if the list requested does not exist, return a NULL pointer
	if (m_spklistArray.GetSize() > 0)
	{
		// spike list requested exist, select it and return a pointer to it
		if (ichan >= 0 && ichan < m_spklistArray.GetSize())
		{
			pspklist = &m_spklistArray[ichan];
			m_currspklist = ichan;
		}
	}
	return pspklist;
}

CSpikeList*	CSpikeDoc::GetSpkListCurrent()
{
	CSpikeList* pspklist= NULL;
	pspklist = &m_spklistArray[m_currspklist];
	ASSERT(pspklist);
	return pspklist;
}



