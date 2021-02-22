// spikedoc.cpp : implementation file
//
// derived from CDocument

#include "StdAfx.h"
#include "Acqdatad.h"
#include "Spikedoc.h"
//#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CSpikeDoc, CDocument, 1 /* schema number*/)

CSpikeDoc::CSpikeDoc()
{
	m_wVersion = 7;
	m_detectiondate = CTime::GetCurrentTime();
	m_comment = "";
	m_acqfile = "";
	m_acqtime = m_detectiondate;
	m_acqrate = 1.f;
	m_acqsize = 0;
	m_newpath = "";
	spikelist_array.SetSize(1);
	m_currspklist = -1; //0;
}

void CSpikeDoc::ClearData()
{
	m_spkclass.EraseData();
	if (m_stimIntervals.nitems > 0)
	{
		m_stimIntervals.intervalsArray.RemoveAll();
		m_stimIntervals.nitems = 0;
	}
	spikelist_array.SetSize(1);
	spikelist_array[0].EraseData();
}

BOOL CSpikeDoc::OnNewDocument()
{
	ClearData();
	DeleteContents();
	m_strPathName.Empty();
	SetModifiedFlag(FALSE);

	return TRUE;
}

CSpikeDoc::~CSpikeDoc()
{
	ClearData();
}

BEGIN_MESSAGE_MAP(CSpikeDoc, CDocument)

END_MESSAGE_MAP()

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
		m_stimIntervals.Serialize(ar);

		nitems = spikelist_array.GetSize();	// added Aug 23, 2005
		ar << nitems;
		for (int i = 0; i < nitems; i++)
			spikelist_array[i].Serialize(ar);
		m_spkclass.Serialize(ar);
	}
	else
	{
		WORD wwVersion;
		ar >> wwVersion;					// R1
		if (wwVersion == 7)
			readVersion7(ar);
		else if (wwVersion == 6)
			readVersion6(ar);
		else if (wwVersion < 6)
			readBeforeVersion6(ar, wwVersion);
		else
		{
			ASSERT(FALSE);
			CString message;
			message.Format(_T("Version not recognized: %i"), wwVersion);
			AfxMessageBox(message, MB_OK);
		}
	}
}
void CSpikeDoc::SortStimArray()
{
	const auto nsti = m_stimIntervals.intervalsArray.GetSize();
	if (nsti == 0 || (m_stimIntervals.intervalsArray[nsti - 1] > m_stimIntervals.intervalsArray[0]))
		return;

	// bubble sort from bottom to top
	for (auto j = 0; j < nsti; j++)
	{
		int imin = m_stimIntervals.intervalsArray[j];
		for (auto k = j + 1; k < nsti; k++)
		{
			if (m_stimIntervals.intervalsArray[k] < imin)
			{
				imin = m_stimIntervals.intervalsArray[k];
				m_stimIntervals.intervalsArray[k] = m_stimIntervals.intervalsArray[j];
				m_stimIntervals.intervalsArray[j] = imin;
			}
		}
	}
}

void CSpikeDoc::readBeforeVersion6(CArchive& ar, WORD wwVersion)
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
		ar >> m_stimIntervals.nitems;	// R9 - load number of items
		m_stimIntervals.intervalsArray.Serialize(ar);	// R10 - read data from file
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
			for (int i = 0; i < isize; i++)
				m_stimIntervals.Serialize(ar);
			SortStimArray();
		}
	}
	// read stimulus and spike classes
	spikelist_array.RemoveAll();
	spikelist_array.SetSize(1);
	spikelist_array[0].Serialize(ar);
	m_spkclass.Serialize(ar);
}

void CSpikeDoc::readVersion6(CArchive& ar)
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
		for (int i = 0; i < isize; i++)
			m_stimIntervals.Serialize(ar);
	}

	// read stimulus and spike classes
	spikelist_array.RemoveAll();
	spikelist_array.SetSize(1);
	spikelist_array[0].Serialize(ar);
	SortStimArray();
	m_spkclass.Serialize(ar);
}

void CSpikeDoc::readVersion7(CArchive& ar)
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
	for (int i = 0; i < isize; i++)
		m_stimIntervals.Serialize(ar);
	SortStimArray();

	spikelist_array.RemoveAll();
	ar >> isize;

	spikelist_array.SetSize(isize);
	for (int i = 0; i < isize; i++)
		spikelist_array[i].Serialize(ar);
	m_spkclass.Serialize(ar);
}

// CSpikeDoc commands

BOOL CSpikeDoc::OnSaveDocument(LPCTSTR pszPathName)
{
	// check that path name has ".spk"
	CString docname = pszPathName;	// copy name into CString object
	const auto i = docname.ReverseFind('.');		// find extension separator
	if (i > 0)
		docname = docname.Left(i);	// clip name to remove extension
	docname += ".spk";				// add "fresh" extension (spk)

	CFileStatus status;	
	const auto b_flag_exists = CFile::GetStatus(docname, status);

	if (b_flag_exists && (status.m_attribute & CFile::readOnly))
	{
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, docname);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
		case IDYES:
		{
			// If so, either Save or Update, as appropriate DoSave
			const auto p_template = GetDocTemplate();
			ASSERT(p_template != NULL);
			if (!m_newpath.IsEmpty())
			{
				const auto j = docname.ReverseFind('\\') + 1;
				if (j != -1)
					docname = docname.Mid(j);
				docname = m_newpath + docname;
			}

			if (!AfxGetApp()->DoPromptFileName(docname,
				AFX_IDS_SAVEFILE,
				OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
				return FALSE;
			if (!AfxGetApp()->DoPromptFileName(docname,
				AFX_IDS_SAVEFILECOPY,
				OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
				return FALSE;

			// don't even attempt to save
			const auto k = docname.ReverseFind('\\') + 1;	// find last occurence of antislash
			if (k != -1)
				m_newpath = docname.Left(k);
		}
		break;

		case IDNO:
		case IDCANCEL:
		default:
			return FALSE;
		}
	}

	CFile f;
	CFileException e;
	try
	{
		f.Open(docname, CFile::modeCreate | CFile::modeWrite, &e);
		CArchive ar(&f, CArchive::store);
		Serialize(ar);
		ar.Flush();
		ar.Close();
		f.Close();
		SetModifiedFlag(FALSE);
	}
	catch (CFileException* pe)
	{
		f.Abort();
		ReportSaveLoadException(docname, pe, FALSE, AFX_IDP_FAILED_TO_SAVE_DOC);
		pe->Delete();
		return FALSE;
	}
	SetModifiedFlag(FALSE);	// mark the document as clean
	return TRUE;
}

BOOL CSpikeDoc::OnOpenDocument(LPCTSTR pszPathName)
{
	ClearData();
	CFileStatus status;
	if (!CFile::GetStatus(pszPathName, status))
		return FALSE;

	CFile f;
	CFileException fe;
	auto b_read = TRUE;
	const CString filename = pszPathName;
	if (f.Open(filename, CFile::modeRead | CFile::shareDenyNone, &fe))
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
			for (auto i = 0; i < spikelist_array.GetSize(); i++)
			{
				if (!spikelist_array[i].IsClassListValid())	// if class list not valid:
				{
					spikelist_array[i].UpdateClassList();	// rebuild list of classes
					SetModifiedFlag();						// and set modified flag
				}
			}
		}
		catch (CArchiveException* e)
		{
			e->Delete();
			b_read = FALSE;
		}
	}
	else
	{
		ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		b_read = FALSE;
	}
	return b_read;
}

void CSpikeDoc::InitSourceDoc(CAcqDataDoc* p_document)
{
	// load parameters from file
	const auto pwave_format = p_document->GetpWaveFormat();
	m_acqtime = pwave_format->acqtime;
	m_acqsize = p_document->GetDOCchanLength();
	m_acqrate = pwave_format->chrate;
	m_acqcomment = pwave_format->cs_comment;
	m_wformat = *pwave_format;
}

CString CSpikeDoc::GetFileInfos()
{
	TCHAR sz[64];
	const auto psz = sz;
	const auto psep_tab = _T("\t");
	const auto psep_rc = _T("\r\n");
	auto cs_out = GetPathName();
	cs_out.MakeUpper();
	cs_out += psep_tab + m_detectiondate.Format(_T("%c"));
	cs_out += psep_rc + m_comment;
	cs_out += psep_rc;

	cs_out += _T("*** SOURCE DATA ***\r\n");
	cs_out += m_acqfile + psep_rc;
	cs_out += m_acqcomment + psep_rc;
	cs_out += m_acqtime.Format(_T("%#d-%B-%Y")) + psep_rc;

	cs_out += _T("*** SPIKE LIST ***\r\n");

	long lnspikes = 0;
	const auto pspklist = &spikelist_array[m_currspklist];
	if (pspklist->GetTotalSpikes() > 0)
		lnspikes = pspklist->GetTotalSpikes();
	wsprintf(psz, _T("n spikes = %li\r\n"), lnspikes);
	cs_out += sz;

	return cs_out;
}

// post stimulus histogram, ISI, autocorrelation
// vdS->exportdatatype == 0
// each line represents 1 recording
// we start with a header, then each file is scanned and exported
// if nintervals < 0: export all spikes // feb 23, 2009
void CSpikeDoc::_ExportSpkLatencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spikelist_array[ispklist];
	const auto totalspikes = pspklist->GetTotalSpikes();

	// export time of nintervals first spikes
	auto nspikes = nintervals;
	if (nspikes > totalspikes || nspikes < 0)
		nspikes = totalspikes;
	const auto rate = pspklist->GetAcqSampRate();

	for (auto j = 0; j < nspikes; j++)
	{
		// skip classes not requested
		const auto cla = pspklist->GetSpikeClass(j);
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)
			continue;

		// export time
		const auto tval = static_cast<float>(pspklist->GetSpikeTime(j)) / rate;
		cs_dummy.Format(_T("\t%lf"), tval);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::ExportSpkLatencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment)
{
	CString cs_dummy;
	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	auto pspklist = &spikelist_array[m_currspklist];

	if ((vdS->spikeclassoption == -1
		|| vdS->spikeclassoption == 1)
		&& !pspklist->IsClassListValid())
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->GetNbclasses() - 1;
		for (auto j = i; j >= 0; j--, i--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->GetNbclasses() - 1;

	// loop header..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		// check if class is Ok
		const auto nbspk_for_thisclass = pspklist->GetclassNbspk(kclass);
		if ((FALSE == vdS->bexportzero) && (nbspk_for_thisclass == 0))
			continue;
		const auto iclass = pspklist->GetclassID(kclass);
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkLatencies(pSF, vdS, nintervals, m_currspklist, iclass);
	}
}

void CSpikeDoc::_ExportSpkPSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spikelist_array[ispklist];
	auto nbins = 1;

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:		// PSTH
		nbins = vdS->nbins;
		break;
	case EXPORT_ISI:		// ISI
		nbins = vdS->nbinsISI;
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		nbins = vdS->nbinsISI;
		break;
	default:
		break;
	}

	// ................................DATA
	// spike list -- prepare parameters
	auto sum = plSum0;
	long n;
	for (auto i = 0; i < nbins; i++, sum++)
		*sum = 0;

	// export number of spikes / interval
	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:		// PSTH
		BuildPSTH(vdS, plSum0, iclass);
		break;
	case EXPORT_ISI:		// ISI
		n = BuildISI(vdS, plSum0, iclass);
		cs_dummy.Format(_T("\t%li"), n);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		n = BuildAUTOCORR(vdS, plSum0, iclass);
		cs_dummy.Format(_T("\t%li"), n);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	default:
		break;
	}

	// export nb spikes / bin: print nb
	for (auto i = 0; i < nbins; i++)
	{
		if ((vdS->bexportzero == TRUE) || (*(plSum0 + i) >= 0))
			cs_dummy.Format(_T("\t%li"), *(plSum0 + i));
		else
			cs_dummy = _T("\t");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}

	// export stimulus occurence time(s) that fit(s) into the time interval requested
	if (vdS->exportdatatype == EXPORT_PSTH && m_stimIntervals.intervalsArray.GetSize() > 0)
	{
		auto samprate = pspklist->GetAcqSampRate();
		if (samprate == 0.f) {
			samprate = m_acqrate;
		}
		ASSERT(samprate != 0.f);
		const auto istim0 = 0;
		const auto istim1 = m_stimIntervals.intervalsArray.GetSize() - 1;
		auto iioffset0 = 0;
		if (!vdS->babsolutetime)
			iioffset0 = m_stimIntervals.intervalsArray.GetAt(vdS->istimulusindex);
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;
		auto b_up = -1;

		for (auto istim = istim0; istim <= istim1; istim++)
		{
			const int iistim = m_stimIntervals.intervalsArray.GetAt(istim);
			b_up *= -1;
			if (iistim < iistart)
				continue;
			if (iistim > iiend)
				break;

			const auto tval = (static_cast<float>(iistim) - iioffset0) / samprate;
			if (b_up < 0)
				cs_dummy.Format(_T("\t(-)%lf"), tval);
			else
				cs_dummy.Format(_T("\t(+)%lf"), tval);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::ExportSpkPSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, const CString&
	csFileComment)
{
	CString cs_dummy;
	CSpikeList* pspklist = &spikelist_array[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0 = 0;
	int class1 = 0;
	if ((vdS->spikeclassoption == -1			// 1 class selected
		|| vdS->spikeclassoption == 1)			// all classes selected but displayed on different lines
		&& !pspklist->IsClassListValid())
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption == -1)				// only 1 class selected
	{
		// search item index with correct class ID
		auto i = pspklist->GetNbclasses() - 1;
		for (auto j = i; j >= 0; i--, j--)
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
				cs_dummy.Format(_T("\t0"));
				pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
				break;
			default:
				break;
			}
			return;
		}
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->GetNbclasses() - 1;

	// loop header..................
	for (int kclass = class0; kclass <= class1; kclass++)
	{
		// check if class is Ok
		const auto nbspk_for_thisclass = pspklist->GetclassNbspk(kclass);
		const auto iclass = pspklist->GetclassID(kclass);
		// export the comments
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		// test if we should continue
		if (!(FALSE == vdS->bexportzero) && (nbspk_for_thisclass == 0))
			_ExportSpkPSTH(pSF, vdS, plSum0, m_currspklist, iclass);

		cs_dummy = _T("\r\n");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

// export histograms of the amplitude of the spikes found in each file of the file series
// export type = 4
// same tb as PSTH: each line represents one record, 1 class
void CSpikeDoc::_ExportSpkAmplitHistogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, int ispklist, int iclass)
{
	// ................................DATA
	auto pspklist = &spikelist_array[ispklist];
	CString cs_dummy;
	// temp parameters for amplitudes histogram
	const auto histampl_min = static_cast<int>(vdS->histampl_vmin / pspklist->GetAcqVoltsperBin());
	const auto histampl_max = static_cast<int>(vdS->histampl_vmax / pspklist->GetAcqVoltsperBin());
	const auto histampl_binsize = (histampl_max - histampl_min) / vdS->histampl_nbins;
	const auto nbins = 1;
	// update offset
	auto iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimIntervals.nitems > 0)
		iioffset0 = m_stimIntervals.intervalsArray.GetAt(vdS->istimulusindex);

	// clear histogram area
	ASSERT(vdS->exportdatatype == EXPORT_HISTAMPL);
	auto p_hist = pHist0;
	for (auto k = 0; k < vdS->histampl_nbins + 2; k++, p_hist++)
		*p_hist = 0;

	long y_sum = 0;
	long y_sum2 = 0;
	long y_n = 0;

	const auto tspan = vdS->timeend - vdS->timestart;
	const auto tbin = tspan / nbins;
	auto tfirst = vdS->timestart;
	auto rate = pspklist->GetAcqSampRate();
	if (rate == 0.0)
		rate = 1.0f;
	auto iitime1 = static_cast<long>(tfirst * rate);

	for (auto i = 0; i < nbins; i++)
	{
		tfirst = vdS->timestart + tbin * i;
		const auto iitime0 = iitime1;
		iitime1 = static_cast<long>((tfirst + tbin) * rate);

		// export number of spikes / interval
		// ------- count total spk/interval (given bspkclass&bartefacts)
		//long iitotal = 0;
		for (auto j = 0; j < pspklist->GetTotalSpikes(); j++)
		{
			// skip intervals not requested
			const auto iitime = pspklist->GetSpikeTime(j) - iioffset0;
			if ((iitime < iitime0) || (iitime >= iitime1))
				break;

			// skip classes not requested
			const auto cla = pspklist->GetSpikeClass(j);
			if (cla < 0 /*&& !vdS->bartefacts*/)
				continue;
			if (vdS->spikeclassoption != 0 && cla != iclass)
				continue;

			// get value, compute statistics
			int max, min;
			pspklist->GetSpikeExtrema(j, &max, &min);
			auto val = max - min;
			y_sum += val;
			y_sum2 += val * val;
			y_n++;
			// build histogram: compute index and increment bin content
			int index;
			if (val >= histampl_min && val <= histampl_max)
				index = (val - histampl_min) / histampl_binsize + 1;
			else
				index = vdS->histampl_nbins + 1;
			val = *(pHist0 + index) + 1;
			*(pHist0 + index) = val;
		}

		// 4) export histogram
		if (y_n > 0)
		{
			const auto volts_per_bin = pspklist->GetAcqVoltsperBin() * 1000.f;
			cs_dummy.Format(_T("\t%.3lf\t%.3lf\t%i"), (double(y_sum) / y_n) * volts_per_bin, double(y_sum2) * volts_per_bin * volts_per_bin, y_n);
		}
		else
			cs_dummy = _T("\t\t\t0");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		// actual measures
		p_hist = pHist0;
		for (auto k = 0; k < vdS->histampl_nbins + 2; k++, p_hist++)
		{
			if ((vdS->bexportzero == TRUE) || (*p_hist >= 0))
				cs_dummy.Format(_T("\t%i"), *p_hist);
			else
				cs_dummy = _T("\t");
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::ExportSpkAmplitHistogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, const CString&
	csFileComment)
{
	auto pspklist = &spikelist_array[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	if ((vdS->spikeclassoption == -1
		|| vdS->spikeclassoption == 1)
		&& !pspklist->IsClassListValid())
		pspklist->UpdateClassList();

	// just one class selected: see if there is at least 1 spike with this class
	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->GetNbclasses() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (pspklist->GetclassID(i) == vdS->classnb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
			return ExportSpkFileComment(pSF, vdS, vdS->classnb, csFileComment);
		// set class boundaries to the index of the class having the proper ID
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->GetNbclasses() - 1;

	// loop over all classes..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		const auto iclass = pspklist->GetclassID(kclass);
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkAmplitHistogram(pSF, vdS, pHist0, m_currspklist, iclass);
	}
}

// option 1: spike intervals
// option 2: spike extrema (max and min)
// option 3: export spike amplitudes (max - min)
// one file at a time, organized in columns

void CSpikeDoc::ExportSpkAttributesOneFile(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	int iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimIntervals.nitems > 0)
	{
		iioffset0 = m_stimIntervals.intervalsArray.GetAt(vdS->istimulusindex);
	}

	// ................................DATA
	auto pspklist = &spikelist_array[m_currspklist];
	CString cs_dummy = _T("\r\ntime(s)\tclass");
	auto rate = pspklist->GetAcqSampRate();
	if (rate == 0.0)
		rate = 1.0f;

	switch (vdS->exportdatatype)
	{
	case EXPORT_EXTREMA:
		cs_dummy += _T("\tmax(mV)\tmin(mV)");
		break;
	case EXPORT_AMPLIT:
	{
		cs_dummy += _T("\tamplitude(mV)\ttime(ms)");
		const auto ratems = rate / 1000.f;
		const auto xfirst = static_cast<float>(pspklist->m_imaxmin1SL) / ratems;
		const auto xlast = static_cast<float>(pspklist->m_imaxmin2SL) / ratems;
		CString cs;
		cs.Format(_T(" [%.3f to %.3f]"), xfirst, xlast);
		cs_dummy += cs;
	}
	break;
	default:;
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike list -- prepare parameters
	const auto tspan = vdS->timeend - vdS->timestart;
	auto tfirst = vdS->timestart;
	auto iitime1 = static_cast<long>(tfirst * rate);

	tfirst = vdS->timestart + tspan;
	const auto iitime0 = iitime1;
	iitime1 = static_cast<long>((tfirst + tspan) * rate);
	const auto vper_bin = pspklist->GetAcqVoltsperBin() * 1000.f;
	const auto binzero = pspklist->GetAcqBinzero();
	int imax, imin;
	const auto ifirst = pspklist->m_imaxmin1SL;
	const auto ilast = pspklist->m_imaxmin2SL;
	const auto ratems = rate / 1000.f;

	// export data: loop through all spikes
	for (auto j = 0; j < pspklist->GetTotalSpikes(); j++)
	{
		// skip intervals not requested
		const auto iitime = pspklist->GetSpikeTime(j) - iioffset0;
		if ((iitime < iitime0) || (iitime >= iitime1))
			break;
		// skip classes not requested
		const auto cla = pspklist->GetSpikeClass(j);
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;

		// export time and spike time
		const auto tval = static_cast<float>(iitime) / rate;
		cs_dummy.Format(_T("\r\n%lf\t%i"), tval, cla);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		switch (vdS->exportdatatype)
		{
			// 2) export time interval (absolute time)
		case EXPORT_INTERV:
			break;
			// 3) export extrema
		case EXPORT_EXTREMA:
			int max, min;
			pspklist->GetSpikeExtrema(j, &max, &min);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(max) - binzero) * vper_bin, (static_cast<double>(min) - binzero) * vper_bin);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;
			// 4) export max - min
		case EXPORT_AMPLIT:
			pspklist->MeasureSpikeMaxThenMinEx(j, &max, &imax, &min, &imin, ifirst, ilast);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(max) - min) * vper_bin, (static_cast<double>(imin) - imax) / ratems);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;
		case EXPORT_SPIKEPOINTS:
			for (auto index = 0; index < pspklist->GetSpikeLength(); index++)
			{
				const auto val = pspklist->GetSpikeValAt(j, index);
				cs_dummy.Format(_T("\t%.3lf"), double(val) * vper_bin);
				pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			}
			break;
		default:;
		}
	}
}

// Describe the type of data exported
void CSpikeDoc::ExportTableTitle(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles)
{
	CString cs_dummy;

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:		cs_dummy = _T("Histogram nb spikes/bin");			break;
	case EXPORT_ISI:		cs_dummy = _T("Intervals between spikes histogram");	break;
	case EXPORT_AUTOCORR:	cs_dummy = _T("Autocorrelation histogram");			break;
	case EXPORT_LATENCY:	cs_dummy = _T("Latency first 10 spikes");			break;
	case EXPORT_AVERAGE:	cs_dummy = _T("Spike average waveform");				break;
	case EXPORT_INTERV:		cs_dummy = _T("Spike time intervals"); nfiles = 1;	break;
	case EXPORT_EXTREMA:	cs_dummy = _T("Spike amplitude extrema"); nfiles = 1; break;
	case EXPORT_AMPLIT:		cs_dummy = _T("Spike amplitudes"); nfiles = 1;		break;
	case EXPORT_HISTAMPL:	cs_dummy = _T("Spike amplitude histograms");			break;
	case EXPORT_SPIKEPOINTS: cs_dummy = _T("Spike data points");					break;
	default: vdS->exportdatatype = 0; cs_dummy = _T("Histogram nb spikes/bin");	break;
	}
	auto t = CTime::GetCurrentTime();				// current date & time
	CString cs_date;									// first string to receive date and time
	cs_date.Format(_T("  created on %d-%d-%d at %d:%d"), t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(), t.GetMinute());
	cs_dummy += cs_date;
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
	case EXPORT_EXTREMA:
		cs_dummy.Format(_T("\r\nnbins= %i\tbin size (s):\t%.3f\tt start (s):\t%.3f\tt end (s):\t%.3f\r\nn files = %i\r\n"),
			vdS->nbins,
			static_cast<float>((vdS->timeend - vdS->timestart) / vdS->nbins),
			vdS->timestart,
			vdS->timeend,
			nfiles);
		break;
	case EXPORT_AVERAGE:
		cs_dummy.Format(_T("\r\ndata:\tN\txi=mean amplitude (mV)\tSum square of amplitudes\r\nn points:\t%i\r\n"),
			GetSpkList_Current()->GetSpikeLength());
		break;
	case EXPORT_HISTAMPL:
	case EXPORT_LATENCY:
	default:
		cs_dummy.Format(_T("\r\n\r\n\r\n"));
		break;
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

// Row of column headers for the database parameters
// returns number of columns used
void CSpikeDoc::ExportTableColHeaders_db(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString cs_dummy;

	// these infos are stored in the database - what we do here, is equivalent of doing a query
	cs_dummy.Format(_T("\r\nID\tFilename"));
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	auto ncolumns = 2;
	cs_dummy.Empty();
	if (vdS->bacqdate)
	{	// ................     acq date, acq time
		cs_dummy += _T("\tacq_date\tacq_time");
		ncolumns += 2;
	}
	if (vdS->bacqcomments)
	{	// ................     acq comments, stim, conc, type
		cs_dummy += _T("\tExpt\tinsectID\tssID\tinsect\tstrain\tsex\tlocation\toperator\tmore");
		ncolumns += 9;
		cs_dummy += _T("\tstim1\tconc1\trepeat1\tstim2\tconc2\trepeat2\ttype\ttag");
		ncolumns += 8;
	}

	// this is specific to the spike file (not stored in the database)
	if (vdS->bspkcomments)
	{	// ................     spike comments
		cs_dummy += _T("\tspk_comment");
		ncolumns += 1;
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike detection chan
	if (vdS->btotalspikes)
	{	// ................     total spikes
		cs_dummy = _T("\tspk_threshold\ttotal_spikes\tnb_classes\tduration(s)");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		ncolumns += 4;
	}

	// export spike detect chan, channel and class
	cs_dummy = _T("\tdetectchan\tchan\tclass");
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	ncolumns += 3;

	vdS->ncommentcolumns = ncolumns;
}

// Row (continuation) of column headers for the measures
void CSpikeDoc::ExportTableColHeaders_data(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString cs_dummy;
	// header of the data
	auto nbins = 0;
	float tspan;
	auto tbin = 1.f;
	auto tstart = vdS->timestart;
	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:		// PSTH
		nbins = vdS->nbins;
		tspan = vdS->timeend - vdS->timestart;
		tbin = tspan / nbins;
		break;
	case EXPORT_ISI:		// ISI
		nbins = vdS->nbinsISI;
		tbin = vdS->binISI;
		tstart = 0;
		cs_dummy = _T("\tN");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR:	// Autocorr
		nbins = vdS->nbinsISI;
		tspan = vdS->binISI * nbins;
		tbin = vdS->binISI;
		tstart = -tspan / 2.f;
		cs_dummy = _T("\tN");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_HISTAMPL:
		nbins = vdS->histampl_nbins;
		tstart = vdS->histampl_vmin;
		tbin = (vdS->histampl_vmax - vdS->histampl_vmin) * 1000.f / nbins;
		nbins += 2;
		tstart -= tbin;
		cs_dummy = _T("\tmean\tsum2\tNelmts");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AVERAGE:
	{
		const auto npoints = GetSpkList_Current()->GetSpikeLength();
		cs_dummy = _T("\tN");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		CString cs;
		cs_dummy.Empty();
		for (auto i = 0; i < npoints; i++)
		{
			cs.Format(_T("\tx%i"), i);
			cs_dummy += cs;
		}
		for (auto i = 0; i < npoints; i++)
		{
			cs.Format(_T("\tSx2_%i"), i);
			cs_dummy += cs;
		}
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
	break;

	default:
		break;
	}

	// ................     loop to scan all time intervals
	cs_dummy.Empty();
	if (vdS->exportdatatype == EXPORT_LATENCY)
	{
		nbins = 10;
		for (auto i = 0; i < nbins; i++)
		{
			CString cs;
			cs.Format(_T("\t%i"), i);
			cs_dummy += cs;
		}
	}
	else
	{
		for (auto i = 0; i < nbins; i++)
		{
			CString cs;
			cs.Format(_T("\tb_%.3f"), static_cast<double> (tstart) + static_cast<double> (tbin) * i);
			cs_dummy += cs;
		}
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

// output descriptors of each record
// TODO: output descriptors from the database and not from the spike files ??
void CSpikeDoc::ExportSpkFileComment(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass, const CString& csFileComment)
{
	CString cs_dummy;
	CString cs_temp;
	CString cs_tab = _T("\t");

	// spike file
	if (csFileComment.IsEmpty())
	{
		cs_dummy = GetPathName();
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		// source data file items
		if (vdS->bacqdate)				// source data time and date
		{
			cs_dummy.Format(_T("\t%s"), (LPCTSTR)m_acqtime.Format(_T("%#d %m %Y\t%X")));
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
		// source data comments
		if (vdS->bacqcomments)
		{
			cs_dummy = cs_tab + m_acqcomment;
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			cs_dummy.Format(_T("\t%i"), m_wformat.insectID);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy.Format(_T("\t%i"), m_wformat.sensillumID);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy = cs_tab + m_wformat.csInsectname;
			cs_dummy += cs_tab + m_wformat.csStrain;
			cs_dummy += cs_tab + m_wformat.csSex;
			cs_dummy += cs_tab + m_wformat.csLocation;
			cs_dummy += cs_tab + m_wformat.csOperator;
			cs_dummy += cs_tab + m_wformat.csMoreComment;

			cs_dummy += cs_tab + m_wformat.csStimulus;
			cs_dummy += cs_tab + m_wformat.csConcentration;
			cs_temp.Format(_T("%i"), m_wformat.repeat);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wformat.csStimulus2;
			cs_dummy += cs_tab + m_wformat.csConcentration2;
			cs_temp.Format(_T("%i"), m_wformat.repeat2);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wformat.csSensillum;
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		pSF->Write(csFileComment, csFileComment.GetLength() * sizeof(TCHAR));
	}

	// spike file additional comments
	if (vdS->bspkcomments)
	{
		pSF->Write(cs_tab, cs_tab.GetLength() * sizeof(TCHAR));
		pSF->Write(m_comment, m_comment.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	const auto pspklist = &spikelist_array[m_currspklist];

	if (vdS->btotalspikes)
	{
		cs_dummy.Format(_T("\t%f"), pspklist->GetdetectThresholdmV());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), pspklist->GetTotalSpikes());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), pspklist->GetNbclasses());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		const auto tduration = static_cast<float>(m_acqsize) / static_cast<float>(m_acqrate);
		cs_dummy.Format(_T("\t%f"), tduration);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
	// spike list item, spike class
	cs_dummy.Format(_T("\t%i \t%s \t%i"), vdS->ichan, static_cast<LPCTSTR>(pspklist->GetComment()), iclass);
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
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
	long n = 0;
	const auto pspklist = &spikelist_array[m_currspklist];

	const auto nspikes = pspklist->GetTotalSpikes();
	if (nspikes <= 0)
		return 0;
	const auto rate = pspklist->GetAcqSampRate();
	ASSERT(rate != 0.f);

	// check validity of istimulusindex
	auto istimulusindex = vdS->istimulusindex;
	if (istimulusindex > m_stimIntervals.intervalsArray.GetSize() - 1)
		istimulusindex = m_stimIntervals.intervalsArray.GetSize() - 1;
	if (istimulusindex < 0)
		istimulusindex = 0;
	const auto istim0 = istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimIntervals.intervalsArray.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimIntervals.npercycle > 1 && increment > m_stimIntervals.npercycle)
			increment = m_stimIntervals.npercycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		auto iioffset0 = 0;
		if (!vdS->babsolutetime)
		{
			if (m_stimIntervals.nitems > 0)
				iioffset0 = m_stimIntervals.intervalsArray.GetAt(istim);
			else
				iioffset0 = static_cast<long>(-(vdS->timestart * rate));
		}
		const auto iitime_start = static_cast<long>(vdS->timestart * rate) + iioffset0;
		const auto iitime_end = static_cast<long>(vdS->timeend * rate) + iioffset0;
		auto iibinsize = static_cast<long>(vdS->timebin * rate); //(iitime_end - iitime_start) / vdS->nbins;
		if (iibinsize <= 0)
			iibinsize = 1;
		ASSERT(iibinsize > 0);

		// check file size and position pointer at the first spike within the bin
		auto i0 = 0;
		while ((i0 < nspikes) && (pspklist->GetSpikeTime(i0) < iitime_start))
			i0++;

		for (auto j = i0; j < nspikes; j++)
		{
			// skip intervals not requested
			const int iitime = pspklist->GetSpikeTime(j);
			if (iitime >= iitime_end)
				break;

			// skip classes not requested (artefact or wrong class)
			if (pspklist->GetSpikeClass(j) < 0 /*&& !vdS->bartefacts*/)
				continue;
			if (vdS->spikeclassoption != 0 && pspklist->GetSpikeClass(j) != iclass)
				continue;

			int ii = (iitime - iitime_start) / iibinsize;
			if (ii >= vdS->nbins)
				ii = vdS->nbins;
			(*(plSum0 + ii))++;
		}
	}
	return n;
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
	const auto pspklist = &spikelist_array[m_currspklist];

	const auto samprate = pspklist->GetAcqSampRate();		// sampling rate
	long n = 0;
	ASSERT(samprate != 0.0f);	// converting iitime into secs and back needs <> 0!
	const auto nspikes = pspklist->GetTotalSpikes();	// this is total nb of spikes within file
	const auto binsize = vdS->binISI;				// bin size (in secs)
	const auto iibinsize = static_cast<long>(binsize * samprate);
	ASSERT(iibinsize > 0);

	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimIntervals.intervalsArray.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimIntervals.npercycle > 1 && increment > m_stimIntervals.npercycle)
			increment = m_stimIntervals.npercycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		auto iistart = static_cast<long>(vdS->timestart * samprate);	// boundaries
		auto iiend = static_cast<long>(vdS->timeend * samprate);
		if (!vdS->babsolutetime && m_stimIntervals.nitems > 0)	// adjust boundaries if ref is made to
		{										// a stimulus
			iistart += m_stimIntervals.intervalsArray.GetAt(istim);
			iiend += m_stimIntervals.intervalsArray.GetAt(istim);
		}

		// find first spike within interval requested
		// assume all spikes are ordered with respect to their occurence time
		long iitime0 = 0;
		int j;
		for (j = 0; j < nspikes; j++)
		{
			iitime0 = pspklist->GetSpikeTime(j);
			if (iitime0 <= iiend && iitime0 >= iistart)
				break;	// found!
		}
		if (nspikes <= 0 || j >= nspikes)
			return 0;

		// build histogram
		for (auto i = j + 1; i < nspikes; i++)
		{
			const auto iitime = pspklist->GetSpikeTime(i);
			if (iitime > iiend)
				break;
			ASSERT(iitime > iitime0);
			if (iitime < iitime0)
			{
				iitime0 = iitime;
				continue;
			}
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;
			const auto ii = static_cast<int>((iitime - iitime0) / iibinsize);
			if (ii <= static_cast<int>(vdS->nbinsISI))
			{
				(*(plSum0 + ii))++;
				n++;
			}
			iitime0 = iitime;
		}
	}
	return n;
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
	long n = 0;			// number of pivot spikes used to build autocorrelation
	const auto pspklist = &spikelist_array[m_currspklist];
	const auto nspikes = pspklist->GetTotalSpikes();		// number of spikes in that file
	if (nspikes <= 0)			// exit if no spikes
		return n;

	const auto samprate = pspklist->GetAcqSampRate();	// sampling rate
	ASSERT(samprate != 0.f);
	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimIntervals.intervalsArray.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimIntervals.npercycle > 1 && increment > m_stimIntervals.npercycle)
			increment = m_stimIntervals.npercycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		long iioffset0 = 0;								// initial offset
		if (!vdS->babsolutetime)						// if stimulus locking
		{												// get time of reference stim
			if (m_stimIntervals.nitems > 0)
				iioffset0 = m_stimIntervals.intervalsArray.GetAt(istim);
			else
				iioffset0 = static_cast<long>(-(vdS->timestart * samprate));
		}
		// adjust time limits to cope with stimulus locking
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;

		// histogram parameters
		const auto x = vdS->binISI * vdS->nbinsISI * samprate;
		auto iispan = static_cast<long>(x);
		if (x - float(iispan) > 0.5f)
			iispan++;
		const auto iibinsize = iispan / vdS->nbinsISI;
		ASSERT(iibinsize > 0);
		iispan /= 2;

		// search with the first spike falling into that window
		auto i0 = 0;
		while ((i0 < nspikes) && (pspklist->GetSpikeTime(i0) < iistart))
			i0++;

		// build histogram external loop search 'pivot spikes'
		for (auto i = i0; i < nspikes; i++)
		{
			const auto iitime0 = pspklist->GetSpikeTime(i);	// get spike time
			if (iitime0 > iiend)						// stop loop if out of range
				break;
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;								// discard if class not requested
			n++;										// update nb of pivot spikes
			// search backwards first spike that is ok
			auto i1 = i;
			const int ifirst = iitime0 - iispan;
			while (i1 > 0 && pspklist->GetSpikeTime(i1) > ifirst)
				i1--;

			// internal loop: build autoc
			for (auto j = i1; j < nspikes; j++)
			{
				if (j == i)								// discard spikes w. same time
					continue;
				const auto iitime = pspklist->GetSpikeTime(j) - iitime0;
				if (abs(iitime) >= iispan)
					continue;
				if (vdS->spikeclassoption && pspklist->GetSpikeClass(j) != iclass)
					continue;

				const int tiitime = ((iitime + iispan) / iibinsize);
				if (tiitime >= static_cast<int>(vdS->nbinsISI))
					continue;
				ASSERT(tiitime >= 0);
				ASSERT(tiitime < vdS->nbinsISI);
				(*(plSum0 + tiitime))++;
			}
		}
	}
	return n;
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
	long n = 0;					// number of 'pivot spikes'
	auto pspklist = &spikelist_array[m_currspklist];

	const auto nspikes = pspklist->GetTotalSpikes();
	if (nspikes <= 0)			// return if no spikes in that file
		return n;

	// lock PSTH to stimulus if requested
	const auto samprate = pspklist->GetAcqSampRate();
	ASSERT(samprate != 0.f);

	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimIntervals.intervalsArray.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimIntervals.npercycle > 1 && increment > m_stimIntervals.npercycle)
			increment = m_stimIntervals.npercycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		auto iioffset0 = 0;
		if (!vdS->babsolutetime)
		{
			if (m_stimIntervals.nitems > 0)
				iioffset0 = m_stimIntervals.intervalsArray.GetAt(istim);
			else
				iioffset0 = static_cast<long>(-(vdS->timestart * samprate));
		}
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;
		const auto iilength = iiend - iistart;

		// histogran parameters
		const auto x = vdS->binISI * vdS->nbinsISI * samprate;
		auto iispan = static_cast<long>(x);
		if (x - float(iispan) > 0.5f)
			iispan++;
		const auto iiautocorrbinsize = iispan / vdS->nbinsISI;
		iispan /= 2;
		const auto iidummy = vdS->nbins * vdS->nbinsISI;

		// build histogram
		auto i0 = 0;								// search first spike within interval
		while (pspklist->GetSpikeTime(i0) < iistart)
		{
			i0++;								// loop until found
			if (i0 >= nspikes)
				return 0L;
		}

		// external loop: pivot spikes
		for (auto i = i0; i < nspikes; i++)		// loop over spikes A
		{
			// find an appropriate spike
			const auto iitime0 = pspklist->GetSpikeTime(i);
			if (iitime0 >= iiend)				// exit loop
				break;
			if (vdS->spikeclassoption && pspklist->GetSpikeClass(i) != iclass)
				continue;
			n++;								// update nb of pivot spikes
			// compute base index (where to store autocorrelation for this pivot spike)
			int i_psth = ((iitime0 - iistart) * vdS->nbins) / iilength;
			ASSERT(i_psth >= 0);
			ASSERT(i_psth < vdS->nbins);
			i_psth *= vdS->nbinsISI;

			// get nb of counts around that spike
			// loop backwards to find first spike OK
			const int ifirst = iitime0 - iispan;		// temp value
			auto i1 = i;
			while (i1 > 0 && pspklist->GetSpikeTime(i1) > ifirst)
				i1--;
			// internal loop build histogram
			for (auto j = i1; j < nspikes; j++)
			{
				if (j == i)
					continue;

				const auto iitime = pspklist->GetSpikeTime(j) - iitime0;
				if (iitime >= iispan)
					break;
				if (iitime < -iispan)
					continue;

				if (vdS->spikeclassoption && pspklist->GetSpikeClass(j) != iclass)
					continue;

				const int tiitime = (iitime + iispan) / iiautocorrbinsize;
				ASSERT(tiitime >= 0);
				ASSERT(tiitime < vdS->nbinsISI);
				if (tiitime > static_cast<int>(vdS->nbinsISI))
					continue;
				const auto ii = i_psth + tiitime;
				ASSERT(ii <= iidummy);
				(*(plSum0 + ii))++;
			}
		}
	}
	return n;
}

void CSpikeDoc::_ExportSpkAverageWave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, int ispklist, int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spikelist_array[ispklist];
	// update offset
	auto iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimIntervals.nitems > 0)
		iioffset0 = m_stimIntervals.intervalsArray.GetAt(vdS->istimulusindex);
	// prepare parameters
	const auto rate = pspklist->GetAcqSampRate();
	const auto iitime_start = static_cast<long>(vdS->timestart * rate) + iioffset0;
	const auto iitime_end = static_cast<long>(vdS->timeend * rate) + iioffset0;
	const auto nspikes = pspklist->GetTotalSpikes();

	// clear histogram area if histogram of amplitudes
	ASSERT(vdS->exportdatatype == EXPORT_AVERAGE);
	auto p_doubl = pDoubl0 + 1;
	const auto spklen = int(*pDoubl0);	// spk length + nb items
	auto i0 = spklen * 2 + 2;
	for (auto k = 1; k < i0; k++, p_doubl++)
		*p_doubl = 0;

	// check file size and position pointer at the first spike within the bin
	i0 = 0;
	while ((i0 < nspikes) && (pspklist->GetSpikeTime(i0) < iitime_start))
		i0++;

	// ------- count total spk/interval (given bspkclass&bartefacts)
	const short binzero = pspklist->GetAcqBinzero();
	for (auto j = i0; j < nspikes; j++)
	{
		// skip intervals not requested
		const auto iitime = pspklist->GetSpikeTime(j);
		if (iitime >= iitime_end)
			break;
		// skip classes not requested
		const auto cla = pspklist->GetSpikeClass(j);
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)
			continue;
		// get value, compute statistics
		auto p_spik = pspklist->GetpSpikeData(j);
		const auto p_n = pDoubl0 + 1;
		auto p_sum = pDoubl0 + 2;
		auto p_sum2 = pDoubl0 + 2 + spklen;
		for (auto k = 0; k < spklen; k++, p_sum++, p_sum2++, p_spik++)
		{
			const auto x = static_cast<double>(*p_spik) - binzero;
			*p_sum += x;
			*p_sum2 += (x * x);
		}
		(*p_n)++;
	}

	// 4) export histogram
	const auto p_n = pDoubl0 + 1;
	auto p_sum = pDoubl0 + 2;
	auto p_sum2 = pDoubl0 + 2 + spklen;
	if (*p_n > 0)
	{
		const auto vper_bin = double(pspklist->GetAcqVoltsperBin()) * 1000.f;
		const auto vper_bin2 = vper_bin * vper_bin;
		const auto y_n = *p_n;
		cs_dummy.Format(_T("\t%i"), int(y_n));
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		for (auto k = 0; k < spklen; k++, p_sum++)
		{
			cs_dummy.Format(_T("\t%.3lf"), ((*p_sum) / y_n) * vper_bin);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}

		for (auto k = 0; k < spklen; k++, p_sum2++)
		{
			cs_dummy.Format(_T("\t%.3lf"), (*p_sum2) * vper_bin2);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		cs_dummy = _T("\t\t\t0");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::ExportSpkAverageWave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, const CString csFileComment)
{
	auto pspklist = &spikelist_array[m_currspklist];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	if ((vdS->spikeclassoption == -1
		|| vdS->spikeclassoption == 1)
		&& !pspklist->IsClassListValid())
		pspklist->UpdateClassList();

	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->GetNbclasses() - 1;
		for (auto j = i; j >= 0; i--, j--)
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
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->GetNbclasses() - 1;

	// loop header..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		// ................................COMMENTS
		const auto iclass = pspklist->GetclassID(kclass);
		ExportSpkFileComment(pSF, vdS, iclass, csFileComment);
		_ExportSpkAverageWave(pSF, vdS, pDoubl0, m_currspklist, iclass);
	}
}

CSpikeList* CSpikeDoc::SetSpkList_AsCurrent(int ichan)
{
	CSpikeList* pspklist = nullptr;
	if (spikelist_array.GetSize() > 0 && ichan >= 0 && ichan < spikelist_array.GetSize())
	{
		pspklist = &spikelist_array[ichan];
		m_currspklist = ichan;
	}
	return pspklist;
}

CSpikeList* CSpikeDoc::GetSpkList_Current()
{
	if (m_currspklist >= 0)
		return &spikelist_array[m_currspklist];
	else
		return nullptr;
}

CSpikeList* CSpikeDoc::GetSpkList_At(int ichan) {
	return &spikelist_array[ichan];
}