// AwavFile.cpp

#include "stdafx.h"
#include "dataheader_Atlab.h"
#include "subfileitem.h"
#include "datafile_Awave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// class CDataFileAWAVE

// Construction / Destruction

CDataFileAWAVE::CDataFileAWAVE() : CDataFileX()
{
	m_idType= DOCTYPE_AWAVE;
	m_csType = "AWAVE";
	m_csFiledesc = "aWave 001.060298";
	m_ulOffsetHeader= ULONGLONG(m_csFiledesc.GetLength())+1;
	m_bHeaderSize= 256;
	m_bmodified = false;
}

CDataFileAWAVE::CDataFileAWAVE(CFile* pfile)
{
	m_idType= DOCTYPE_AWAVE;
	m_csType= "AWAVE";
	m_csFiledesc = "aWave 001.260197";
	m_ulOffsetHeader= ULONGLONG(m_csFiledesc.GetLength())+1;
	m_bHeaderSize= 256;
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
	char* pdummy[257];				// create string with 256 chars
	ASSERT(pdummy != NULL);
	memset(&pdummy[0], ' ', 256);
	Write (LPCSTR(pdummy), 256);	// write it

	// write file signature
	Seek(0, CFile::begin);			// position pointer
	int len = m_csFiledesc.GetLength();
	Write(m_csFiledesc.GetBuffer(len), (UINT) len);
	m_csFiledesc.ReleaseBuffer();
	WriteFileMap();
	return TRUE;
}

// check if the datafile has a recognizable header format

BOOL CDataFileAWAVE::CheckFileType(CFile* pfile, int bSignatureOffset)
{
	pfile->Seek(0, CFile::begin);
	char signature[6];
	pfile->Read(signature, 5);
	signature[5] = '\0';
	CStringA csSignature(signature);
	BOOL flag = DOCTYPE_UNKNOWN;
	if (csSignature.CompareNoCase(m_csFiledesc.Left(5)) == 0)	
		flag = m_idType;
	return flag;
}

// load horizontal tags if they are present

BOOL CDataFileAWAVE::ReadHZtags(CTagList* pHZtags)
{
	CSubfileItem* pStruct;					// CStruct pointer
	if (!m_structMap.Lookup(STRUCT_HZTAGS, (CObject*&)pStruct))
		return FALSE;

	if (pStruct->GetDataLength() >0)
	{
		Seek(pStruct->GetDataOffset(), CFile::begin);
		if (!pHZtags->Read(this))
			return FALSE;
	}
	return TRUE;
}

// load vertical tags if they are present

BOOL CDataFileAWAVE::ReadVTtags(CTagList* pVTtags)
{
	CSubfileItem* pStruct;					// CStruct pointer
	if (!m_structMap.Lookup(STRUCT_VTAGS, (CObject*&)pStruct))
		return FALSE;

	if (pStruct->GetDataLength() >0)
	{
		Seek(pStruct->GetDataOffset(), CFile::begin);
		if (!pVTtags->Read(this))
			return FALSE;
	}
	return TRUE;
}

// delete map structure. this map is used to record the position of the
// different objects stored into the file

void CDataFileAWAVE::DeleteMap()
{
	if (m_structMap.IsEmpty())				// delete nothing if structure is
		return;								// empty
	
	POSITION Pos =  m_structMap.GetStartPosition();	 // get position of first elmt
	WORD wKey;									// key associated
	CSubfileItem* pStruct;						// pointer to object
	while (Pos != nullptr)							// loop over all elements
	{											// get struct pointer & key
		m_structMap.GetNextAssoc(Pos, wKey, (CObject*&) pStruct);
		delete pStruct;							// delete structure
	}
	m_structMap.RemoveAll();					// finish the work
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
	Seek(m_ulOffsetHeader, CFile::begin);

	// Write header: loop over all elements of the map; at the end, mark a stop	

	// how much items fit within header ?
	int sizeItem = sizeof(SUBF_DESCRIP);// size of one element
	int nbitemsmax =  (int) (m_bHeaderSize - m_ulOffsetHeader)/sizeItem ;
	nbitemsmax--;						// minus struct_jump or struc_end
	int nbitems = 0;					// init nb structures to zero

	// loop throught the map & write subfile descriptors
	// skip end-of-file descriptor
	// add a "jump" descriptor if header (256) is full
	// and continue to write descriptors further at the end of the file

	ULONGLONG ulSize = m_ulOffsetHeader;	// length of header / compare w. 256
	CSubfileItem* pStruct;				// structure with descriptor
	WORD wkey;							// key value
	POSITION pos = m_structMap.GetStartPosition();	// position	

	while (pos != nullptr)					// loop through entire map
	{									// get pointer to object & key
		m_structMap.GetNextAssoc( pos, wkey, (CObject*&) pStruct );
		if (wkey == STRUCT_END)			// skip struct_end
			continue;					// look for next item
		if (nbitems == nbitemsmax)		// jump to the end of the file?
		{
			CSubfileItem* pJStruct;
			if (!m_structMap.Lookup(STRUCT_JUMP, (CObject*&) pJStruct))
			{
				pJStruct = new CSubfileItem(STRUCT_JUMP, "JUMP_TAG", 512, 0, NORMAL_MODE);
				ASSERT(pJStruct != NULL);
				m_structMap.SetAt(STRUCT_JUMP, pStruct);
			}
			pJStruct->SetDataOffset(GetLength());
			pJStruct->Write(this);
			SeekToEnd();
		}
		ulSize += pStruct->Write(this);	// write structure
		nbitems++;
	}

	// map is written, end list with a stop descriptor.
	// write STRUCT_END (with a stop), create & add to map if not present
	if (!m_structMap.Lookup(STRUCT_END, (CObject*&) pStruct))
	{
		pStruct = new CSubfileItem(STRUCT_END, "END_TAGS", 512, 0, NORMAL_MODE);
		ASSERT(pStruct != NULL);
		m_structMap.SetAt(STRUCT_END, pStruct);		
	}	
	pStruct->SetDataOffset(GetLength()-1);
	ulSize += pStruct->Write(this);		// write STRUCT_END
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

BOOL CDataFileAWAVE::WriteHZtags(CTagList* ptags)
{
	CSubfileItem* pStruct;

	// save vertical tags
	if (!m_structMap.Lookup(STRUCT_HZTAGS, (CObject*&) pStruct))
	{
		pStruct = new CSubfileItem(STRUCT_HZTAGS, "HZ_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(pStruct != NULL);
		m_structMap.SetAt(STRUCT_HZTAGS, pStruct);
	}
	SeekToEnd();							// CFile: end of file (EOF)	
	pStruct->SetDataOffset(GetPosition());	// get offset to EOF and save pos 	
	pStruct->SetDataLength(ptags->Write(this));	// write ACQDEF there & save length
	WriteFileMap();
	return TRUE;
}

BOOL CDataFileAWAVE::WriteVTtags(CTagList* ptags)
{
	CSubfileItem* pStruct;

	// save vertical tags
	if (!m_structMap.Lookup(STRUCT_VTAGS, (CObject*&) pStruct))
	{
		pStruct = new CSubfileItem(STRUCT_VTAGS, "VT_TAGS:", 512, 0, NORMAL_MODE);
		ASSERT(pStruct != NULL);
		m_structMap.SetAt(STRUCT_VTAGS, pStruct);
	}
	SeekToEnd();							// CFile: end of file (EOF)	
	pStruct->SetDataOffset(GetPosition());	// get offset to EOF and save pos 	
	pStruct->SetDataLength(ptags->Write(this));	// write ACQDEF there & save length
	WriteFileMap();
	return TRUE;
}


void CDataFileAWAVE::ReportSaveLoadException(LPCTSTR lpszPathName,
	CException* e, BOOL bSaving, UINT nIDP)
{
	if (e != nullptr)
	{
		ASSERT_VALID(e);
		if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
		{
			switch (((CFileException*)e)->m_cause)
			{
			case CFileException::fileNotFound:
			case CFileException::badPath:
				nIDP = AFX_IDP_FAILED_INVALID_PATH;
				break;
			case CFileException::diskFull:
				nIDP = AFX_IDP_FAILED_DISK_FULL;
				break;
			case CFileException::accessDenied:
				nIDP = bSaving ? AFX_IDP_FAILED_ACCESS_WRITE :
						AFX_IDP_FAILED_ACCESS_READ;
				if (((CFileException*)e)->m_lOsError == ERROR_WRITE_PROTECT)
					nIDP = IDS_WRITEPROTECT;
				break;
			case CFileException::tooManyOpenFiles:
				nIDP = IDS_TOOMANYFILES;
				break;
			case CFileException::directoryFull:
				nIDP = IDS_DIRFULL;
				break;
			case CFileException::sharingViolation:
				nIDP = IDS_SHAREVIOLATION;
				break;
			case CFileException::lockViolation:
			case CFileException::badSeek:
			case CFileException::genericException:
			case CFileException::invalidFile:
			case CFileException::hardIO:
				nIDP = bSaving ? AFX_IDP_FAILED_IO_ERROR_WRITE :
						AFX_IDP_FAILED_IO_ERROR_READ;
				break;
			default:
				break;
			}
			CString prompt;
			AfxFormatString1(prompt, nIDP, lpszPathName);
			AfxMessageBox(prompt, MB_ICONEXCLAMATION, nIDP);
			return;
		}
	}	
	return;
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
	CSubfileItem* pStruct;				// get descriptor
	int itemmax=0;
	WORD wkey;							// key value
	POSITION pos = m_structMap.GetStartPosition();	// position	
	// search nb of STRUCT_DATA items
	while (pos != nullptr)					// loop through entire map
	{									// get pointer to object & key
		m_structMap.GetNextAssoc( pos, wkey, (CObject*&) pStruct );
		if (wkey == STRUCT_DATA)			// skip struct_end
			if (pStruct->GetItemnb() > itemmax)
				itemmax = pStruct->GetItemnb();
	}
	itemmax++;
	ULONGLONG lActual= Seek(0, CFile::end);	// file pointer position

	if (m_structMap.Lookup(STRUCT_DATA, (CObject*&) pStruct))
		delete pStruct;

	pStruct = new CSubfileItem(STRUCT_DATA,			// ucCode
									"DATA:",		// csLabel x
									(long) lActual,	// lOffset x
									0,				// lLength
									NORMAL_MODE,	// ucEncoding
									itemmax);		// itemnb
	ASSERT(pStruct != NULL);
	m_structMap.SetAt(STRUCT_DATA, pStruct);	// create new descriptor

	m_bmodified = TRUE;
	m_ulbytescount = 0;
	return TRUE;
}


// append data to the file
// assume that file index correctly set by a previous call to SeekData
// assume no exception generated by CFile

BOOL CDataFileAWAVE::DataAppend(short* pBU, UINT uibytesLength)
{
	m_bmodified = TRUE;					// tell awave that data were added
	Write(pBU, uibytesLength);			// write data
	m_ulbytescount += (ULONGLONG) uibytesLength;
	return TRUE; //
}

// stop appending data, update pointers, structures

BOOL CDataFileAWAVE::DataAppendStop()
{
	CSubfileItem* pStruct;				// get descriptor	
	if (!m_structMap.Lookup(STRUCT_DATA, (CObject*&) pStruct))
		return 0;	
	pStruct->SetDataLength(m_ulbytescount);	
	WriteFileMap();
	return TRUE;
}


// WriteDataInfos:  save data parameters
// if previous parameters exist, they are not erased from the file
// but no reference from it are kept in the descriptors
// this is potentially dangerous because the file could grow infinitely

BOOL CDataFileAWAVE::WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC)
{
	CSubfileItem* pStruct = nullptr;

	// save ACQDEF
	if (!m_structMap.Lookup(STRUCT_ACQDEF, (CObject*&) pStruct))
	{	
		pStruct = new CSubfileItem(STRUCT_ACQDEF, "ACQDEF:", 512, 0, NORMAL_MODE);
		ASSERT(pStruct);
		m_structMap.SetAt(STRUCT_ACQDEF, pStruct);
	}
	SeekToEnd();							// CFile: end of file (EOF)	
	pStruct->SetDataOffset(GetPosition());	// get offset to EOF and save pos 
	ULONGLONG ulLenwF = pwF->Write(this);
	pStruct->SetDataLength(ulLenwF);

	// save ACQCHAN _ array
	if (!m_structMap.Lookup(STRUCT_ACQCHAN, (CObject*&) pStruct))
	{
		pStruct = new CSubfileItem(STRUCT_ACQCHAN, "ACQCHAN:", 512, 0, NORMAL_MODE);
		ASSERT(pStruct );
		m_structMap.SetAt(STRUCT_ACQCHAN, pStruct);
	}
	SeekToEnd();							// CFile: end of file (EOF)	
	pStruct->SetDataOffset(GetPosition());	// get offset to EOF & save pos
	ULONGLONG uLenwC = pwC->Write(this);
	pStruct->SetDataLength(uLenwC);

	// update header with these infos
	WriteFileMap();
	return TRUE;
}

// BOOL ReadDataInfos()
// read and initialize the members variable

BOOL CDataFileAWAVE::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{	
	DeleteMap();							// cleanup map

	// fill map by reading all descriptors until end of list	
	Seek(m_ulOffsetHeader, CFile::begin);	// position pointer

	CSubfileItem* pStruct;					// CStruct pointer
	WORD ucCode;							// struct code
	do										// loop until STRUCT_END
	{
		pStruct = new CSubfileItem;			// create new structure
		ASSERT(pStruct);
		pStruct->Read(this);				// read struct from file
		ucCode = pStruct->GetCode();		// get code
		m_structMap.SetAt(ucCode, pStruct);	// add struct to map
		if (ucCode == STRUCT_JUMP)			// if jump struct, update file pointer
			Seek(pStruct->GetDataOffset(), CFile::begin);
	} while(ucCode != STRUCT_END);			// stop?

	// read ACQDEF structure: get file pointer and load data
	if (m_structMap.Lookup(STRUCT_ACQDEF, (CObject*&)pStruct))
	{
		if (pStruct->GetDataLength() >0)
		{
			Seek(pStruct->GetDataOffset(), CFile::begin);
			if (!pWFormat->Read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQDEF\n")); 
		}
	}

	// read ACQCHAN datas: get file pointer and load data
	if (m_structMap.Lookup(STRUCT_ACQCHAN, (CObject*&)pStruct))
	{
		if (pStruct->GetDataLength() >0)
		{
			Seek(pStruct->GetDataOffset(), CFile::begin);
			if (!pArray->Read(this))
				AfxMessageBox(_T("Error reading STRUCT_ACQCHAN\n"));
		}
	}

	// get pointer to data area
	if (m_structMap.Lookup(STRUCT_DATA, (CObject*&)pStruct))
		m_ulOffsetData = pStruct->GetDataOffset();
	return TRUE;
}
