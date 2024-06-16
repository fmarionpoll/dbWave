// AwavFile.cpp

#include "StdAfx.h"
//#include "dataheader_Atlab.H"
#include "subfileitem.h"
#include "datafile_Awave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// class CDataFileAWAVE

// Construction / Destruction

CDataFileAWAVE::CDataFileAWAVE()
{
	m_idType = DOCTYPE_AWAVE;
	m_csType = "AWAVE";
	m_csFiledesc = "aWave 001.060298";
	m_ulOffsetHeader = static_cast<ULONGLONG>(m_csFiledesc.GetLength()) + 1;
	m_bHeaderSize = 256;
	m_bmodified = false;
}

CDataFileAWAVE::CDataFileAWAVE(CFile* pfile) : m_bmodified(false)
{
	m_idType = DOCTYPE_AWAVE;
	m_csType = "AWAVE";
	m_csFiledesc = "aWave 001.260197";
	m_ulOffsetHeader = static_cast<ULONGLONG>(m_csFiledesc.GetLength()) + 1;
	m_bHeaderSize = 256;
	m_hFile = pfile->m_hFile; // CFile

	// if file length zero, initialize file
	if (pfile->GetLength() < 1)
		InitFile();
}

// delete CDataFileAWAVE object

CDataFileAWAVE::~CDataFileAWAVE()
{
	DeleteMap();
}

BOOL CDataFileAWAVE::InitFile()
{
	// file type - ASCII infos : awave file signature
	char* pdummy[257]; // create string with 256 chars
	ASSERT(pdummy != NULL);
	memset(&pdummy[0], ' ', 256);
	Write(LPCSTR(pdummy), 256); // write it

	// write file signature
	Seek(0, begin); // position pointer
	const auto len = m_csFiledesc.GetLength();
	Write(m_csFiledesc.GetBuffer(len), static_cast<UINT>(len));
	m_csFiledesc.ReleaseBuffer();
	WriteFileMap();
	return TRUE;
}

// check if the datafile has a recognizable header format

int CDataFileAWAVE::CheckFileType(CString& csFileName)
{
	Seek(0, begin);
	char signature[6];
	Read(signature, 5);
	signature[5] = '\0';
	CStringA cs_signature(signature);
	auto flag = DOCTYPE_UNKNOWN;
	if (cs_signature.CompareNoCase(m_csFiledesc.Left(5)) == 0)
		flag = m_idType;
	return flag;
}

// load horizontal tags if they are present

BOOL CDataFileAWAVE::ReadHZtags(TagList* pHZtags)
{
	CSubfileItem* p_struct; // CStruct pointer
	if (!m_structMap.Lookup(STRUCT_HZTAGS, reinterpret_cast<CObject*&>(p_struct)))
		return FALSE;

	if (p_struct->GetDataLength() > 0)
	{
		Seek(p_struct->GetDataOffset(), begin);
		if (!pHZtags->Read(this))
			return FALSE;
	}
	return TRUE;
}

// load vertical tags if they are present

BOOL CDataFileAWAVE::ReadVTtags(TagList* pVTtags)
{
	CSubfileItem* p_struct; // CStruct pointer
	if (!m_structMap.Lookup(STRUCT_VTAGS, reinterpret_cast<CObject*&>(p_struct)))
		return FALSE;

	if (p_struct->GetDataLength() > 0)
	{
		Seek(p_struct->GetDataOffset(), begin);
		if (!pVTtags->Read(this))
			return FALSE;
	}
	return TRUE;
}

// delete map structure. this map is used to record the position of the
// different objects stored into the file

void CDataFileAWAVE::DeleteMap()
{
	if (m_structMap.IsEmpty()) // delete nothing if structure is
		return; // empty

	auto pos = m_structMap.GetStartPosition(); // get position of first elmt
	WORD w_key; // key associated
	CSubfileItem* p_struct; // pointer to object
	while (pos != nullptr) // loop over all elements
	{
		// get struct pointer & key
		m_structMap.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		delete p_struct; // delete structure
	}
	m_structMap.RemoveAll(); // finish the work
}

// update map associated with objects contained within file
// this map records the position and extent of the objects stored into the file
// assume	that all subfiles are already stored into the file
//			that CSubfileItem descriptors are uptodate
// and thus write descriptors, first into the header (first 256 bytes of the file
// -minus initial signature) and then past the end of the file (or the last subfile
// record). This "jump" is added if necessary -special descriptor.

void CDataFileAWAVE::WriteFileMap()
{
	// set CFile pos after the signature
	Seek(m_ulOffsetHeader, begin);

	// Write header: loop over all elements of the map; at the end, mark a stop

	// how much items fit within header ?
	const int size_item = sizeof(SUBF_DESCRIP); // size of one element
	auto nbitemsmax = static_cast<int>(m_bHeaderSize - m_ulOffsetHeader) / size_item;
	nbitemsmax--; // minus struct_jump or struc_end
	auto nbitems = 0; // init nb structures to zero

	// loop through the map & write subfile descriptors
	// skip end-of-file descriptor
	// add a "jump" descriptor if header (256) is full
	// and continue to write descriptors further at the end of the file

	auto ul_size = m_ulOffsetHeader; // length of header / compare w. 256
	CSubfileItem* p_struct; // structure with descriptor
	WORD w_key; // key value
	auto pos = m_structMap.GetStartPosition(); // position

	while (pos != nullptr) // loop through entire map
	{
		// get pointer to object & key
		m_structMap.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		if (w_key == STRUCT_END) // skip struct_end
			continue; // look for next item
		if (nbitems == nbitemsmax) // jump to the end of the file?
		{
			CSubfileItem* p_j_struct;
			if (!m_structMap.Lookup(STRUCT_JUMP, reinterpret_cast<CObject*&>(p_j_struct)))
			{
				p_j_struct = new CSubfileItem(STRUCT_JUMP, "JUMP_TAG", 512, 0, NORMAL_MODE);
				ASSERT(p_j_struct != NULL);
				m_structMap.SetAt(STRUCT_JUMP, p_struct);
			}
			p_j_struct->SetDataOffset(GetLength());
			p_j_struct->Write(this);
			SeekToEnd();
		}
		ul_size += p_struct->Write(this); // write structure
		nbitems++;
	}

	// map is written, end list with a stop descriptor.
	// write STRUCT_END (with a stop), create & add to map if not present
	if (!m_structMap.Lookup(STRUCT_END, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_END, "END_TAGS", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_structMap.SetAt(STRUCT_END, p_struct);
	}
	p_struct->SetDataOffset(GetLength() - 1);
	ul_size += p_struct->Write(this); // write STRUCT_END
}

////////////////////////////////////////////////////////////////////
// Operations

////////////////////////////////////////////////////////////////////////////////////
//	Add/Save subfiles
//	If these structures were already saved, old data are out of scope - not removed
//	ie the pointer to the old data is modified but the data are still in the file.
//	To purge a file with many changes, perform a file save, or saveas
// !! no error checking is performed here..

// write tags horizontal or vertical

BOOL CDataFileAWAVE::WriteHZtags(TagList* ptags)
{
	CSubfileItem* p_struct;

	// save vertical tags
	if (!m_structMap.Lookup(STRUCT_HZTAGS, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_HZTAGS, "HZ_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_structMap.SetAt(STRUCT_HZTAGS, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->SetDataOffset(GetPosition()); // get offset to EOF and save pos
	p_struct->SetDataLength(ptags->Write(this)); // write ACQDEF there & save length
	WriteFileMap();
	return TRUE;
}

BOOL CDataFileAWAVE::WriteVTtags(TagList* ptags)
{
	CSubfileItem* p_struct;

	// save vertical tags
	if (!m_structMap.Lookup(STRUCT_VTAGS, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_VTAGS, "VT_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct != NULL);
		m_structMap.SetAt(STRUCT_VTAGS, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->SetDataOffset(GetPosition()); // get offset to EOF and save pos
	p_struct->SetDataLength(ptags->Write(this)); // write ACQDEF there & save length
	WriteFileMap();
	return TRUE;
}

void CDataFileAWAVE::ReportSaveLoadException(const LPCTSTR lpsz_path_name,
                                             CException* e, const BOOL bSaving, UINT n_idp)
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
				n_idp = bSaving ? AFX_IDP_FAILED_ACCESS_WRITE : AFX_IDP_FAILED_ACCESS_READ;
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
				n_idp = bSaving ? AFX_IDP_FAILED_IO_ERROR_WRITE : AFX_IDP_FAILED_IO_ERROR_READ;
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

////////////////////////////////////////////////////////////////////

// Append data to current data buffer: init, do, stop
// Close File
// save file

// position data pointer to the start of data structure
// if none, create one
// returns file position of data structure
// assume that byteIndex is correct, ex by requesting position past the end of the file
// (assume no exception generated by CFile)

BOOL CDataFileAWAVE::DataAppendStart()
{
	CSubfileItem* p_struct; // get descriptor
	auto item_max = 0;
	WORD w_key; // key value
	auto pos = m_structMap.GetStartPosition(); // position
	// search nb of STRUCT_DATA items
	while (pos != nullptr) // loop through entire map
	{
		// get pointer to object & key
		m_structMap.GetNextAssoc(pos, w_key, reinterpret_cast<CObject*&>(p_struct));
		if (w_key == STRUCT_DATA) // skip struct_end
			if (p_struct->GetItemnb() > item_max)
				item_max = p_struct->GetItemnb();
	}
	item_max++;
	const auto l_actual = Seek(0, end); // file pointer position

	if (m_structMap.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		delete p_struct;

	p_struct = new CSubfileItem(STRUCT_DATA,	// ucCode
	                            "DATA:",		// csLabel x
	                            static_cast<long>(l_actual), // lOffset x
	                            0,				// lLength
	                            NORMAL_MODE,	// ucEncoding
	                            item_max);		// itemnb
	ASSERT(p_struct != NULL);
	m_structMap.SetAt(STRUCT_DATA, p_struct); // create new descriptor

	m_bmodified = TRUE;
	m_ulbytescount = 0;
	return TRUE;
}

// append data to the file
// assume that file index correctly set by a previous call to SeekData
// assume no exception generated by CFile

BOOL CDataFileAWAVE::DataAppend(short* pBU, UINT uibytesLength)
{
	m_bmodified = TRUE; 
	Write(pBU, uibytesLength); 
	m_ulbytescount += static_cast<ULONGLONG>(uibytesLength);
	return TRUE; 
}

// stop appending data, update pointers, structures

BOOL CDataFileAWAVE::DataAppendStop()
{
	CSubfileItem* p_struct; 
	if (!m_structMap.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		return 0;
	p_struct->SetDataLength(m_ulbytescount);
	WriteFileMap();
	return TRUE;
}

// WriteDataInfos:  save data parameters
// if previous parameters exist, they are not erased from the file
// but no reference from it are kept in the descriptors
// this is potentially dangerous because the file could grow infinitely

BOOL CDataFileAWAVE::WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC)
{
	CSubfileItem* p_struct = nullptr;

	// save ACQDEF
	if (!m_structMap.Lookup(STRUCT_ACQDEF, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_ACQDEF, "ACQDEF:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct);
		m_structMap.SetAt(STRUCT_ACQDEF, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->SetDataOffset(GetPosition()); // get offset to EOF and save pos
	const ULONGLONG ulLenwF = pwF->write(this);
	p_struct->SetDataLength(ulLenwF);

	// save ACQCHAN _ array
	if (!m_structMap.Lookup(STRUCT_ACQCHAN, reinterpret_cast<CObject*&>(p_struct)))
	{
		p_struct = new CSubfileItem(STRUCT_ACQCHAN, "ACQCHAN:", 512, 0, NORMAL_MODE);
		ASSERT(p_struct);
		m_structMap.SetAt(STRUCT_ACQCHAN, p_struct);
	}
	SeekToEnd(); // CFile: end of file (EOF)
	p_struct->SetDataOffset(GetPosition()); // get offset to EOF & save pos
	const ULONGLONG u_lenw_c = pwC->Write(this);
	p_struct->SetDataLength(u_lenw_c);

	// update header with these infos
	WriteFileMap();
	return TRUE;
}

// BOOL ReadDataInfos()
// read and initialize the members variable

BOOL CDataFileAWAVE::ReadDataInfos(CWaveBuf* pBuf)
{
	CWaveFormat* pWFormat = pBuf->GetpWaveFormat();
	CWaveChanArray* pArray = pBuf->GetpWavechanArray();
	DeleteMap(); // cleanup map

	// fill map by reading all descriptors until end of list
	Seek(m_ulOffsetHeader, begin); // position pointer

	CSubfileItem* p_struct; // CStruct pointer
	WORD ucCode; // struct code
	do // loop until STRUCT_END
	{
		p_struct = new CSubfileItem; // create new structure
		ASSERT(p_struct);
		p_struct->Read(this); // read struct from file
		ucCode = p_struct->GetCode(); // get code
		m_structMap.SetAt(ucCode, p_struct); // add struct to map
		if (ucCode == STRUCT_JUMP) // if jump struct, update file pointer
			Seek(p_struct->GetDataOffset(), begin);
	}
	while (ucCode != STRUCT_END); // stop?

	// read ACQDEF structure: get file pointer and load data
	if (m_structMap.Lookup(STRUCT_ACQDEF, reinterpret_cast<CObject*&>(p_struct)))
	{
		if (p_struct->GetDataLength() > 0)
		{
			Seek(p_struct->GetDataOffset(), begin);
			if (!pWFormat->read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQDEF\n"));
		}
	}

	// read ACQCHAN datas: get file pointer and load data
	if (m_structMap.Lookup(STRUCT_ACQCHAN, reinterpret_cast<CObject*&>(p_struct)))
	{
		if (p_struct->GetDataLength() > 0)
		{
			Seek(p_struct->GetDataOffset(), begin);
			if (!pArray->Read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQCHAN\n"));
		}
	}

	// get pointer to data area
	if (m_structMap.Lookup(STRUCT_DATA, reinterpret_cast<CObject*&>(p_struct)))
		m_ulOffsetData = p_struct->GetDataOffset();
	return TRUE;
}
