#include "StdAfx.h"
#include "subfileitem.h"
#include "datafile_Awave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CDataFileAWAVE::CDataFileAWAVE()
{
	m_id_type = DOCTYPE_AWAVE;
	m_cs_type = "AWAVE";
	cs_file_desc_ = "aWave 001.060298";
	m_ul_offset_header = static_cast<LONGLONG>(cs_file_desc_.GetLength()) + static_cast<LONGLONG>(1);
	m_b_header_size = 256;
	b_modified_ = false;
}

CDataFileAWAVE::CDataFileAWAVE(const CFile* p_file) : b_modified_(false)
{
	m_id_type = DOCTYPE_AWAVE;
	m_cs_type = "AWAVE";
	cs_file_desc_ = "aWave 001.260197";
	m_ul_offset_header = static_cast<LONGLONG>(cs_file_desc_.GetLength()) + static_cast<LONGLONG>(1);
	m_b_header_size = 256;
	m_hFile = p_file->m_hFile; // CFile

	// if file length zero, initialize file
	if (p_file->GetLength() < 1)
		init_file();
}

// delete CDataFileAWAVE object

CDataFileAWAVE::~CDataFileAWAVE()
{
	delete_map();
}

BOOL CDataFileAWAVE::init_file()
{
	// file type - ASCII infos : awave file signature
	char* p_dummy[257]; 
	ASSERT(p_dummy != NULL);
	memset(&p_dummy[0], ' ', 256);
	Write(reinterpret_cast<LPCSTR>(p_dummy), 256); 

	// write file signature
	Seek(0, begin); 
	const auto len = cs_file_desc_.GetLength();
	Write(cs_file_desc_.GetBuffer(len), static_cast<UINT>(len));
	cs_file_desc_.ReleaseBuffer();
	write_file_map();
	return TRUE;
}

// check if the datafile has a recognizable header format

int CDataFileAWAVE::check_file_type(CString& cs_file_name)
{
	Seek(0, begin);
	char signature[6];
	Read(signature, 5);
	signature[5] = '\0';
	const CStringA cs_signature(signature);
	auto flag = DOCTYPE_UNKNOWN;
	if (cs_signature.CompareNoCase(cs_file_desc_.Left(5)) == 0)
		flag = m_id_type;
	return flag;
}

// load horizontal tags if they are present

BOOL CDataFileAWAVE::read_hz_tags(TagList* p_hz_tags)
{
	CSubfileItem* p_struct; // CStruct pointer
	if (!m_struct_map_.Lookup(STRUCT_HZTAGS, reinterpret_cast<CObject*&>(p_struct)))
		return FALSE;

	if (p_struct->get_data_length() > 0)
	{
		Seek(p_struct->get_data_offset(), begin);
		if (!p_hz_tags->read(this))
			return FALSE;
	}
	return TRUE;
}

// load vertical tags if they are present

BOOL CDataFileAWAVE::read_vt_tags(TagList* p_vt_tags)
{
	CSubfileItem* p_struct; // CStruct pointer
	if (!m_struct_map_.Lookup(STRUCT_VTAGS, reinterpret_cast<CObject*&>(p_struct)))
		return FALSE;

	if (p_struct->get_data_length() > 0)
	{
		Seek(p_struct->get_data_offset(), begin);
		if (!p_vt_tags->read(this))
			return FALSE;
	}
	return TRUE;
}

// delete map structure. this map is used to record the position of the
// different objects stored into the file

void CDataFileAWAVE::delete_map()
{
	if (m_struct_map_.IsEmpty()) 
		return;

	auto pos = m_struct_map_.GetStartPosition();
	
	while (pos != nullptr) 
	{
		CSubfileItem* p_struct;
		WORD w_key;
		m_struct_map_.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		delete p_struct; 
	}
	m_struct_map_.RemoveAll(); 
}

// update map associated with objects contained within file
// this map records the position and extent of the objects stored into the file
// assume	that all sub_files are already stored into the file
//			that C_Sub_fileItem descriptors are up_to_date
// and thus write descriptors, first into the header (first 256 bytes of the file
// -minus initial signature) and then past the end of the file (or the last sub_file
// record). This "jump" is added if necessary -special descriptor.

void CDataFileAWAVE::write_file_map()
{
	// set CFile pos after the signature
	Seek(m_ul_offset_header, begin);

	// Write header: loop over all elements of the map; at the end, mark a stop

	// how many items fit within header ?
	constexpr int size_item = sizeof(SUBF_DESCRIP); // size of one element
	auto nb_items_max = static_cast<int>(m_b_header_size - m_ul_offset_header) / size_item;
	nb_items_max--; // minus struct_jump or struct_end
	auto nb_items = 0; // init nb structures to zero

	// loop through the map & write sub_file descriptors
	// skip end-of-file descriptor
	// add a "jump" descriptor if header (256) is full
	// and continue to write descriptors further at the end of the file

	ULONGLONG ul_size = m_ul_offset_header; // length of header / compare w. 256
	CSubfileItem* p_struct; // structure with descriptor
	WORD w_key; // key value
	auto pos = m_struct_map_.GetStartPosition(); // position

	while (pos != nullptr) // loop through entire map
	{
		// get pointer to object & key
		m_struct_map_.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		if (w_key == STRUCT_END) 
			continue; 
		if (nb_items == nb_items_max) // jump to the end of the file?
		{
			CSubfileItem* p_j_struct;
			if (!m_struct_map_.Lookup(STRUCT_JUMP, reinterpret_cast<CObject*&>(p_j_struct)))
			{
				p_j_struct = new CSubfileItem(STRUCT_JUMP, "JUMP_TAG", 512, 0, NORMAL_MODE);
				ASSERT(p_j_struct != NULL);
				m_struct_map_.SetAt(STRUCT_JUMP, p_struct);
			}
			p_j_struct->set_data_offset(GetLength());
			p_j_struct->write(this);
			SeekToEnd();
		}
		ul_size += p_struct->write(this); 
		nb_items++;
	}

	// map is written, end list with a stop descriptor.
	// write STRUCT_END (with a stop), create & add to map if not present
	if (!m_struct_map_.Lookup(STRUCT_END, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_END, "END_TAGS", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_struct_map_.SetAt(STRUCT_END, p_struct);
	}
	p_struct->set_data_offset(GetLength() - 1);
	ul_size += p_struct->write(this); // write STRUCT_END
}


//	Add/Save sub_files
//	If these structures were already saved, old data are out of scope - not removed
//	ie the pointer to the old data is modified but the data are still in the file.
//	To purge a file with many changes, perform a file save, or saveas
// !! no error checking is performed here..

// write tags horizontal or vertical

BOOL CDataFileAWAVE::write_hz_tags(TagList* p_tags)
{
	CSubfileItem* p_struct;

	// save vertical tags
	if (!m_struct_map_.Lookup(STRUCT_HZTAGS, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_HZTAGS, "HZ_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_struct_map_.SetAt(STRUCT_HZTAGS, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->set_data_offset(GetPosition()); // get offset to EOF and save pos
	p_struct->set_data_length(p_tags->write(this)); // write ACQ_DEF there & save length
	write_file_map();
	return TRUE;
}

BOOL CDataFileAWAVE::write_vt_tags(TagList* p_tags)
{
	CSubfileItem* p_struct;

	// save vertical tags
	if (!m_struct_map_.Lookup(STRUCT_VTAGS, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_VTAGS, "VT_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_struct_map_.SetAt(STRUCT_VTAGS, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->set_data_offset(GetPosition()); // get offset to EOF and save pos
	p_struct->set_data_length(p_tags->write(this)); // write ACQ_DEF there & save length
	write_file_map();
	return TRUE;
}

void CDataFileAWAVE::report_save_load_exception(const LPCTSTR lpsz_path_name,
                                             CException* e, const BOOL b_saving, UINT n_idp)
{
	if (e != nullptr)
	{
		ASSERT_VALID(e);
		if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
		{
			auto* ee = reinterpret_cast<CFileException*>(e);
			switch (ee->m_cause)
			{
			case CFileException::fileNotFound:
			case CFileException::badPath:
				n_idp = AFX_IDP_FAILED_INVALID_PATH;
				break;
			case CFileException::diskFull:
				n_idp = AFX_IDP_FAILED_DISK_FULL;
				break;
			case CFileException::accessDenied:
				n_idp = b_saving ? AFX_IDP_FAILED_ACCESS_WRITE : AFX_IDP_FAILED_ACCESS_READ;
				if (ee->m_lOsError == ERROR_WRITE_PROTECT)
					n_idp = IDS_WRITEPROTECT;
				break;
			case CFileException::tooManyOpenFiles:
				n_idp = IDS_TOOMANYFILES;
				break;
			case CFileException::directoryFull:
				n_idp = IDS_DIRFULL;
				break;
			case CFileException::sharingViolation:
				n_idp = IDS_SHAREVIOLATION;
				break;
			case CFileException::lockViolation:
			case CFileException::badSeek:
			case CFileException::genericException:
			case CFileException::invalidFile:
			case CFileException::hardIO:
				n_idp = b_saving ? AFX_IDP_FAILED_IO_ERROR_WRITE : AFX_IDP_FAILED_IO_ERROR_READ;
				break;
			default:
				break;
			}
			CString prompt;
			AfxFormatString1(prompt, n_idp, lpsz_path_name);
			AfxMessageBox(prompt, MB_ICONEXCLAMATION, n_idp);
			return;
		}
	}
}

// Append data to current data buffer: init, do, stop
// Close File
// save file

// position data pointer to the start of data structure
// if none, create one
// returns file position of data structure
// assume that byteIndex is correct, ex by requesting position past the end of the file
// (assume no exception generated by CFile)

BOOL CDataFileAWAVE::data_append_start()
{
	CSubfileItem* p_struct; // get descriptor
	auto item_max = 0;
	WORD w_key; // key value
	auto pos = m_struct_map_.GetStartPosition(); // position
	// search nb of STRUCT_DATA items
	while (pos != nullptr) // loop through entire map
	{
		// get pointer to object & key
		m_struct_map_.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		if (w_key == STRUCT_DATA) // skip struct_end
			if (p_struct->get_item_nb() > item_max)
				item_max = p_struct->get_item_nb();
	}
	item_max++;
	const auto l_actual = Seek(0, end); // file pointer position

	if (m_struct_map_.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		delete p_struct;

	p_struct = new CSubfileItem(STRUCT_DATA,	// ucCode
	                            "DATA:",		// csLabel x
	                            static_cast<long>(l_actual), // lOffset x
	                            0,				// lLength
	                            NORMAL_MODE,	// ucEncoding
	                            item_max);		// item_b
	ASSERT(p_struct != NULL);
	m_struct_map_.SetAt(STRUCT_DATA, p_struct); // create new descriptor

	b_modified_ = TRUE;
	m_ul_bytes_count = 0;
	return TRUE;
}

// append data to the file
// assume that file index correctly set by a previous call to SeekData
// assume no exception generated by CFile

BOOL CDataFileAWAVE::data_append(short* pBU, UINT uibytesLength)
{
	b_modified_ = TRUE; 
	Write(pBU, uibytesLength); 
	m_ul_bytes_count += static_cast<ULONGLONG>(uibytesLength);
	return TRUE; 
}

// stop appending data, update pointers, structures

BOOL CDataFileAWAVE::data_append_stop()
{
	CSubfileItem* p_struct; 
	if (!m_struct_map_.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		return 0;
	p_struct->set_data_length(m_ul_bytes_count);
	write_file_map();
	return TRUE;
}

// WriteDataInfos:  save data parameters
// if previous parameters exist, they are not erased from the file
// but no reference from it are kept in the descriptors
// this is potentially dangerous because the file could grow infinitely

BOOL CDataFileAWAVE::write_data_infos(CWaveFormat* pwF, CWaveChanArray* pwC)
{
	CSubfileItem* p_struct = nullptr;

	// save ACQDEF
	if (!m_struct_map_.Lookup(STRUCT_ACQDEF, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_ACQDEF, "ACQDEF:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct);
		m_struct_map_.SetAt(STRUCT_ACQDEF, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->set_data_offset(GetPosition()); // get offset to EOF and save pos
	const ULONGLONG ul_len_wf = pwF->write(this);
	p_struct->set_data_length(ul_len_wf);

	// save ACQ_CHAN _ array
	if (!m_struct_map_.Lookup(STRUCT_ACQCHAN, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_ACQCHAN, "ACQCHAN:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct);
		m_struct_map_.SetAt(STRUCT_ACQCHAN, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->set_data_offset(GetPosition()); // get offset to EOF & save pos
	const ULONGLONG u_len_wc = pwC->Write(this);
	p_struct->set_data_length(u_len_wc);

	// update header with these infos
	write_file_map();
	return TRUE;
}

// BOOL ReadDataInfos()
// read and initialize the members variable

BOOL CDataFileAWAVE::read_data_infos(CWaveBuf* p_buf)
{
	CWaveFormat* p_w_format = p_buf->get_p_wave_format();
	CWaveChanArray* p_array = p_buf->get_p_wave_chan_array();
	delete_map(); // cleanup map

	// fill map by reading all descriptors until end of list
	Seek(m_ul_offset_header, begin); // position pointer

	CSubfileItem* p_struct; // CStruct pointer
	WORD uc_code; // struct code
	do // loop until STRUCT_END
	{
		p_struct = new CSubfileItem; // create new structure
		ASSERT(p_struct);
		p_struct->read(this); // read struct from file
		uc_code = p_struct->get_code(); // get code
		m_struct_map_.SetAt(uc_code, p_struct); // add struct to map
		if (uc_code == STRUCT_JUMP) // if jump struct, update file pointer
			Seek(p_struct->get_data_offset(), begin);
	}
	while (uc_code != STRUCT_END); // stop?

	// read ACQ_DEF structure: get file pointer and load data
	if (m_struct_map_.Lookup(STRUCT_ACQDEF, reinterpret_cast<CObject*&>(p_struct)))
	{
		if (p_struct->get_data_length() > 0)
		{
			Seek(p_struct->get_data_offset(), begin);
			if (!p_w_format->read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQDEF\n"));
		}
	}

	// read ACQ_CHAN data: get file pointer and load data
	if (m_struct_map_.Lookup(STRUCT_ACQCHAN, reinterpret_cast<CObject*&>(p_struct)))
	{
		if (p_struct->get_data_length() > 0)
		{
			Seek(p_struct->get_data_offset(), begin);
			if (!p_array->Read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQCHAN\n"));
		}
	}

	// get pointer to data area
	if (m_struct_map_.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		m_ul_offset_data = p_struct->get_data_offset();
	return TRUE;
}
