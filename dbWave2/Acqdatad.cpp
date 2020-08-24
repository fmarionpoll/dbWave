// Acqdatad.cpp : implementation file
//

#include "StdAfx.h"

#include "Acqparam.h"		// data acquisition struct: wave format, wave chans
#include "Taglines.h"		// tags
#include "Acqdatad.h"

#include "datafile_Atlab.h"
#include "datafile_ASD.h"
#include "datafile_mcid.h"
#include "importgenericdatadlg.h"
#include "datafile_Awave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_BUFFER_LENGTH_AS_BYTES  614400//102400
//  1 s at 10 kHz =  1(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 60 000
// 10 s at 10 kHz = 10(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 600 000
// with a multiple of 1024 =  614400

/////////////////////////////////////////////////////////////////////////////
// CAcqDataDoc

IMPLEMENT_DYNCREATE(CAcqDataDoc, CDocument)

BEGIN_MESSAGE_MAP(CAcqDataDoc, CDocument)

END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////
// Construction / Destruction

//**************************************************************************
//function:  CAcqDataDoc()
//purpose: Create a new CAcqDataDoc object
//parameters: none
//returns: none
//comments: constructor used by dynamic creation
// **************************************************************************/

CAcqDataDoc::CAcqDataDoc()
{
	m_bdefined = FALSE;		// data are not defined

	m_lDOCchanLength = 0;	// file chan data length
	m_DOCnbchans = 0;			// nb chans
	m_lBUFchanSize = 0;		// size of the read buffer
	m_lBUFchanFirst = 0;	// index first point
	m_lBUFchanLast = 0;		// index last point
	m_lBUFSize = 0;			// size of the read buffer (?)
	m_lBUFmaxSize = MAX_BUFFER_LENGTH_AS_BYTES; // max buf size
	m_iOffsetInt = sizeof(short);	// offset in bytes
	m_tBUFfirst = 0;
	m_tBUFlast = 1;
	m_tBUFtransform = 0;
	m_tBUFsourcechan = 0;

	m_pWBuf = nullptr;
	m_pXFile = nullptr;
	m_bValidReadBuffer = FALSE;
	m_bValidTransfBuffer = FALSE;
	m_bRemoveOffset = TRUE;
}

/**************************************************************************
 function:  ~CAcqDataDoc()
 purpose: Remove from memory an CAcqDataDoc object
 parameters: none
 returns: none
 comments:
 **************************************************************************/

CAcqDataDoc::~CAcqDataDoc()
{
	// delete structures created
	SAFE_DELETE(m_pWBuf);
	SAFE_DELETE(m_pXFile);
}

////////////////////////////////////////////////////////////////////////////
// Implementation

/**************************************************************************
 function:  OnSaveDocument(const char* pszName)
 purpose: Save the file named pszName
 parameters:
	_ const char* pszName : name of the file to be saved
 returns: TRUE if operation was successful, else returns FALSE
 comments:
 **************************************************************************/

BOOL CAcqDataDoc::OnSaveDocument(CString& szPathName)
{
	const BOOL flag = SaveAs(szPathName, FALSE);
	if (flag)
		SetModifiedFlag(FALSE);	// mark the document as clean

	return flag;
}

/**************************************************************************
 function:  OnOpenDocument(CString &szPathName)
 purpose: Open the file called szPathName
 parameters:
	_ CString &szPathName : Name of the file to be opened
 returns: TRUE if operation was successful, else returns FALSE
 comments:
 **************************************************************************/

BOOL CAcqDataDoc::OnOpenDocument(CString& sz_path_name)
{
	// close data file if already opened
	if (m_pXFile != nullptr
		&& m_pXFile->m_hFile != CFile::hFileNull)
	{
		auto filename = m_pXFile->GetFileName();
		if (filename.CompareNoCase(sz_path_name) == 0)
			return TRUE;
		m_pXFile->Close();
	}
	// set file reading buffer as dirty
	m_bValidReadBuffer = FALSE;

	// check if file can be opened - exit if it can't and return an empty object
	CFileStatus r_status;
	const auto b_open = CFile::GetStatus(sz_path_name, r_status);
	if (!b_open || r_status.m_size <= 4096)	// avoid to open 1kb files ...
	{
		SAFE_DELETE(m_pXFile);				// delete data file object if any
		return FALSE;						// and return
	}
	ASSERT(sz_path_name.Right(3) != _T("del"));

	// check file type (we are now sure that the file exists)
	auto b_found_match = OpenAcqFile(sz_path_name);

	// the format of the data file was not recognized - see if we can import
	if (b_found_match < 0)
	{
		auto p_dlg = new CImportGenericDataDlg;

		// init parameters
		auto cs_array = new CStringArray;				// dlg needs an array of strings
		ASSERT(cs_array != NULL);
		cs_array->Add(sz_path_name);
		p_dlg->m_pfilenameArray = cs_array;			// pass address of array
		p_dlg->bConvert = TRUE;						// tell that conversion is allowed
		auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());	// get pointer to application
		p_dlg->piivO = &(p_app->options_import);
		m_pXFile->Close();							// close file
		SAFE_DELETE(m_pXFile);						// delete object

		// call dialog
		const auto result = p_dlg->DoModal();				// start dialog
		delete p_dlg;
		if (IDOK != result || 0 == cs_array->GetSize())
			return FALSE;

		// change name of files here (rename)
		auto filename_old = sz_path_name;
		const auto count = filename_old.ReverseFind('\\') + 1;
		filename_old.Insert(count, _T("OLD_"));

		// check if same file already exist
		CFileStatus status;	// file status: time creation, ...
		auto b_flag_exist = CFile::GetStatus(filename_old, status);
		if (b_flag_exist != 0)
			CFile::Remove(filename_old);
		try
		{
			CFile::Rename(sz_path_name, filename_old);
		}
		catch (CFileException* pEx)
		{
			CString cs;
			cs.Format(_T("File not found, cause = %i\n"), pEx->m_cause);
			cs = sz_path_name + cs;
			ATLTRACE2(cs);
			pEx->Delete();
		}

		filename_old = cs_array->GetAt(0);
		b_flag_exist = CFile::GetStatus(sz_path_name, status);

		if (b_flag_exist != 0)
			CFile::Remove(sz_path_name);
		try
		{
			CFile::Rename(filename_old, sz_path_name);
		}
		catch (CFileException* pEx)
		{
			CString cs;
			cs.Format(_T("File not found, cause = %i\n"), pEx->m_cause);
			cs = sz_path_name + cs;
			ATLTRACE2(cs);
			pEx->Delete();
		}

		delete cs_array;

		m_pXFile = new CDataFileAWAVE;					// create an aWave data file
		ASSERT(m_pXFile != NULL);					// check validity
		b_found_match = OpenAcqFile(sz_path_name);		// open corresponding file
	}
	return b_found_match;
}

// open data file and load first data block
int CAcqDataDoc::CheckFileTypeFromName(CString& sz_path_name)
{
	// open file using the last object type used or the default type
	CFileException fe;
	if (m_pXFile != nullptr && m_pXFile->m_hFile != CFile::hFileNull)	// close if file currently opened
		m_pXFile->Close();
	CFileStatus r_status;
	const auto flag = CFile::GetStatus(sz_path_name, r_status);
	if (!flag)
		return flag;

	// create standard object if p file is null
	if (m_pXFile == nullptr)
		m_pXFile = new CDataFileAWAVE;

	// open file
	UINT u_open_flag = (r_status.m_attribute & 0x01) ? CFile::modeRead : CFile::modeReadWrite;
	u_open_flag |= CFile::shareDenyNone | CFile::typeBinary;//, &fe;
	if (!m_pXFile->Open(sz_path_name, u_open_flag))
	{
		m_pXFile->Abort();
		return false;
	}

	// create buffer
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;
	ASSERT(m_pWBuf != NULL);	// check object created properly

	// read data & create object according to file signature (checkfiletype)
	const auto i_id = CheckFileType(m_pXFile);
	m_pXFile->Close();
	return i_id;
}

BOOL CAcqDataDoc::OpenAcqFile(CString& cs_filename)
{
	// open file using the last object type used or the default type
	CFileException fe;
	if (m_pXFile != nullptr && m_pXFile->m_hFile != CFile::hFileNull)	// close if file currently opened
		m_pXFile->Close();
	CFileStatus r_status;
	const auto flag = CFile::GetStatus(cs_filename, r_status);

	// what if file not found? exit!
	if (!flag)
		return flag;

	// create standard object if pxfile is null
	if (m_pXFile == nullptr)
		m_pXFile = new CDataFileAWAVE;

	// open file
	UINT u_open_flag = (r_status.m_attribute & 0x01) ? CFile::modeRead : CFile::modeReadWrite;
	u_open_flag |= CFile::shareDenyNone | CFile::typeBinary; // , &fe;
	if (!m_pXFile->Open(cs_filename, u_open_flag))
	{
		m_pXFile->Abort();
		return FALSE;
	}

	// create buffer
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;
	ASSERT(m_pWBuf != NULL);	// check object created properly
	const auto p_chan_array = GetpWavechanArray();
	const auto p_wave_format = GetpWaveFormat();

	// read data & create object according to file signature (checkfiletype)
	const auto i_id = CheckFileType(m_pXFile);
	if (m_pXFile == nullptr || m_pXFile->m_idType != i_id)
	{
		SAFE_DELETE(m_pXFile);
		switch (i_id)
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
			//case DOCTYPE_PCCLAMP		5	// PCCLAMP document (not implemented yet)
			//case DOCTYPE_SAPID 		6	// SAPID document (not implemented yet)
			//case DOCTYPE_UNKNOWN		-1	// type of the document not accepted
		default:
			m_pXFile = new CDataFileX;
			break;
		}
		m_pXFile->Open(cs_filename, u_open_flag);	// open file again, this time with using the correct object
	}

	// return with error if format not known
	if (i_id == DOCTYPE_UNKNOWN)
	{
		AllocBUF();
		return -1;
	}

	// save file pointer, read data header and Tags
	const auto b_flag = m_pXFile->ReadDataInfos(p_wave_format, p_chan_array);
	AllocBUF();
	m_pXFile->ReadVTtags(&m_vt_tags);
	m_pXFile->ReadHZtags(&m_hz_tags);

	return b_flag;
}

/**************************************************************************
 function:  OnNewDocument()
 purpose: Create a new document
 parameters: none
 returns: TRUE if operation was successful, else returns FALSE
 comments:
 **************************************************************************/

BOOL CAcqDataDoc::OnNewDocument()
{
	//if (!CDocument::OnNewDocument())
	//	return FALSE;
	DeleteContents();
	m_strPathName.Empty();      // no path name yet
	SetModifiedFlag(FALSE);     // make clean
	//OnDocumentEvent(onAfterNewDocument); // this call causes a crash as the template is not defined for CAcqDataDoc

	if (m_pWBuf == nullptr)
	{
		CString cs_dummy;
		cs_dummy.Empty();
		CreateAcqFile(cs_dummy);
		ASSERT(m_pWBuf != NULL);
	}
	m_pWBuf->WBDatachanSetnum(1);	// create at least one channel
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Export Data

/**************************************************************************
 function: GetFileInfos
 purpose:
 parameters:
 returns:
 comments:
 **************************************************************************/

CString CAcqDataDoc::GetDataFileInfos(OPTIONS_VIEWDATA* pVD)
{
	const CString sep('\t');

	CString cs_dummy;
	auto cs_out = GetPathName();
	cs_out.MakeLower();					// lower case

	const auto p_wave_format = GetpWaveFormat();

	// date and time
	if (pVD->bacqdate)
		cs_out += (p_wave_format->acqtime).Format("\t%#d %B %Y");
	if (pVD->bacqtime)
		cs_out += (p_wave_format->acqtime).Format("\t%X");

	// file size
	if (pVD->bfilesize)                                      // file size
	{
		cs_dummy.Format(_T("\t%-10li"), GetDOCchanLength());
		cs_out += cs_dummy;
		cs_dummy.Format(_T("\t nchans=%i\t"), p_wave_format->scan_count);
		cs_out += cs_dummy;
	}

	if (pVD->bacqcomments) {
		cs_dummy = p_wave_format->GetComments(sep);
		cs_out += cs_dummy;
	}

	if (pVD->bacqchcomment || pVD->bacqchsetting)
	{
		CString cs;
		for (auto i_chan = 0; i_chan < p_wave_format->scan_count; i_chan++)
		{
			const auto p_chan = (GetpWavechanArray())->get_p_channel(i_chan);
			if (pVD->bacqchcomment)
				cs_out += sep + p_chan->am_csComment;
			if (pVD->bacqchsetting)
			{
				cs.Format(_T("\theadstage=%s\tgain=%.0f\tfilter= %s\t%i Hz"),
					static_cast<LPCTSTR>(p_chan->am_csheadstage),
					p_chan->am_gaintotal,
					static_cast<LPCTSTR>(p_chan->am_csInputpos),
					p_chan->am_lowpass);
				cs_out += cs;
			}
		}
	}
	return cs_out;
}

void CAcqDataDoc::ExportDataFile_to_TXTFile(CStdioFile* pdataDest)
{
	const _TCHAR sep = '\n';	// lines separator "\n\r"
	const _TCHAR sep2 = ',';	// columns separator

	auto cs_out = _T("filename=") + GetPathName();
	cs_out.MakeLower();					// lower case
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

	cs_out.Format(_T("sampling rate(Hz)= %f"), p_wave_format->chrate);
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
		const auto p_chan = (GetpWavechanArray())->get_p_channel(i_chan);
		cs_out += p_chan->am_csComment;
		if (i_chan < p_wave_format->scan_count - 1)
			cs_out += sep2;
	}
	pdataDest->WriteString(cs_out);

	// loop to read actual data
	ReadDataBlock(0);
	const auto mv_factor = p_wave_format->fullscale_Volts / p_wave_format->binspan * 1000.;
	for (long j = 0; j < GetDOCchanLength(); j++)
	{
		CString cs;
		cs_out = sep;
		for (auto channel = 0; channel < p_wave_format->scan_count; channel++)
		{
			const auto p_chan = (GetpWavechanArray())->get_p_channel(channel);
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

/////////////////////////////////////////////////////////////////////////////
// CAcqDataDoc diagnostic
#ifdef _DEBUG
void CAcqDataDoc::AssertValid() const
{
	CDocument::AssertValid();
}
void CAcqDataDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////
// check file type

int CAcqDataDoc::CheckFileType(CFile* f) const
{
	auto i_id = DOCTYPE_UNKNOWN;
	if (m_pXFile != nullptr)
	{
		i_id = m_pXFile->CheckFileType(f);
		if (i_id != DOCTYPE_UNKNOWN)
			return m_pXFile->m_idType;	// return file type ID
	}

	// not of current type
	// check if ATLAB file
	if (i_id < 0)
	{
		auto* p_file = new (CDataFileATLAB);
		ASSERT(p_file != NULL);
		i_id = p_file->CheckFileType(f);
		delete p_file;
	}

	// check if aWave file
	if (i_id < 0)
	{
		auto* pFileX = new (CDataFileAWAVE);
		ASSERT(pFileX != NULL);
		i_id = pFileX->CheckFileType(f);
		delete pFileX;
	}

	// check if ASD Syntech file
	if (i_id < 0)
	{
		auto* pFileX = new (CDataFileASD);
		ASSERT(pFileX != NULL);
		i_id = pFileX->CheckFileType(f);
		delete pFileX;
	}

	// check if MCID Halifax file
	if (i_id < 0)
	{
		auto* pFileX = new (CDataFileMCID);
		ASSERT(pFileX != NULL);
		i_id = pFileX->CheckFileType(f);
		delete pFileX;
	}

	return i_id;
}

// adjust size of the buffer for data read from file
// update buffer parameters
BOOL CAcqDataDoc::AdjustBUF(int i_num_elements)
{
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;

	ASSERT(m_pWBuf != NULL);
	const auto p_wf = GetpWaveFormat();
	m_lDOCchanLength = p_wf->sample_count / static_cast<long>(p_wf->scan_count);
	m_DOCnbchans = p_wf->scan_count;
	p_wf->duration = static_cast<float>(m_lDOCchanLength) / p_wf->chrate;
	m_lBUFSize = i_num_elements * p_wf->scan_count;

	m_lBUFchanSize = m_lBUFSize / static_cast<long>(p_wf->scan_count);
	m_lBUFchanFirst = 0;
	m_lBUFchanLast = m_lBUFchanSize - 1;
	m_lBUFSize = m_lBUFchanSize * p_wf->scan_count;
	m_lBUFmaxSize = m_lBUFSize * sizeof(short);

	// alloc RW buffer
	return m_pWBuf->CreateWBuffer(m_lBUFchanSize, p_wf->scan_count);
}

// allocate buffers to read data
// adjust size of the buffer according to MAX_BUFFER_LENGTH_AS_BYTES
BOOL CAcqDataDoc::AllocBUF()
{
	if (m_pWBuf == nullptr)
		m_pWBuf = new CWaveBuf;

	ASSERT(m_pWBuf != NULL);	// check object created properly
	CWaveFormat* pwF = GetpWaveFormat();
	m_lDOCchanLength = pwF->sample_count / static_cast<long>(pwF->scan_count);
	m_DOCnbchans = pwF->scan_count;
	pwF->duration = static_cast<float>(m_lDOCchanLength) / pwF->chrate;

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
	return m_pWBuf->CreateWBuffer(m_lBUFchanSize, pwF->scan_count);
}

// returns the address of the raw data buffer
// and the number of interleaved channels
short* CAcqDataDoc::LoadRawDataParams(int* nb_channels)
{
	*nb_channels = GetScanCount();
	return m_pWBuf->GetWBAdrRawDataBuf();
}

/**************************************************************************
 function:		LoadRawData (long* l_first, long* l_last, short n_span)

 purpose:		load data from file into memory buffer
 parameters:
	_ l_first = file channel index of first pt (address of variable)
	_ l_last = file channel index of last point (address of variable)
	_ n_span = try having n_span points before l_first and n_span after l_last
			  (necessary for transforms)
 returns:		l_first , l_last: chan file indexes of first and last pt
				buffer: data loaded from file
				TRUE if no error
 comments:      load as much data as possible within buffer
 **************************************************************************/
BOOL CAcqDataDoc::LoadRawData(long* l_first, long* l_last, const int n_span)
{
	auto flag = TRUE;

	if ((*l_first - n_span < m_lBUFchanFirst)
		|| (*l_last + n_span > m_lBUFchanLast) || !m_bValidReadBuffer)
	{
		flag = ReadDataBlock(*l_first - n_span);
		m_bValidReadBuffer = TRUE;
		m_bValidTransfBuffer = FALSE;
	}

	*l_first = m_lBUFchanFirst + n_span;
	*l_last = m_lBUFchanLast;
	if (m_lBUFchanLast < m_lDOCchanLength - 1)
		*l_last -= n_span;

	return flag;
}

/**************************************************************************
 function:	ReadDataBlock(long l_first)
 purpose:	Read a data block from a given file offset
 parameters:
	_ l_first =
 return:	FALSE if something is wrong;
			TRUE & update m_lBUFchanFirst, m_lBUFchanLast
 comments:
 **************************************************************************/

BOOL CAcqDataDoc::ReadDataBlock(long l_first)
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
	l_first = m_lBUFchanFirst * m_DOCnbchans;;

	// reallocate buffer if needed
	if (m_pWBuf->GetWBNumElements() != m_lBUFchanSize)
		AllocBUF();

	// read data from file
	if (m_pXFile != nullptr)
	{
		short* p_buffer = m_pWBuf->GetWBAdrRawDataBuf();
		ASSERT(p_buffer != NULL);
		const auto l_size = m_pXFile->ReadData(l_first, m_lBUFSize * sizeof(short), p_buffer);
		m_lBUFchanLast = m_lBUFchanFirst + l_size / m_DOCnbchans - 1;

		// remove offset so that data are signed short (for offset binary data of 12 or 16 bits resolution)
		const auto w_bin_zero = static_cast<WORD>(m_pWBuf->m_waveFormat.binzero);
		if (m_bRemoveOffset && w_bin_zero != NULL)
		{
			auto* pw_buf = reinterpret_cast<WORD*>(m_pWBuf->GetWBAdrRawDataBuf());
			for (long i = 0; i < l_size; i++, pw_buf++)
				*pw_buf -= w_bin_zero;
		}
		return TRUE;
	}
	else
		return FALSE;
}

// read data infos from the file descriptor
// and update data buffer size accordingly
void CAcqDataDoc::ReadDataInfos()
{
	ASSERT(m_pXFile != NULL);
	m_pXFile->ReadDataInfos(GetpWaveFormat(), GetpWavechanArray());
	AllocBUF();
}

/**************************************************************************
 function:		BGetVal (short channel, long lIndex)
 purpose: Returns value and refreshes data if necessery
 parameters:
	_ short channel : channel of the current plot
	_ long lIndex : index of the current plot
 returns:
 comments:
 **************************************************************************/

int CAcqDataDoc::BGetVal(const int channel, const long l_index)
{
	if ((l_index < m_lBUFchanFirst) || (l_index > m_lBUFchanLast))
		ReadDataBlock(l_index);
	const int index = l_index - m_lBUFchanFirst;
	return *(m_pWBuf->GetWBAdrRawDataElmt(channel, index));
}

/**************************************************************************
 function:		LoadTransfData (long* l_first, long* l_last, short transformType, short chan)

 purpose:		load data from file into memory buffer and store given transform
				into transform buffer
 parameters:	l_first = file channel index of first pt (address of variable)
				l_last = file channel index of last point (address of variable)
				transformType = index to transformation request
				sourcechan = source channel

 returns:		l_first , l_last: chan file indexes of first and last pt
				buffer: data loaded from file

 comments:      load as many data as possible within buffer
 **************************************************************************/

short* CAcqDataDoc::LoadTransfData(const long l_first, const long l_last, const int transform_type, const int source_channel)
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

	if (m_pWBuf->GetWBAdrTransfData() == nullptr)
		m_pWBuf->InitWBTransformBuffer();

	const auto i_span = m_pWBuf->GetWBTransformSpan(transform_type);
	const auto l_span = static_cast<long>(i_span);

	// ASSERT make sure that all data requested are within the buffer ...
	ASSERT(!(l_first < m_lBUFchanFirst) && !(l_last > m_lBUFchanLast));
	if (((l_first - l_span) < m_lBUFchanFirst) || ((l_last + l_span) > m_lBUFchanLast)) // we should never get there
		ReadDataBlock(l_first - l_span);							// but, just in case

	auto n_points = static_cast<int>(l_last - l_first + 1);
	const int n_channels = GetpWaveFormat()->scan_count;
	ASSERT(source_channel < n_channels);											// make sure this is a valid channel
	const int i_offset = (l_first - m_lBUFchanFirst) * n_channels + source_channel;
	auto lp_source = m_pWBuf->GetWBAdrRawDataBuf() + i_offset;

	// call corresponding one-pass routine
	auto lp_destination = m_pWBuf->GetWBAdrTransfData();

	// check if source l_first can be used
	const auto b_isLFirstLower = (l_first < l_span);
	if (b_isLFirstLower)	// no: skip these data and erase later corresp transform
	{
		lp_source += n_channels * i_span;
		lp_destination += i_span;
		n_points -= i_span;
	}

	// check if source l_last can be used
	const auto b_isLLastGreater = (l_last > m_lDOCchanLength - l_span);
	if (b_isLLastGreater)	// no: skip these data and erase later corresp transform
	{
		n_points -= i_span;
	}

	if (n_points <= 0)
	{
		// erase data at the end of the buffer
		const int i_cx = i_span + l_last - l_first + 1;
		auto lp_dest0 = m_pWBuf->GetWBAdrTransfData();
		for (auto cx = 0; cx < i_cx; cx++, lp_dest0++)
			*lp_dest0 = 0;
	}
	else if (!b_already_done)
	{
		switch (transform_type)
		{
		case 0:  m_pWBuf->BCopy(lp_source, lp_destination, n_points); break;
		case 1:	 m_pWBuf->BDeriv(lp_source, lp_destination, n_points); break;
		case 2:	 m_pWBuf->BLanczo2(lp_source, lp_destination, n_points); break;
		case 3:	 m_pWBuf->BLanczo3(lp_source, lp_destination, n_points); break;
		case 4:	 m_pWBuf->BDeri1f3(lp_source, lp_destination, n_points); break;
		case 5:	 m_pWBuf->BDeri2f3(lp_source, lp_destination, n_points); break;
		case 6:	 m_pWBuf->BDeri2f5(lp_source, lp_destination, n_points); break;
		case 7:	 m_pWBuf->BDeri3f3(lp_source, lp_destination, n_points); break;
		case 8:	 m_pWBuf->BDiffer1(lp_source, lp_destination, n_points); break;
		case 9:	 m_pWBuf->BDiffer2(lp_source, lp_destination, n_points); break;
		case 10: m_pWBuf->BDiffer3(lp_source, lp_destination, n_points); break;
		case 11: m_pWBuf->BDiffer10(lp_source, lp_destination, n_points); break;
		case 12: m_pWBuf->BMovAvg30(lp_source, lp_destination, n_points); break;
		case 13: m_pWBuf->BMedian30(lp_source, lp_destination, n_points); break;
		case 14: m_pWBuf->BMedian35(lp_source, lp_destination, n_points); break;
		case 15: m_pWBuf->BRMS(lp_source, lp_destination, n_points);
		default: break;
		}

		// set undefined pts equal to first valid point
		if (b_isLFirstLower)
		{
			auto lp_dest0 = m_pWBuf->GetWBAdrTransfData();
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

/**************************************************************************
 function:		BuildTransfData (int transformType, int chan)

 purpose:		build transf data from data loaded within raw data buffer
				store into transform buffer
 parameters:	m_lBUFchanFirst = file channel index of first pt (assume data buffer is valid)
				m_lBUFchanLast = file channel index of last point (assume data buffer is valid)
				transformType = index to transformation request
				sourcechan = source channel

 returns:		BOOL flag = true if operation completed

 comments:
 **************************************************************************/

BOOL CAcqDataDoc::BuildTransfData(const int transform_type, const int source_channel)
{
	// make sure that transform buffer is ready
	if (m_pWBuf->GetWBAdrTransfData() == nullptr)
		m_pWBuf->InitWBTransformBuffer();
	auto flag = TRUE;

	// init parameters
	auto lp_source = m_pWBuf->GetWBAdrRawDataBuf() + source_channel;
	const int nb_channels = GetpWaveFormat()->scan_count;
	ASSERT(source_channel < nb_channels);

	// adjust pointers according to nspan - (fringe data) and set flags erase these data at the end
	auto lp_dest = m_pWBuf->GetWBAdrTransfData();
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
	case 0:  m_pWBuf->BCopy(lp_source, lp_dest, n_points); break;
	case 1:	 m_pWBuf->BDeriv(lp_source, lp_dest, n_points); break;
	case 2:	 m_pWBuf->BLanczo2(lp_source, lp_dest, n_points); break;
	case 3:	 m_pWBuf->BLanczo3(lp_source, lp_dest, n_points); break;
	case 4:	 m_pWBuf->BDeri1f3(lp_source, lp_dest, n_points); break;
	case 5:	 m_pWBuf->BDeri2f3(lp_source, lp_dest, n_points); break;
	case 6:	 m_pWBuf->BDeri2f5(lp_source, lp_dest, n_points); break;
	case 7:	 m_pWBuf->BDeri3f3(lp_source, lp_dest, n_points); break;
	case 8:	 m_pWBuf->BDiffer1(lp_source, lp_dest, n_points); break;
	case 9:	 m_pWBuf->BDiffer2(lp_source, lp_dest, n_points); break;
	case 10: m_pWBuf->BDiffer3(lp_source, lp_dest, n_points); break;
	case 11: m_pWBuf->BDiffer10(lp_source, lp_dest, n_points); break;
	case 12: m_pWBuf->BMovAvg30(lp_source, lp_dest, n_points); break;
	case 13: m_pWBuf->BMedian30(lp_source, lp_dest, n_points); break;
	case 14: m_pWBuf->BMedian35(lp_source, lp_dest, n_points); break;
	case 15: m_pWBuf->BRMS(lp_source, lp_dest, n_points);
	default: flag = FALSE; break;
	}

	// set undefined pts equal to first valid point
	if (i_span_between_points > 0)
	{
		auto lp_dest0 = m_pWBuf->GetWBAdrTransfData();
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
		n_points += i_span_between_points;

		lp_dest0 = m_pWBuf->GetWBAdrTransfData() + n_points;
		for (auto cx = i_span_between_points; cx > 0; cx--, lp_dest0++)
			*lp_dest0 = 0;
	}
	return flag;
}

//////////////////////////////////////////////////////////////////////////////
// awave file
// create new awave file
/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::CreateAcqFile(CString& cs_file_name)
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

		CFileException fe;

		if (!m_pXFile->Open(cs_file_name, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone, &fe)) //shareDenyNone
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

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::WriteHZtags(CTagList* p_tags)
{
	if (p_tags == nullptr)
		p_tags = &m_hz_tags;
	if (p_tags == nullptr || p_tags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteHZtags(p_tags);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::WriteVTtags(CTagList* p_tags)
{
	if (p_tags == nullptr)
		p_tags = &m_vt_tags;
	if (p_tags == nullptr || p_tags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteVTtags(p_tags);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppendStart()
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

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppend(short* p_buffer, const UINT bytes_length) const
{
	return m_pXFile->DataAppend(p_buffer, bytes_length);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppendStop()
{
	m_pXFile->DataAppendStop();
	const auto p_wave_format = GetpWaveFormat();
	p_wave_format->sample_count = static_cast<long>(m_pXFile->m_ulbytescount / sizeof(short));
	p_wave_format->duration = static_cast<float>(p_wave_format->sample_count) / static_cast<float>(p_wave_format->scan_count)
		/ p_wave_format->chrate;
	m_pXFile->WriteDataInfos(p_wave_format, GetpWavechanArray());
	m_pXFile->Flush();
	return TRUE;
}

BOOL CAcqDataDoc::AcqSaveDataDescriptors()
{
	const auto flag = m_pXFile->WriteDataInfos(GetpWaveFormat(), GetpWavechanArray());
	m_pXFile->Flush();
	return flag;
}

// delete file on the disk
void CAcqDataDoc::AcqDeleteFile() const
{
	const auto cs_file_path = m_pXFile->GetFilePath();
	m_pXFile->Close();
	CFile::Remove(cs_file_path);
}

void CAcqDataDoc::AcqCloseFile() const
{
	if (m_pXFile != nullptr && m_pXFile->m_hFile != CFile::hFileNull)
		m_pXFile->Close();
}

/////////////////////////////////////////////////////////////////////////////
// save current data file under a new name
// default : save under awave format
// (other options not implemented yet)
// if "overwrite" (saveas is the same name than the source file:
//    create new file, copy into old file, delete new file

BOOL CAcqDataDoc::SaveAs(CString& new_name, BOOL b_check_over_write, const int i_type)
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
	CFileStatus r_status;	// file status: time creation, ...
	const auto b_file_already_exists = CFile::GetStatus(new_name, r_status);
	if (b_file_already_exists != 0 && b_check_over_write)
	{
		auto prompt = new_name;
		prompt += _T("\nThis file already exists.\nReplace existing file?");
		if (AfxMessageBox(prompt, MB_YESNO) != IDYES)
			return FALSE;       // don't continue
	}

	// check if collision with current file
	auto b_is_duplicate_name = FALSE;
	if (b_file_already_exists &&
		cs_former_name.CompareNoCase(r_status.m_szFullName) == 0) // used in conjunction with CFile::GetStatus()
	{
		b_is_duplicate_name = TRUE;
		dummy_name = new_name;
		dummy_name += _T("_tmp.dat");
	}

	// create new file
	auto* p_new_doc = new CAcqDataDoc;
	if (!p_new_doc->CreateAcqFile(dummy_name))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete p_new_doc;
		return FALSE;
	}

	// save data header
	if (GetpWaveFormat()->scan_count < GetpWavechanArray()->channel_get_number())
	{
		const auto last_channel = GetpWaveFormat()->scan_count - 1;
		for (auto i = GetpWavechanArray()->channel_get_number() - 1; i > last_channel; i--)
			GetpWavechanArray()->channel_remove(i);
	}

	// save data
	p_new_doc->AcqDoc_DataAppendStart();
	auto n_samples = GetpWaveFormat()->sample_count;

	// position source file index to start of data
	m_pXFile->Seek(m_pXFile->m_ulOffsetData, CFile::begin);
	auto p_buf = m_pWBuf->GetWBAdrRawDataBuf();// buffer to store data
	auto l_buf_size = m_lBUFSize;		// length of the buffer

	while (n_samples > 0)				// loop until the end of the file
	{
		// read data from source file into buffer
		if (n_samples < m_lBUFSize)		// adjust buftempsize
			l_buf_size = n_samples;		// then, store data in temporary buffer
		const long n_bytes = l_buf_size * sizeof(short);
		m_pXFile->Read(p_buf, n_bytes);
		if (i_type == 3)	// ASD file
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
		n_samples -= l_buf_size;		// update counter and loop
	}

	// stop appending data, update dependent struct
	p_new_doc->AcqDoc_DataAppendStop();

	// save other objects if exist (tags, others)

	if (m_hz_tags.GetNTags() > 0)
		p_new_doc->WriteHZtags(&m_hz_tags);
	if (m_vt_tags.GetNTags() > 0)
		p_new_doc->WriteVTtags(&m_vt_tags);

	// if destination name == source: remove source and rename destination
	if (b_is_duplicate_name)
	{
		// copy dummy name into csFormerName
		const auto dw_new_length = p_new_doc->m_pXFile->GetLength();
		m_pXFile->SetLength(dw_new_length);
		m_pXFile->SeekToBegin();
		p_new_doc->m_pXFile->SeekToBegin();

		p_buf = m_pWBuf->GetWBAdrRawDataBuf();// buffer to store data
		const auto n_bytes = m_lBUFSize * sizeof(short);
		DWORD dw_read;
		do
		{
			dw_read = p_new_doc->m_pXFile->Read(p_buf, n_bytes);
			m_pXFile->Write(p_buf, dw_read);
		} while (dw_read > 0);

		// file is transferred, destroy temporary file
		p_new_doc->m_pXFile->Close();
		//CFile::Remove(dummyName);

		// delete current file object and open saved-as file ??
		m_pXFile->Close();
		SAFE_DELETE(m_pXFile);

		// create / update CDataFileX associated object
		m_pXFile = new CDataFileAWAVE; //((CFile*) this);
		ASSERT(m_pXFile != NULL);

		// open saved file
		CFileException fe;
		if (!m_pXFile->Open(new_name, CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary, &fe))
		{
			m_pXFile->Abort();
			return FALSE;
		}
	}

	// destroy object created
	delete p_new_doc;

	return TRUE;
}