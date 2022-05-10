#include "StdAfx.h"
#include "Taglist.h"
#include "AcqDataDoc.h"
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
{
}

AcqDataDoc::~AcqDataDoc()
{
	// delete structures created
	SAFE_DELETE(m_pWBuf)
	SAFE_DELETE(m_pXFile)
}

BOOL AcqDataDoc::OnSaveDocument(CString& szPathName)
{
	const BOOL flag = SaveAs(szPathName, FALSE);
	if (flag)
		SetModifiedFlag(FALSE); // mark the document as clean
	return flag;
}

BOOL AcqDataDoc::OnOpenDocument(CString& sz_path_name)
{
	// close data file unless it is already opened
	if (m_pXFile != nullptr)
		m_pXFile->CloseDataFile();

	// set file reading buffer as dirty
	m_bValidReadBuffer = FALSE;

	// check if file can be opened - exit if it can't and return an empty object
	CFileStatus status;
	const auto b_open = CFile::GetStatus(sz_path_name, status);
	if (!b_open || status.m_size <= 4096) // patch to avoid to open 1kb files ...
	{
		SAFE_DELETE(m_pXFile)
		return FALSE;
	}
	ASSERT(sz_path_name.Right(3) != _T("del"));

	auto b_found_match = openAcqFile(sz_path_name);
	if (b_found_match < 0)
		b_found_match = importFile(sz_path_name);

	return b_found_match;
}

int AcqDataDoc::importFile(CString& sz_path_name)
{
	m_pXFile->CloseDataFile();
	SAFE_DELETE(m_pXFile)
	CString filename_new = sz_path_name;
	if (!dlgImportDataFile(filename_new))
		return FALSE;

	// add "old_" to filename
	CString filename_old = sz_path_name;
	const auto count = filename_old.ReverseFind('\\') + 1;
	filename_old.Insert(count, _T("OLD_"));

	// TODO check if this is right???
	removeFile(filename_old);
	renameFile2As1(sz_path_name, filename_old);
	//removeFile(sz_path_name);
	//renameFile2As1(sz_path_name, filename_new);

	m_pXFile = new CDataFileAWAVE;
	ASSERT(m_pXFile != NULL);
	int b_found_match = openAcqFile(sz_path_name);
	return b_found_match;
}

void AcqDataDoc::removeFile(CString file1)
{
	CFileStatus status;
	auto b_flag_exist = CFile::GetStatus(file1, status);
	if (b_flag_exist != 0)
		CFile::Remove(file1);
}

void AcqDataDoc::renameFile2As1(CString file1, CString file2)
{
	try
	{
		CFile::Rename(file2, file1);
	}
	catch (CFileException* pEx)
	{
		CString cs;
		cs.Format(_T("File not found, cause = %i\n"), pEx->m_cause);
		cs = file2 + cs;
		AfxMessageBox(cs, MB_OK);
		pEx->Delete();
	}
}

bool AcqDataDoc::dlgImportDataFile(CString& sz_path_name)
{
	auto cs_array = new CStringArray; // dlg needs an array of strings
	ASSERT(cs_array != NULL);
	cs_array->Add(sz_path_name);

	auto p_dlg = new DlgImportGenericData;
	p_dlg->m_pfilenameArray = cs_array; // address of array
	p_dlg->bConvert = TRUE; // conversion is allowed
	p_dlg->piivO = &(dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_import);
	if (IDOK != p_dlg->DoModal() || 0 == cs_array->GetSize())
		return false;

	sz_path_name = cs_array->GetAt(0);
	return true;
}

BOOL AcqDataDoc::openAcqFile(CString& cs_filename)
{
	CFileException fe;
	CFileStatus status;

	// open file
	UINT u_open_flag = (status.m_attribute & 0x01) ? CFile::modeRead : CFile::modeReadWrite;
	u_open_flag |= CFile::shareDenyNone | CFile::typeBinary;

	int dataTypesArray[] = {
		DOCTYPE_AWAVE, DOCTYPE_SMR, DOCTYPE_ATLAB, DOCTYPE_ASDSYNTECH, DOCTYPE_MCID, DOCTYPE_UNKNOWN
	};
	int arrSize = sizeof(dataTypesArray) / sizeof(dataTypesArray[0]);
	auto id_type = DOCTYPE_UNKNOWN;
	for (int id = 0; id < arrSize; id++)
	{
		if (m_pXFile != nullptr)
			delete m_pXFile;
		instanciateDataFileObject(id);
		if (0 != m_pXFile->OpenDataFile(cs_filename, u_open_flag))
			id_type = m_pXFile->CheckFileType(cs_filename);
		if (id_type != DOCTYPE_UNKNOWN)
			break;
	}

	if (m_pXFile == nullptr || m_pXFile->m_idType == DOCTYPE_UNKNOWN)
	{
		AllocBUF();
		return false;
	}

	// save file pointer, read data header and Tags
	m_pXFileType = id_type;
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;
	ASSERT(m_pWBuf != NULL);
	const auto b_flag = m_pXFile->ReadDataInfos(m_pWBuf);

	// create buffer
	AllocBUF();
	m_pXFile->ReadVTtags(m_pWBuf->GetpVTtags());
	m_pXFile->ReadHZtags(m_pWBuf->GetpHZtags());

	return b_flag;
}

BOOL AcqDataDoc::OnNewDocument()
{
	//if (!CDocument::OnNewDocument())
	//	return FALSE;
	DeleteContents();
	m_strPathName.Empty(); // no path name yet
	SetModifiedFlag(FALSE); // make clean

	if (m_pWBuf == nullptr)
	{
		CString cs_dummy;
		cs_dummy.Empty();
		CreateAcqFile(cs_dummy);
		ASSERT(m_pWBuf != NULL);
	}
	m_pWBuf->WBDatachanSetnum(1); // create at least one channel
	return TRUE;
}

CString AcqDataDoc::GetDataFileInfos(OPTIONS_VIEWDATA* pVD)
{
	const CString sep('\t');

	CString cs_dummy;
	auto cs_out = GetPathName();
	cs_out.MakeLower(); // lower case

	const auto p_wave_format = GetpWaveFormat();

	// date and time
	if (pVD->bacqdate)
		cs_out += (p_wave_format->acqtime).Format("\t%#d %B %Y");
	if (pVD->bacqtime)
		cs_out += (p_wave_format->acqtime).Format("\t%X");

	// file size
	if (pVD->bfilesize) // file size
	{
		cs_dummy.Format(_T("\t%-10li"), GetDOCchanLength());
		cs_out += cs_dummy;
		cs_dummy.Format(_T("\t nchans=%i\t"), p_wave_format->scan_count);
		cs_out += cs_dummy;
	}

	if (pVD->bacqcomments)
	{
		cs_dummy = p_wave_format->GetComments(sep);
		cs_out += cs_dummy;
	}

	if (pVD->bacqchcomment || pVD->bacqchsetting)
	{
		CString cs;
		for (auto i_chan = 0; i_chan < p_wave_format->scan_count; i_chan++)
		{
			const auto p_chan = (GetpWavechanArray())->Get_p_channel(i_chan);
			if (pVD->bacqchcomment)
				cs_out += sep + p_chan->am_csComment;
			if (pVD->bacqchsetting)
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

void AcqDataDoc::ExportDataFile_to_TXTFile(CStdioFile* pdataDest)
{
	const _TCHAR sep = '\n'; // lines separator "\n\r"
	const _TCHAR sep2 = ','; // columns separator

	auto cs_out = _T("filename=") + GetPathName();
	cs_out.MakeLower(); // lower case
	cs_out += sep;
	pdataDest->WriteString(cs_out);

	const auto p_wave_format = GetpWaveFormat();
	// date and time
	cs_out = (p_wave_format->acqtime).Format(_T("acqdate= %#d %B %Y"));
	cs_out += sep;
	pdataDest->WriteString(cs_out);

	cs_out = (p_wave_format->acqtime).Format(_T("acqtime= %X"));
	cs_out += sep;
	pdataDest->WriteString(cs_out);

	cs_out.Format(_T("sampling rate(Hz)= %f"), p_wave_format->sampling_rate_per_channel);
	cs_out += sep;
	pdataDest->WriteString(cs_out);

	cs_out.Format(_T("nsamples=%-10li"), GetDOCchanLength());
	cs_out += sep;
	pdataDest->WriteString(cs_out);

	cs_out.Format(_T("nchans=%i"), p_wave_format->scan_count);
	pdataDest->WriteString(cs_out);

	cs_out = p_wave_format->GetComments(&sep, TRUE);
	pdataDest->WriteString(cs_out);

	cs_out = sep;
	for (auto i_chan = 0; i_chan < p_wave_format->scan_count; i_chan++)
	{
		const auto p_chan = (GetpWavechanArray())->Get_p_channel(i_chan);
		cs_out += p_chan->am_csComment;
		if (i_chan < p_wave_format->scan_count - 1)
			cs_out += sep2;
	}
	pdataDest->WriteString(cs_out);

	// loop to read actual data
	readDataBlock(0);
	const auto mv_factor = p_wave_format->fullscale_volts / p_wave_format->binspan * 1000.;
	for (long j = 0; j < GetDOCchanLength(); j++)
	{
		CString cs;
		cs_out = sep;
		for (auto channel = 0; channel < p_wave_format->scan_count; channel++)
		{
			const auto p_chan = (GetpWavechanArray())->Get_p_channel(channel);
			const auto value = BGetVal(channel, j);
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
		pdataDest->WriteString(cs_out);
	}
	pdataDest->WriteString(&sep);
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

void AcqDataDoc::instanciateDataFileObject(int docType)
{
	switch (docType)
	{
	case DOCTYPE_AWAVE:
		m_pXFile = new CDataFileAWAVE;
		break;
	case DOCTYPE_ATLAB:
		m_pXFile = new CDataFileATLAB;
		break;
	case DOCTYPE_ASDSYNTECH:
		m_pXFile = new CDataFileASD;
		break;
	case DOCTYPE_MCID:
		m_pXFile = new CDataFileMCID;
		break;
	case DOCTYPE_SMR:
		m_pXFile = new CDataFileFromCEDSpike2;
		break;
	//case DOCTYPE_PCCLAMP		5	// PCCLAMP document (not implemented yet)
	//case DOCTYPE_SAPID 		6	// SAPID document (not implemented yet)
	//case DOCTYPE_UNKNOWN		-1	// type of the document not accepted
	default:
		m_pXFile = new CDataFileX;
		break;
	}
}

// adjust size of the buffer for data read from file
// update buffer parameters
BOOL AcqDataDoc::AdjustBUF(int i_num_elements)
{
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;

	ASSERT(m_pWBuf != NULL);
	const auto p_wf = GetpWaveFormat();
	m_lDOCchanLength = p_wf->get_nb_points_sampled_per_channel();
	m_DOCnbchans = p_wf->scan_count;
	p_wf->duration = static_cast<float>(m_lDOCchanLength) / p_wf->sampling_rate_per_channel;
	m_lBUFSize = i_num_elements * p_wf->scan_count;

	m_lBUFchanSize = m_lBUFSize / static_cast<long>(p_wf->scan_count);
	m_lBUFchanFirst = 0;
	m_lBUFchanLast = m_lBUFchanSize - 1;
	m_lBUFSize = m_lBUFchanSize * p_wf->scan_count;
	m_lBUFmaxSize = m_lBUFSize * sizeof(short);

	// alloc RW buffer
	return m_pWBuf->createWBuffer(m_lBUFchanSize, p_wf->scan_count);
}

// allocate buffers to read data
// adjust size of the buffer according to MAX_BUFFER_LENGTH_AS_BYTES
BOOL AcqDataDoc::AllocBUF()
{
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;

	ASSERT(m_pWBuf != NULL); // check object created properly
	CWaveFormat* pwF = GetpWaveFormat();

	m_lDOCchanLength = pwF->get_nb_points_sampled_per_channel();
	m_DOCnbchans = pwF->scan_count;
	pwF->duration = static_cast<float>(m_lDOCchanLength) / pwF->sampling_rate_per_channel;

	const int i_num_elements = m_lDOCchanLength;
	if (i_num_elements * pwF->scan_count > m_lBUFSize)
		m_lBUFSize = i_num_elements * pwF->scan_count;

	if (m_lBUFSize > static_cast<long>(MAX_BUFFER_LENGTH_AS_BYTES / sizeof(short)))
		m_lBUFSize = MAX_BUFFER_LENGTH_AS_BYTES / sizeof(short);

	m_lBUFchanSize = m_lBUFSize / static_cast<long>(pwF->scan_count);
	m_lBUFchanFirst = 0;
	m_lBUFchanLast = m_lBUFchanSize - 1;
	m_lBUFSize = m_lBUFchanSize * pwF->scan_count;
	m_lBUFmaxSize = m_lBUFSize * sizeof(short);

	// alloc RW buffer
	return m_pWBuf->createWBuffer(m_lBUFchanSize, pwF->scan_count);
}

// returns the address of the raw data buffer
// and the number of interleaved channels
short* AcqDataDoc::LoadRawDataParams(int* nb_channels) const
{
	*nb_channels = GetScanCount();
	return m_pWBuf->getWBAdrRawDataBuf();
}

BOOL AcqDataDoc::LoadRawData(long* l_first, long* l_last, const int n_span)
{
	auto flag = TRUE;

	if ((*l_first - n_span < m_lBUFchanFirst)
		|| (*l_last + n_span > m_lBUFchanLast) || !m_bValidReadBuffer)
	{
		flag = readDataBlock(*l_first - n_span);
		m_bValidReadBuffer = TRUE;
		m_bValidTransfBuffer = FALSE;
	}

	*l_first = m_lBUFchanFirst + n_span;
	*l_last = m_lBUFchanLast;
	if (m_lBUFchanLast < m_lDOCchanLength - 1)
		*l_last -= n_span;

	return flag;
}

BOOL AcqDataDoc::readDataBlock(long l_first)
{
	// check limits
	if (l_first < 0)
		l_first = 0;
	// compute limits
	m_lBUFchanFirst = l_first;
	m_lBUFchanLast = m_lBUFchanFirst + m_lBUFchanSize - 1;
	if (m_lBUFchanLast > m_lDOCchanLength)
	{
		if (m_lDOCchanLength < m_lBUFchanSize)
		{
			m_lBUFchanSize = m_lDOCchanLength;
			m_lBUFchanLast = m_lBUFchanSize - 1;
		}
		m_lBUFchanFirst = m_lDOCchanLength - m_lBUFchanSize;
	}
	l_first = m_lBUFchanFirst * m_DOCnbchans;

	// reallocate buffer if needed
	if (m_pWBuf->GetWBNumElements() != m_lBUFchanSize)
		AllocBUF();

	// read data from file
	if (m_pXFile != nullptr)
	{
		short* p_buffer = m_pWBuf->getWBAdrRawDataBuf();
		ASSERT(p_buffer != NULL);
		//TRACE("ReadDataBlock l_first= %i nbPoints = %i doc length= %i \n", l_first, m_lBUFSize, m_lDOCchanLength);
		auto l_size = m_pXFile->ReadAdcData(l_first, m_lBUFSize * sizeof(short), p_buffer, GetpWavechanArray());
		//if (l_size == 0)
		//	return FALSE;

		// ugly patch:
		l_size = m_lBUFSize;
		m_lBUFchanLast = m_lBUFchanFirst + l_size / m_DOCnbchans - 1;

		// remove offset so that data are signed short (for offset binary data of 12 or 16 bits resolution)
		const auto w_bin_zero = static_cast<WORD>(m_pWBuf->m_waveFormat.binzero);
		if (m_bRemoveOffset && w_bin_zero != NULL)
		{
			auto* pw_buf = reinterpret_cast<WORD*>(m_pWBuf->getWBAdrRawDataBuf());
			for (long i = 0; i < l_size; i++, pw_buf++)
				*pw_buf -= w_bin_zero;
		}
		return TRUE;
	}
	return FALSE;
}

void AcqDataDoc::ReadDataInfos()
{
	ASSERT(m_pXFile != NULL);
	m_pXFile->ReadDataInfos(m_pWBuf);
	AllocBUF();
}

short AcqDataDoc::BGetVal(const int channel, const long l_index)
{
	if ((l_index < m_lBUFchanFirst) || (l_index > m_lBUFchanLast))
		readDataBlock(l_index);
	const int index = l_index - m_lBUFchanFirst;
	return *(m_pWBuf->getWBAdrRawDataElmt(channel, index));
}

short* AcqDataDoc::LoadTransfData(const long l_first, const long l_last, const int transform_type,
                                  const int source_channel)
{
	const BOOL b_already_done = (m_bValidTransfBuffer
		&& (l_first == m_tBUFfirst)
		&& (l_last == m_tBUFlast)
		&& (m_tBUFtransform == transform_type)
		&& (m_tBUFsourcechan == source_channel));
	m_tBUFtransform = transform_type;
	m_tBUFsourcechan = source_channel;
	m_tBUFfirst = l_first;
	m_tBUFlast = l_last;

	if (m_pWBuf->getWBAdrTransfData() == nullptr)
		m_pWBuf->InitWBTransformBuffer();

	const auto i_span = m_pWBuf->GetWBTransformSpan(transform_type);
	const auto l_span = static_cast<long>(i_span);

	// ASSERT make sure that all data requested are within the buffer ...
	ASSERT(!(l_first < m_lBUFchanFirst) && !(l_last > m_lBUFchanLast));
	if (((l_first - l_span) < m_lBUFchanFirst) || ((l_last + l_span) > m_lBUFchanLast)) // we should never get there
		readDataBlock(l_first - l_span); // but, just in case

	auto n_points = static_cast<int>(l_last - l_first + 1);
	const int n_channels = GetpWaveFormat()->scan_count;
	ASSERT(source_channel < n_channels); // make sure this is a valid channel
	const int i_offset = (l_first - m_lBUFchanFirst) * n_channels + source_channel;
	auto lp_source = m_pWBuf->getWBAdrRawDataBuf() + i_offset;

	// call corresponding one-pass routine
	auto lp_destination = m_pWBuf->getWBAdrTransfData();

	// check if source l_first can be used
	const auto b_isLFirstLower = (l_first < l_span);
	if (b_isLFirstLower) // no: skip these data and erase later corresp transform
	{
		lp_source += n_channels * i_span;
		lp_destination += i_span;
		n_points -= i_span;
	}

	// check if source l_last can be used
	const auto b_isLLastGreater = (l_last > m_lDOCchanLength - l_span);
	if (b_isLLastGreater) // no: skip these data and erase later corresp transform
	{
		n_points -= i_span;
	}

	if (n_points <= 0)
	{
		// erase data at the end of the buffer
		const int i_cx = i_span + l_last - l_first + 1;
		auto lp_dest0 = m_pWBuf->getWBAdrTransfData();
		for (auto cx = 0; cx < i_cx; cx++, lp_dest0++)
			*lp_dest0 = 0;
	}
	else if (!b_already_done)
	{
		switch (transform_type)
		{
		case 0: m_pWBuf->BCopy(lp_source, lp_destination, n_points);
			break;
		case 1: m_pWBuf->BDeriv(lp_source, lp_destination, n_points);
			break;
		case 2: m_pWBuf->BLanczo2(lp_source, lp_destination, n_points);
			break;
		case 3: m_pWBuf->BLanczo3(lp_source, lp_destination, n_points);
			break;
		case 4: m_pWBuf->BDeri1f3(lp_source, lp_destination, n_points);
			break;
		case 5: m_pWBuf->BDeri2f3(lp_source, lp_destination, n_points);
			break;
		case 6: m_pWBuf->BDeri2f5(lp_source, lp_destination, n_points);
			break;
		case 7: m_pWBuf->BDeri3f3(lp_source, lp_destination, n_points);
			break;
		case 8: m_pWBuf->BDiffer1(lp_source, lp_destination, n_points);
			break;
		case 9: m_pWBuf->BDiffer2(lp_source, lp_destination, n_points);
			break;
		case 10: m_pWBuf->BDiffer3(lp_source, lp_destination, n_points);
			break;
		case 11: m_pWBuf->BDiffer10(lp_source, lp_destination, n_points);
			break;
		case 12: m_pWBuf->BMovAvg30(lp_source, lp_destination, n_points);
			break;
		case 13: m_pWBuf->BMedian30(lp_source, lp_destination, n_points);
			break;
		case 14: m_pWBuf->BMedian35(lp_source, lp_destination, n_points);
			break;
		case 15: m_pWBuf->BRMS(lp_source, lp_destination, n_points);
		default: break;
		}

		// set undefined pts equal to first valid point
		if (b_isLFirstLower)
		{
			auto lp_dest0 = m_pWBuf->getWBAdrTransfData();
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
	m_bValidTransfBuffer = TRUE;
	return lp_destination;
}

BOOL AcqDataDoc::BuildTransfData(const int transform_type, const int source_channel)
{
	// make sure that transform buffer is ready
	if (m_pWBuf->getWBAdrTransfData() == nullptr)
		m_pWBuf->InitWBTransformBuffer();
	auto flag = TRUE;

	// init parameters
	auto lp_source = m_pWBuf->getWBAdrRawDataBuf() + source_channel;
	const int nb_channels = GetpWaveFormat()->scan_count;
	ASSERT(source_channel < nb_channels);

	// adjust pointers according to nspan - (fringe data) and set flags erase these data at the end
	auto lp_dest = m_pWBuf->getWBAdrTransfData();
	const auto i_span_between_points = m_pWBuf->GetWBTransformSpan(transform_type);
	const auto l_span_between_points = static_cast<long>(i_span_between_points);
	const auto l_first = m_lBUFchanFirst + l_span_between_points;
	const auto l_last = m_lBUFchanLast - l_span_between_points;
	lp_source += nb_channels * i_span_between_points;
	lp_dest += i_span_between_points;
	int n_points = l_last - l_first + 1;
	ASSERT(n_points > 0);

	switch (transform_type)
	{
	case 0: m_pWBuf->BCopy(lp_source, lp_dest, n_points);
		break;
	case 1: m_pWBuf->BDeriv(lp_source, lp_dest, n_points);
		break;
	case 2: m_pWBuf->BLanczo2(lp_source, lp_dest, n_points);
		break;
	case 3: m_pWBuf->BLanczo3(lp_source, lp_dest, n_points);
		break;
	case 4: m_pWBuf->BDeri1f3(lp_source, lp_dest, n_points);
		break;
	case 5: m_pWBuf->BDeri2f3(lp_source, lp_dest, n_points);
		break;
	case 6: m_pWBuf->BDeri2f5(lp_source, lp_dest, n_points);
		break;
	case 7: m_pWBuf->BDeri3f3(lp_source, lp_dest, n_points);
		break;
	case 8: m_pWBuf->BDiffer1(lp_source, lp_dest, n_points);
		break;
	case 9: m_pWBuf->BDiffer2(lp_source, lp_dest, n_points);
		break;
	case 10: m_pWBuf->BDiffer3(lp_source, lp_dest, n_points);
		break;
	case 11: m_pWBuf->BDiffer10(lp_source, lp_dest, n_points);
		break;
	case 12: m_pWBuf->BMovAvg30(lp_source, lp_dest, n_points);
		break;
	case 13: m_pWBuf->BMedian30(lp_source, lp_dest, n_points);
		break;
	case 14: m_pWBuf->BMedian35(lp_source, lp_dest, n_points);
		break;
	case 15: m_pWBuf->BRMS(lp_source, lp_dest, n_points);
	default: flag = FALSE;
		break;
	}

	// set undefined pts equal to first valid point
	if (i_span_between_points > 0)
	{
		auto lp_dest0 = m_pWBuf->getWBAdrTransfData();
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
		n_points += i_span_between_points;

		lp_dest0 = m_pWBuf->getWBAdrTransfData() + n_points;
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
	}
	return flag;
}

BOOL AcqDataDoc::CreateAcqFile(CString& cs_file_name)
{
	if (!cs_file_name.IsEmpty())
	{
		// Create file - send message if creation failed
		if (m_pXFile == nullptr)
		{
			m_pXFile = new CDataFileAWAVE;
			ASSERT(m_pXFile != NULL);
		}
		if (m_pXFile->m_idType != DOCTYPE_AWAVE)
		{
			delete m_pXFile;
			m_pXFile = new CDataFileAWAVE;
			ASSERT(m_pXFile != NULL);
		}

		if (!m_pXFile->OpenDataFile(cs_file_name, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone))
		//shareDenyNone
		{
			AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
			return FALSE;
		}
		m_pXFile->InitFile();
	}

	// create object as file
	AllocBUF();

	return TRUE;
}

BOOL AcqDataDoc::WriteHZtags(TagList* p_tags)
{
	if (p_tags == nullptr)
		p_tags = m_pWBuf->GetpHZtags();
	if (p_tags == nullptr || p_tags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteHZtags(p_tags);
}

BOOL AcqDataDoc::WriteVTtags(TagList* p_tags)
{
	if (p_tags == nullptr)
		p_tags = m_pWBuf->GetpVTtags();
	if (p_tags == nullptr || p_tags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteVTtags(p_tags);
}

BOOL AcqDataDoc::AcqDoc_DataAppendStart()
{
	// start from fresh?
	if (m_pXFile == nullptr || m_pXFile->m_idType != DOCTYPE_AWAVE)
	{
		delete m_pXFile;
		m_pXFile = new CDataFileAWAVE;
		ASSERT(m_pXFile != NULL);
	}
	return m_pXFile->DataAppendStart();
}

BOOL AcqDataDoc::AcqDoc_DataAppend(short* p_buffer, const UINT bytes_length) const
{
	return m_pXFile->DataAppend(p_buffer, bytes_length);
}

BOOL AcqDataDoc::AcqDoc_DataAppendStop()
{
	m_pXFile->DataAppendStop();
	const auto p_wave_format = GetpWaveFormat();
	p_wave_format->sample_count = static_cast<long>(m_pXFile->m_ulbytescount / sizeof(short));
	p_wave_format->duration = static_cast<float>(p_wave_format->sample_count) / static_cast<float>(p_wave_format->
			scan_count)
		/ p_wave_format->sampling_rate_per_channel;
	m_pXFile->WriteDataInfos(p_wave_format, GetpWavechanArray());
	m_pXFile->Flush();
	return TRUE;
}

BOOL AcqDataDoc::AcqSaveDataDescriptors()
{
	const auto flag = m_pXFile->WriteDataInfos(GetpWaveFormat(), GetpWavechanArray());
	m_pXFile->Flush();
	return flag;
}

void AcqDataDoc::AcqDeleteFile() const
{
	const auto cs_file_path = m_pXFile->GetFilePath();
	m_pXFile->CloseDataFile();
	CFile::Remove(cs_file_path);
}

void AcqDataDoc::AcqCloseFile() const
{
	if (m_pXFile != nullptr)
		m_pXFile->CloseDataFile();
}

BOOL AcqDataDoc::SaveAs(CString& new_name, BOOL b_check_over_write, const int i_type)
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
	auto cs_former_name = m_pXFile->GetFilePath();

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
	auto* p_new_doc = new AcqDataDoc;
	if (!p_new_doc->CreateAcqFile(dummy_name))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete p_new_doc;
		return FALSE;
	}

	// save data header
	if (GetpWaveFormat()->scan_count < GetpWavechanArray()->ChanArray_getSize())
	{
		const auto last_channel = GetpWaveFormat()->scan_count - 1;
		for (auto i = GetpWavechanArray()->ChanArray_getSize() - 1; i > last_channel; i--)
			GetpWavechanArray()->ChanArray_removeAt(i);
	}

	// save data
	p_new_doc->AcqDoc_DataAppendStart();
	auto n_samples = GetpWaveFormat()->sample_count;

	// position source file index to start of data
	m_pXFile->Seek(m_pXFile->m_ulOffsetData, CFile::begin);
	auto p_buf = m_pWBuf->getWBAdrRawDataBuf(); // buffer to store data
	auto l_buf_size = m_lBUFSize; // length of the buffer

	while (n_samples > 0) // loop until the end of the file
	{
		// read data from source file into buffer
		if (n_samples < m_lBUFSize) // adjust buftempsize
			l_buf_size = n_samples; // then, store data in temporary buffer
		const long n_bytes = l_buf_size * sizeof(short);
		m_pXFile->Read(p_buf, n_bytes);
		if (i_type == 3) // ASD file
		{
			auto p_buf2 = p_buf;
			for (long i = 0; i < n_bytes; i++)
			{
				*p_buf2 = *p_buf2 / 2;
				p_buf2 += 2;
			}
		}
		// save buffer
		if (!p_new_doc->AcqDoc_DataAppend(p_buf, n_bytes))
			break;
		n_samples -= l_buf_size; // update counter and loop
	}

	// stop appending data, update dependent struct
	p_new_doc->AcqDoc_DataAppendStop();

	// save other objects if exist (tags, others)

	if (m_pWBuf->GetpHZtags()->GetNTags() > 0)
		p_new_doc->WriteHZtags(m_pWBuf->GetpHZtags());
	if (m_pWBuf->GetpVTtags()->GetNTags() > 0)
		p_new_doc->WriteVTtags(m_pWBuf->GetpVTtags());

	// if destination name == source: remove source and rename destination
	if (b_is_duplicate_name)
	{
		// copy dummy name into csFormerName
		const auto dw_new_length = p_new_doc->m_pXFile->GetLength();
		m_pXFile->SetLength(dw_new_length);
		m_pXFile->SeekToBegin();
		p_new_doc->m_pXFile->SeekToBegin();

		p_buf = m_pWBuf->getWBAdrRawDataBuf(); // buffer to store data
		const auto n_bytes = m_lBUFSize * sizeof(short);
		DWORD dw_read;
		do
		{
			dw_read = p_new_doc->m_pXFile->Read(p_buf, n_bytes);
			m_pXFile->Write(p_buf, dw_read);
		}
		while (dw_read > 0);

		// file is transferred, destroy temporary file
		p_new_doc->m_pXFile->CloseDataFile();
		//CFile::Remove(dummyName);

		// delete current file object and open saved-as file ??
		m_pXFile->CloseDataFile();
		SAFE_DELETE(m_pXFile)

		// create / update CDataFileX associated object
		m_pXFile = new CDataFileAWAVE; //((CFile*) this);
		ASSERT(m_pXFile != NULL);

		// open saved file
		if (!m_pXFile->OpenDataFile(new_name, CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary))
			return FALSE;
	}

	// destroy object created
	delete p_new_doc;

	return TRUE;
}
