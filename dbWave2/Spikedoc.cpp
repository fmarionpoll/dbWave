#include "StdAfx.h"
#include "AcqDataDoc.h"
#include "Spikedoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CSpikeDoc, CDocument, 1 /* schema number*/)

CSpikeDoc::CSpikeDoc()
= default;

void CSpikeDoc::clear_data()
{
	m_spike_class.EraseData();
	if (m_stimulus_intervals.n_items > 0)
	{
		m_stimulus_intervals.RemoveAll();
		m_stimulus_intervals.n_items = 0;
	}
	spike_list_array.SetSize(1);
	spike_list_array[0].erase_data();
}

BOOL CSpikeDoc::OnNewDocument()
{
	clear_data();
	DeleteContents();
	m_strPathName.Empty();
	SetModifiedFlag(FALSE);

	return TRUE;
}

CSpikeDoc::~CSpikeDoc()
{
	clear_data();
}

BEGIN_MESSAGE_MAP(CSpikeDoc, CDocument)

END_MESSAGE_MAP()

void CSpikeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		m_wVersion = 7;
		ar << m_wVersion;
		serialize_v7(ar);
	}
	else
	{
		WORD wwVersion; ar >> wwVersion;
		if (wwVersion == 7)
			serialize_v7(ar);
		else if (wwVersion == 6)
			read_version6(ar);
		else if (wwVersion < 6)
			read_before_version6(ar, wwVersion);
		else
		{
			CString message;
			message.Format(_T("Spike file version not recognized: %i"), wwVersion);
			AfxMessageBox(message, MB_OK);
		}
	}
}

void CSpikeDoc::sort_stim_array()
{
	const auto nsti = m_stimulus_intervals.GetSize();
	if (nsti == 0 || (m_stimulus_intervals.GetAt(nsti - 1) > m_stimulus_intervals.GetAt(0)))
		return;

	// bubble sort from bottom to top
	for (auto j = 0; j < nsti; j++)
	{
		int imin = m_stimulus_intervals.GetAt(j);
		for (auto k = j + 1; k < nsti; k++)
		{
			if (m_stimulus_intervals.GetAt(k) < imin)
			{
				imin = m_stimulus_intervals.GetAt(k);
				m_stimulus_intervals.SetAt(k, m_stimulus_intervals.GetAt(j));
				m_stimulus_intervals.SetAt(j, imin);
			}
		}
	}
}

void CSpikeDoc::read_before_version6(CArchive& ar, WORD wwVersion)
{
	if (wwVersion >= 2 && wwVersion < 5)
	{
		ar >> m_wave_format.csStimulus >> m_wave_format.csConcentration;
		ar >> m_wave_format.csSensillum;
	}
	ar >> m_detection_date >> m_comment; // R2-3
	ar >> m_acquisition_file_name >> m_acquisition_comment >> m_acquisition_time; // R4-6
	ar >> m_acquisition_rate >> m_acquisition_size; // R7-8

	if (wwVersion >= 3 && wwVersion <= 5)
	{
		ar >> m_stimulus_intervals.n_items; // R9 - load number of items
		m_stimulus_intervals.Serialize(ar); // R10 - read data from file
		sort_stim_array();
	}

	if (wwVersion >= 4)
	{
		int nitems; // presumably 1
		ar >> nitems; // R11
		ASSERT(nitems == 1);
		m_wave_format.Serialize(ar); // R12
		ar >> nitems; // R13 -  normally (-1): end of list
		if (nitems > 0)
		{
			int isize;
			ar >> isize;
			ASSERT(isize == 1);
			for (int i = 0; i < isize; i++)
				m_stimulus_intervals.Serialize(ar);
			sort_stim_array();
		}
	}
	// read stimulus and spike classes
	spike_list_array.RemoveAll();
	spike_list_array.SetSize(1);
	spike_list_array[0].Serialize(ar); // v2 to v5
	//m_spike_class.Serialize(ar);
}


void CSpikeDoc::serialize_acquisition_parameters(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_detection_date;			// W2
		ar << m_comment;				// W3
		ar << m_acquisition_file_name;	// W4
		ar << m_acquisition_comment;	// W5
		ar << m_acquisition_time;		// W6
		ar << m_acquisition_rate;		// W7
		ar << m_acquisition_size;		// W8

		constexpr int n_items = 1; ar << n_items;
	}
	else
	{
		ar >> m_detection_date;			// W2
		ar >> m_comment;				// W3
		ar >> m_acquisition_file_name;	// W4
		ar >> m_acquisition_comment;	// W5
		ar >> m_acquisition_time;		// W6
		ar >> m_acquisition_rate;		// W7
		ar >> m_acquisition_size;		// W8

		// version 4
		int n_items; ar >> n_items;		// W9
		ASSERT(n_items == 1);
	}
	m_wave_format.Serialize(ar);
}

void CSpikeDoc::serialize_stimulus_intervals(CArchive& ar)
{
	int n_items = 1;
	if (ar.IsStoring())
	{
		ar << n_items;
		sort_stim_array();
	}
	else
	{
		ar >> n_items;
	}
	m_stimulus_intervals.Serialize(ar);
}

void CSpikeDoc::serialize_spike_list_arrays(CArchive& ar)
{
	int n_spike_arrays = 0;
	if (ar.IsStoring())
	{
		n_spike_arrays = spike_list_array.GetSize();
		ar << n_spike_arrays;
	}
	else
	{
		ar >> n_spike_arrays;
		spike_list_array.RemoveAll();
		spike_list_array.SetSize(n_spike_arrays);
	}

	for (int i = 0; i < n_spike_arrays; i++)
		spike_list_array[i].Serialize(ar); // v6-v7
}

void CSpikeDoc::read_version6(CArchive& ar)
{
	serialize_acquisition_parameters(ar);
	
	int n_items = 0; ar >> n_items;
	if (n_items > 0)
	{
		int i_size; ar >> i_size;
		ASSERT(i_size == 1);
		for (int i = 0; i < i_size; i++)
			m_stimulus_intervals.Serialize(ar);
	}
	sort_stim_array();

	// read stimulus and spike classes
	spike_list_array.RemoveAll();
	spike_list_array.SetSize(1);
	spike_list_array[0].Serialize(ar); // v6
	
	//m_spike_class.Serialize(ar);
}

void CSpikeDoc::serialize_v7(CArchive& ar)
{
	serialize_acquisition_parameters(ar);
	serialize_stimulus_intervals(ar);
	serialize_spike_list_arrays(ar);
	m_spike_class.Serialize(ar);
}

// CSpikeDoc commands
void CSpikeDoc::set_file_extension_as_spk(CString& fileName)
{
	const auto i = fileName.ReverseFind('.');
	if (i > 0)
		fileName = fileName.Left(i); 
	fileName += ".spk";	
}

BOOL CSpikeDoc::OnSaveDocument(LPCTSTR pszPathName)
{
	// check that path name has ".spk"
	CString fileName = pszPathName;
	if (fileName.IsEmpty())
		return false;
	set_file_extension_as_spk(fileName);

	CFileStatus status;
	const auto b_flag_exists = CFile::GetStatus(fileName, status);

	if (b_flag_exists && (status.m_attribute & CFile::readOnly))
	{
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, fileName);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
		case IDYES:
			{
				// If so, either Save or Update, as appropriate DoSave
				const auto p_template = GetDocTemplate();
				ASSERT(p_template != NULL);
				if (!m_new_path.IsEmpty())
				{
					const auto j = fileName.ReverseFind('\\') + 1;
					if (j != -1)
						fileName = fileName.Mid(j);
					fileName = m_new_path + fileName;
				}

				if (!AfxGetApp()->DoPromptFileName(fileName,
				                                   AFX_IDS_SAVEFILE,
				                                   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
					return FALSE;
				if (!AfxGetApp()->DoPromptFileName(fileName,
				                                   AFX_IDS_SAVEFILECOPY,
				                                   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
					return FALSE;

				// don't even attempt to save
				const auto k = fileName.ReverseFind('\\') + 1; // find last occurence of antislash
				if (k != -1)
					m_new_path = fileName.Left(k);
			}
			break;

		case IDNO:
		case IDCANCEL:
		default:
			return FALSE;
		}
	}

	CFile f;
	CFileException fe;
	if (f.Open(fileName, CFile::modeCreate | CFile::modeWrite, &fe))
	{
		try
		{
			CArchive ar(&f, CArchive::store);
			Serialize(ar);
			ar.Flush();
			ar.Close();
		}
		catch (CArchiveException* e)
		{
			e->Delete();
		}
		f.Close();
		SetModifiedFlag(FALSE);
	}
	else
	{
		f.Abort();
		ReportSaveLoadException(fileName, &fe, FALSE, AFX_IDP_FAILED_TO_SAVE_DOC);
		fe.Delete();
		return FALSE;
	}
	SetModifiedFlag(FALSE);
	return TRUE;
}

BOOL CSpikeDoc::OnOpenDocument(LPCTSTR pszPathName)
{
	clear_data();
	CFileStatus status;
	if (!CFile::GetStatus(pszPathName, status))
		return FALSE;

	CFile f;
	CFileException fe;
	auto b_read = TRUE;
	const CString filename = pszPathName;
	if (f.Open(filename, CFile::modeRead | CFile::shareDenyNone, &fe))
	{
		m_current_spike_list = 0;
		try
		{
			CArchive ar(&f, CArchive::load);
			Serialize(ar);
			ar.Flush();
			ar.Close();
			f.Close();
			SetModifiedFlag(FALSE);

			// update nb of classes
			for (auto i = 0; i < spike_list_array.GetSize(); i++)
			{
				if (!spike_list_array[i].is_class_list_valid()) 
				{
					spike_list_array[i].update_class_list();
					SetModifiedFlag(); 
				}
			}
		}
		catch (CArchiveException* e)
		{
			e->Delete();
			//b_read = FALSE;
		}
	}
	else
	{
		ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		b_read = FALSE;
	}
	return b_read;
}

void CSpikeDoc::init_source_doc(AcqDataDoc* p_document)
{
	// load parameters from file
	const auto wave_format = p_document->GetpWaveFormat();
	m_acquisition_time = wave_format->acqtime;
	m_acquisition_size = p_document->GetDOCchanLength();
	m_acquisition_rate = wave_format->sampling_rate_per_channel;
	m_acquisition_comment = wave_format->cs_comment;
	m_wave_format.Copy( wave_format);
}

CString CSpikeDoc::get_file_infos()
{
	TCHAR sz[64];
	const auto psz = sz;
	const auto psep_tab = _T("\t");
	const auto psep_rc = _T("\r\n");
	auto cs_out = GetPathName();
	cs_out.MakeUpper();
	cs_out += psep_tab + m_detection_date.Format(_T("%c"));
	cs_out += psep_rc + m_comment;
	cs_out += psep_rc;

	cs_out += _T("*** SOURCE DATA ***\r\n");
	cs_out += m_acquisition_file_name + psep_rc;
	cs_out += m_acquisition_comment + psep_rc;
	cs_out += m_acquisition_time.Format(_T("%#d-%B-%Y")) + psep_rc;

	cs_out += _T("*** SPIKE LIST ***\r\n");

	long lnspikes = 0;
	const auto pspklist = &spike_list_array[m_current_spike_list];
	if (pspklist->get_spikes_count() > 0)
		lnspikes = pspklist->get_spikes_count();
	wsprintf(psz, _T("n spikes = %li\r\n"), lnspikes);
	cs_out += sz;

	return cs_out;
}

// post stimulus histogram, ISI, autocorrelation
// vdS->exportdatatype == 0
// each line represents 1 recording
// we start with a header, then each file is scanned and exported
// if nintervals < 0: export all spikes // feb 23, 2009
void CSpikeDoc::export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, int ispklist, int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spike_list_array[ispklist];
	const auto totalspikes = pspklist->get_spikes_count();

	// export time of nintervals first spikes
	auto nspikes = nintervals;
	if (nspikes > totalspikes || nspikes < 0)
		nspikes = totalspikes;
	const auto rate = pspklist->get_acq_samp_rate();

	for (auto j = 0; j < nspikes; j++)
	{
		Spike* pSpike = pspklist->get_spike(j);

		// skip classes not requested
		const auto cla = pSpike->get_class_id();
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)
			continue;

		// export time
		const auto tval = static_cast<float>(pSpike->get_time()) / rate;
		cs_dummy.Format(_T("\t%lf"), tval);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::export_spk_latencies(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nintervals, CString csFileComment)
{
	CString cs_dummy;
	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	auto pspklist = &spike_list_array[m_current_spike_list];

	if ((vdS->spikeclassoption == -1
			|| vdS->spikeclassoption == 1)
		&& !pspklist->is_class_list_valid())
		pspklist->update_class_list();

	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->get_classes_count() - 1;
		for (auto j = i; j >= 0; j--, i--)
			if (pspklist->get_class_id(i) == vdS->classnb)
				break;
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->get_classes_count() - 1;

	// loop header..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		// check if class is Ok
		const auto nbspk_for_thisclass = pspklist->get_class_n_items(kclass);
		if ((FALSE == vdS->bexportzero) && (nbspk_for_thisclass == 0))
			continue;
		const auto iclass = pspklist->get_class_id(kclass);
		export_spk_file_comment(pSF, vdS, iclass, csFileComment);
		export_spk_latencies(pSF, vdS, nintervals, m_current_spike_list, iclass);
	}
}

void CSpikeDoc::export_spk_PSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, int ispklist, int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spike_list_array[ispklist];
	auto nbins = 1;

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH: // PSTH
		nbins = vdS->nbins;
		break;
	case EXPORT_ISI: // ISI
		nbins = vdS->nbinsISI;
		break;
	case EXPORT_AUTOCORR: // Autocorr
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
	case EXPORT_PSTH: // PSTH
		build_PSTH(vdS, plSum0, iclass);
		break;
	case EXPORT_ISI: // ISI
		n = build_ISI(vdS, plSum0, iclass);
		cs_dummy.Format(_T("\t%li"), n);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR: // Autocorr
		n = build_autocorr(vdS, plSum0, iclass);
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
	if (vdS->exportdatatype == EXPORT_PSTH && m_stimulus_intervals.GetSize() > 0)
	{
		auto samprate = pspklist->get_acq_samp_rate();
		if (samprate == 0.f)
		{
			samprate = m_acquisition_rate;
		}
		ASSERT(samprate != 0.f);
		const auto istim0 = 0;
		const auto istim1 = m_stimulus_intervals.GetSize() - 1;
		auto iioffset0 = 0;
		if (!vdS->babsolutetime)
			iioffset0 = m_stimulus_intervals.GetAt(vdS->istimulusindex);
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;
		auto b_up = -1;

		for (auto istim = istim0; istim <= istim1; istim++)
		{
			const int iistim = m_stimulus_intervals.GetAt(istim);
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

void CSpikeDoc::export_spk_PSTH(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* plSum0, const CString&
                              csFileComment)
{
	CString cs_dummy;
	SpikeList* pspklist = &spike_list_array[m_current_spike_list];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	int class0 = 0;
	int class1 = 0;
	if ((vdS->spikeclassoption == -1 // 1 class selected
			|| vdS->spikeclassoption == 1) // all classes selected but displayed on different lines
		&& !pspklist->is_class_list_valid())
		pspklist->update_class_list();

	if (vdS->spikeclassoption == -1) // only 1 class selected
	{
		// search item index with correct class ID
		auto i = pspklist->get_classes_count() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (pspklist->get_class_id(i) == vdS->classnb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
		{
			export_spk_file_comment(pSF, vdS, vdS->classnb, csFileComment);
			// dummy export if requested export nb spikes / bin: print nb
			switch (vdS->exportdatatype)
			{
			case EXPORT_ISI: // ISI
			case EXPORT_AUTOCORR: // Autocorr
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
		class1 = pspklist->get_classes_count() - 1;

	// loop header..................
	for (int kclass = class0; kclass <= class1; kclass++)
	{
		// check if class is Ok
		const auto nbspk_for_thisclass = pspklist->get_class_n_items(kclass);
		const auto iclass = pspklist->get_class_id(kclass);
		// export the comments
		export_spk_file_comment(pSF, vdS, iclass, csFileComment);
		// test if we should continue
		if (!(FALSE == vdS->bexportzero) && (nbspk_for_thisclass == 0))
			export_spk_PSTH(pSF, vdS, plSum0, m_current_spike_list, iclass);

		cs_dummy = _T("\r\n");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

// export histograms of the amplitude of the spikes found in each file of the file series
// export type = 4
// same tb as PSTH: each line represents one record, 1 class
void CSpikeDoc::export_spk_amplitude_histogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, int ispklist,
                                          int iclass)
{
	// ................................DATA
	auto pspklist = &spike_list_array[ispklist];
	CString cs_dummy;
	// temp parameters for amplitudes histogram
	const auto histampl_min = static_cast<int>(vdS->histampl_vmin / pspklist->get_acq_voltsper_bin());
	const auto histampl_max = static_cast<int>(vdS->histampl_vmax / pspklist->get_acq_voltsper_bin());
	const auto histampl_binsize = (histampl_max - histampl_min) / vdS->histampl_nbins;
	const auto nbins = 1;
	// update offset
	auto iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimulus_intervals.n_items > 0)
		iioffset0 = m_stimulus_intervals.GetAt(vdS->istimulusindex);

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
	auto rate = pspklist->get_acq_samp_rate();
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
		for (auto j = 0; j < pspklist->get_spikes_count(); j++)
		{
			Spike* pSpike = pspklist->get_spike(j);

			// skip intervals not requested
			const auto ii_time = pSpike->get_time() - iioffset0;
			if ((ii_time < iitime0) || (ii_time >= iitime1))
				break;

			// skip classes not requested
			const auto cla = pSpike->get_class_id();
			if (cla < 0 /*&& !vdS->bartefacts*/)
				continue;
			if (vdS->spikeclassoption != 0 && cla != iclass)
				continue;

			// get value, compute statistics
			short max, min;
			pSpike->get_max_min(&max, &min);
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
			const auto volts_per_bin = pspklist->get_acq_voltsper_bin() * 1000.f;
			cs_dummy.Format(_T("\t%.3lf\t%.3lf\t%i"), (static_cast<double>(y_sum) / y_n) * volts_per_bin,
			                static_cast<double>(y_sum2) * volts_per_bin * volts_per_bin, y_n);
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

void CSpikeDoc::export_spk_amplitude_histogram(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, long* pHist0, const CString&
                                         csFileComment)
{
	auto pspklist = &spike_list_array[m_current_spike_list];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	if ((vdS->spikeclassoption == -1
			|| vdS->spikeclassoption == 1)
		&& !pspklist->is_class_list_valid())
		pspklist->update_class_list();

	// just one class selected: see if there is at least 1 spike with this class
	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->get_classes_count() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (pspklist->get_class_id(i) == vdS->classnb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
			return export_spk_file_comment(pSF, vdS, vdS->classnb, csFileComment);
		// set class boundaries to the index of the class having the proper ID
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->get_classes_count() - 1;

	// loop over all classes..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		const auto iclass = pspklist->get_class_id(kclass);
		export_spk_file_comment(pSF, vdS, iclass, csFileComment);
		export_spk_amplitude_histogram(pSF, vdS, pHist0, m_current_spike_list, iclass);
	}
}

// option 1: spike intervals
// option 2: spike extrema (max and min)
// option 3: export spike amplitudes (max - min)
// one file at a time, organized in columns

void CSpikeDoc::export_spk_attributes_one_file(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	int iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimulus_intervals.n_items > 0)
	{
		iioffset0 = m_stimulus_intervals.GetAt(vdS->istimulusindex);
	}

	// ................................DATA
	auto pspklist = &spike_list_array[m_current_spike_list];
	CString cs_dummy = _T("\r\ntime(s)\tclass");
	auto rate = pspklist->get_acq_samp_rate();
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
	default: ;
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike list -- prepare parameters
	const auto tspan = vdS->timeend - vdS->timestart;
	auto tfirst = vdS->timestart;
	auto iitime1 = static_cast<long>(tfirst * rate);

	tfirst = vdS->timestart + tspan;
	const auto iitime0 = iitime1;
	iitime1 = static_cast<long>((tfirst + tspan) * rate);
	const auto vper_bin = pspklist->get_acq_voltsper_bin() * 1000.f;
	const auto binzero = pspklist->get_acq_binzero();
	int imax, imin;
	const auto ifirst = pspklist->m_imaxmin1SL;
	const auto ilast = pspklist->m_imaxmin2SL;
	const auto ratems = rate / 1000.f;

	// export data: loop through all spikes
	for (auto j = 0; j < pspklist->get_spikes_count(); j++)
	{
		Spike* pSpike = pspklist->get_spike(j);

		// skip intervals not requested
		const auto ii_time = pSpike->get_time() - iioffset0;
		if ((ii_time < iitime0) || (ii_time >= iitime1))
			break;
		// skip classes not requested
		const auto cla = pSpike->get_class_id();
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;

		// export time and spike time
		const auto tval = static_cast<float>(ii_time) / rate;
		cs_dummy.Format(_T("\r\n%lf\t%i"), tval, cla);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		switch (vdS->exportdatatype)
		{
		// 2) export time interval (absolute time)
		case EXPORT_INTERV:
			break;
		// 3) export extrema
		case EXPORT_EXTREMA:
			short value_max, value_min;
			pSpike->get_max_min(&value_max, &value_min);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(value_max) - binzero) * vper_bin,
			                (static_cast<double>(value_min) - binzero) * vper_bin);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;
		// 4) export max - min
		case EXPORT_AMPLIT:
			pSpike->MeasureMaxThenMinEx(&value_max, &imax, &value_min, &imin, ifirst, ilast);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(value_max) - value_min) * vper_bin,
			                (static_cast<double>(imin) - imax) / ratems);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;
		case EXPORT_SPIKEPOINTS:
			for (auto index = 0; index < pspklist->get_spike_length(); index++)
			{
				const auto val = pSpike->get_value_at_offset(index);
				cs_dummy.Format(_T("\t%.3lf"), static_cast<double>(val) * vper_bin);
				pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			}
			break;
		default: ;
		}
	}
}

// Describe the type of data exported
void CSpikeDoc::export_table_title(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int nfiles)
{
	CString cs_dummy;

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH: cs_dummy = _T("Histogram nb spikes/bin");
		break;
	case EXPORT_ISI: cs_dummy = _T("Intervals between spikes histogram");
		break;
	case EXPORT_AUTOCORR: cs_dummy = _T("Autocorrelation histogram");
		break;
	case EXPORT_LATENCY: cs_dummy = _T("Latency first 10 spikes");
		break;
	case EXPORT_AVERAGE: cs_dummy = _T("Spike average waveform");
		break;
	case EXPORT_INTERV: cs_dummy = _T("Spike time intervals");
		nfiles = 1;
		break;
	case EXPORT_EXTREMA: cs_dummy = _T("Spike amplitude extrema");
		nfiles = 1;
		break;
	case EXPORT_AMPLIT: cs_dummy = _T("Spike amplitudes");
		nfiles = 1;
		break;
	case EXPORT_HISTAMPL: cs_dummy = _T("Spike amplitude histograms");
		break;
	case EXPORT_SPIKEPOINTS: cs_dummy = _T("Spike data points");
		break;
	default: vdS->exportdatatype = 0;
		cs_dummy = _T("Histogram nb spikes/bin");
		break;
	}
	auto t = CTime::GetCurrentTime(); // current date & time
	CString cs_date; // first string to receive date and time
	cs_date.Format(_T("  created on %d-%d-%d at %d:%d"), t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(),
	               t.GetMinute());
	cs_dummy += cs_date;
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH:
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
	case EXPORT_EXTREMA:
		cs_dummy.Format(
			_T("\r\nnbins= %i\tbin size (s):\t%.3f\tt start (s):\t%.3f\tt end (s):\t%.3f\r\nn files = %i\r\n"),
			vdS->nbins,
			(vdS->timeend - vdS->timestart) / vdS->nbins,
			vdS->timestart,
			vdS->timeend,
			nfiles);
		break;
	case EXPORT_AVERAGE:
		cs_dummy.Format(_T("\r\ndata:\tN\txi=mean amplitude (mV)\tSum square of amplitudes\r\nn points:\t%i\r\n"),
		                get_spk_list_current()->get_spike_length());
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
void CSpikeDoc::export_table_col_headers_db(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString cs_dummy;

	// these infos are stored in the database - what we do here, is equivalent of doing a query
	cs_dummy.Format(_T("\r\nID\tFilename"));
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	auto ncolumns = 2;
	cs_dummy.Empty();
	if (vdS->bacqdate)
	{
		// ................     acq date, acq time
		cs_dummy += _T("\tacq_date\tacq_time");
		ncolumns += 2;
	}
	if (vdS->bacqcomments)
	{
		// ................     acq comments, stim, conc, type
		cs_dummy += _T("\tExpt\tinsectID\tssID\tinsect\tstrain\tsex\tlocation\toperator\tmore");
		ncolumns += 9;
		cs_dummy += _T("\tstim1\tconc1\trepeat1\tstim2\tconc2\trepeat2\ttype\tflag");
		ncolumns += 8;
	}

	// this is specific to the spike file (not stored in the database)
	if (vdS->bspkcomments)
	{
		// ................     spike comments
		cs_dummy += _T("\tspk_comment");
		ncolumns += 1;
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike detection chan
	if (vdS->btotalspikes)
	{
		// ................     total spikes
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
void CSpikeDoc::export_table_col_headers_data(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS)
{
	CString cs_dummy;
	// header of the data
	auto nbins = 0;
	float tspan;
	auto tbin = 1.f;
	auto tstart = vdS->timestart;
	switch (vdS->exportdatatype)
	{
	case EXPORT_PSTH: // PSTH
		nbins = vdS->nbins;
		tspan = vdS->timeend - vdS->timestart;
		tbin = tspan / nbins;
		break;
	case EXPORT_ISI: // ISI
		nbins = vdS->nbinsISI;
		tbin = vdS->binISI;
		tstart = 0;
		cs_dummy = _T("\tN");
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR: // Autocorr
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
			const auto npoints = get_spk_list_current()->get_spike_length();
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
			cs.Format(_T("\tb_%.3f"), static_cast<double>(tstart) + static_cast<double>(tbin) * i);
			cs_dummy += cs;
		}
	}
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

// output descriptors of each record
// TODO: output descriptors from the database and not from the spike files ??
void CSpikeDoc::export_spk_file_comment(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, int iclass,
                                     const CString& csFileComment)
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
		if (vdS->bacqdate) // source data time and date
		{
			cs_dummy.Format(_T("\t%s"), (LPCTSTR)m_acquisition_time.Format(_T("%#d %m %Y\t%X")));
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
		// source data comments
		if (vdS->bacqcomments)
		{
			cs_dummy = cs_tab + m_acquisition_comment;
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			cs_dummy.Format(_T("\t%i"), m_wave_format.insectID);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy.Format(_T("\t%i"), m_wave_format.sensillumID);
			pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy = cs_tab + m_wave_format.csInsectname;
			cs_dummy += cs_tab + m_wave_format.csStrain;
			cs_dummy += cs_tab + m_wave_format.csSex;
			cs_dummy += cs_tab + m_wave_format.csLocation;
			cs_dummy += cs_tab + m_wave_format.csOperator;
			cs_dummy += cs_tab + m_wave_format.csMoreComment;

			cs_dummy += cs_tab + m_wave_format.csStimulus;
			cs_dummy += cs_tab + m_wave_format.csConcentration;
			cs_temp.Format(_T("%i"), m_wave_format.repeat);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wave_format.csStimulus2;
			cs_dummy += cs_tab + m_wave_format.csConcentration2;
			cs_temp.Format(_T("%i"), m_wave_format.repeat2);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wave_format.csSensillum;
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
	const auto pspklist = &spike_list_array[m_current_spike_list];

	if (vdS->btotalspikes)
	{
		cs_dummy.Format(_T("\t%f"), pspklist->get_detection_parameters()->detect_threshold_mv);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), pspklist->get_spikes_count());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), pspklist->get_classes_count());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		const auto tduration = static_cast<float>(m_acquisition_size) / m_acquisition_rate;
		cs_dummy.Format(_T("\t%f"), tduration);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
	// spike list item, spike class
	cs_dummy.Format(_T("\t%i \t%s \t%i"), vdS->ichan, (LPCTSTR)pspklist->get_detection_parameters()->comment, iclass);
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

long CSpikeDoc::build_PSTH(OPTIONS_VIEWSPIKES* options_view_spikes, long* plSum0, int iclass)
{
	// ajust parameters
	long n = 0;
	const auto spike_list = &spike_list_array[m_current_spike_list];

	const auto n_spikes = spike_list->get_spikes_count();
	if (n_spikes <= 0)
		return 0;
	const auto rate = spike_list->get_acq_samp_rate();
	ASSERT(rate != 0.f);

	// check validity of istimulusindex
	auto stimulus_index = options_view_spikes->istimulusindex;
	if (stimulus_index > m_stimulus_intervals.GetSize() - 1)
		stimulus_index = m_stimulus_intervals.GetSize() - 1;
	if (stimulus_index < 0)
		stimulus_index = 0;
	const auto stimulus_index_0 = stimulus_index;
	auto stimulus_index_1 = stimulus_index_0 + 1;
	auto increment = 2;
	if (options_view_spikes->bCycleHist && !options_view_spikes->babsolutetime)
	{
		stimulus_index_1 = m_stimulus_intervals.GetSize();
		increment = options_view_spikes->nstipercycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto stimulus_index_i = stimulus_index_0; stimulus_index_i < stimulus_index_1; stimulus_index_i += increment, n++)
	{
		auto iioffset0 = 0;
		if (!options_view_spikes->babsolutetime)
		{
			if (m_stimulus_intervals.n_items > 0)
				iioffset0 = m_stimulus_intervals.GetAt(stimulus_index_i);
			else
				iioffset0 = static_cast<long>(-(options_view_spikes->timestart * rate));
		}
		const auto iitime_start = static_cast<long>(options_view_spikes->timestart * rate) + iioffset0;
		const auto iitime_end = static_cast<long>(options_view_spikes->timeend * rate) + iioffset0;
		auto iibinsize = static_cast<long>(options_view_spikes->timebin * rate); //(iitime_end - iitime_start) / options_view_spikes->nbins;
		if (iibinsize <= 0)
			iibinsize = 1;
		ASSERT(iibinsize > 0);

		// check file size and position pointer at the first spike within the bin
		auto i0 = 0;
		while ((i0 < n_spikes) && (spike_list->get_spike(i0)->get_time() < iitime_start))
			i0++;

		for (auto j = i0; j < n_spikes; j++)
		{
			Spike* pSpike = spike_list->get_spike(j);

			// skip intervals not requested
			const int ii_time = pSpike->get_time();
			if (ii_time >= iitime_end)
				break;

			// skip classes not requested (artefact or wrong class)
			if (pSpike->get_class_id() < 0 /*&& !options_view_spikes->bartefacts*/)
				continue;
			if (options_view_spikes->spikeclassoption != 0 && pSpike->get_class_id() != iclass)
				continue;

			int ii = (ii_time - iitime_start) / iibinsize;
			if (ii >= options_view_spikes->nbins)
				ii = options_view_spikes->nbins;
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

long CSpikeDoc::build_ISI(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	const auto pspklist = &spike_list_array[m_current_spike_list];

	const auto samprate = pspklist->get_acq_samp_rate(); // sampling rate
	long n = 0;
	ASSERT(samprate != 0.0f); // converting ii_time into secs and back needs <> 0!
	const auto nspikes = pspklist->get_spikes_count(); // this is total nb of spikes within file
	const auto binsize = vdS->binISI; // bin size (in secs)
	const auto iibinsize = static_cast<long>(binsize * samprate);
	ASSERT(iibinsize > 0);

	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimulus_intervals.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		auto iistart = static_cast<long>(vdS->timestart * samprate); // boundaries
		auto iiend = static_cast<long>(vdS->timeend * samprate);
		if (!vdS->babsolutetime && m_stimulus_intervals.n_items > 0) // adjust boundaries if ref is made to
		{
			// a stimulus
			iistart += m_stimulus_intervals.GetAt(istim);
			iiend += m_stimulus_intervals.GetAt(istim);
		}

		// find first spike within interval requested
		// assume all spikes are ordered with respect to their occurence time
		long iitime0 = 0;
		int j;
		for (j = 0; j < nspikes; j++)
		{
			Spike* pSpike = pspklist->get_spike(j);
			iitime0 = pSpike->get_time();
			if (iitime0 <= iiend && iitime0 >= iistart)
				break; 
		}
		if (nspikes <= 0 || j >= nspikes)
			return 0;

		// build histogram
		for (auto i = j + 1; i < nspikes; i++)
		{
			Spike* pSpike = pspklist->get_spike(i);
			const auto ii_time = pSpike->get_time();
			if (ii_time > iiend)
				break;
			ASSERT(ii_time > iitime0);
			if (ii_time < iitime0)
			{
				iitime0 = ii_time;
				continue;
			}
			if (vdS->spikeclassoption && pSpike->get_class_id() != iclass)
				continue;
			const auto ii = static_cast<int>((ii_time - iitime0) / iibinsize);
			if (ii <= vdS->nbinsISI)
			{
				(*(plSum0 + ii))++;
				n++;
			}
			iitime0 = ii_time;
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

long CSpikeDoc::build_autocorr(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	long n = 0; // number of pivot spikes used to build autocorrelation
	const auto pspklist = &spike_list_array[m_current_spike_list];
	const auto nspikes = pspklist->get_spikes_count(); // number of spikes in that file
	if (nspikes <= 0) // exit if no spikes
		return n;

	const auto samprate = pspklist->get_acq_samp_rate(); // sampling rate
	ASSERT(samprate != 0.f);
	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimulus_intervals.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		long iioffset0 = 0; // initial offset
		if (!vdS->babsolutetime) // if stimulus locking
		{
			// get time of reference stim
			if (m_stimulus_intervals.n_items > 0)
				iioffset0 = m_stimulus_intervals.GetAt(istim);
			else
				iioffset0 = static_cast<long>(-(vdS->timestart * samprate));
		}
		// adjust time limits to cope with stimulus locking
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;

		// histogram parameters
		const auto x = vdS->binISI * vdS->nbinsISI * samprate;
		auto iispan = static_cast<long>(x);
		if (x - static_cast<float>(iispan) > 0.5f)
			iispan++;
		const auto iibinsize = iispan / vdS->nbinsISI;
		ASSERT(iibinsize > 0);
		iispan /= 2;

		// search with the first spike falling into that window
		auto i0 = 0;
		while ((i0 < nspikes) && (pspklist->get_spike(i0)->get_time() < iistart))
			i0++;

		// build histogram external loop search 'pivot spikes'
		for (auto i = i0; i < nspikes; i++)
		{
			const auto iitime0 = pspklist->get_spike(i)->get_time(); // get spike time
			if (iitime0 > iiend) // stop loop if out of range
				break;
			if (vdS->spikeclassoption && pspklist->get_spike(i)->get_class_id() != iclass)
				continue; // discard if class not requested
			n++; // update nb of pivot spikes
			// search backwards first spike that is ok
			auto i1 = i;
			const int ifirst = iitime0 - iispan;
			while (i1 > 0 && pspklist->get_spike(i1)->get_time() > ifirst)
				i1--;

			// internal loop: build autoc
			for (auto j = i1; j < nspikes; j++)
			{
				Spike* pSpike = pspklist->get_spike(j);

				if (j == i) // discard spikes w. same time
					continue;
				const auto ii_time = pSpike->get_time() - iitime0;
				if (abs(ii_time) >= iispan)
					continue;
				if (vdS->spikeclassoption && pSpike->get_class_id() != iclass)
					continue;

				const int tiitime = ((ii_time + iispan) / iibinsize);
				if (tiitime >= vdS->nbinsISI)
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

long CSpikeDoc::build_PSTHautocorr(OPTIONS_VIEWSPIKES* vdS, long* plSum0, int iclass)
{
	long n = 0; // number of 'pivot spikes'
	auto pspklist = &spike_list_array[m_current_spike_list];

	const auto nspikes = pspklist->get_spikes_count();
	if (nspikes <= 0) // return if no spikes in that file
		return n;

	// lock PSTH to stimulus if requested
	const auto samprate = pspklist->get_acq_samp_rate();
	ASSERT(samprate != 0.f);

	const auto istim0 = vdS->istimulusindex;
	auto istim1 = istim0 + 1;
	auto increment = 2;
	if (vdS->bCycleHist && !vdS->babsolutetime)
	{
		istim1 = m_stimulus_intervals.GetSize();
		increment = vdS->nstipercycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto istim = istim0; istim < istim1; istim += increment, n++)
	{
		auto iioffset0 = 0;
		if (!vdS->babsolutetime)
		{
			if (m_stimulus_intervals.n_items > 0)
				iioffset0 = m_stimulus_intervals.GetAt(istim);
			else
				iioffset0 = static_cast<long>(-(vdS->timestart * samprate));
		}
		const auto iistart = static_cast<long>(vdS->timestart * samprate) + iioffset0;
		const auto iiend = static_cast<long>(vdS->timeend * samprate) + iioffset0;
		const auto iilength = iiend - iistart;

		// histogran parameters
		const auto x = vdS->binISI * vdS->nbinsISI * samprate;
		auto iispan = static_cast<long>(x);
		if (x - static_cast<float>(iispan) > 0.5f)
			iispan++;
		const auto iiautocorrbinsize = iispan / vdS->nbinsISI;
		iispan /= 2;
		const auto iidummy = vdS->nbins * vdS->nbinsISI;

		// build histogram
		auto i0 = 0; // search first spike within interval
		while (pspklist->get_spike(i0)->get_time() < iistart)
		{
			i0++; // loop until found
			if (i0 >= nspikes)
				return 0L;
		}

		// external loop: pivot spikes
		for (auto i = i0; i < nspikes; i++) // loop over spikes A
		{
			Spike* pSpike = pspklist->get_spike(i);

			// find an appropriate spike
			const auto iitime0 = pSpike->get_time();
			if (iitime0 >= iiend) // exit loop
				break;
			if (vdS->spikeclassoption && pSpike->get_class_id() != iclass)
				continue;
			n++; // update nb of pivot spikes
			// compute base index (where to store autocorrelation for this pivot spike)
			int i_psth = ((iitime0 - iistart) * vdS->nbins) / iilength;
			ASSERT(i_psth >= 0);
			ASSERT(i_psth < vdS->nbins);
			i_psth *= vdS->nbinsISI;

			// get nb of counts around that spike
			// loop backwards to find first spike OK
			const int ifirst = iitime0 - iispan; // temp value
			auto i1 = i;
			while (i1 > 0 && pspklist->get_spike(i1)->get_time() > ifirst)
				i1--;
			// internal loop build histogram
			for (auto j = i1; j < nspikes; j++)
			{
				if (j == i)
					continue;

				const Spike* p_spike_j = pspklist->get_spike(j);
				const auto ii_time = p_spike_j->get_time() - iitime0;
				if (ii_time >= iispan)
					break;
				if (ii_time < -iispan)
					continue;

				if (vdS->spikeclassoption && p_spike_j->get_class_id() != iclass)
					continue;

				const int tiitime = (ii_time + iispan) / iiautocorrbinsize;
				ASSERT(tiitime >= 0);
				ASSERT(tiitime < vdS->nbinsISI);
				if (tiitime > vdS->nbinsISI)
					continue;
				const auto ii = i_psth + tiitime;
				ASSERT(ii <= iidummy);
				(*(plSum0 + ii))++;
			}
		}
	}
	return n;
}

void CSpikeDoc::export_spk_average_wave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0, int ispklist,
                                      int iclass)
{
	CString cs_dummy;
	const auto pspklist = &spike_list_array[ispklist];
	// update offset
	auto iioffset0 = 0;
	if (!vdS->babsolutetime && m_stimulus_intervals.n_items > 0)
		iioffset0 = m_stimulus_intervals.GetAt(vdS->istimulusindex);
	// prepare parameters
	const auto rate = pspklist->get_acq_samp_rate();
	const auto iitime_start = static_cast<long>(vdS->timestart * rate) + iioffset0;
	const auto iitime_end = static_cast<long>(vdS->timeend * rate) + iioffset0;
	const auto nspikes = pspklist->get_spikes_count();

	// clear histogram area if histogram of amplitudes
	ASSERT(vdS->exportdatatype == EXPORT_AVERAGE);
	auto p_doubl = pDoubl0 + 1;
	const auto spklen = static_cast<int>(*pDoubl0); // spk length + nb items
	auto i0 = spklen * 2 + 2;
	for (auto k = 1; k < i0; k++, p_doubl++)
		*p_doubl = 0;

	// check file size and position pointer at the first spike within the bin
	i0 = 0;
	while ((i0 < nspikes) && (pspklist->get_spike(i0)->get_time() < iitime_start))
		i0++;

	// ------- count total spk/interval (given bspkclass&bartefacts)
	const short binzero = pspklist->get_acq_binzero();
	for (auto j = i0; j < nspikes; j++)
	{
		Spike* pSpike = pspklist->get_spike(j);
		// skip intervals not requested
		const auto ii_time = pSpike->get_time();
		if (ii_time >= iitime_end)
			break;
		// skip classes not requested
		const auto cla = pSpike->get_class_id();
		if (cla < 0 /*&& !vdS->bartefacts*/)
			continue;
		if (vdS->spikeclassoption != 0 && cla != iclass)
			continue;
		// get value, compute statistics
		auto p_spik = pSpike->get_p_data();
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
		const auto vper_bin = static_cast<double>(pspklist->get_acq_voltsper_bin()) * 1000.f;
		const auto vper_bin2 = vper_bin * vper_bin;
		const auto y_n = *p_n;
		cs_dummy.Format(_T("\t%i"), static_cast<int>(y_n));
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

void CSpikeDoc::export_spk_average_wave(CSharedFile* pSF, OPTIONS_VIEWSPIKES* vdS, double* pDoubl0,
                                     const CString csFileComment)
{
	auto pspklist = &spike_list_array[m_current_spike_list];

	// spike class: -1(one:selected); 0(all); 1(all:splitted)
	auto class0 = 0;
	auto class1 = 0;
	if ((vdS->spikeclassoption == -1
			|| vdS->spikeclassoption == 1)
		&& !pspklist->is_class_list_valid())
		pspklist->update_class_list();

	if (vdS->spikeclassoption == -1)
	{
		// search item index with correct class ID
		auto i = pspklist->get_classes_count() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (pspklist->get_class_id(i) == vdS->classnb)
				break;

		// if none is found, export descriptor and exit
		if (i < 0)
		{
			export_spk_file_comment(pSF, vdS, vdS->classnb, csFileComment);
			return;
		}
		class0 = i;
		class1 = i;
	}
	else if (vdS->spikeclassoption == 1)
		class1 = pspklist->get_classes_count() - 1;

	// loop header..................
	for (auto kclass = class0; kclass <= class1; kclass++)
	{
		// ................................COMMENTS
		const auto iclass = pspklist->get_class_id(kclass);
		export_spk_file_comment(pSF, vdS, iclass, csFileComment);
		export_spk_average_wave(pSF, vdS, pDoubl0, m_current_spike_list, iclass);
	}
}

SpikeList* CSpikeDoc::set_spk_list_as_current(int spike_list_index)
{
	SpikeList* spike_list = nullptr;
	if (spike_list_array.GetSize() > 0 && spike_list_index >= 0 && spike_list_index < spike_list_array.GetSize())
	{
		spike_list = &spike_list_array[spike_list_index];
		m_current_spike_list = spike_list_index;
	}
	return spike_list;
}

SpikeList* CSpikeDoc::get_spk_list_current()
{
	if (m_current_spike_list >= 0)
		return &spike_list_array[m_current_spike_list];
	return nullptr;
}

SpikeList* CSpikeDoc::get_spk_list_at(int spike_list_index)
{
	return &spike_list_array[spike_list_index];
}
