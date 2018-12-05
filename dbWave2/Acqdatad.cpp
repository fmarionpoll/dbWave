// acqdatad.cpp : implementation file
//

#include "stdafx.h"
#include <time.h>
#include <afxconv.h>           // For LPTSTR -> LPSTR macros
#include "Acqparam.h"		// data acquisition struct: wave format, wave chans
#include "Taglines.h"		// tags
#include "acqdatad.h"

//#include "datafile_Awave.h"
#include "datafile_Atlab.h"
#include "datafile_ASD.h"
#include "datafile_mcid.h"
#include "ImportGenericDataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_BUFLENGTH_BYTES  614400//102400 
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
	m_DOCnbchans=0;			// nb chans
	m_lBUFchanSize = 0;		// size of the read buffer
	m_lBUFchanFirst = 0;	// index first point
	m_lBUFchanLast = 0;		// index last point
	m_lBUFSize = 0;			// size of the read buffer (?)
	m_lBUFmaxSize = MAX_BUFLENGTH_BYTES; // max buf size
	m_iOffsetInt = sizeof(short);	// offset in bytes
	m_tBUFfirst = 0;
	m_tBUFlast = 1;
	m_tBUFtransform = 0;
	m_tBUFsourcechan = 0;
		
	m_pWBuf = NULL;
	m_pXFile = NULL;
	m_bValidReadBuffer = FALSE;
	m_bValidTransfBuffer = FALSE;
	m_bRemoveOffset = TRUE;
}

/**************************************************************************
 function:  ~CAcqDataDoc()
 purpose: Remove from menory an CAcqDataDoc object
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

BOOL CAcqDataDoc::OnSaveDocument(CString &szPathName)
{
	BOOL flag = SaveAs(szPathName, FALSE);
	if (flag) 
		SetModifiedFlag(FALSE);	// mark the document as clean

	return flag;
}

/**************************************************************************
 function:  OnOpenDocument(CString &szPathName)
 purpose: Open the file called szPathName
 parameters:
	_ CString &szPathName : Name of the file to be openned
 returns: TRUE if operation was successfull, else returns FALSE	
 comments:
 **************************************************************************/

BOOL CAcqDataDoc::OnOpenDocument(CString &szPathName)
{
	// close data file if already opened
	if (m_pXFile != NULL 
		&& m_pXFile->m_hFile != CFile::hFileNull)
	{
		CString csfilename = m_pXFile->GetFileName();
		if (csfilename.CompareNoCase(szPathName) == 0)
			return TRUE;
		m_pXFile->Close();
	}
	// set file reading buffer as dirty
	m_bValidReadBuffer = FALSE;

	// check if file can be opened - exit if it can't and return an empty object
	CFileStatus rStatus;
	BOOL bOpen = CFile::GetStatus(szPathName, rStatus);
	if (!bOpen || rStatus.m_size <= 4096)	// avoid to open 1kb files ...
	{	
		SAFE_DELETE(m_pXFile);				// delete data file object if any
		return FALSE;						// and return
	}
	ASSERT(szPathName.Right(3) != _T("del"));

	// check file type (we are now sure that the file exists)
	BOOL bFoundMatch = OpenAcqFile(szPathName);

	// the format of the data file was not recognized - see if we can import
	if (bFoundMatch <0)
	{
		CImportGenericDataDlg* pdlg = new CImportGenericDataDlg;

		// init parameters
		CStringArray* pArray = new CStringArray;	// dlg needs an array of strings
		ASSERT(pArray != NULL);
		pArray->Add(szPathName);
		pdlg->m_pfilenameArray = pArray;			// pass address of array
		pdlg->bConvert = TRUE;						// tell that conversion is allowed
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// get pointer to application
		pdlg->piivO= &(pApp->ivO);
		m_pXFile->Close();							// close file
		SAFE_DELETE(m_pXFile);						// delete object
		
		// call dialog
		int iresult = pdlg->DoModal();				// start dialog
		delete pdlg;
		if (IDOK != iresult || 0 == pArray->GetSize())
			return FALSE;

		// change name of files here (rename)
		CString csfilename_old = szPathName;
		int icount = csfilename_old.ReverseFind('\\')+1;
		csfilename_old.Insert(icount, _T("OLD_"));

		// check if same file already exist
		CFileStatus rStatus;	// file status: time creation, ...
		BOOL bFlagExist = CFile::GetStatus(csfilename_old, rStatus);
		if (bFlagExist != 0)
			CFile::Remove(csfilename_old);
		TRY
		{
			CFile::Rename( szPathName, csfilename_old );
		}
		CATCH( CFileException, e )
		{
			#ifdef _DEBUG
				afxDump << "File " << szPathName << " not found, cause = " << e->m_cause << "\n";
			#endif
		}
		END_CATCH
		
		csfilename_old = pArray->GetAt(0);
		bFlagExist = CFile::GetStatus(szPathName, rStatus);

		if (bFlagExist != 0)
			CFile::Remove(szPathName);
		TRY
		{
			CFile::Rename(csfilename_old, szPathName);
		}
		CATCH( CFileException, e )
		{
			#ifdef _DEBUG
				afxDump << "File " << csfilename_old << " not found, cause = " << e->m_cause << "\n";
			#endif
		}
		END_CATCH
		delete pArray;

		m_pXFile = new CDataFileAWAVE;					// create an aWave data file
		ASSERT(m_pXFile != NULL);					// check validity
		bFoundMatch = OpenAcqFile(szPathName);		// open corresponding file
	}	
	return bFoundMatch;
}

// open data file and load first data block
int CAcqDataDoc::CheckFileTypeFromName(CString& szPathName)
{
	// open file using the last object type used or the default type
	CFileException fe;
	if (m_pXFile != NULL && m_pXFile->m_hFile != CFile::hFileNull)	// close if file currently opened
		m_pXFile->Close();
	CFileStatus rStatus;
	BOOL flag = CFile::GetStatus(szPathName, rStatus);

	// what if file not found? exit!
	if (!flag)
		return flag;

	// create standard object if pxfile is null
	if (m_pXFile == NULL)
		m_pXFile = new CDataFileAWAVE;

	// open file
	UINT uOpenflag= (rStatus.m_attribute & 0x01)?CFile::modeRead : CFile::modeReadWrite;
	uOpenflag |= CFile::shareDenyNone | CFile::typeBinary, &fe;
	if (!m_pXFile->Open(szPathName, uOpenflag ))
	{
		m_pXFile->Abort();
		return FALSE;
	}

	// create buffer
	if (m_pWBuf == NULL)
		m_pWBuf = new CWaveBuf;
	ASSERT (m_pWBuf != NULL);	// check object created properly

	// read data & create object according to file signature (checkfiletype)
	int iID = CheckFileType(m_pXFile);
	m_pXFile->Close();
	return iID;
}

BOOL CAcqDataDoc::OpenAcqFile(CString &szfilename)
{
	// open file using the last object type used or the default type
	CFileException fe;
	if (m_pXFile != NULL && m_pXFile->m_hFile != CFile::hFileNull)	// close if file currently opened
		m_pXFile->Close();
	CFileStatus rStatus;
	BOOL flag = CFile::GetStatus(szfilename, rStatus);

	// what if file not found? exit!
	if (!flag)
		return flag;

	// create standard object if pxfile is null
	if (m_pXFile == NULL)
		m_pXFile = new CDataFileAWAVE;

	// open file
	UINT uOpenflag= (rStatus.m_attribute & 0x01)?CFile::modeRead : CFile::modeReadWrite;
	uOpenflag |= CFile::shareDenyNone | CFile::typeBinary, &fe;
	if (!m_pXFile->Open(szfilename, uOpenflag ))
	{
		m_pXFile->Abort();
		return FALSE;
	}

	// create buffer
	if (m_pWBuf == NULL)
		m_pWBuf = new CWaveBuf;
	ASSERT (m_pWBuf != NULL);	// check object created properly
	CWaveChanArray* pchanArray = GetpWavechanArray();
	CWaveFormat* pwaveFormat = GetpWaveFormat();

	// read data & create object according to file signature (checkfiletype)
	int iID = CheckFileType(m_pXFile);
	if (m_pXFile == NULL 
		|| m_pXFile->m_idType != iID)
	{
		SAFE_DELETE(m_pXFile);
		switch (iID)
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
			//#define DOCTYPE_PCCLAMP		5	// PCCLAMP document (not implemented yet)
			//#define DOCTYPE_SAPID 		6	// SAPID document (not implemented yet)
			//#define DOCTYPE_UNKNOWN		-1	// The type of the document isn't accept by 
		default:
			m_pXFile = new CDataFileX;
			break;
		}
		m_pXFile->Open(szfilename, uOpenflag );	// open file again, this time with using the correct object
	}

	// return with error if format not known
	if (iID == DOCTYPE_UNKNOWN)
	{
		AllocBUF();
		return -1;
	}

	// save file pointer, read data header and Tags
	BOOL bflag = m_pXFile->ReadDataInfos(pwaveFormat, pchanArray);
	AllocBUF();
	m_pXFile->ReadVTtags(&m_VTtags);
	m_pXFile->ReadHZtags(&m_HZtags);

	return bflag;
}

/**************************************************************************
 function:  OnNewDocument()
 purpose: Create a new document	
 parameters: none
 returns: TRUE if operation was successfull, else returns FALSE
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

	if (m_pWBuf == NULL)
	{
		CString csDummy;
		csDummy.Empty();
		CreateAcqFile(csDummy);
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
	CString sep('\t');
	
	CString csOut;
	CString csDummy;
	csOut = GetPathName();
	csOut.MakeLower();					// lower case

	CWaveFormat* pwaveFormat = GetpWaveFormat();

	// date and time
	if (pVD->bacqdate)
		csOut += (pwaveFormat->acqtime).Format("\t%#d %B %Y");
	if (pVD->bacqtime) 
		csOut += (pwaveFormat->acqtime).Format("\t%X");

	// file size
	if (pVD->bfilesize)                                      // file size
	{
		csDummy.Format( _T("\t%-10li") , GetDOCchanLength());
		csOut += csDummy;
		csDummy.Format(_T("\t nchans=%i\t"), pwaveFormat->scan_count);
		csOut += csDummy;
	}

	if (pVD->bacqcomments) {
		csDummy = pwaveFormat->GetComments(sep);
		csOut += csDummy;
	}

	if (pVD->bacqchcomment || pVD->bacqchsetting)
	{
		CString cs;
		for (int ichan=0; ichan < pwaveFormat->scan_count; ichan++)
		{
			CWaveChan* pChan = (GetpWavechanArray())->GetWaveChan(ichan);
			if (pVD->bacqchcomment)
				csOut += sep + pChan->am_csComment;
			if (pVD->bacqchsetting)
			{
				cs.Format(_T("\theadstage=%s\tgain=%.0f\tfilter= %s\t%i Hz"), 
					(LPCTSTR) pChan->am_csheadstage,
					pChan->am_gaintotal, 
					(LPCTSTR) pChan->am_csInputpos,
					pChan->am_lowpass);
				csOut += cs;
			}
		}
	}
	return csOut;
}

void CAcqDataDoc::ExportDataFile_to_TXTFile(CStdioFile* pdataDest)
{
	_TCHAR sep = '\n';	// lines separator "\n\r"
	_TCHAR sep2 = ',';	// columns separator
	CString csOut;

	csOut = _T("filename=")+ GetPathName();
	csOut.MakeLower();					// lower case
	csOut += sep;
	pdataDest->WriteString(csOut);
	
	CWaveFormat* pwaveFormat = GetpWaveFormat();
	// date and time
	csOut = (pwaveFormat->acqtime).Format(_T("acqdate= %#d %B %Y"));
	csOut += sep;
	pdataDest->WriteString(csOut);
	
	csOut = (pwaveFormat->acqtime).Format(_T("acqtime= %X"));
	csOut += sep;
	pdataDest->WriteString(csOut);
	
	csOut.Format(_T("sampling rate(Hz)= %f"), pwaveFormat->chrate);
	csOut += sep;
	pdataDest->WriteString(csOut);
	
	csOut.Format(_T("nsamples=%-10li"), GetDOCchanLength());
	csOut += sep;
	pdataDest->WriteString(csOut);
	
	csOut.Format(_T("nchans=%i"), pwaveFormat->scan_count);
	pdataDest->WriteString(csOut);
	
	csOut = pwaveFormat->GetComments(&sep, TRUE);
	pdataDest->WriteString(csOut);

	csOut = sep;
	for (int ichan=0; ichan < pwaveFormat->scan_count; ichan++)
	{
		CWaveChan* pChan = (GetpWavechanArray())->GetWaveChan(ichan);
		csOut += pChan->am_csComment;
		if (ichan < pwaveFormat->scan_count - 1)
			csOut += sep2;
	}
	pdataDest->WriteString(csOut);

	// loop to read actual data
	ReadDataBlock(0); 
	double mVfactor = pwaveFormat->fullscale_Volts / pwaveFormat->binspan * 1000.;
	for (long j = 0; j < GetDOCchanLength(); j++)
	{
		CString cs;
		csOut= sep;
		for (int ichan=0; ichan < pwaveFormat->scan_count; ichan++)
		{
			CWaveChan* pChan = (GetpWavechanArray())->GetWaveChan(ichan);
			int ival = BGetVal(ichan, j);
			double changain =((double) pChan->am_gainheadstage)*((double) pChan->am_gainAD )*((double) pChan->am_gainpre) *((double) pChan->am_gainpost);
			double xval = ival* mVfactor / changain ;
			cs.Format(_T("%f"), xval);
			csOut+= cs;
			if (ichan < pwaveFormat->scan_count - 1)
				csOut += sep2;
		}
		pdataDest->WriteString(csOut);
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

int CAcqDataDoc::CheckFileType(CFile* f)
{
	int iID=DOCTYPE_UNKNOWN;
	if (m_pXFile != NULL)
	{
		iID = m_pXFile->CheckFileType(f);
		if (iID != DOCTYPE_UNKNOWN)
			return m_pXFile->m_idType;	// return file type ID
	}

	// not of current type
	// check if ATLAB file
	if (iID < 0)
	{
		CDataFileATLAB* pFileX = new (CDataFileATLAB);
		ASSERT(pFileX != NULL);
		iID = pFileX->CheckFileType(f);
		delete pFileX;
	}

	// check if aWave file
	if (iID < 0)
	{
		CDataFileAWAVE* pFileX = new (CDataFileAWAVE);
		ASSERT(pFileX != NULL);
		iID = pFileX ->CheckFileType(f);
		delete pFileX;
	}

	// check if ASD Syntech file
	if (iID < 0)
	{
		CDataFileASD* pFileX = new (CDataFileASD);
		ASSERT(pFileX != NULL);
		iID = pFileX ->CheckFileType(f);
		delete pFileX;
	}

	// check if MCID Halifax file
	if (iID < 0)
	{
		CDataFileMCID* pFileX = new (CDataFileMCID);
		ASSERT(pFileX != NULL);
		iID = pFileX ->CheckFileType(f);
		delete pFileX;
	}

	return iID;
}

// adjust size of the buffer for data read from file
// update buffer parameters
BOOL CAcqDataDoc::AdjustBUF(int iNumElements)
{
	BOOL bnewBuf= FALSE;
	if (m_pWBuf == NULL)
	{
		m_pWBuf = new CWaveBuf;
		bnewBuf = TRUE;
	}
	ASSERT (m_pWBuf != NULL);
	CWaveFormat* pwF= GetpWaveFormat();
	m_lDOCchanLength = pwF->sample_count /(long) pwF->scan_count;
	m_DOCnbchans = pwF->scan_count;
	pwF->duration = (float)m_lDOCchanLength/pwF->chrate;
	m_lBUFSize = iNumElements*pwF->scan_count;
	
	m_lBUFchanSize = m_lBUFSize / (long) pwF->scan_count;
	m_lBUFchanFirst = 0;
	m_lBUFchanLast = m_lBUFchanSize-1;
	m_lBUFSize = m_lBUFchanSize*pwF->scan_count;
	m_lBUFmaxSize=m_lBUFSize*sizeof(short);

	// alloc RW buffer
	return m_pWBuf->CreateWBuffer(m_lBUFchanSize, pwF->scan_count);
}

// allocate buffers to read data 
// adjust size of the buffer according to MAX_BUFLENGTH_BYTES
BOOL CAcqDataDoc::AllocBUF()
{
	BOOL bnewBuf= FALSE;
	if (m_pWBuf == NULL)
	{
		m_pWBuf = new CWaveBuf;
		bnewBuf = TRUE;
	}
	ASSERT (m_pWBuf != NULL);	// check object created properly	
	CWaveFormat* pwF= GetpWaveFormat();
	m_lDOCchanLength = pwF->sample_count /(long) pwF->scan_count;
	m_DOCnbchans = pwF->scan_count;
	pwF->duration = (float)m_lDOCchanLength/pwF->chrate;

	int iNumElements = m_lDOCchanLength;	
	if(iNumElements*pwF->scan_count > m_lBUFSize)
		m_lBUFSize = iNumElements*pwF->scan_count;
	
	if (m_lBUFSize > (long) (MAX_BUFLENGTH_BYTES/sizeof(short)))
		m_lBUFSize = MAX_BUFLENGTH_BYTES/sizeof(short);

	m_lBUFchanSize = m_lBUFSize / (long) pwF->scan_count;
	m_lBUFchanFirst = 0;
	m_lBUFchanLast = m_lBUFchanSize-1;
	m_lBUFSize = m_lBUFchanSize*pwF->scan_count;
	m_lBUFmaxSize=m_lBUFSize*sizeof(short);

	// alloc RW buffer
	return m_pWBuf->CreateWBuffer(m_lBUFchanSize, pwF->scan_count);
}

// returns the address of the raw data buffer
// and the number of interleaved channels
short*	CAcqDataDoc::LoadRawDataParams(int* nchans) 
{
	*nchans = GetpWaveFormat()->scan_count; 
	return m_pWBuf->GetWBAdrRawDataBuf();
}

/**************************************************************************
 function:		LoadRawData (long* lFirst, long* lLast, short nspan)

 purpose:		load data from file into memory buffer
 parameters:	
	_ lFirst = file channel index of first pt (address of variable)
	_ lLast = file channel index of last point (address of variable)
	_ nspan = try having nspan points before lFirst and nspan after lLast
			  (necessary for transforms)
 returns:		lFirst , lLast: chan file indexes of first and last pt 
				buffer: data loaded from file
				TRUE if no error
 comments:      load as much data as possible within buffer
 **************************************************************************/
BOOL CAcqDataDoc::LoadRawData(long* lFirst, long* lLast, int nspan)
{
		BOOL flag = TRUE;

	if ((*lFirst - nspan < m_lBUFchanFirst) 
	|| (*lLast + nspan > m_lBUFchanLast) || !m_bValidReadBuffer)
	{
		flag = ReadDataBlock(*lFirst - nspan);
		m_bValidReadBuffer = TRUE;
		m_bValidTransfBuffer = FALSE;	
	}

	*lFirst = m_lBUFchanFirst + nspan;
	*lLast = m_lBUFchanLast;
	if (m_lBUFchanLast < m_lDOCchanLength-1)
		*lLast -= nspan;

	return flag;
}

/**************************************************************************
 function:	ReadDataBlock(long lFirst)
 purpose:	Read a data block from a given file offset
 parameters:
	_ lFirst =
 return:	FALSE if something is wrong;
			TRUE & update m_lBUFchanFirst, m_lBUFchanLast
 comments: 
 **************************************************************************/

BOOL CAcqDataDoc::ReadDataBlock(long lFirst)
{
	// check limits
	if (lFirst < 0) 
		lFirst = 0;
	// compute limits
	m_lBUFchanFirst = lFirst;
	m_lBUFchanLast = m_lBUFchanFirst + m_lBUFchanSize-1;
	if (m_lBUFchanLast > m_lDOCchanLength)
	{
		if (m_lDOCchanLength < m_lBUFchanSize)
		{
			m_lBUFchanSize = m_lDOCchanLength;
			m_lBUFchanLast = m_lBUFchanSize-1;
		}
		m_lBUFchanFirst = m_lDOCchanLength - m_lBUFchanSize;
	}
	lFirst = m_lBUFchanFirst * m_DOCnbchans;;
	
	// reallocate buffer if needed
	if (m_pWBuf->GetWBNumElements() != m_lBUFchanSize)
		AllocBUF();

	// read data from file
	if (m_pXFile != NULL)
	{
		short* pBuffer = m_pWBuf->GetWBAdrRawDataBuf();
		ASSERT(pBuffer != NULL);
		long lSize = m_pXFile->ReadData(lFirst, m_lBUFSize*sizeof(short), pBuffer);
		m_lBUFchanLast = m_lBUFchanFirst + lSize/m_DOCnbchans - 1;

		// remove offset so that data are signed short (for offset binary data of 12 or 16 bits resolution)
		WORD wbinzero = (WORD) m_pWBuf->m_waveFormat.binzero;
		if (m_bRemoveOffset && wbinzero != NULL)
		{
			WORD* pwBuf = (WORD*) m_pWBuf->GetWBAdrRawDataBuf();
			for (long i = 0; i < lSize; i++, pwBuf++)
				*pwBuf -= wbinzero;
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
 function:		BGetVal (short chan, long lIndex)
 purpose: Returns value and refreshes data if necessery
 parameters:
	_ short chan : channel of the current plot
	_ long lIndex : index of the current plot
 returns:			
 comments:      
 **************************************************************************/

int CAcqDataDoc::BGetVal(int chan, long lIndex)
{
	if ((lIndex < m_lBUFchanFirst) || (lIndex > m_lBUFchanLast))
		ReadDataBlock(lIndex);
	int index = lIndex-m_lBUFchanFirst;	
	return *(m_pWBuf->GetWBAdrRawDataElmt(chan, index));
}

/**************************************************************************
 function:		LoadTransfData (long* lFirst, long* lLast, short transformType, short chan)

 purpose:		load data from file into memory buffer and store given transform
				into transform buffer
 parameters:	lFirst = file channel index of first pt (address of variable)
				lLast = file channel index of last point (address of variable)
				transformType = index to transformation request
				sourcechan = source channel

 returns:		lFirst , lLast: chan file indexes of first and last pt 
				buffer: data loaded from file

 comments:      load as many data as possible within buffer
 **************************************************************************/

short* CAcqDataDoc::LoadTransfData(long lFirst, long lLast, int transformType, int ns)
{
	BOOL bAlreadyDone = (m_bValidTransfBuffer 
					 && (lFirst == m_tBUFfirst )
					 && (lLast == m_tBUFlast)
					 && (m_tBUFtransform == transformType)
					 && (m_tBUFsourcechan == ns));
	m_tBUFtransform = transformType;
	m_tBUFsourcechan = ns;
	m_tBUFfirst = lFirst;
	m_tBUFlast = lLast;

	if (m_pWBuf->GetWBAdrTransfData() == NULL)
		m_pWBuf->InitWBTransformMode(transformType);
	BOOL flag = TRUE;
	int ispan = m_pWBuf->GetWBTransformSpan(transformType);
	long lspan = (long) ispan;
	
	// ASSERT make sure that all data requested are within the buffer ...
	ASSERT(!(lFirst < m_lBUFchanFirst) && !(lLast > m_lBUFchanLast));
	if (((lFirst-lspan) < m_lBUFchanFirst) || ((lLast+lspan) > m_lBUFchanLast)) // we should never get there
		flag = ReadDataBlock(lFirst-lspan);							// but, just in case 

	int npoints = (int) (lLast - lFirst +1);
	int nchans = GetpWaveFormat()->scan_count;
	ASSERT(ns < nchans);											// make sure this is a valid channel
	int ioffset = (lFirst-m_lBUFchanFirst)* nchans + ns;
	short* lpSource = m_pWBuf->GetWBAdrRawDataBuf()	+ ioffset;

	// call corresponding one-pass routine
	short* lpDest = m_pWBuf->GetWBAdrTransfData();

	// check if source lFirst can be used
	BOOL bcond1 = (lFirst < lspan);
	if (bcond1)	// no: skip these data and erase later corresp transform
	{
		lpSource += nchans * ispan;
		lpDest += ispan;
		npoints -= ispan;
	}

	// check if source lLast can be used
	BOOL bcond2 = (lLast > m_lDOCchanLength-lspan);
	if (bcond2)	// no: skip these data and erase later corresp transform
	{
		npoints -= ispan;
	}

	if (npoints <= 0)
	{
		// erase data at the end of the buffer
		int cx = ispan + lLast - lFirst+1;
		short* lpDest0 = m_pWBuf->GetWBAdrTransfData();
		for (cx ; cx>0; cx--, lpDest0++)
			*lpDest0 = 0;
	}
	else if (!bAlreadyDone)
	{
		switch (transformType)
		{
			case 0:  m_pWBuf->BCopy	    (lpSource,  lpDest, npoints); break;
			case 1:	 m_pWBuf->BDeriv	(lpSource,  lpDest, npoints); break;
			case 2:	 m_pWBuf->BLanczo2	(lpSource,  lpDest, npoints); break;
			case 3:	 m_pWBuf->BLanczo3	(lpSource,  lpDest, npoints); break;
			case 4:	 m_pWBuf->BDeri1f3	(lpSource,  lpDest, npoints); break;
			case 5:	 m_pWBuf->BDeri2f3	(lpSource,  lpDest, npoints); break;
			case 6:	 m_pWBuf->BDeri2f5	(lpSource,  lpDest, npoints); break;
			case 7:	 m_pWBuf->BDeri3f3	(lpSource,  lpDest, npoints); break;
			case 8:	 m_pWBuf->BDiffer1	(lpSource,  lpDest, npoints); break;
			case 9:	 m_pWBuf->BDiffer2	(lpSource,  lpDest, npoints); break;
			case 10: m_pWBuf->BDiffer3	(lpSource,  lpDest, npoints); break;
			case 11: m_pWBuf->BDiffer10	(lpSource,  lpDest, npoints); break;		
			case 12: m_pWBuf->BMovAvg30	(lpSource,  lpDest, npoints); break;
			case 13: m_pWBuf->BMedian30	(lpSource,  lpDest, npoints); break;
			case 14: m_pWBuf->BMedian35	(lpSource,  lpDest, npoints); break;
			case 15: m_pWBuf->BRMS      (lpSource,  lpDest, npoints);
			default: flag = FALSE; break; 
		}
		
		// set undefined pts equal to first valid point
		if (bcond1)
		{	
			short* lpDest0 = m_pWBuf->GetWBAdrTransfData();
			for (int cx = ispan; cx>0; cx--, lpDest0++)
				*lpDest0 = 0;
			npoints += ispan;
			lpDest -= ispan;
		}

		// set undefined points at the end equal to last valid point
		if (bcond2)
		{
			short* lpDest0 = lpDest + npoints;
			for (int cx = ispan; cx>0; cx--, lpDest0++)
				*lpDest0 = 0;
		}
	}
	m_bValidTransfBuffer = TRUE;
	return lpDest;
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

BOOL CAcqDataDoc::BuildTransfData(int transformType, int ns)
{
	// make sure that transform buffer is ready
	if (m_pWBuf->GetWBAdrTransfData() == NULL)
		m_pWBuf->InitWBTransformMode(transformType);
	BOOL flag = TRUE;

	// init parameters
	short* lpSource = m_pWBuf->GetWBAdrRawDataBuf()	+ ns;
	int nchans = GetpWaveFormat()->scan_count;
	ASSERT(ns < nchans);

	// adjust pointers according to nspan - (fringe data) and set flags erase these data at the end
	short* lpDest = m_pWBuf->GetWBAdrTransfData();
	int ispan = m_pWBuf->GetWBTransformSpan(transformType);
	long lspan = (long) ispan;
	long lFirst = m_lBUFchanFirst+lspan;
	long lLast = m_lBUFchanLast-lspan;
	lpSource += nchans * ispan;
	lpDest += ispan;
	int npoints = lLast - lFirst +1;
	ASSERT(npoints > 0);

	switch (transformType)
	{
		case 0:  m_pWBuf->BCopy	    (lpSource,  lpDest, npoints); break;
		case 1:	 m_pWBuf->BDeriv	(lpSource,  lpDest, npoints); break;
		case 2:	 m_pWBuf->BLanczo2	(lpSource,  lpDest, npoints); break;
		case 3:	 m_pWBuf->BLanczo3	(lpSource,  lpDest, npoints); break;
		case 4:	 m_pWBuf->BDeri1f3	(lpSource,  lpDest, npoints); break;
		case 5:	 m_pWBuf->BDeri2f3	(lpSource,  lpDest, npoints); break;
		case 6:	 m_pWBuf->BDeri2f5	(lpSource,  lpDest, npoints); break;
		case 7:	 m_pWBuf->BDeri3f3	(lpSource,  lpDest, npoints); break;
		case 8:	 m_pWBuf->BDiffer1	(lpSource,  lpDest, npoints); break;
		case 9:	 m_pWBuf->BDiffer2	(lpSource,  lpDest, npoints); break;
		case 10: m_pWBuf->BDiffer3	(lpSource,  lpDest, npoints); break;
		case 11: m_pWBuf->BDiffer10	(lpSource,  lpDest, npoints); break;		
		case 12: m_pWBuf->BMovAvg30	(lpSource,  lpDest, npoints); break;
		case 13: m_pWBuf->BMedian30	(lpSource,  lpDest, npoints); break;
		case 14: m_pWBuf->BMedian35	(lpSource,  lpDest, npoints); break;
		case 15: m_pWBuf->BRMS      (lpSource,  lpDest, npoints);
		default: flag = FALSE; break; 
	}
	
	// set undefined pts equal to first valid point
	if (ispan > 0)
	{	
		short* lpDest0 = m_pWBuf->GetWBAdrTransfData();
		for (int cx = ispan; cx>0; cx--, lpDest0++)
			*lpDest0 = 0;
		npoints += ispan;
		lpDest -= ispan;
	
		lpDest0 = m_pWBuf->GetWBAdrTransfData() + npoints;
		for (int cx = ispan; cx>0; cx--, lpDest0++)
			*lpDest0 = 0;
	}
	return flag;
}

//////////////////////////////////////////////////////////////////////////////
// awave file
// create new awave file
/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::CreateAcqFile(CString &csFileName)
{
	if (!csFileName.IsEmpty())
	{
		// Create file - send message if creation failed
		if (m_pXFile == NULL)
		{
			m_pXFile = new CDataFileAWAVE;
			ASSERT(m_pXFile != NULL);
		}
		if (m_pXFile->m_idType != DOCTYPE_AWAVE)
		{
			delete m_pXFile;
			m_pXFile = new CDataFileAWAVE;
			ASSERT (m_pXFile != NULL);
		}

		CFileException fe;	

		if (!m_pXFile->Open(csFileName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone, &fe)) //shareDenyNone
		{
			AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
			return FALSE;
		}
		m_pXFile->InitFile();
	}

	// create Awave object as file
	AllocBUF();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::WriteHZtags(CTagList* ptags)
{
	if (ptags == NULL)
		ptags = &m_HZtags;
	if (ptags == NULL || ptags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteHZtags(ptags);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::WriteVTtags(CTagList* ptags)
{
	if (ptags == NULL)
		ptags = &m_VTtags;
	if (ptags == NULL || ptags->GetNTags() == 0)
		return TRUE;
	return m_pXFile->WriteVTtags(ptags);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppendStart()
{
	// start from fresh?
	if (m_pXFile == NULL  || m_pXFile->m_idType != DOCTYPE_AWAVE)
	{
		delete m_pXFile;
		m_pXFile = new CDataFileAWAVE;
		ASSERT (m_pXFile != NULL);
	}

	return m_pXFile->DataAppendStart();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppend(short* pBU,  UINT uibytesLength)
{
	m_pXFile->DataAppend(pBU, uibytesLength);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CAcqDataDoc::AcqDoc_DataAppendStop()
{
	m_pXFile->DataAppendStop();
	CWaveFormat* pwaveFormat = GetpWaveFormat();
	pwaveFormat->sample_count= (long) (m_pXFile->m_ulbytescount/sizeof(short));
	pwaveFormat->duration = pwaveFormat->sample_count
					/pwaveFormat->scan_count
					/pwaveFormat->chrate;
	m_pXFile->WriteDataInfos(pwaveFormat, GetpWavechanArray());
	m_pXFile->Flush();
	return TRUE;
}

BOOL CAcqDataDoc::AcqSaveDataDescriptors()
{
	BOOL flag = m_pXFile->WriteDataInfos(GetpWaveFormat(), GetpWavechanArray());
	m_pXFile->Flush();
	return flag;
}

// delete file on the disk
void CAcqDataDoc::AcqDeleteFile()
{
	CString csFilePath = m_pXFile->GetFilePath();
	m_pXFile->Close();
	CFile::Remove(csFilePath);
}

void CAcqDataDoc::AcqCloseFile()
{
	if (m_pXFile != NULL && m_pXFile->m_hFile != CFile::hFileNull)
		m_pXFile->Close();
}

/////////////////////////////////////////////////////////////////////////////
// save current data file under a new name
// default : save under awave format
// (other options not implemented yet)
// if "overwrite" (saveas is the same name than the source file: 
//    create new file, copy into old file, delete new file

BOOL CAcqDataDoc::SaveAs(CString &sznewName, BOOL bCheckOverWrite, int iType)
{
	// check if file of correct type
	CString ext;
	int iExt = sznewName.ReverseFind('.');
	if (iExt > 0) 
	{
		ext = sznewName.Right(sznewName.GetLength() - iExt -1);
		BOOL bDat = (ext.Compare(_T("dat")) == 0);
		if (!bDat)
		{
			ext = sznewName.Left(iExt +1);
			sznewName = ext + _T("dat");
		}
	}

	CString dummyName = sznewName;
	CString csFormerName = m_pXFile->GetFilePath();

	// check if same file already exist
	CFileStatus rStatus;	// file status: time creation, ...
	BOOL bFlagExist = CFile::GetStatus(sznewName, rStatus);
	if (bFlagExist != 0 && bCheckOverWrite)
	{
		CString prompt=sznewName;
		prompt += _T("\nThis file already exists.\nReplace existing file?");
		if (AfxMessageBox(prompt, MB_YESNO) != IDYES)
			return FALSE;       // don't continue
	}

	// check if collision with current file
	BOOL bCollision = FALSE;
	if (bFlagExist &&
		csFormerName.CompareNoCase(rStatus.m_szFullName) == 0) // used in conjunction with CFile::GetStatus()
	{
		bCollision = TRUE;
		dummyName = sznewName;
		dummyName += _T("_tmp.dat");
	}

	// create new file
	CAcqDataDoc* pNewDoc = new CAcqDataDoc;
	if(!pNewDoc->CreateAcqFile(dummyName))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete pNewDoc;
		return FALSE;
	}
	
	// save data header
	if (GetpWaveFormat()->scan_count < GetpWavechanArray()->ChannelGetnum())
	{
		int lastchannel = GetpWaveFormat()->scan_count-1;
		for (int i= GetpWavechanArray()->ChannelGetnum()-1; i> lastchannel; i--)
			GetpWavechanArray()->ChannelRemove(i);
	}

	// save data
	pNewDoc->AcqDoc_DataAppendStart();
	long nsamples = GetpWaveFormat()->sample_count;

	// position source file index to start of data
	m_pXFile->Seek(m_pXFile->m_ulOffsetData, CFile::begin);
	short* pBuf = m_pWBuf->GetWBAdrRawDataBuf();// buffer to store data
	long BufTempSize = m_lBUFSize;		// length of the buffer

	while (nsamples > 0)				// loop until the end of the file
	{
		// read data from source file into buffer
		if (nsamples < m_lBUFSize)		// adjust buftempsize
			BufTempSize = nsamples;		// then, store data in temporary buffer
		long nbytes = BufTempSize*sizeof(short);
		m_pXFile->Read(pBuf, nbytes);
		if (iType == 3)	// ASD file
		{
			short *pBuf2 = pBuf;
			for (int i=0; i< nbytes; i++, pBuf2 +2)
				*pBuf2 = *pBuf2 /2;
		}
		pNewDoc->AcqDoc_DataAppend(pBuf, nbytes); // save buffer
		nsamples -= BufTempSize;		// update counter and loop
	}
	
	// stop appending data, update dependent struct
	pNewDoc->AcqDoc_DataAppendStop();

	// save other objects if exist (tags, others)
	BOOL flag=TRUE;
	if (m_HZtags.GetNTags() > 0)
		flag = pNewDoc->WriteHZtags(&m_HZtags);
	if (m_VTtags.GetNTags() > 0)
		flag = pNewDoc->WriteVTtags(&m_VTtags);

	// if destination name == source: remove source and rename destination
	if (bCollision)
	{
		// copy dummy name into csFormerName
		ULONGLONG dwNewLength= pNewDoc->m_pXFile->GetLength();
		m_pXFile->SetLength( dwNewLength );
		m_pXFile->SeekToBegin();
		pNewDoc->m_pXFile->SeekToBegin();

		pBuf = m_pWBuf->GetWBAdrRawDataBuf();// buffer to store data
		long nbytes = m_lBUFSize*sizeof(short);
		DWORD dwRead;
		do
		{
			dwRead = pNewDoc->m_pXFile->Read(pBuf, nbytes);
			m_pXFile->Write(pBuf, dwRead);
		}
		while (dwRead > 0);

		// file is transferred, destroy temporary file
		pNewDoc->m_pXFile->Close();
		//CFile::Remove(dummyName);

		// delete current file object and open saved-as file ??
		m_pXFile->Close();
		SAFE_DELETE(m_pXFile);
	
		// create / update CDataFileX associated object	
		m_pXFile = new CDataFileAWAVE; //((CFile*) this);
		ASSERT(m_pXFile != NULL);

		// open saved file
		CFileException fe;	
		if (!m_pXFile->Open(sznewName, CFile::modeReadWrite | CFile::shareDenyNone| CFile::typeBinary, &fe))
		{		
			m_pXFile->Abort();
			return FALSE;
		}
	}

	// destroy object created
	delete pNewDoc;

	return TRUE;
}

