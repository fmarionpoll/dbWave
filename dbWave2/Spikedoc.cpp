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
	if (m_stimulus_intervals.n_items > 0)
	{
		m_stimulus_intervals.remove_all();
		m_stimulus_intervals.n_items = 0;
	}

	spike_list_array_.SetSize(1);
	spike_list_array_[0].erase_data();
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
		w_version_ = 7;
		ar << w_version_;
		serialize_v7(ar);
	}
	else
	{
		WORD version; ar >> version;
		if (version == 7)
			serialize_v7(ar);
		else if (version == 6)
			read_version6(ar);
		else if (version < 6)
			read_before_version6(ar, version);
		else
		{
			CString message;
			message.Format(_T("Spike file version not recognized: %i"), version);
			AfxMessageBox(message, MB_OK);
		}
	}
}

void CSpikeDoc::sort_stimulus_array()
{
	const auto stimulus_intervals_count = m_stimulus_intervals.get_size();
	if (stimulus_intervals_count == 0 || (m_stimulus_intervals.get_at(stimulus_intervals_count - 1) > m_stimulus_intervals.get_at(0)))
		return;

	// bubble sort from bottom to top
	for (auto j = 0; j < stimulus_intervals_count; j++)
	{
		int lowest_stimulus_interval = m_stimulus_intervals.get_at(j);
		for (auto k = j + 1; k < stimulus_intervals_count; k++)
		{
			if (m_stimulus_intervals.get_at(k) < lowest_stimulus_interval)
			{
				lowest_stimulus_interval = m_stimulus_intervals.get_at(k);
				m_stimulus_intervals.set_at(k, m_stimulus_intervals.get_at(j));
				m_stimulus_intervals.set_at(j, lowest_stimulus_interval);
			}
		}
	}
}

void CSpikeDoc::read_before_version6(CArchive& ar, WORD w_version)
{
	if (w_version >= 2 && w_version < 5)
	{
		ar >> m_wave_format.cs_stimulus >> m_wave_format.cs_concentration;
		ar >> m_wave_format.cs_sensillum;
	}
	ar >> detection_date_ >> comment_; // R2-3
	ar >> acquisition_file_name_ >> acquisition_comment_ >> acquisition_time_; // R4-6
	ar >> acquisition_rate_ >> acquisition_size_; // R7-8

	if (w_version >= 3 && w_version <= 5)
	{
		ar >> m_stimulus_intervals.n_items; // R9 - load number of items
		m_stimulus_intervals.Serialize(ar); // R10 - read data from file
		sort_stimulus_array();
	}

	if (w_version >= 4)
	{
		int n_items; // presumably 1
		ar >> n_items; // R11
		ASSERT(n_items == 1);
		m_wave_format.Serialize(ar); // R12
		ar >> n_items; // R13 -  normally (-1): end of list
		if (n_items > 0)
		{
			int i_size;
			ar >> i_size;
			ASSERT(i_size == 1);
			for (int i = 0; i < i_size; i++)
				m_stimulus_intervals.Serialize(ar);
			sort_stimulus_array();
		}
	}
	// read stimulus and spike classes
	spike_list_array_.RemoveAll();
	spike_list_array_.SetSize(1);
	spike_list_array_[0].Serialize(ar); // v2 to v5
	//m_spike_class.Serialize(ar);
}


void CSpikeDoc::serialize_acquisition_parameters(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << detection_date_;			// W2
		ar << comment_;				// W3
		ar << acquisition_file_name_;	// W4
		ar << acquisition_comment_;	// W5
		ar << acquisition_time_;		// W6
		ar << acquisition_rate_;		// W7
		ar << acquisition_size_;		// W8

		constexpr int n_items = 1; ar << n_items;
	}
	else
	{
		ar >> detection_date_;			// W2
		ar >> comment_;				// W3
		ar >> acquisition_file_name_;	// W4
		ar >> acquisition_comment_;	// W5
		ar >> acquisition_time_;		// W6
		ar >> acquisition_rate_;		// W7
		ar >> acquisition_size_;		// W8

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
		sort_stimulus_array();
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
		n_spike_arrays = spike_list_array_.GetSize();
		ar << n_spike_arrays;
	}
	else
	{
		ar >> n_spike_arrays;
		spike_list_array_.RemoveAll();
		spike_list_array_.SetSize(n_spike_arrays);
	}

	for (int i = 0; i < n_spike_arrays; i++)
	{
		SpikeList& spk_list = spike_list_array_.GetAt(i);
		spk_list.Serialize(ar); // v6-v7
	}
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
	sort_stimulus_array();

	// read stimulus and spike classes
	spike_list_array_.RemoveAll();
	spike_list_array_.SetSize(1);
	spike_list_array_[0].Serialize(ar); // v6
}

void CSpikeDoc::serialize_v7(CArchive& ar)
{
	serialize_acquisition_parameters(ar);
	serialize_stimulus_intervals(ar);
	serialize_spike_list_arrays(ar);
}

// CSpikeDoc commands
void CSpikeDoc::set_file_extension_as_spk(CString& file_name)
{
	const auto i = file_name.ReverseFind('.');
	if (i > 0)
		file_name = file_name.Left(i); 
	file_name += ".spk";	
}

BOOL CSpikeDoc::OnSaveDocument(const LPCTSTR psz_path_name)
{
	// check that path name has ".spk"
	CString file_name = psz_path_name;
	if (file_name.IsEmpty())
		return false;
	set_file_extension_as_spk(file_name);

	CFileStatus status;
	const auto b_flag_exists = CFile::GetStatus(file_name, status);

	if (b_flag_exists && (status.m_attribute & CFile::readOnly))
	{
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, file_name);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
		case IDYES:
			{
				// If so, either Save or Update, as appropriate DoSave
				const auto p_template = GetDocTemplate();
				ASSERT(p_template != NULL);
				if (!new_path_.IsEmpty())
				{
					const auto j = file_name.ReverseFind('\\') + 1;
					if (j != -1)
						file_name = file_name.Mid(j);
					file_name = new_path_ + file_name;
				}

				if (!AfxGetApp()->DoPromptFileName(file_name,
				                                   AFX_IDS_SAVEFILE,
				                                   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
					return FALSE;
				if (!AfxGetApp()->DoPromptFileName(file_name,
				                                   AFX_IDS_SAVEFILECOPY,
				                                   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, p_template))
					return FALSE;

				// don't even attempt to save
				const auto k = file_name.ReverseFind('\\') + 1; // find last occurence of anti-slash
				if (k != -1)
					new_path_ = file_name.Left(k);
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
	if (f.Open(file_name, CFile::modeCreate | CFile::modeWrite, &fe))
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
		ReportSaveLoadException(file_name, &fe, FALSE, AFX_IDP_FAILED_TO_SAVE_DOC);
		fe.Delete();
		return FALSE;
	}
	SetModifiedFlag(FALSE);
	return TRUE;
}

BOOL CSpikeDoc::OnOpenDocument(const LPCTSTR psz_path_name)
{
	clear_data();
	CFileStatus status;
	if (!CFile::GetStatus(psz_path_name, status))
		return FALSE;

	CFile f;
	CFileException fe;
	auto b_read = TRUE;
	const CString filename = psz_path_name;
	if (f.Open(filename, CFile::modeRead | CFile::shareDenyNone, &fe))
	{
		current_spike_list_index_ = 0;
		try
		{
			CArchive ar(&f, CArchive::load);
			Serialize(ar);
			ar.Flush();
			ar.Close();
			f.Close();
			SetModifiedFlag(FALSE);

			// update nb of classes
			for (auto i = 0; i < spike_list_array_.GetSize(); i++)
			{
				if (!spike_list_array_[i].is_class_list_valid()) 
				{
					spike_list_array_[i].update_class_list();
					SetModifiedFlag(); 
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

void CSpikeDoc::init_source_doc(const AcqDataDoc* p_document)
{
	// load parameters from file
	const auto wave_format = p_document->get_wave_format();
	acquisition_time_ = wave_format->acquisition_time;
	acquisition_size_ = p_document->get_doc_channel_length();
	acquisition_rate_ = wave_format->sampling_rate_per_channel;
	acquisition_comment_ = wave_format->cs_comment;
	m_wave_format.copy( wave_format);
}

CString CSpikeDoc::get_file_infos()
{
	TCHAR sz[64];
	const auto psz = sz;
	const auto separator_tab = _T("\t");
	const auto separator_rc = _T("\r\n");
	auto cs_out = GetPathName();
	cs_out.MakeUpper();
	cs_out += separator_tab + detection_date_.Format(_T("%c"));
	cs_out += separator_rc + comment_;
	cs_out += separator_rc;

	cs_out += _T("*** SOURCE DATA ***\r\n");
	cs_out += acquisition_file_name_ + separator_rc;
	cs_out += acquisition_comment_ + separator_rc;
	cs_out += acquisition_time_.Format(_T("%#d-%B-%Y")) + separator_rc;

	cs_out += _T("*** SPIKE LIST ***\r\n");

	long spike_count = 0;
	const auto spike_list = &spike_list_array_[current_spike_list_index_];
	if (spike_list->get_spikes_count() > 0)
		spike_count = spike_list->get_spikes_count();
	wsprintf(psz, _T("n spikes = %li\r\n"), spike_count);
	cs_out += sz;

	return cs_out;
}

// post stimulus histogram, ISI, autocorrelation
// options_view_spikes->export_data_type == 0
// each line represents 1 recording
// we start with a header, then each file is scanned and exported
// if n_intervals < 0: export all spikes // feb 23, 2009
void CSpikeDoc::export_spk_latencies(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, const int intervals_count, const int spike_list_index, const int class_index)
{
	CString cs_dummy;
	const auto spike_list = &spike_list_array_[spike_list_index];
	const auto spikes_count = spike_list->get_spikes_count();

	// export time of intervals_count first spikes
	auto n_spikes = intervals_count;
	if (n_spikes > spikes_count || n_spikes < 0)
		n_spikes = spikes_count;
	const auto rate = spike_list->get_acq_sampling_rate();

	for (auto j = 0; j < n_spikes; j++)
	{
		const Spike* spike = spike_list->get_spike(j);

		// skip classes not requested
		const auto cla = spike->get_class_id();
		if (cla < 0 /*&& !options_view_spikes->b_artefacts*/)
			continue;
		if (options_view_spikes->spike_class_option != 0 && cla != class_index)
			continue;

		// export time
		const auto t_value = static_cast<float>(spike->get_time()) / rate;
		cs_dummy.Format(_T("\t%lf"), t_value);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::export_spk_latencies(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, const int n_intervals, const CString& cs_file_comment)
{
	CString cs_dummy;
	// spike class: -1(one:selected); 0(all); 1(all:split)
	auto class0 = 0;
	auto class1 = 0;
	auto spike_list = &spike_list_array_[current_spike_list_index_];

	if ((options_view_spikes->spike_class_option == -1
			|| options_view_spikes->spike_class_option == 1)
		&& !spike_list->is_class_list_valid())
		spike_list->update_class_list();

	if (options_view_spikes->spike_class_option == -1)
	{
		// search item index with correct class ID
		auto i = spike_list->get_classes_count() - 1;
		for (auto j = i; j >= 0; j--, i--)
			if (spike_list->get_class_id(i) == options_view_spikes->class_nb)
				break;
		class0 = i;
		class1 = i;
	}
	else if (options_view_spikes->spike_class_option == 1)
		class1 = spike_list->get_classes_count() - 1;

	// loop header..................
	for (auto class_index = class0; class_index <= class1; class_index++)
	{
		// check if class is Ok
		const auto class_n_items = spike_list->get_class_descriptor_from_id(class_index)->get_class_n_items();
		if ((FALSE == options_view_spikes->b_export_zero) && (class_n_items == 0))
			continue;
		const auto class_id = spike_list->get_class_id(class_index);
		export_spk_file_comment(shared_file, options_view_spikes, class_id, cs_file_comment);
		export_spk_latencies(shared_file, options_view_spikes, n_intervals, current_spike_list_index_, class_id);
	}
}

void CSpikeDoc::export_spk_psth(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* sum0, const int spike_list_index, const int class_index)
{
	CString cs_dummy;
	const auto spike_list = &spike_list_array_[spike_list_index];
	auto n_bins = 1;

	switch (options_view_spikes->export_data_type)
	{
	case EXPORT_PSTH: // PSTH
		n_bins = options_view_spikes->n_bins;
		break;
	case EXPORT_ISI: // ISI
		n_bins = options_view_spikes->n_bins_isi;
		break;
	case EXPORT_AUTOCORR: // Autocorrelation
		n_bins = options_view_spikes->n_bins_isi;
		break;
	default:
		break;
	}

	// ................................DATA
	// spike list -- prepare parameters
	auto sum = sum0;
	long n;
	for (auto i = 0; i < n_bins; i++, sum++)
		*sum = 0;

	// export number of spikes / interval
	switch (options_view_spikes->export_data_type)
	{
	case EXPORT_PSTH: // PSTH
		build_psth(options_view_spikes, sum0, class_index);
		break;
	case EXPORT_ISI: // ISI
		n = build_isi(options_view_spikes, sum0, class_index);
		cs_dummy.Format(_T("\t%li"), n);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR: // Autocorrelation
		n = build_autocorrelation(options_view_spikes, sum0, class_index);
		cs_dummy.Format(_T("\t%li"), n);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	default:
		break;
	}

	// export nb spikes / bin: print nb
	for (auto i = 0; i < n_bins; i++)
	{
		if ((options_view_spikes->b_export_zero == TRUE) || (*(sum0 + i) >= 0))
			cs_dummy.Format(_T("\t%li"), *(sum0 + i));
		else
			cs_dummy = _T("\t");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}

	// export stimulus occurence time(s) that fit(s) into the time interval requested
	if (options_view_spikes->export_data_type == EXPORT_PSTH && m_stimulus_intervals.get_size() > 0)
	{
		auto sampling_rate = spike_list->get_acq_sampling_rate();
		if (sampling_rate == 0.f)
		{
			sampling_rate = acquisition_rate_;
		}
		ASSERT(sampling_rate != 0.f);
		constexpr auto stimulus_index0 = 0;
		const auto stimulus_index1 = m_stimulus_intervals.get_size() - 1;
		auto ii_offset0 = 0;
		if (!options_view_spikes->b_absolute_time)
			ii_offset0 = m_stimulus_intervals.get_at(options_view_spikes->i_stimulus_index);
		const auto ii_start = static_cast<long>(options_view_spikes->time_start * sampling_rate) + ii_offset0;
		const auto ii_end = static_cast<long>(options_view_spikes->time_end * sampling_rate) + ii_offset0;
		auto b_up = -1;

		for (auto stimulus_index = stimulus_index0; stimulus_index <= stimulus_index1; stimulus_index++)
		{
			const int ii_stimulus = m_stimulus_intervals.get_at(stimulus_index);
			b_up *= -1;
			if (ii_stimulus < ii_start)
				continue;
			if (ii_stimulus > ii_end)
				break;

			const auto t_value = (static_cast<float>(ii_stimulus) - ii_offset0) / sampling_rate;
			if (b_up < 0)
				cs_dummy.Format(_T("\t(-)%lf"), t_value);
			else
				cs_dummy.Format(_T("\t(+)%lf"), t_value);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::export_spk_psth(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* pl_sum0, const CString&
                              cs_file_comment)
{
	CString cs_dummy;
	SpikeList* spike_list = &spike_list_array_[current_spike_list_index_];

	// spike class: -1(one:selected); 0(all); 1(all:split)
	int class0 = 0;
	int class1 = 0;
	if ((options_view_spikes->spike_class_option == -1 // 1 class selected
			|| options_view_spikes->spike_class_option == 1) // all classes selected but displayed on different lines
		&& !spike_list->is_class_list_valid())
		spike_list->update_class_list();

	if (options_view_spikes->spike_class_option == -1) // only 1 class selected
	{
		// search item index with correct class ID
		auto i = spike_list->get_classes_count() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (spike_list->get_class_id(i) == options_view_spikes->class_nb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
		{
			export_spk_file_comment(shared_file, options_view_spikes, options_view_spikes->class_nb, cs_file_comment);
			// dummy export if requested export nb spikes / bin: print nb
			switch (options_view_spikes->export_data_type)
			{
			case EXPORT_ISI: 
			case EXPORT_AUTOCORR: 
				cs_dummy.Format(_T("\t0"));
				shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
				break;
			default:
				break;
			}
			return;
		}
		class0 = i;
		class1 = i;
	}
	else if (options_view_spikes->spike_class_option == 1)
		class1 = spike_list->get_classes_count() - 1;

	// loop header..................
	for (int class_index = class0; class_index <= class1; class_index++)
	{
		// check if class is Ok
		const auto class_n_items = spike_list->get_class_descriptor_from_id(class_index)->get_class_n_items();
		const auto class_id = spike_list->get_class_id(class_index);
		// export the comments
		export_spk_file_comment(shared_file, options_view_spikes, class_id, cs_file_comment);
		// test if we should continue
		if (!options_view_spikes->b_export_zero && (class_n_items == 0))
			export_spk_psth(shared_file, options_view_spikes, pl_sum0, current_spike_list_index_, class_id);

		cs_dummy = _T("\r\n");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

// export histograms of the amplitude of the spikes found in each file of the file series
// export type = 4
// same tb as PSTH: each line represents one record, 1 class
void CSpikeDoc::export_spk_amplitude_histogram(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* p_hist0, const int spike_list_index,
                                               const int class_index)
{
	// ................................DATA
	auto spike_list = &spike_list_array_[spike_list_index];
	CString cs_dummy;
	// temp parameters for amplitudes histogram
	const auto histogram_amplitude_min = static_cast<int>(options_view_spikes->hist_ampl_v_min / spike_list->get_acq_volts_per_bin());
	const auto histogram_amplitude_max = static_cast<int>(options_view_spikes->hist_ampl_v_max / spike_list->get_acq_volts_per_bin());
	const auto histogram_amplitude_bin_size = (histogram_amplitude_max - histogram_amplitude_min) / options_view_spikes->hist_ampl_n_bins;
	constexpr auto n_bins = 1;
	// update offset
	auto ii_offset0 = 0;
	if (!options_view_spikes->b_absolute_time && m_stimulus_intervals.n_items > 0)
		ii_offset0 = m_stimulus_intervals.get_at(options_view_spikes->i_stimulus_index);

	// clear histogram area
	ASSERT(options_view_spikes->export_data_type == EXPORT_HISTAMPL);
	auto p_hist = p_hist0;
	for (auto k = 0; k < options_view_spikes->hist_ampl_n_bins + 2; k++, p_hist++)
		*p_hist = 0;

	long y_sum = 0;
	long y_sum2 = 0;
	long y_n = 0;

	const auto t_span = options_view_spikes->time_end - options_view_spikes->time_start;
	const auto t_bin = t_span / n_bins;
	auto t_first = options_view_spikes->time_start;
	auto rate = spike_list->get_acq_sampling_rate();
	if (rate == 0.0)
		rate = 1.0f;
	auto ii_time1 = static_cast<long>(t_first * rate);

	for (auto i = 0; i < n_bins; i++)
	{
		t_first = options_view_spikes->time_start + t_bin * i;
		const auto ii_time0 = ii_time1;
		ii_time1 = static_cast<long>((t_first + t_bin) * rate);

		// export number of spikes / interval
		// ------- count total spk/interval (given b_spk_class & b_artefacts)
	
		for (auto j = 0; j < spike_list->get_spikes_count(); j++)
		{
			const Spike* spike = spike_list->get_spike(j);

			// skip intervals not requested
			const auto ii_time = spike->get_time() - ii_offset0;
			if ((ii_time < ii_time0) || (ii_time >= ii_time1))
				break;

			// skip classes not requested
			const auto cla = spike->get_class_id();
			if (cla < 0 /*&& !options_view_spikes->b_artefacts*/)
				continue;
			if (options_view_spikes->spike_class_option != 0 && cla != class_index)
				continue;

			// get value, compute statistics
			int max, min;
			spike->get_max_min(&max, &min);
			auto val = max - min;
			y_sum += val;
			y_sum2 += val * val;
			y_n++;
			// build histogram: compute index and increment bin content
			int index;
			if (val >= histogram_amplitude_min && val <= histogram_amplitude_max)
				index = (val - histogram_amplitude_min) / histogram_amplitude_bin_size + 1;
			else
				index = options_view_spikes->hist_ampl_n_bins + 1;
			val = *(p_hist0 + index) + 1;
			*(p_hist0 + index) = val;
		}

		// 4) export histogram
		if (y_n > 0)
		{
			const auto volts_per_bin = spike_list->get_acq_volts_per_bin() * 1000.f;
			cs_dummy.Format(_T("\t%.3lf\t%.3lf\t%i"), (static_cast<double>(y_sum) / y_n) * volts_per_bin,
			                static_cast<double>(y_sum2) * volts_per_bin * volts_per_bin, y_n);
		}
		else
			cs_dummy = _T("\t\t\t0");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		// actual measures
		p_hist = p_hist0;
		for (auto k = 0; k < options_view_spikes->hist_ampl_n_bins + 2; k++, p_hist++)
		{
			if ((options_view_spikes->b_export_zero == TRUE) || (*p_hist >= 0))
				cs_dummy.Format(_T("\t%i"), *p_hist);
			else
				cs_dummy = _T("\t");
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
}

void CSpikeDoc::export_spk_amplitude_histogram(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, long* p_hist, const CString&
                                         cs_file_comment)
{
	const auto spike_list = &spike_list_array_[current_spike_list_index_];

	// spike class: -1(one:selected); 0(all); 1(all:split)
	auto class0 = 0;
	auto class1 = 0;
	if ((options_view_spikes->spike_class_option == -1
			|| options_view_spikes->spike_class_option == 1)
		&& !spike_list->is_class_list_valid())
		spike_list->update_class_list();

	// just one class selected: see if there is at least 1 spike with this class
	if (options_view_spikes->spike_class_option == -1)
	{
		// search item index with correct class ID
		auto i = spike_list->get_classes_count() - 1;
		for (auto j = i; j >= 0; i--, j--)
			if (spike_list->get_class_id(i) == options_view_spikes->class_nb)
				break;
		// if none is found, export descriptor and exit
		if (i < 0)
			return export_spk_file_comment(shared_file, options_view_spikes, options_view_spikes->class_nb, cs_file_comment);
		// set class boundaries to the index of the class having the proper ID
		class0 = i;
		class1 = i;
	}
	else if (options_view_spikes->spike_class_option == 1)
		class1 = spike_list->get_classes_count() - 1;

	// loop over all classes..................
	for (auto class_index = class0; class_index <= class1; class_index++)
	{
		const auto class_id = spike_list->get_class_id(class_index);
		export_spk_file_comment(shared_file, options_view_spikes, class_id, cs_file_comment);
		export_spk_amplitude_histogram(shared_file, options_view_spikes, p_hist, current_spike_list_index_, class_id);
	}
}

// option 1: spike intervals
// option 2: spike extrema (max and min)
// option 3: export spike amplitudes (max - min)
// one file at a time, organized in columns

void CSpikeDoc::export_spk_attributes_one_file(CSharedFile* shared_file, const options_view_spikes* options_view_spikes)
{
	int ii_offset0 = 0;
	if (!options_view_spikes->b_absolute_time && m_stimulus_intervals.n_items > 0)
	{
		ii_offset0 = m_stimulus_intervals.get_at(options_view_spikes->i_stimulus_index);
	}

	// ................................DATA
	const auto spike_list = &spike_list_array_[current_spike_list_index_];
	CString cs_dummy = _T("\r\ntime(s)\tclass");
	auto rate = spike_list->get_acq_sampling_rate();
	if (rate == 0.0)
		rate = 1.0f;

	switch (options_view_spikes->export_data_type)
	{
	case EXPORT_EXTREMA:
		cs_dummy += _T("\tmax(mV)\tmin(mV)");
		break;
	case EXPORT_AMPLIT:
		{
			cs_dummy += _T("\tamplitude(mV)\ttime(ms)");
			const auto rate_ms = rate / 1000.f;
			const auto x_first = static_cast<float>(spike_list->shape_t1) / rate_ms;
			const auto x_last = static_cast<float>(spike_list->shape_t2) / rate_ms;
			CString cs;
			cs.Format(_T(" [%.3f to %.3f]"), x_first, x_last);
			cs_dummy += cs;
		}
		break;
	default: ;
	}
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike list -- prepare parameters
	const auto t_span = options_view_spikes->time_end - options_view_spikes->time_start;
	auto t_first = options_view_spikes->time_start;
	auto ii_time1 = static_cast<long>(t_first * rate);

	t_first = options_view_spikes->time_start + t_span;
	const auto ii_time0 = ii_time1;
	ii_time1 = static_cast<long>((t_first + t_span) * rate);
	const auto volts_per_bin = spike_list->get_acq_volts_per_bin() * 1000.f;
	const auto bin_zero = spike_list->get_acq_bin_zero();
	int i_max, i_min;
	const auto i_first = spike_list->shape_t1;
	const auto i_last = spike_list->shape_t2;
	const auto rate_ms = rate / 1000.f;

	// export data: loop through all spikes
	for (auto j = 0; j < spike_list->get_spikes_count(); j++)
	{
		Spike* pSpike = spike_list->get_spike(j);

		// skip intervals not requested
		const auto ii_time = pSpike->get_time() - ii_offset0;
		if ((ii_time < ii_time0) || (ii_time >= ii_time1))
			break;
		// skip classes not requested
		const auto cla = pSpike->get_class_id();
		if (cla < 0 /*&& !options_view_spikes->b_artefacts*/)
			continue;

		// export time and spike time
		const auto t_val = static_cast<float>(ii_time) / rate;
		cs_dummy.Format(_T("\r\n%lf\t%i"), t_val, cla);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		int value_max, value_min;

		switch (options_view_spikes->export_data_type)
		{
		// 2) export time interval (absolute time)
		case EXPORT_INTERV:
			break;

		// 3) export extrema
		case EXPORT_EXTREMA:
			pSpike->get_max_min(&value_max, &value_min);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(value_max) - bin_zero) * volts_per_bin,
			                (static_cast<double>(value_min) - bin_zero) * volts_per_bin);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;

		// 4) export max - min
		case EXPORT_AMPLIT:
			pSpike->measure_max_then_min_ex(&value_max, &i_max, &value_min, &i_min, i_first, i_last);
			cs_dummy.Format(_T("\t%.3lf\t%.3lf"), (static_cast<double>(value_max) - value_min) * volts_per_bin,
			                (static_cast<double>(i_min) - i_max) / rate_ms);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			break;

		case EXPORT_SPIKEPOINTS:
			for (auto index = 0; index < spike_list->get_spike_length(); index++)
			{
				const auto val = pSpike->get_value_at_offset(index);
				cs_dummy.Format(_T("\t%.3lf"), static_cast<double>(val) * volts_per_bin);
				shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			}
			break;
		default: ;
		}
	}
}

// Describe the type of data exported
void CSpikeDoc::export_table_title(CSharedFile* shared_file, options_view_spikes* options_view_spikes, int n_files)
{
	CString cs_dummy;

	switch (options_view_spikes->export_data_type)
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
		n_files = 1;
		break;
	case EXPORT_EXTREMA: cs_dummy = _T("Spike amplitude extrema");
		n_files = 1;
		break;
	case EXPORT_AMPLIT: cs_dummy = _T("Spike amplitudes");
		n_files = 1;
		break;
	case EXPORT_HISTAMPL: cs_dummy = _T("Spike amplitude histograms");
		break;
	case EXPORT_SPIKEPOINTS: cs_dummy = _T("Spike data points");
		break;
	default: options_view_spikes->export_data_type = 0;
		cs_dummy = _T("Histogram nb spikes/bin");
		break;
	}
	const auto t = CTime::GetCurrentTime(); // current date & time
	CString cs_date; // first string to receive date and time
	cs_date.Format(_T("  created on %d-%d-%d at %d:%d"), t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(),
	               t.GetMinute());
	cs_dummy += cs_date;
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	switch (options_view_spikes->export_data_type)
	{
	case EXPORT_PSTH:
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
	case EXPORT_EXTREMA:
		cs_dummy.Format(
			_T("\r\nnbins= %i\tbin size (s):\t%.3f\tt start (s):\t%.3f\tt end (s):\t%.3f\r\nn files = %i\r\n"),
			options_view_spikes->n_bins,
			(options_view_spikes->time_end - options_view_spikes->time_start) / options_view_spikes->n_bins,
			options_view_spikes->time_start,
			options_view_spikes->time_end,
			n_files);
		break;
	case EXPORT_AVERAGE:
		cs_dummy.Format(_T("\r\ndata:\tN\txi=mean amplitude (mV)\tSum square of amplitudes\r\nn points:\t%i\r\n"),
		                get_spike_list_current()->get_spike_length());
		break;
	case EXPORT_HISTAMPL:
	case EXPORT_LATENCY:
	default:
		cs_dummy.Format(_T("\r\n\r\n\r\n"));
		break;
	}
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

// Row of column headers for the database parameters
// returns number of columns used
void CSpikeDoc::export_headers_descriptors(CSharedFile* shared_file, options_view_spikes* options_view_spikes)
{
	CString cs_dummy;

	// these infos are stored in the database - what we do here, is equivalent of doing a query
	cs_dummy.Format(_T("\r\nID\tFilename"));
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	auto n_columns = 2;
	cs_dummy.Empty();
	if (options_view_spikes->b_acq_date)
	{
		// ................     acq date, acq time
		cs_dummy += _T("\tacq_date\tacq_time");
		n_columns += 2;
	}
	if (options_view_spikes->b_acq_comments)
	{
		// ................     acq comments, stimulus, concentration, type, etc
		cs_dummy += _T("\tExpt\tinsectID\tssID\tinsect\tstrain\tsex\tlocation\toperator\tmore");
		n_columns += 9;
		cs_dummy += _T("\tstim1\tconc1\trepeat1\tstim2\tconc2\trepeat2\ttype\tflag");
		n_columns += 8;
	}

	// this is specific to the spike file (not stored in the database)
	if (options_view_spikes->b_spk_comments)
	{
		// ................     spike comments
		cs_dummy += _T("\tspk_comment");
		n_columns += 1;
	}
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// spike detection chan
	if (options_view_spikes->b_total_spikes)
	{
		// ................     total spikes
		cs_dummy = _T("\tspk_threshold\ttotal_spikes\tnb_classes\tduration(s)");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		n_columns += 4;
	}

	// export spike detect chan, channel and class
	cs_dummy = _T("\tdetectchan\tchan\tclass");
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	n_columns += 3;
	// class descriptor
	cs_dummy = _T("\tdescriptor");
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	n_columns += 1;

	options_view_spikes->n_comment_columns = n_columns;
}

// Row (continuation) of column headers for the measures
void CSpikeDoc::export_headers_data(CSharedFile* shared_file, const options_view_spikes* options_view_spikes)
{
	CString cs_dummy;
	// header of the data
	auto n_bins = 0;
	float t_span;
	auto t_bin = 1.f;
	auto t_start = options_view_spikes->time_start;
	switch (options_view_spikes->export_data_type)
	{
	case EXPORT_PSTH: 
		n_bins = options_view_spikes->n_bins;
		t_span = options_view_spikes->time_end - options_view_spikes->time_start;
		t_bin = t_span /static_cast<float>(n_bins);
		break;
	case EXPORT_ISI:
		n_bins = options_view_spikes->n_bins_isi;
		t_bin = options_view_spikes->bin_isi;
		t_start = 0;
		cs_dummy = _T("\tN");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AUTOCORR:
		n_bins = options_view_spikes->n_bins_isi;
		t_span = options_view_spikes->bin_isi *static_cast<float>(n_bins);
		t_bin = options_view_spikes->bin_isi;
		t_start = -t_span / 2.f;
		cs_dummy = _T("\tN");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_HISTAMPL:
		n_bins = options_view_spikes->hist_ampl_n_bins;
		t_start = options_view_spikes->hist_ampl_v_min;
		t_bin = (options_view_spikes->hist_ampl_v_max - options_view_spikes->hist_ampl_v_min) * 1000.f / n_bins;
		n_bins += 2;
		t_start -= t_bin;
		cs_dummy = _T("\tmean\tsum2\tNelmts");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		break;
	case EXPORT_AVERAGE:
		{
			const auto n_points = get_spike_list_current()->get_spike_length();
			cs_dummy = _T("\tN");
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			CString cs;
			cs_dummy.Empty();
			for (auto i = 0; i < n_points; i++)
			{
				cs.Format(_T("\tx%i"), i);
				cs_dummy += cs;
			}
			for (auto i = 0; i < n_points; i++)
			{
				cs.Format(_T("\tSx2_%i"), i);
				cs_dummy += cs;
			}
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
		break;

	default:
		break;
	}

	// ................     loop to scan all time intervals
	cs_dummy.Empty();
	if (options_view_spikes->export_data_type == EXPORT_LATENCY)
	{
		n_bins = 10;
		for (auto i = 0; i < n_bins; i++)
		{
			CString cs;
			cs.Format(_T("\t%i"), i);
			cs_dummy += cs;
		}
	}
	else
	{
		for (auto i = 0; i < n_bins; i++)
		{
			CString cs;
			cs.Format(_T("\tb_%.3f"), static_cast<double>(t_start) + static_cast<double>(t_bin) * i);
			cs_dummy += cs;
		}
	}
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

// output descriptors of each record
// TODO: output descriptors from the database and not from the spike files ??
void CSpikeDoc::export_spk_file_comment(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, const int class_index,
                                     const CString& cs_file_comment)
{
	CString cs_dummy;
	const CString cs_tab = _T("\t");

	// spike file
	if (cs_file_comment.IsEmpty())
	{
		cs_dummy = GetPathName();
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		// source data file items
		if (options_view_spikes->b_acq_date) // source data time and date
		{
			cs_dummy.Format(_T("\t%s"), (LPCTSTR)acquisition_time_.Format(_T("%#d %m %Y\t%X")));
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
		// source data comments
		if (options_view_spikes->b_acq_comments)
		{
			CString cs_temp;
			cs_dummy = cs_tab + acquisition_comment_;
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
			cs_dummy.Format(_T("\t%i"), m_wave_format.insect_id);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy.Format(_T("\t%i"), m_wave_format.sensillum_id);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

			cs_dummy = cs_tab + m_wave_format.cs_insect_name;
			cs_dummy += cs_tab + m_wave_format.cs_strain;
			cs_dummy += cs_tab + m_wave_format.cs_sex;
			cs_dummy += cs_tab + m_wave_format.cs_location;
			cs_dummy += cs_tab + m_wave_format.cs_operator;
			cs_dummy += cs_tab + m_wave_format.cs_more_comment;

			cs_dummy += cs_tab + m_wave_format.cs_stimulus;
			cs_dummy += cs_tab + m_wave_format.cs_concentration;
			cs_temp.Format(_T("%i"), m_wave_format.repeat);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wave_format.cs_stimulus2;
			cs_dummy += cs_tab + m_wave_format.cs_concentration2;
			cs_temp.Format(_T("%i"), m_wave_format.repeat2);
			cs_dummy += cs_tab + cs_temp;
			cs_dummy += cs_tab + m_wave_format.cs_sensillum;
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		shared_file->Write(cs_file_comment, cs_file_comment.GetLength() * sizeof(TCHAR));
	}

	// spike file additional comments
	if (options_view_spikes->b_spk_comments)
	{
		shared_file->Write(cs_tab, cs_tab.GetLength() * sizeof(TCHAR));
		shared_file->Write(comment_, comment_.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	const auto p_spk_list = &spike_list_array_[current_spike_list_index_];

	if (options_view_spikes->b_total_spikes)
	{
		cs_dummy.Format(_T("\t%f"), p_spk_list->get_detection_parameters()->detect_threshold_mv);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), p_spk_list->get_spikes_count());
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("\t%i"), p_spk_list->get_classes_count());
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		const auto t_duration = static_cast<float>(acquisition_size_) / acquisition_rate_;
		cs_dummy.Format(_T("\t%f"), t_duration);
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
	// spike list item, spike class
	cs_dummy.Format(_T("\t%i \t%s \t%i"), options_view_spikes->i_chan, (LPCTSTR)p_spk_list->get_detection_parameters()->comment, class_index);
	shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	// spike class descriptor
	const CString cs = _T("\t") + p_spk_list->get_class_description_from_id(class_index);
	shared_file->Write(cs, cs.GetLength() * sizeof(TCHAR));

}

// compute post_stimulus histogram
// in:
//	OPTION_VIEW_SPIKES
//		n_bins		number of bins
//		time_start	first valid spike occurence time (s)
//		time_end		last valid spike occurence (s)
//		spike_class_option	0=all spike class, -1:only one, 1=exclude artefacts
//		class_nb		if !spike_class_option, compute only from spikes with this class nb
//		b_absolute_time	false=correct time with first stimulus time
//		time_bin		bin duration (in s)
//	long* plSum0 array of longs to receive histogram bins
//	i_class		class selected
// out:
//	*plSum0
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::build_psth(const options_view_spikes* options_view_spikes, long* pl_sum0, const int class_index)
{
	// adjust parameters
	long n = 0;
	const auto spike_list = &spike_list_array_[current_spike_list_index_];

	const auto n_spikes = spike_list->get_spikes_count();
	if (n_spikes <= 0)
		return 0;
	const auto rate = spike_list->get_acq_sampling_rate();
	ASSERT(rate != 0.f);

	// check validity of stimulus_index
	auto stimulus_index = options_view_spikes->i_stimulus_index;
	if (stimulus_index > m_stimulus_intervals.get_size() - 1)
		stimulus_index = m_stimulus_intervals.get_size() - 1;
	if (stimulus_index < 0)
		stimulus_index = 0;
	const auto stimulus_index_0 = stimulus_index;
	auto stimulus_index_1 = stimulus_index_0 + 1;
	auto increment = 2;
	if (options_view_spikes->b_cycle_hist && !options_view_spikes->b_absolute_time)
	{
		stimulus_index_1 = m_stimulus_intervals.get_size();
		increment = options_view_spikes->n_stimuli_per_cycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto stimulus_index_i = stimulus_index_0; stimulus_index_i < stimulus_index_1; stimulus_index_i += increment, n++)
	{
		auto ii_offset_0 = 0;
		if (!options_view_spikes->b_absolute_time)
		{
			if (m_stimulus_intervals.n_items > 0)
				ii_offset_0 = m_stimulus_intervals.get_at(stimulus_index_i);
			else
				ii_offset_0 = static_cast<long>(-(options_view_spikes->time_start * rate));
		}
		const auto ii_time_start = static_cast<long>(options_view_spikes->time_start * rate) + ii_offset_0;
		const auto ii_time_end = static_cast<long>(options_view_spikes->time_end * rate) + ii_offset_0;
		auto ii_bin_size = static_cast<long>(options_view_spikes->time_bin * rate);
		if (ii_bin_size <= 0)
			ii_bin_size = 1;
		ASSERT(ii_bin_size > 0);

		// check file size and position pointer at the first spike within the bin
		auto i0 = 0;
		while ((i0 < n_spikes) && (spike_list->get_spike(i0)->get_time() < ii_time_start))
			i0++;

		for (auto j = i0; j < n_spikes; j++)
		{
			const Spike* spike = spike_list->get_spike(j);

			// skip intervals not requested
			const int ii_time = spike->get_time();
			if (ii_time >= ii_time_end)
				break;

			// skip classes not requested (artefact or wrong class)
			if (spike->get_class_id() < 0 /*&& !options_view_spikes->b_artefacts*/)
				continue;
			if (options_view_spikes->spike_class_option != 0 && spike->get_class_id() != class_index)
				continue;

			int ii = (ii_time - ii_time_start) / ii_bin_size;
			if (ii >= options_view_spikes->n_bins)
				ii = options_view_spikes->n_bins;
			(*(pl_sum0 + ii))++;
		}
	}
	return n;
}

// compute inter_spike intervals histogram
// in:
//	OPTION_VIEW_SPIKES
//		n_bins_ISI	number of bins
//		time_start	first valid spike occurence time (s)
//		time_end		last valid spike occurence (s)
//		spike_class_option	0=all spike class, -1:only one, 1=exclude artefacts
//		class_nb		if !spike_class_option, compute only from spikes with this class nb
//		b_absolute_time	false=correct time with first stimulus time
//		binISI		bin duration (in s)

//	long* plSum0 array of longs to receive histogram bins
//	i_class		class selected
// out:
//	*plSum0
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::build_isi(const options_view_spikes* options_view_spikes, long* pl_sum0, const int class_index)
{
	const auto spike_list = &spike_list_array_[current_spike_list_index_];

	const auto sampling_rate = spike_list->get_acq_sampling_rate(); // sampling rate
	long n = 0;
	ASSERT(sampling_rate != 0.0f); // converting ii_time into secs and back needs <> 0!
	const auto spike_count = spike_list->get_spikes_count(); // this is total nb of spikes within file
	const auto bin_size = options_view_spikes->bin_isi; // bin size (in secs)
	const auto ii_bin_size = static_cast<long>(bin_size * sampling_rate);
	ASSERT(ii_bin_size > 0);

	const auto stimulus_index_0 = options_view_spikes->i_stimulus_index;
	auto stimulus_index_1 = stimulus_index_0 + 1;
	auto increment = 2;
	if (options_view_spikes->b_cycle_hist && !options_view_spikes->b_absolute_time)
	{
		stimulus_index_1 = m_stimulus_intervals.get_size();
		increment = options_view_spikes->n_stimuli_per_cycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto stimulus_index = stimulus_index_0; stimulus_index < stimulus_index_1; stimulus_index += increment, n++)
	{
		auto ii_start = static_cast<long>(options_view_spikes->time_start * sampling_rate); 
		auto ii_end = static_cast<long>(options_view_spikes->time_end * sampling_rate);
		if (!options_view_spikes->b_absolute_time && m_stimulus_intervals.n_items > 0) 
		{
			ii_start += m_stimulus_intervals.get_at(stimulus_index);
			ii_end += m_stimulus_intervals.get_at(stimulus_index);
		}

		// find first spike within interval requested
		// assume all spikes are ordered with respect to their occurence time
		long ii_time0 = 0;
		int j;
		for (j = 0; j < spike_count; j++)
		{
			const Spike* spike = spike_list->get_spike(j);
			ii_time0 = spike->get_time();
			if (ii_time0 <= ii_end && ii_time0 >= ii_start)
				break; 
		}
		if (spike_count <= 0 || j >= spike_count)
			return 0;

		// build histogram
		for (auto i = j + 1; i < spike_count; i++)
		{
			Spike* pSpike = spike_list->get_spike(i);
			const auto ii_time = pSpike->get_time();
			if (ii_time > ii_end)
				break;
			ASSERT(ii_time > ii_time0);
			if (ii_time < ii_time0)
			{
				ii_time0 = ii_time;
				continue;
			}
			if (options_view_spikes->spike_class_option && pSpike->get_class_id() != class_index)
				continue;
			const auto ii = static_cast<int>((ii_time - ii_time0) / ii_bin_size);
			if (ii <= options_view_spikes->n_bins_isi)
			{
				(*(pl_sum0 + ii))++;
				n++;
			}
			ii_time0 = ii_time;
		}
	}
	return n;
}

// compute autocorrelation
// in:
//		OPTION_VIEW_SPIKES
//		n_bins_ISI	number of bins
//		time_start	first valid spike occurence time (s)
//		time_end		last valid spike occurence (s)
//		spike_class_option	0=all spike class, -1:only one, 1=exclude artefacts
//		class_nb		if !spike_class_option, compute only from spikes with this class nb
//		b_absolute_time	false=correct time with first stimulus time
//		binISI		bin duration (in s)
//		long* plSum0 array of longs to receive histogram bins
//		i_class		class selected
// out:
//		*plSum0
//		number of spikes used as time ref for autocorrelation
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::build_autocorrelation(const options_view_spikes* options_view_spikes, long* sum0, const int class_index)
{
	long n = 0; // number of pivot spikes used to build autocorrelation
	const auto spike_list = &spike_list_array_[current_spike_list_index_];
	const auto spikes_count = spike_list->get_spikes_count();
	if (spikes_count <= 0) 
		return n;

	const auto sampling_rate = spike_list->get_acq_sampling_rate();
	ASSERT(sampling_rate != 0.f);
	const auto first_stimulus_index = options_view_spikes->i_stimulus_index;
	auto next_stimulus_index = first_stimulus_index + 1;
	auto increment = 2;
	if (options_view_spikes->b_cycle_hist && !options_view_spikes->b_absolute_time)
	{
		next_stimulus_index = m_stimulus_intervals.get_size();
		increment = options_view_spikes->n_stimuli_per_cycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto stimulus_index = first_stimulus_index; stimulus_index < next_stimulus_index; stimulus_index += increment, n++)
	{
		long initial_offset = 0; 
		if (!options_view_spikes->b_absolute_time)
		{
			if (m_stimulus_intervals.n_items > 0)
				initial_offset = m_stimulus_intervals.get_at(stimulus_index);
			else
				initial_offset = static_cast<long>(-(options_view_spikes->time_start * sampling_rate));
		}
		// adjust time limits to cope with stimulus locking
		const auto ii_start = static_cast<long>(options_view_spikes->time_start * sampling_rate) + initial_offset;
		const auto ii_end = static_cast<long>(options_view_spikes->time_end * sampling_rate) + initial_offset;

		// histogram parameters
		const auto x = options_view_spikes->bin_isi * options_view_spikes->n_bins_isi * sampling_rate;
		auto ii_span = static_cast<long>(x);
		if (x - static_cast<float>(ii_span) > 0.5f)
			ii_span++;
		const auto ii_bin_size = ii_span / options_view_spikes->n_bins_isi;
		ASSERT(ii_bin_size > 0);
		ii_span /= 2;

		// search with the first spike falling into that window
		auto i0 = 0;
		while ((i0 < spikes_count) && (spike_list->get_spike(i0)->get_time() < ii_start))
			i0++;

		// build histogram external loop search 'pivot spikes'
		for (auto i = i0; i < spikes_count; i++)
		{
			const auto ii_time0 = spike_list->get_spike(i)->get_time(); 
			if (ii_time0 > ii_end) 
				break;

			if (options_view_spikes->spike_class_option && spike_list->get_spike(i)->get_class_id() != class_index)
				continue;

			n++; // update nb of pivot spikes
			// search backwards first spike that is ok
			auto i1 = i;
			const int i_first = ii_time0 - ii_span;
			while (i1 > 0 && spike_list->get_spike(i1)->get_time() > i_first)
				i1--;

			// internal loop: build autocorrelation
			for (auto j = i1; j < spikes_count; j++)
			{
				const Spike* spike = spike_list->get_spike(j);

				if (j == i) // discard spikes w. same time
					continue;
				const auto ii_time = spike->get_time() - ii_time0;
				if (abs(ii_time) >= ii_span)
					continue;
				if (options_view_spikes->spike_class_option && spike->get_class_id() != class_index)
					continue;

				const int t_ii_time = ((ii_time + ii_span) / ii_bin_size);
				if (t_ii_time >= options_view_spikes->n_bins_isi)
					continue;
				ASSERT(t_ii_time >= 0);
				ASSERT(t_ii_time < options_view_spikes->n_bins_isi);
				(*(sum0 + t_ii_time))++;
			}
		}
	}
	return n;
}

// compute peri-stimulus-histogram & autocorrelation
// in:
//		OPTION_VIEW_SPIKES
//		time_bin		PSTH bin duration (s)
//		n_bins_ISI	number of bins
//		time_start	first valid spike occurence time (s)
//		time_end		last valid spike occurence (s)
//		spike_class_option	0=all spike class, -1:only one, 1=exclude artefacts
//		class_nb		if !spike_class_option, compute only from spikes with this class nb
//		b_absolute_time	false=correct time with first stimulus time
//		binISI		bin duration (in s)
//		long* plSum0 array of longs to receive histogram bins
//		i_class		class selected
// out:
//	*plSum0  structure:  [row=n_bins_ISI; col=n_bins]
//	number of spikes used as time ref for autocorrelation
// ATTENTION! no bounds checking performed on plSum0

long CSpikeDoc::build_psth_autocorrelation(const options_view_spikes* options_view_spikes, long* sum0, const int class_index)
{
	long n_pivot_spikes = 0; // number of 'pivot spikes'
	const auto spike_list = &spike_list_array_[current_spike_list_index_];

	const auto spike_count = spike_list->get_spikes_count();
	if (spike_count <= 0)
		return n_pivot_spikes;

	// lock PSTH to stimulus if requested
	const auto sampling_rate = spike_list->get_acq_sampling_rate();
	ASSERT(sampling_rate != 0.f);

	const auto stimulus_index_0 = options_view_spikes->i_stimulus_index;
	auto stimulus_index_1 = stimulus_index_0 + 1;
	auto increment = 2;
	if (options_view_spikes->b_cycle_hist && !options_view_spikes->b_absolute_time)
	{
		stimulus_index_1 = m_stimulus_intervals.get_size();
		increment = options_view_spikes->n_stimuli_per_cycle;
		if (m_stimulus_intervals.n_per_cycle > 1 && increment > m_stimulus_intervals.n_per_cycle)
			increment = m_stimulus_intervals.n_per_cycle;
		increment *= 2;
	}

	for (auto stimulus_index = stimulus_index_0; stimulus_index < stimulus_index_1; stimulus_index += increment, n_pivot_spikes++)
	{
		auto ii_offset_0 = 0;
		if (!options_view_spikes->b_absolute_time)
		{
			if (m_stimulus_intervals.n_items > 0)
				ii_offset_0 = m_stimulus_intervals.get_at(stimulus_index);
			else
				ii_offset_0 = static_cast<long>(-(options_view_spikes->time_start * sampling_rate));
		}
		const auto ii_start = static_cast<long>(options_view_spikes->time_start * sampling_rate) + ii_offset_0;
		const auto ii_end = static_cast<long>(options_view_spikes->time_end * sampling_rate) + ii_offset_0;
		const auto ii_length = ii_end - ii_start;

		// histogram parameters
		const auto x = options_view_spikes->bin_isi * options_view_spikes->n_bins_isi * sampling_rate;
		auto ii_span = static_cast<long>(x);
		if (x - static_cast<float>(ii_span) > 0.5f)
			ii_span++;
		const auto ii_autocorrelation_bin_size = ii_span / options_view_spikes->n_bins_isi;
		ii_span /= 2;
		const auto ii_dummy = options_view_spikes->n_bins * options_view_spikes->n_bins_isi;

		// build histogram
		auto i0 = 0; 
		while (spike_list->get_spike(i0)->get_time() < ii_start)
		{
			i0++; // loop until found
			if (i0 >= spike_count)
				return 0L;
		}

		// external loop: pivot spikes
		for (auto i = i0; i < spike_count; i++) // loop over spikes A
		{
			const Spike* spike = spike_list->get_spike(i);

			// find an appropriate spike
			const auto ii_time_0 = spike->get_time();
			if (ii_time_0 >= ii_end) 
				break;
			if (options_view_spikes->spike_class_option && spike->get_class_id() != class_index)
				continue;
			n_pivot_spikes++; // update nb of pivot spikes
			// compute base index (where to store autocorrelation for this pivot spike)
			int psth_index = ((ii_time_0 - ii_start) * options_view_spikes->n_bins) / ii_length;
			ASSERT(psth_index >= 0);
			ASSERT(psth_index < options_view_spikes->n_bins);
			psth_index *= options_view_spikes->n_bins_isi;

			// get nb of counts around that spike
			// loop backwards to find first spike OK
			const int i_first = ii_time_0 - ii_span; // temp value
			auto i1 = i;
			while (i1 > 0 && spike_list->get_spike(i1)->get_time() > i_first)
				i1--;
			// internal loop build histogram
			for (auto j = i1; j < spike_count; j++)
			{
				if (j == i)
					continue;

				const Spike* p_spike_j = spike_list->get_spike(j);
				const auto ii_time = p_spike_j->get_time() - ii_time_0;
				if (ii_time >= ii_span)
					break;
				if (ii_time < -ii_span)
					continue;

				if (options_view_spikes->spike_class_option && p_spike_j->get_class_id() != class_index)
					continue;

				const int t_ii_time = (ii_time + ii_span) / ii_autocorrelation_bin_size;
				ASSERT(t_ii_time >= 0);
				ASSERT(t_ii_time < options_view_spikes->n_bins_isi);
				if (t_ii_time > options_view_spikes->n_bins_isi)
					continue;
				const auto ii = psth_index + t_ii_time;
				ASSERT(ii <= ii_dummy);
				(*(sum0 + ii))++;
			}
		}
	}
	return n_pivot_spikes;
}

void CSpikeDoc::export_spk_average_wave(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, double* value, const int spike_list_index,
                                        const int class_index)
{
	CString cs_dummy;
	const auto spike_list = &spike_list_array_[spike_list_index];
	auto ii_offset_0 = 0;
	if (!options_view_spikes->b_absolute_time && m_stimulus_intervals.n_items > 0)
		ii_offset_0 = m_stimulus_intervals.get_at(options_view_spikes->i_stimulus_index);
	// prepare parameters
	const auto rate = spike_list->get_acq_sampling_rate();
	const auto ii_time_start = static_cast<long>(options_view_spikes->time_start * rate) + ii_offset_0;
	const auto ii_time_end = static_cast<long>(options_view_spikes->time_end * rate) + ii_offset_0;
	const auto spike_count = spike_list->get_spikes_count();

	// clear histogram area if histogram of amplitudes
	ASSERT(options_view_spikes->export_data_type == EXPORT_AVERAGE);
	auto p_value = value + 1;
	const auto spike_length = static_cast<int>(*value); // spk length + nb items
	auto i0 = spike_length * 2 + 2;
	for (auto k = 1; k < i0; k++, p_value++)
		*p_value = 0;

	// check file size and position pointer at the first spike within the bin
	i0 = 0;
	while ((i0 < spike_count) && (spike_list->get_spike(i0)->get_time() < ii_time_start))
		i0++;

	// ------- count total spk/interval (given b_spk_class & b_artefacts)
	const auto bin_zero = spike_list->get_acq_bin_zero();
	for (auto j = i0; j < spike_count; j++)
	{
		const Spike* spike = spike_list->get_spike(j);
		const auto ii_time = spike->get_time();
		if (ii_time >= ii_time_end)
			break;

		// skip classes not requested
		const auto cla = spike->get_class_id();
		if (cla < 0 /*&& !options_view_spikes->b_artefacts*/)
			continue;

		if (options_view_spikes->spike_class_option != 0 && cla != class_index)
			continue;

		// get value, compute statistics
		auto spike_data = spike->get_p_data();
		const auto p_n = value + 1;
		auto p_sum = value + 2;
		auto p_sum2 = value + 2 + spike_length;
		for (auto k = 0; k < spike_length; k++, p_sum++, p_sum2++, spike_data++)
		{
			const auto x = static_cast<double>(*spike_data) - bin_zero;
			*p_sum += x;
			*p_sum2 += (x * x);
		}
		(*p_n)++;
	}

	// 4) export histogram
	const auto p_n = value + 1;
	auto p_sum = value + 2;
	auto p_sum2 = value + 2 + spike_length;
	if (*p_n > 0)
	{
		const auto volts_per_bin = static_cast<double>(spike_list->get_acq_volts_per_bin()) * 1000.f;
		const auto volts_per_bin2 = volts_per_bin * volts_per_bin;
		const auto y_n = *p_n;
		cs_dummy.Format(_T("\t%i"), static_cast<int>(y_n));
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		for (auto k = 0; k < spike_length; k++, p_sum++)
		{
			cs_dummy.Format(_T("\t%.3lf"), ((*p_sum) / y_n) * volts_per_bin);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}

		for (auto k = 0; k < spike_length; k++, p_sum2++)
		{
			cs_dummy.Format(_T("\t%.3lf"), (*p_sum2) * volts_per_bin2);
			shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		}
	}
	else
	{
		cs_dummy = _T("\t\t\t0");
		shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}
}

void CSpikeDoc::export_spk_average_wave(CSharedFile* shared_file, const options_view_spikes* options_view_spikes, double* value,
                                     const CString& cs_file_comment)
{
	const auto spike_list = &spike_list_array_[current_spike_list_index_];

	// spike class: -1(one:selected); 0(all); 1(all:split)
	auto class_index0 = 0;
	auto class_index1 = 0;
	if ((options_view_spikes->spike_class_option == -1
			|| options_view_spikes->spike_class_option == 1)
		&& !spike_list->is_class_list_valid())
		spike_list->update_class_list();

	if (options_view_spikes->spike_class_option == -1)
	{
		// search item index with correct class ID
		auto class_max = spike_list->get_classes_count() - 1;
		for (auto class_index = class_max; class_index >= 0; class_max--, class_index--)
			if (spike_list->get_class_id(class_max) == options_view_spikes->class_nb)
				break;

		// if none is found, export descriptor and exit
		if (class_max < 0)
		{
			export_spk_file_comment(shared_file, options_view_spikes, options_view_spikes->class_nb, cs_file_comment);
			return;
		}
		class_index0 = class_max;
		class_index1 = class_max;
	}
	else if (options_view_spikes->spike_class_option == 1)
		class_index1 = spike_list->get_classes_count() - 1;

	// loop header..................
	for (auto class_index = class_index0; class_index <= class_index1; class_index++)
	{
		const auto class_id = spike_list->get_class_id(class_index);
		export_spk_file_comment(shared_file, options_view_spikes, class_id, cs_file_comment);
		export_spk_average_wave(shared_file, options_view_spikes, value, current_spike_list_index_, class_id);
	}
}

SpikeList* CSpikeDoc::set_index_current_spike_list(int spike_list_index)
{
	SpikeList* spike_list = nullptr;
	if (spike_list_array_.GetSize() > 0 && spike_list_index >= 0 && spike_list_index < spike_list_array_.GetSize())
	{
		spike_list = &spike_list_array_[spike_list_index];
		current_spike_list_index_ = spike_list_index;
	}
	return spike_list;
}

SpikeList* CSpikeDoc::get_spike_list_current()
{
	if (current_spike_list_index_ >= 0)
		return &spike_list_array_[current_spike_list_index_];
	return nullptr;
}

SpikeList* CSpikeDoc::get_spike_list_at(int spike_list_index)
{
	return &spike_list_array_[spike_list_index];
}
