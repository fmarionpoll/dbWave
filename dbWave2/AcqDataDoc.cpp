#include "StdAfx.h"
#include "Taglist.h"
#include "AcqDataDoc.h"

#include "ADAcqDataDoc.h"
#include "datafile_Atlab.h"
#include "datafile_ASD.h"
#include "datafile_mcid.h"
#include "datafile_CEDSpike2.h"
#include "DlgImportGenericData.h"
#include "datafile_Awave.h"
#include "dbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(AcqDataDoc, CDocument)

BEGIN_MESSAGE_MAP(AcqDataDoc, CDocument)

END_MESSAGE_MAP()

AcqDataDoc::AcqDataDoc()
= default;

AcqDataDoc::~AcqDataDoc()
{
	SAFE_DELETE(p_w_buf)
	SAFE_DELETE(x_file)
}

BOOL AcqDataDoc::save_document(CString& sz_path_name)
{
	const BOOL flag = save_as(sz_path_name, FALSE);
	if (flag)
		SetModifiedFlag(FALSE); // mark the document as clean
	return flag;
}

BOOL AcqDataDoc::open_document(CString& sz_path_name)
{
	// close data file unless it is already opened
	if (x_file != nullptr)
		x_file->close_data_file();

	// set file reading buffer as dirty
	buf_valid_data = FALSE;

	// check if file can be opened - exit if it can't and return an empty object
	CFileStatus status;
	const auto b_open = CFile::GetStatus(sz_path_name, status);
	if (!b_open || status.m_size <= 4096) // patch to avoid to open 1kb files ...
	{
		SAFE_DELETE(x_file)
		return FALSE;
	}
	ASSERT(sz_path_name.Right(3) != _T("del"));

	auto b_found_match = open_acq_file(sz_path_name, status);
	if (b_found_match < 0)
		b_found_match = import_file(sz_path_name);

	return b_found_match;
}

int AcqDataDoc::import_file(CString& sz_path_name)
{
	x_file->close_data_file();
	SAFE_DELETE(x_file)
	CString filename_new = sz_path_name;
	if (!dlg_import_data_file(filename_new))
		return FALSE;

	// add "old_" to filename
	CString filename_renamed = sz_path_name;
	const auto count = filename_renamed.ReverseFind('\\') + 1;
	filename_renamed.Insert(count, _T("OLD_"));

	// TODO check if this is right
	remove_file(filename_renamed);
	rename_file(sz_path_name, filename_renamed);

	x_file = new CDataFileAWAVE;
	ASSERT(x_file != NULL);
	CFileStatus status;
	CFile::GetStatus(sz_path_name, status);
	const int b_found_match = open_acq_file(sz_path_name, status);
	return b_found_match;
}

void AcqDataDoc::remove_file(const CString& file1)
{
	CFileStatus status;
	if (CFile::GetStatus(file1, status) != 0)
		CFile::Remove(file1);
}

void AcqDataDoc::rename_file(const CString& filename_old, const CString& filename_new )
{
	try
	{
		CFile::Rename(filename_old, filename_new);
	}
	catch (CFileException* pEx)
	{
		CString cs;
		cs.Format(_T("File not found, cause = %i\n"), pEx->m_cause);
		cs = filename_old + cs;
		AfxMessageBox(cs, MB_OK);
		pEx->Delete();
	}
}

bool AcqDataDoc::dlg_import_data_file(CString& sz_path_name)
{
	const auto cs_array = new CStringArray; 
	ASSERT(cs_array != NULL);
	cs_array->Add(sz_path_name);

	const auto p_dlg = new DlgImportGenericData;
	p_dlg->m_p_file_name_array = cs_array;
	p_dlg->b_convert = TRUE; 
	p_dlg->options_import = &(static_cast<CdbWaveApp*>(AfxGetApp())->options_import);
	if (IDOK != p_dlg->DoModal() || 0 == cs_array->GetSize())
		return false;

	sz_path_name = cs_array->GetAt(0);
	return true;
}

BOOL AcqDataDoc::open_acq_file(CString& cs_filename, const CFileStatus& status)
{
	CFileException fe;

	// open file
	UINT u_open_flag = (status.m_attribute & 0x01) ? CFile::modeRead : CFile::modeReadWrite;
	u_open_flag |= CFile::shareDenyNone | CFile::typeBinary;

	constexpr int data_types_array[] = {
		DOCTYPE_AWAVE, DOCTYPE_SMR, DOCTYPE_ATLAB, DOCTYPE_ASDSYNTECH, DOCTYPE_MCID, DOCTYPE_UNKNOWN
	};
	constexpr int array_size = std::size(data_types_array);
	auto id_type = DOCTYPE_UNKNOWN;
	for (int id = 0; id < array_size; id++)
	{
		delete x_file;
		instantiate_data_file_object(id);
		if (0 != x_file->open_data_file(cs_filename, u_open_flag))
			id_type = x_file->check_file_type(cs_filename);
		if (id_type != DOCTYPE_UNKNOWN)
			break;
	}

	if (x_file == nullptr || x_file->m_id_type == DOCTYPE_UNKNOWN)
	{
		allocate_buffer();
		return false;
	}

	// save file pointer, read data header and Tags
	x_file_type = id_type;
	if (p_w_buf == nullptr)
		p_w_buf = new CWaveBuf;
	ASSERT(p_w_buf != NULL);
	const auto b_flag = x_file->read_data_infos(p_w_buf);

	// create buffer
	allocate_buffer();
	x_file->read_vt_tags(p_w_buf->get_p_vt_tags());
	x_file->read_hz_tags(p_w_buf->get_p_hz_tags());

	return b_flag;
}

BOOL AcqDataDoc::OnNewDocument()
{
	DeleteContents();
	m_strPathName.Empty(); // no path name yet
	SetModifiedFlag(FALSE); // make clean

	if (p_w_buf == nullptr)
	{
		CString cs_dummy;
		cs_dummy.Empty();
		acq_create_file(cs_dummy);
		ASSERT(p_w_buf != NULL);
	}
	p_w_buf->create_buffer_with_n_channels(1); // create at least one channel
	return TRUE;
}

CString AcqDataDoc::get_data_file_infos(const options_view_data* p_vd) const
{
	const CString sep('\t');
	CString cs_dummy;
	auto cs_out = GetPathName();
	cs_out.MakeLower();
	const auto waveformat = get_wave_format();

	// date and time
	if (p_vd->b_acq_date)
		cs_out += (waveformat->acquisition_time).Format("\t%#d %B %Y");
	if (p_vd->b_acq_time)
		cs_out += (waveformat->acquisition_time).Format("\t%X");

	// file size
	if (p_vd->b_file_size) // file size
	{
		cs_dummy.Format(_T("\t%-10li"), get_doc_channel_length());
		cs_out += cs_dummy;
		cs_dummy.Format(_T("\t nchans=%i\t"), waveformat->scan_count);
		cs_out += cs_dummy;
	}

	if (p_vd->b_acq_comments)
	{
		cs_dummy = waveformat->get_comments(sep);
		cs_out += cs_dummy;
	}

	if (p_vd->b_acq_channel_comment || p_vd->b_acq_channel_setting)
	{
		CString cs;
		for (auto i_chan = 0; i_chan < waveformat->scan_count; i_chan++)
		{
			const auto p_chan = (get_wave_channels_array())->get_p_channel(i_chan);
			if (p_vd->b_acq_channel_comment)
				cs_out += sep + p_chan->am_csComment;
			if (p_vd->b_acq_channel_setting)
			{
				cs.Format(_T("\theadstage=%s\tgain=%.0f\tfilter= %s\t%i Hz"),
						(LPCTSTR)p_chan->am_csheadstage,
				        p_chan->am_gaintotal,
						(LPCTSTR)p_chan->am_csInputpos,
				        p_chan->am_lowpass);
				cs_out += cs;
			}
		}
	}
	return cs_out;
}

void AcqDataDoc::export_data_file_to_txt_file(CStdioFile* pdata_dest)
{
	constexpr _TCHAR sep = '\n'; 
	constexpr _TCHAR sep2 = ',';

	auto cs_out = _T("filename=") + GetPathName();
	cs_out.MakeLower();
	cs_out += sep;
	pdata_dest->WriteString(cs_out);

	const auto p_wave_format = get_wave_format();
	// date and time
	cs_out = (p_wave_format->acquisition_time).Format(_T("acqdate= %#d %B %Y"));
	cs_out += sep;
	pdata_dest->WriteString(cs_out);

	cs_out = (p_wave_format->acquisition_time).Format(_T("acqtime= %X"));
	cs_out += sep;
	pdata_dest->WriteString(cs_out);

	cs_out.Format(_T("sampling rate(Hz)= %f"), p_wave_format->sampling_rate_per_channel);
	cs_out += sep;
	pdata_dest->WriteString(cs_out);

	cs_out.Format(_T("nsamples=%-10li"), get_doc_channel_length());
	cs_out += sep;
	pdata_dest->WriteString(cs_out);

	cs_out.Format(_T("nchans=%i"), p_wave_format->scan_count);
	pdata_dest->WriteString(cs_out);

	cs_out = p_wave_format->get_comments(&sep, TRUE);
	pdata_dest->WriteString(cs_out);

	cs_out = sep;
	for (auto i_chan = 0; i_chan < p_wave_format->scan_count; i_chan++)
	{
		const auto p_chan = (get_wave_channels_array())->get_p_channel(i_chan);
		cs_out += p_chan->am_csComment;
		if (i_chan < p_wave_format->scan_count - 1)
			cs_out += sep2;
	}
	pdata_dest->WriteString(cs_out);

	// loop to read actual data
	read_data_block(0);
	const auto mv_factor = p_wave_format->full_scale_volts / static_cast<float>(p_wave_format->bin_span) * 1000.;
	for (long j = 0; j < get_doc_channel_length(); j++)
	{
		CString cs;
		cs_out = sep;
		for (auto channel = 0; channel < p_wave_format->scan_count; channel++)
		{
			const auto p_chan = (get_wave_channels_array())->get_p_channel(channel);
			const auto value = get_value_from_buffer(channel, j);
			const auto channel_gain = static_cast<double>(p_chan->am_gainheadstage)
				* static_cast<double>(p_chan->am_gainAD)
				* static_cast<double>(p_chan->am_gainpre)
				* static_cast<double>(p_chan->am_gainpost);
			const auto x_value = value * mv_factor / channel_gain;
			cs.Format(_T("%f"), x_value);
			cs_out += cs;
			if (channel < p_wave_format->scan_count - 1)
				cs_out += sep2;
		}
		pdata_dest->WriteString(cs_out);
	}
	pdata_dest->WriteString(&sep);
}

#ifdef _DEBUG
void AcqDataDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void AcqDataDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void AcqDataDoc::instantiate_data_file_object(const int doc_type)
{
	switch (doc_type)
	{
	case DOCTYPE_AWAVE:
		x_file = new CDataFileAWAVE;
		break;
	case DOCTYPE_ATLAB:
		x_file = new CDataFileATLAB;
		break;
	case DOCTYPE_ASDSYNTECH:
		x_file = new CDataFileASD;
		break;
	case DOCTYPE_MCID:
		x_file = new CDataFileMCID;
		break;
	case DOCTYPE_SMR:
		x_file = new CDataFileFromCEDSpike2;
		break;
	//case DOCTYPE_PCCLAMP		5	// PCCLAMP document (not implemented yet)
	//case DOCTYPE_SAPID 		6	// SAPID document (not implemented yet)
	//case DOCTYPE_UNKNOWN		-1	// type of the document not accepted
	default:
		x_file = new CDataFileX;
		break;
	}
}

// adjust size of the buffer for data read from file
// update buffer parameters
BOOL AcqDataDoc::adjust_buffer(const int elements_count)
{
	if (p_w_buf == nullptr)
		p_w_buf = new CWaveBuf;

	ASSERT(p_w_buf != NULL);
	const auto p_wf = get_wave_format();
	doc_channel_length = p_wf->get_nb_points_sampled_per_channel();
	doc_n_channels = p_wf->scan_count;
	p_wf->duration = static_cast<float>(doc_channel_length) / p_wf->sampling_rate_per_channel;
	buf_size = elements_count * p_wf->scan_count;

	buf_channel_size = buf_size / static_cast<long>(p_wf->scan_count);
	buf_channel_first = 0;
	buf_channel_last = buf_channel_size - 1;
	buf_size = buf_channel_size * p_wf->scan_count;
	buf_max_size = buf_size * static_cast<long>(sizeof(short));

	// alloc RW buffer
	return p_w_buf->create_w_buffer(buf_channel_size, p_wf->scan_count);
}

// allocate buffers to read data
// adjust size of the buffer according to MAX_BUFFER_LENGTH_AS_BYTES
BOOL AcqDataDoc::allocate_buffer()
{
	if (p_w_buf == nullptr)
		p_w_buf = new CWaveBuf;

	ASSERT(p_w_buf != NULL); // check object created properly
	CWaveFormat* pwF = get_wave_format();

	doc_channel_length = pwF->get_nb_points_sampled_per_channel();
	doc_n_channels = pwF->scan_count;
	pwF->duration = static_cast<float>(doc_channel_length) / pwF->sampling_rate_per_channel;

	const int i_num_elements = doc_channel_length;
	if (i_num_elements * pwF->scan_count > buf_size)
		buf_size = i_num_elements * pwF->scan_count;

	if (buf_size > static_cast<long>(MAX_BUFFER_LENGTH_AS_BYTES / sizeof(short)))
		buf_size = MAX_BUFFER_LENGTH_AS_BYTES / sizeof(short);

	buf_channel_size = buf_size / static_cast<long>(pwF->scan_count);
	buf_channel_first = 0;
	buf_channel_last = buf_channel_size - 1;
	buf_size = buf_channel_size * pwF->scan_count;
	buf_max_size = buf_size * static_cast<long>(sizeof(short));

	// alloc RW buffer
	return p_w_buf->create_w_buffer(buf_channel_size, pwF->scan_count);
}

BOOL AcqDataDoc::load_raw_data(long* l_first, long* l_last, const int n_span)
{
	auto flag = TRUE;

	if ((*l_first - n_span < buf_channel_first)
		|| (*l_last + n_span > buf_channel_last) || !buf_valid_data)
	{
		flag = read_data_block(*l_first - n_span);
		buf_valid_data = TRUE;
		buf_valid_transform = FALSE;
	}

	*l_first = buf_channel_first + n_span;
	*l_last = buf_channel_last;
	if (buf_channel_last < doc_channel_length - 1)
		*l_last -= n_span;

	return flag;
}

BOOL AcqDataDoc::read_data_block(long l_first)
{
	// check limits
	if (l_first < 0)
		l_first = 0;
	// compute limits
	buf_channel_first = l_first;
	buf_channel_last = buf_channel_first + buf_channel_size - 1;
	if (buf_channel_last > doc_channel_length)
	{
		if (doc_channel_length < buf_channel_size)
		{
			buf_channel_size = doc_channel_length;
			buf_channel_last = buf_channel_size - 1;
		}
		buf_channel_first = doc_channel_length - buf_channel_size;
	}
	l_first = buf_channel_first * doc_n_channels;

	// reallocate buffer if needed
	if (p_w_buf->get_wb_n_elements() != buf_channel_size)
		allocate_buffer();

	// read data from file
	if (x_file != nullptr)
	{
		short* p_buffer = p_w_buf->get_pointer_to_raw_data_buffer();
		ASSERT(p_buffer != NULL);
		x_file->read_adc_data(l_first, buf_size * static_cast<long>(sizeof(short)), p_buffer, get_wave_channels_array());

		// ugly patch: should fail if l_size < m_lBUFSize
		buf_channel_last = buf_channel_first + buf_size / doc_n_channels - 1;

		// remove offset so that data are signed short (for offset binary data of 12 or 16 bits resolution)
		const auto w_bin_zero = static_cast<WORD>(p_w_buf->wave_format_.bin_zero);
		if (b_remove_offset && w_bin_zero != NULL)
		{
			auto* pw_buf = reinterpret_cast<WORD*>(p_w_buf->get_pointer_to_raw_data_buffer());
			for (long i = 0; i < buf_size; i++, pw_buf++)
				*pw_buf -= w_bin_zero;
		}
		return TRUE;
	}
	return FALSE;
}

void AcqDataDoc::read_data_infos()
{
	ASSERT(x_file != NULL);
	x_file->read_data_infos(p_w_buf);
	allocate_buffer();
}

short AcqDataDoc::get_value_from_buffer(const int channel, const long l_index)
{
	if ((l_index < buf_channel_first) || (l_index > buf_channel_last))
		read_data_block(l_index);
	const int index = l_index - buf_channel_first;
	return *(p_w_buf->get_pointer_to_raw_data_element(channel, index));
}

short* AcqDataDoc::load_transformed_data(const long l_first, const long l_last, const int transform_type,
                                  const int source_channel)
{
	const BOOL b_already_done = (buf_valid_transform
		&& (l_first == buf_first_)
		&& (l_last == buf_last_)
		&& (buf_transform_ == transform_type)
		&& (buf_source_channel_ == source_channel));
	buf_transform_ = transform_type;
	buf_source_channel_ = source_channel;
	buf_first_ = l_first;
	buf_last_ = l_last;

	if (p_w_buf->get_pointer_to_transformed_data_buffer() == nullptr)
		p_w_buf->wb_init_transform_buffer();

	const auto i_span = CWaveBuf::wb_get_transform_span(transform_type);
	const auto l_span = static_cast<long>(i_span);

	// ASSERT make sure that all data requested are within the buffer ...
	ASSERT(l_first >= buf_channel_first && (l_last <= buf_channel_last));
	if (((l_first - l_span) < buf_channel_first) || ((l_last + l_span) > buf_channel_last)) // we should never get there
		read_data_block(l_first - l_span); // but, just in case

	auto n_points = static_cast<int>(l_last - l_first + 1);
	const int n_channels = get_wave_format()->scan_count;
	ASSERT(source_channel < n_channels); // make sure this is a valid channel
	const int i_offset = (l_first - buf_channel_first) * n_channels + source_channel;
	auto lp_source = p_w_buf->get_pointer_to_raw_data_buffer() + i_offset;

	// call corresponding one-pass routine
	auto lp_destination = p_w_buf->get_pointer_to_transformed_data_buffer();

	// check if source l_first can be used
	const auto b_isLFirstLower = (l_first < l_span);
	if (b_isLFirstLower) // no: skip these data and erase later 
	{
		lp_source += n_channels * i_span;
		lp_destination += i_span;
		n_points -= i_span;
	}

	// check if source l_last can be used
	const auto b_isLLastGreater = (l_last > doc_channel_length - l_span);
	if (b_isLLastGreater) // no: skip these data and erase later 
	{
		n_points -= i_span;
	}

	if (n_points <= 0)
	{
		// erase data at the end of the buffer
		const int i_cx = i_span + l_last - l_first + 1;
		auto lp_dest0 = p_w_buf->get_pointer_to_transformed_data_buffer();
		for (auto cx = 0; cx < i_cx; cx++, lp_dest0++)
			*lp_dest0 = 0;
	}
	else if (!b_already_done)
	{
		switch (transform_type)
		{
		case 0: p_w_buf->copy(lp_source, lp_destination, n_points);
			break;
		case 1: p_w_buf->low_pass_differentiation(lp_source, lp_destination, n_points);
			break;
		case 2: p_w_buf->low_pass_lanczo_2(lp_source, lp_destination, n_points);
			break;
		case 3: p_w_buf->low_pass_lanczo_3(lp_source, lp_destination, n_points);
			break;
		case 4: p_w_buf->low_pass_derivative_1f3(lp_source, lp_destination, n_points);
			break;
		case 5: p_w_buf->low_pass_derivative_2f3(lp_source, lp_destination, n_points);
			break;
		case 6: p_w_buf->low_pass_derivative_2f5(lp_source, lp_destination, n_points);
			break;
		case 7: p_w_buf->low_pass_derivative_3f3(lp_source, lp_destination, n_points);
			break;
		case 8: p_w_buf->low_pass_diff_1(lp_source, lp_destination, n_points);
			break;
		case 9: p_w_buf->low_pass_diff_2(lp_source, lp_destination, n_points);
			break;
		case 10: p_w_buf->low_pass_diff_3(lp_source, lp_destination, n_points);
			break;
		case 11: p_w_buf->low_pass_diff_10(lp_source, lp_destination, n_points);
			break;
		case 12: p_w_buf->moving_average_30(lp_source, lp_destination, n_points);
			break;
		case 13: p_w_buf->moving_median_30(lp_source, lp_destination, n_points);
			break;
		case 14: p_w_buf->moving_median_35(lp_source, lp_destination, n_points);
			break;
		case 15: p_w_buf->root_to_mean_square(lp_source, lp_destination, n_points);
		default: break;
		}

		// set undefined pts equal to first valid point
		if (b_isLFirstLower)
		{
			auto lp_dest0 = p_w_buf->get_pointer_to_transformed_data_buffer();
			for (auto cx = i_span; cx > 0; cx--, lp_dest0++)
				*lp_dest0 = 0;
			n_points += i_span;
			lp_destination -= i_span;
		}

		// set undefined points at the end equal to last valid point
		if (b_isLLastGreater)
		{
			auto lp_dest0 = lp_destination + n_points;
			for (auto cx = i_span; cx > 0; cx--, lp_dest0++)
				*lp_dest0 = 0;
		}
	}
	buf_valid_transform = TRUE;
	return lp_destination;
}

BOOL AcqDataDoc::build_transformed_data(const int transform_type, const int source_channel) const
{
	// make sure that transform buffer is ready
	if (p_w_buf->get_pointer_to_transformed_data_buffer() == nullptr)
		p_w_buf->wb_init_transform_buffer();
	auto flag = TRUE;

	// init parameters
	auto lp_source = p_w_buf->get_pointer_to_raw_data_buffer() + source_channel;
	const int nb_channels = get_wave_format()->scan_count;
	ASSERT(source_channel < nb_channels);

	// adjust pointers according to n_span - (fringe data) and set flags erase these data at the end
	auto lp_dest = p_w_buf->get_pointer_to_transformed_data_buffer();
	const auto i_span_between_points = CWaveBuf::wb_get_transform_span(transform_type);
	const auto l_span_between_points = static_cast<long>(i_span_between_points);
	const auto l_first = buf_channel_first + l_span_between_points;
	const auto l_last = buf_channel_last - l_span_between_points;
	lp_source += nb_channels * i_span_between_points;
	lp_dest += i_span_between_points;
	int n_points = l_last - l_first + 1;
	ASSERT(n_points > 0);

	switch (transform_type)
	{
	case 0: p_w_buf->copy(lp_source, lp_dest, n_points);
		break;
	case 1: p_w_buf->low_pass_differentiation(lp_source, lp_dest, n_points);
		break;
	case 2: p_w_buf->low_pass_lanczo_2(lp_source, lp_dest, n_points);
		break;
	case 3: p_w_buf->low_pass_lanczo_3(lp_source, lp_dest, n_points);
		break;
	case 4: p_w_buf->low_pass_derivative_1f3(lp_source, lp_dest, n_points);
		break;
	case 5: p_w_buf->low_pass_derivative_2f3(lp_source, lp_dest, n_points);
		break;
	case 6: p_w_buf->low_pass_derivative_2f5(lp_source, lp_dest, n_points);
		break;
	case 7: p_w_buf->low_pass_derivative_3f3(lp_source, lp_dest, n_points);
		break;
	case 8: p_w_buf->low_pass_diff_1(lp_source, lp_dest, n_points);
		break;
	case 9: p_w_buf->low_pass_diff_2(lp_source, lp_dest, n_points);
		break;
	case 10: p_w_buf->low_pass_diff_3(lp_source, lp_dest, n_points);
		break;
	case 11: p_w_buf->low_pass_diff_10(lp_source, lp_dest, n_points);
		break;
	case 12: p_w_buf->moving_average_30(lp_source, lp_dest, n_points);
		break;
	case 13: p_w_buf->moving_median_30(lp_source, lp_dest, n_points);
		break;
	case 14: p_w_buf->moving_median_35(lp_source, lp_dest, n_points);
		break;
	case 15: p_w_buf->root_to_mean_square(lp_source, lp_dest, n_points);
	default: flag = FALSE;
		break;
	}

	// set undefined pts equal to first valid point
	if (i_span_between_points > 0)
	{
		auto lp_dest0 = p_w_buf->get_pointer_to_transformed_data_buffer();
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
		n_points += i_span_between_points;

		lp_dest0 = p_w_buf->get_pointer_to_transformed_data_buffer() + n_points;
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
	}
	return flag;
}

BOOL AcqDataDoc::acq_create_file(CString& cs_file_name)
{
	if (!cs_file_name.IsEmpty())
	{
		// Create file - send message if creation failed
		if (x_file == nullptr)
		{
			x_file = new CDataFileAWAVE;
			ASSERT(x_file != NULL);
		}
		if (x_file->m_id_type != DOCTYPE_AWAVE)
		{
			delete x_file;
			x_file = new CDataFileAWAVE;
			ASSERT(x_file != NULL);
		}

		if (!x_file->open_data_file(cs_file_name, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone))
		{
			AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
			return FALSE;
		}
		x_file->init_file();
	}

	// create object as file
	allocate_buffer();

	return TRUE;
}

BOOL AcqDataDoc::write_hz_tags(TagList* p_tags) const
{
	if (p_tags == nullptr)
		p_tags = p_w_buf->get_p_hz_tags();
	if (p_tags == nullptr || p_tags->get_tag_list_size() == 0)
		return TRUE;
	return x_file->write_hz_tags(p_tags);
}

BOOL AcqDataDoc::write_vt_tags(TagList* p_tags) const
{
	if (p_tags == nullptr)
		p_tags = p_w_buf->get_p_vt_tags();
	if (p_tags == nullptr || p_tags->get_tag_list_size() == 0)
		return TRUE;
	return x_file->write_vt_tags(p_tags);
}

BOOL AcqDataDoc::acq_save_data_descriptors() const
{
	const auto flag = x_file->write_data_infos(get_wave_format(), get_wave_channels_array());
	x_file->Flush();
	return flag;
}

void AcqDataDoc::acq_delete_file() const
{
	const auto cs_file_path = x_file->GetFilePath();
	x_file->close_data_file();
	CFile::Remove(cs_file_path);
}

void AcqDataDoc::acq_close_file() const
{
	if (x_file != nullptr)
		x_file->close_data_file();
}

BOOL AcqDataDoc::save_as(CString& new_name, BOOL b_check_over_write, const int i_type)
{
	const auto i_position_of_extension = new_name.ReverseFind('.');
	if (i_position_of_extension > 0)
	{
		auto ext = new_name.Right(new_name.GetLength() - i_position_of_extension - 1);
		const BOOL is_extension_dat = (ext.Compare(_T("dat")) == 0);
		if (!is_extension_dat)
		{
			ext = new_name.Left(i_position_of_extension + 1);
			new_name = ext + _T("dat");
		}
	}

	auto dummy_name = new_name;
	const auto cs_former_name = x_file->GetFilePath();

	// check if same file already exist
	CFileStatus status;
	const auto b_file_already_exists = CFile::GetStatus(new_name, status);
	if (b_file_already_exists != 0 && b_check_over_write)
	{
		auto prompt = new_name;
		prompt += _T("\nThis file already exists.\nReplace existing file?");
		if (AfxMessageBox(prompt, MB_YESNO) != IDYES)
			return FALSE; // don't continue
	}

	// check if collision with current file
	auto b_is_duplicate_name = FALSE;
	if (b_file_already_exists &&
		cs_former_name.CompareNoCase(status.m_szFullName) == 0) // used in conjunction with CFile::GetStatus()
	{
		b_is_duplicate_name = TRUE;
		dummy_name = new_name;
		dummy_name += _T("_tmp.dat");
	}

	// create new file
	auto* p_new_doc = new ADAcqDataDoc;
	if (!p_new_doc->acq_create_file(dummy_name))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete p_new_doc;
		return FALSE;
	}

	// save data header
	if (get_wave_format()->scan_count < get_wave_channels_array()->chan_array_get_size())
	{
		const auto last_channel = get_wave_format()->scan_count - 1;
		for (auto i = get_wave_channels_array()->chan_array_get_size() - 1; i > last_channel; i--)
			get_wave_channels_array()->chan_array_remove_at(i);
	}

	// save data
	p_new_doc->AcqDoc_DataAppendStart();
	auto n_samples = get_wave_format()->sample_count;

	// position source file index to start of data
	x_file->Seek(static_cast<LONGLONG>(x_file->m_ul_offset_data), CFile::begin);
	auto p_buf = p_w_buf->get_pointer_to_raw_data_buffer(); // buffer to store data
	auto l_buf_size = buf_size; // length of the buffer

	while (n_samples > 0) // loop until the end of the file
	{
		// read data from source file into buffer
		if (n_samples < buf_size) // adjust buf_temp_size
			l_buf_size = n_samples; // then, store data in temporary buffer
		const long n_bytes = l_buf_size * static_cast<long>(sizeof(short));
		x_file->Read(p_buf, n_bytes);
		if (i_type == 3) // ASD file
		{
			auto p_buf2 = p_buf;
			for (long i = 0; i < n_bytes; i++)
			{
				*p_buf2 = static_cast<short>(*p_buf2 / 2);
				p_buf2 += 2;
			}
		}
		// save buffer
		if (!p_new_doc->AcqDoc_DataAppend(p_buf, n_bytes))
			break;
		n_samples -= l_buf_size; 
	}

	// stop appending data, update dependent struct
	if (!p_new_doc->AcqDoc_DataAppendStop())
		return false;

	// save other objects if exist (tags, others)

	if (p_w_buf->get_p_hz_tags()->get_tag_list_size() > 0)
		p_new_doc->write_hz_tags(p_w_buf->get_p_hz_tags());
	if (p_w_buf->get_p_vt_tags()->get_tag_list_size() > 0)
		p_new_doc->write_vt_tags(p_w_buf->get_p_vt_tags());

	// if destination name == source: remove source and rename destination
	if (b_is_duplicate_name)
	{
		// copy dummy name into csFormerName
		const auto dw_new_length = p_new_doc->x_file->GetLength();
		x_file->SetLength(dw_new_length);
		x_file->SeekToBegin();
		p_new_doc->x_file->SeekToBegin();

		p_buf = p_w_buf->get_pointer_to_raw_data_buffer(); 
		const auto n_bytes = buf_size * sizeof(short);
		DWORD dw_read;
		do
		{
			dw_read = p_new_doc->x_file->Read(p_buf, n_bytes);
			x_file->Write(p_buf, dw_read);
		}
		while (dw_read > 0);

		// file is transferred, destroy temporary file
		p_new_doc->x_file->close_data_file();

		// delete current file object and open saved-as file ??
		x_file->close_data_file();
		SAFE_DELETE(x_file)

		// create / update CDataFileX associated object
		x_file = new CDataFileAWAVE; //((CFile*) this);
		ASSERT(x_file != NULL);

		// open saved file
		if (!x_file->open_data_file(new_name, CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary))
			return FALSE;
	}

	// destroy object created
	delete p_new_doc;

	return TRUE;
}
