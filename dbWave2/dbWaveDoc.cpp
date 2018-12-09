// dbWaveDoc.cpp : implementation of the CdbWaveDoc class
//

#include "StdAfx.h"
#include "dbWave.h"

#include "dbMainTable.h"
#include "Spikedoc.h"		// spike document
#include "dbWaveDoc.h"		// dbWave document
#include "dbNewFileDuplicateDlg.h"
#include "ProgDlg.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbWaveDoc

IMPLEMENT_DYNCREATE(CdbWaveDoc, COleDocument)

BEGIN_MESSAGE_MAP(CdbWaveDoc, COleDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE,			COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK,	COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT,	COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT,				COleDocument::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS,		COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS,				COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, COleDocument::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdbWaveDoc construction/destruction

CdbWaveDoc::CdbWaveDoc()
{
	m_pDB = nullptr;
	m_pDat = nullptr;
	m_pSpk = nullptr;
	m_bcallnew = TRUE;
	m_hMyMenu = nullptr;
	m_validTables = FALSE;

	m_icurSpkList = 0;
	m_bClearMdbOnExit = FALSE;
	m_bTranspose = false;
}

/////////////////////////////////////////////////////////////////////////////

CdbWaveDoc::~CdbWaveDoc()
{
	SAFE_DELETE(m_pDB);
	SAFE_DELETE(m_pDat);
	SAFE_DELETE(m_pSpk);

	// store temp mdb filename to delete on exit within the application
	// (it seems it is not possible to erase the file from here)
	if (m_bClearMdbOnExit)
	{
		CdbWaveApp* pApp = nullptr;
		pApp = (CdbWaveApp*) AfxGetApp();
		pApp->m_tempMDBfiles.Add(m_dbFilename);
	}
}

void CdbWaveDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CDocument::UpdateAllViews(pSender, lHint, pHint);
	// passes message OnUpdate() to the mainframe and add a reference to the document that sends it
	CMainFrame* pmain = (CMainFrame*) AfxGetMainWnd();
	pmain->OnUpdate((CView*) this, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;
	m_bcallnew = FALSE;
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	CString cspath = pApp->Get_MyDocuments_MydbWavePath();
	TCHAR szEntry[MAX_PATH];
	unsigned long len=MAX_PATH;
	GetUserName(&szEntry[0], &len);
	CString csExt = szEntry;
	csExt		+= _T("_1");
	cspath		+= _T('\\') + csExt;

	return OnNewDocument(cspath);
}

/////////////////////////////////////////////////////////////////////////////
// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::OnNewDocument(LPCTSTR lpszPathName)
{
	if (m_bcallnew && !COleDocument::OnNewDocument())
		return FALSE;
	m_bcallnew = TRUE;

	CString csName = lpszPathName;
	csName += _T(".mdb");
	if (csName.Find(_T(':')) < 0 
		&& csName.Find(_T("\\\\")) < 0)
	{
		CString cspath = ((CdbWaveApp*) AfxGetApp())->Get_MyDocuments_MydbWavePath();
		csName = cspath + _T('\\') + csName;
	}

	// check if this file is already present, exit if not...
	int iIDresponse=IDYES;	// default: go on if file not found
	CFileStatus status;
	BOOL bExist = CFile::GetStatus(csName, status);
	
	while (bExist)
	{
		CdbNewFileDuplicateDlg dlg;
		dlg.m_pfilein = csName;
		if (IDOK == dlg.DoModal())
		{
			switch (dlg.m_option)
			{
			case 0:				// open file
				m_pDB = new CdbWdatabase;
				m_pDB->Attach(&m_currentDatafileName, &m_currentSpikefileName);
				return OpenDatabase(csName);
				break;
			case 1:				// overwrite file
				CFile::Remove(csName);
				bExist = FALSE;
				break;			
			case 2:				// try another name
				csName = dlg.m_fileout;
				bExist = CFile::GetStatus(csName, status);
				break;
			case 3:				// abort process
			default:
				m_validTables = FALSE;
				return FALSE;
				break;
			}
		}
		else
		{
			m_validTables = FALSE;
			return FALSE;
		}
	}

	// create dbFile
	m_pDB = new CdbWdatabase;
	m_pDB->Attach(&m_currentDatafileName, &m_currentSpikefileName);
	m_pDB->Create(csName, dbLangGeneral, dbVersion30);
	m_dbFilename = csName;
	m_ProposedDataPathName = csName.Left(csName.ReverseFind('.'));
	m_pDB->CreateTables();

	// save file name
	m_validTables = m_pDB->OpenTables();
	SetTitle(csName);
	SetPathName(csName, TRUE);

	// return to caller
	return m_validTables;
}

/////////////////////////////////////////////////////////////////////////////
// CdbWaveDoc serialization

void CdbWaveDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CdbWaveDoc diagnostics

#ifdef _DEBUG
void CdbWaveDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CdbWaveDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CdbWaveDoc commands

BOOL CdbWaveDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// open database document
	CString csnew = lpszPathName;
	CFileStatus status;
	if (!CFile::GetStatus(lpszPathName, status))
		return FALSE;

	csnew.MakeLower();
	CString csExt ;
	csExt.Empty();
	int iext = csnew.ReverseFind('.');
	if (iext > 0)
		csExt = csnew.Right(csnew.GetLength() - iext -1);

	if ((csExt.Compare(_T("mdb")) == 0)||(csExt.Compare(_T("accdb")) == 0))
	{	
		if (!COleDocument::OnOpenDocument(lpszPathName))
			return FALSE;
		BOOL flag= OpenDatabase (lpszPathName);
		return flag;
	}

	// open spike or dat documents
	else if ((csExt.Compare(_T("dat")) == 0) || (csExt.Compare(_T("spk")) == 0)
		|| (csExt.Compare(_T("asd")) == 0))
	{
		if (iext > 0)
			csnew = csnew.Left(iext);
		BOOL flag = OnNewDocument(csnew);
		if (flag)
		{
			CStringArray filenames;
			filenames.Add(lpszPathName);
			ImportDescFromFileList(filenames);
		}
		return flag;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CdbWaveDoc::OpenDatabase (LPCTSTR lpszPathName) 
{
	CdbWdatabase* tmpDB = new CdbWdatabase;
	tmpDB->Attach(&m_currentDatafileName, &m_currentSpikefileName);

	// Attempt to open the new database before replacing our ptr
	try
	{
		tmpDB->Open(lpszPathName);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 4);
		e->Delete();
		tmpDB->Close();
		delete tmpDB;
		return FALSE;
	}
	m_dbFilename = lpszPathName;
	m_ProposedDataPathName = m_dbFilename.Left(m_dbFilename.ReverseFind('.'));

	// pass file pointer to document pointer
	if (m_pDB != nullptr)
	{
		if (m_pDB->IsOpen())
			m_pDB->Close();
		delete m_pDB;
	}
	m_pDB = tmpDB;
	
	// save file name 
	m_validTables = m_pDB->OpenTables();
	SetPathName(lpszPathName, TRUE);

	return m_validTables;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CdbWaveDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// now duplicate file
	CString csoldname = GetPathName();
	BOOL bFailifExists=TRUE;

	BOOL bDone = ::CopyFile(csoldname, lpszPathName, bFailifExists);
	if (bDone == 0)
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			nullptr);

		// Process any inserts in lpMsgBuf.
		CString csError = (LPTSTR) lpMsgBuf;
		csError = _T("Error when copying:\n") + csoldname + _T("\ninto:\n") + lpszPathName + _T("\n\n") + csError;

		// Display the string.
		MessageBox(nullptr, csError, _T("Error"), MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CString CdbWaveDoc::DBDefineCurrentSpikeFileName() {

	if (!DBGetCurrentSpkFileName(TRUE).IsEmpty())
		return m_currentSpikefileName;

	// not found? derive name from data file (same directory)
	m_currentSpikefileName = m_currentDatafileName.Left(m_currentDatafileName.ReverseFind(_T('.')) + 1) + _T("spk");

	int icount = m_currentSpikefileName.ReverseFind(_T('\\'));
	//CString csPath = m_currentSpikefileName.Left(icount);
	CString csName = m_currentSpikefileName.Right(m_currentSpikefileName.GetLength() - icount - 1);

	try 
	{
		m_pDB->m_mainTableSet.Edit();

		// change spike file name
		m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), TRUE);
		m_pDB->m_mainTableSet.m_Filespk = csName;

		m_pDB->m_mainTableSet.m_path2_ID = m_pDB->m_mainTableSet.m_path_ID;
		m_pDB->m_mainTableSet.Update();
	}
	catch(CDaoException* e) 
	{
		DisplayDaoException(e, 10); 
		e->Delete();
	}
	return m_currentSpikefileName;
}


CString CdbWaveDoc::DBGetCurrentDatFileName(BOOL bTest)
{
	m_currentDatafileName = m_pDB->GetDatFilenameFromCurrentRecord();
	CString filename = m_currentDatafileName;
	if (bTest && !IsFilePresent(filename))
		filename.Empty();
	return filename;	
}

CString CdbWaveDoc::DBGetCurrentSpkFileName(BOOL bTest)
{
	m_currentSpikefileName = m_pDB->GetSpkFilenameFromCurrentRecord();
	CString filename = m_currentSpikefileName;
	if (bTest && !IsFilePresent(filename))
		filename.Empty();
	return filename;
}

/////////////////////////////////////////////////////////////////////////////

long CdbWaveDoc::DBGetDataLen()
{
	long datalen = 0;
	try {datalen = m_pDB->m_mainTableSet.m_datalen;}
	catch(CDaoException* e) {DisplayDaoException(e, 11); e->Delete();}

	return datalen;
}

/////////////////////////////////////////////////////////////////////////////

// Open current data file
// return TRUE if OK, FALSE if a problem occurred and the operation has aborted

BOOL CdbWaveDoc::OpenCurrentDataFile()
{
	// data doc to read data files
	if (m_pDat == nullptr)
	{
		m_pDat = new CAcqDataDoc;
		ASSERT(m_pDat != NULL);
	}

	// open document; erase object if operation failed
	if (m_currentDatafileName.IsEmpty() 
		|| !m_pDat->OnOpenDocument(m_currentDatafileName))
	{
		delete m_pDat;
		m_pDat = nullptr;
		return FALSE;
	}
	else
		m_pDat->SetPathName(m_currentDatafileName, FALSE);

	return TRUE;
}

// Open current spike file
// return TRUE if OK and FALSE if operation has aborted

BOOL CdbWaveDoc::OpenCurrentSpikeFile()
{
	// spike doc to read data files
	if (m_pSpk == nullptr)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}
	// open document; erase object if operation fails
	if (m_currentSpikefileName.IsEmpty() 
		|| !m_pSpk->OnOpenDocument(m_currentSpikefileName))
	{
		delete m_pSpk;
		m_pSpk = nullptr;
		return FALSE;
	}
	else
		m_pSpk->SetPathName(m_currentSpikefileName, FALSE);
	return TRUE;
}

void CdbWaveDoc::GetAllSpkMaxMin(BOOL bAllFiles, BOOL bRecalc, int* max, int* min)
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (bAllFiles)
	{
		nfiles = DBGetNRecords();
		ncurrentfile = DBGetCurrentRecordPosition();
	}

	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (bAllFiles)
		{
			DBSetCurrentRecordPosition(ifile);
			OpenCurrentSpikeFile();
			m_pSpk->SetSpkListCurrent(0);
		}
		CSpikeList* pspklist = m_pSpk->GetSpkListCurrent();
		pspklist->GetTotalMaxMin(bRecalc, max, min);
	}

	if (bAllFiles)
	{
		DBSetCurrentRecordPosition(ncurrentfile);
		OpenCurrentSpikeFile();
		m_pSpk->SetSpkListCurrent(0);
	}
}

/////////////////////////////////////////////////////////////////////////////

long CdbWaveDoc::DBGetCurrentRecordPosition()
{
	long ifile = -1;
	try	{ ifile = m_pDB->m_mainTableSet.GetAbsolutePosition(); }
	catch(CDaoException* e) {DisplayDaoException(e, 14); e->Delete();}
	return ifile;
}

long CdbWaveDoc::DBGetCurrentRecordID()
{
	return m_pDB->m_mainTableSet.m_ID;
}

void CdbWaveDoc::DBSetCurrentRecordFlag(int flag)
{
	m_pDB->m_mainTableSet.Edit();
	m_pDB->m_mainTableSet.m_flag = flag;
	m_pDB->m_mainTableSet.Update();
}

void CdbWaveDoc::DBSetPathsRelative_to_DataBaseFile()
{
	m_pDB->ConvertPathTabletoRelativePath();
}

void CdbWaveDoc::DBSetPathsAbsolute()
{
	m_pDB->ConvertPathTabletoAbsolutePath();
}

void CdbWaveDoc::DBTransferDatPathToSpkPath()
{
	m_pDB->m_mainTableSet.CopyPathToPath2();
}

void CdbWaveDoc::DBDeleteUnusedEntries()
{
	m_pDB->DeleteUnusedEntriesInAccessoryTables();
}

/////////////////////////////////////////////////////////////////////////////

void CdbWaveDoc::Setnbspikes(long nspikes)
{
	try 
	{
		m_pDB->m_mainTableSet.Edit();
		m_pDB->m_mainTableSet.m_nspikes = nspikes;
		m_pDB->m_mainTableSet.Update();
	}
	catch(CDaoException* e) {DisplayDaoException(e, 15); e->Delete();}
}

/////////////////////////////////////////////////////////////////////////////

long CdbWaveDoc::Getnbspikes()
{
	return m_pDB->m_mainTableSet.m_nspikes;
}

/////////////////////////////////////////////////////////////////////////////

long CdbWaveDoc::Getnbspikeclasses()
{
	return m_pDB->m_mainTableSet.m_nspikeclasses;
}

/////////////////////////////////////////////////////////////////////////////

void CdbWaveDoc::Setnbspikeclasses(long nclasses)
{
	try 
	{
		m_pDB->m_mainTableSet.Edit();
		m_pDB->m_mainTableSet.m_nspikeclasses = nclasses;
		m_pDB->m_mainTableSet.Update();
	}
	catch(CDaoException* e) {DisplayDaoException(e, 16); e->Delete();}
}

/////////////////////////////////////////////////////////////////////////////

void CdbWaveDoc::ExportDataAsciiComments(CSharedFile* pSF) //, OPTIONS_VIEWDATA* pVD)
{
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep (1);	
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	CString csDummy;
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	OPTIONS_VIEWDATA* pVD = &(pApp->vdP);
	OPTIONS_VIEWSPIKES* pvdS = &(pApp->vdS);

	int indexcurrent = DBGetCurrentRecordPosition();
	int nfiles = DBGetNRecords();

	//-------------------------------------------------------
	// memory allocated -- get pointer to it		
	csDummy.Format(_T("n files = %i\r\n\r\n"), nfiles);
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	
	// loop over all files of the multi-document
	for (int ifile = 0; ifile < nfiles; ifile++)
	{
		// get ith file's comment
		DBSetCurrentRecordPosition(ifile);
		csDummy.Format(_T("%i\t%i\t"),   ifile+1, m_pDB->m_mainTableSet.m_ID);
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

		// set message for dialog box
		// check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i] %s"), ifile+1, nfiles, (LPCTSTR) m_currentDatafileName);
		dlg.SetStatus(cscomment);

		csDummy.Empty();
		if (OpenCurrentDataFile())
		{
			csDummy += m_pDat->GetDataFileInfos(pVD);	// get infos
			if (pVD->bdatabasecols)
				csDummy += ExportDatabaseData();
		}
		else
			csDummy += _T("file not found\t");
		
		csDummy+= _T("\r\n");		// next line
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));

		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}

	//-------------------------------------------------------
	DBSetCurrentRecordPosition(indexcurrent);
	OpenCurrentDataFile();	
}

BOOL CdbWaveDoc::CreateDirectories(CString path)
{
	// first decompose the path into subdirectories
	CStringArray csSubpaths;
	CString newPath = path;
	int fpos = 1;
	BOOL bExist = FALSE;
	do 
	{
		// test if directory already present
		bExist = PathFileExists(newPath);

		// if not, strip the chain down from one subdirectory
		if (!bExist) 
		{
			fpos = newPath.ReverseFind(_T('\\'));
			if (fpos != -1) 
			{
				CString subpath = newPath.Right(newPath.GetLength() - fpos);
				csSubpaths.Add(subpath);
				newPath = newPath.Left(fpos);
			}
		}
	} while (fpos > 0 && bExist == FALSE);

	// now, newPath should be something like "c:" (if data are on a disk) or "" (if data are on a network like "\\partage" )
	for (int i = csSubpaths.GetUpperBound(); i>= 0; i--)
	{
		newPath = newPath + csSubpaths.GetAt(i);
		if (!CreateDirectory(newPath, nullptr))
		{
			AfxMessageBox(IDS_DIRECTORYFAILED);
			return FALSE;
		}
	}
	// then loop through array to create directories
	return TRUE;
}

BOOL CdbWaveDoc::CopyAllFilesintoDirectory(CString path)
{
	// prepare dialog box to tell what is going on ----------------------
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep (1);
	CString cscomment;
	CString csfilecomment = _T("Copy files");
	CString csDummy;
	int indexcurrent = DBGetCurrentRecordPosition();
	int nfiles = DBGetNRecords();
	
	// read all data paths and create new array --------------------------
	dlg.SetStatus(_T("Create destination directories..."));
	CStringArray	csSourcePath;
	CStringArray	csDestPath;
	CUIntArray		uiIDarray;
	CString cspath = path + _T('\\');
	BOOL flag = CreateDirectories(path);		// create root destination directory if necessary
	if (!flag)
		return FALSE;

	// loop over database pathSet : copy paths into 2 string arrays and create subdirectories
	m_pDB->m_pathSet.MoveFirst();
	int nrecords = 0;
	while (!m_pDB->m_pathSet.IsEOF())	
	{
		csSourcePath.Add(m_pDB->m_pathSet.m_cs);
		uiIDarray.Add(m_pDB->m_pathSet.m_ID);
		nrecords++;
		m_pDB->m_pathSet.MoveNext();
	}

	// find root & build new directories
	CString csRoot = csSourcePath[0];
	int icsRootlen = csRoot.GetLength();
	for (int i=1; i< nrecords; i++)
	{
		// trim strings to the same length from the left
		CString csCurrent = csSourcePath[i];
		if (icsRootlen > csCurrent.GetLength())
		{
			csRoot = csRoot.Left(csCurrent.GetLength());
			icsRootlen= csRoot.GetLength();
		}
		if (csCurrent.GetLength() > icsRootlen)
			csCurrent = csCurrent.Left(icsRootlen);

		// loop to find the smallest common string
		for (int j=icsRootlen; j>1; j--)
		{
			if (0 == csCurrent.CompareNoCase(csRoot))
				break;
			// not found, search next subfolder in rootstring and clip current
			int fpos = csRoot.ReverseFind(_T('\\'));
			if (fpos != -1) 
			{
				csRoot = csRoot.Left(fpos);
				icsRootlen = fpos;
			}
			csCurrent = csCurrent.Left(icsRootlen);
		}
	}

	// create new paths and create directories
	if (icsRootlen <= 2)
	{
		CString csBufTemp; 
		for (int i=0; i< nrecords; i++)
		{
			csBufTemp.Format( _T("%06.6lu"), i);
			CString csdummy = cspath + csBufTemp;
			csDestPath.Add(csdummy);
		}
	}
	else
	{
		for (int i=0; i< nrecords; i++)
		{
			CString subPath = csSourcePath.GetAt(i);
			subPath = subPath.Right(subPath.GetLength() - icsRootlen -1);
			CString csdummy = cspath + subPath;
			csDestPath.Add(csdummy);
		}
	}

	// copy database into new database ------------------------------------
	dlg.SetStatus(_T("Copy database ..."));
	CString olddatabase;
	CString destdatabase;
	olddatabase = GetPathName();
	int icount = olddatabase.ReverseFind(_T('\\'))+1;
	CString newpath = path;
	CString newname = olddatabase.Right(olddatabase.GetLength()-icount);
	int fpos = path.ReverseFind(_T('\\'));
	if (fpos != -1) 
	{
		newname = path.Right(path.GetLength() - fpos) + _T(".mdb");
		newpath = path.Left(fpos);
	}
	destdatabase = newpath + newname;

	// create database and copy all records
	flag = OnSaveDocument(destdatabase);
	if (!flag)
		return FALSE;

	// read all data and spike file names and store them into an array ------------
	if (m_pDB->m_mainTableSet.GetRecordCount() < 1)
		return FALSE;

	// create new document and open it to update the paths
	CdbWaveDoc* pNew = new CdbWaveDoc;
	flag = pNew->OnOpenDocument(destdatabase);
	if (!flag)
	{
		delete pNew;
		return FALSE;
	}

	// read filenames & copy records into target tableSet
	dlg.SetStatus(_T("Build a list of files to copy..."));

	// loop through all records of the source table
	flag = TRUE;
	CStringArray	csoldNames;
	CStringArray	csnewNames;
	CUIntArray		uiIDnewpath;
	try
	{
		m_pDB->m_mainTableSet.MoveFirst();
		
		while (!m_pDB->m_mainTableSet.IsEOF()) {
			m_pDB->GetFilenamesFromCurrentRecord();
			// data file
			if (!m_currentDatafileName.IsEmpty())
			{			
				csoldNames.Add(m_currentDatafileName);

				UINT uid = m_pDB->m_mainTableSet.m_path_ID;
				int i=0;
				for (i=0; i< uiIDarray.GetSize(); i++)
					if (uiIDarray.GetAt(i) == uid)
						break;
				uiIDnewpath.Add(i);
				csnewNames.Add(m_pDB->m_mainTableSet.m_Filedat);
			}
			// spike file
			if (!m_currentSpikefileName.IsEmpty())
			{
				csoldNames.Add(m_currentSpikefileName);
				UINT uid = m_pDB->m_mainTableSet.m_path2_ID;
				int i=0;
				for (i=0; i< uiIDarray.GetSize(); i++)
					if (uiIDarray.GetAt(i) == uid)
						break;
				uiIDnewpath.Add(i);
				csnewNames.Add(m_pDB->m_mainTableSet.m_Filespk);
			}
			// move to next record
			m_pDB->m_mainTableSet.MoveNext();
		}
	}
	catch(CDaoException* e) {
		DisplayDaoException(e, 13); 
		e->Delete(); 
		flag=FALSE;
	}
	
	// replace the destination paths
	dlg.SetStatus(_T("Update directories in the destination database..."));

	// change paths
	try
	{
		pNew->m_pDB->m_pathSet.MoveFirst();		
		while (!pNew->m_pDB->m_pathSet.IsEOF())	
		{
			CString path = pNew->m_pDB->m_pathSet.m_cs;
			// find corresponding path in old paths
			int ifound = -1;
			for (int i = 0; i < csSourcePath.GetSize(); i++) 
			{
				if (path.Compare(csSourcePath.GetAt(i)) == 0)
				{
					ifound = i;
					break;
				}
			}
			if (ifound > -1)
			{
				pNew->m_pDB->m_pathSet.Edit();
				pNew->m_pDB->m_pathSet.m_cs= csDestPath.GetAt(ifound);
				pNew->m_pDB->m_pathSet.Update();
			}
			pNew->m_pDB->m_pathSet.MoveNext();
		}
	}
	catch(CDaoException* e) {
		DisplayDaoException(e, 13); 
		e->Delete(); 
		flag=FALSE;
	}

	//-------------------------------------------------------
	// memory allocated -- get pointer to it
	nfiles = csoldNames.GetSize();
	csDummy.Format(_T("n files (*.dat & *.spk) = %i\r\n\r\n"), nfiles);
	
	// loop over all files of the multi-document
	for (int ifile = 0; ifile < nfiles; ifile++)
	{
		// set message for dialog box and check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;
		// get source name
		CString sourceFile	= csoldNames.GetAt(ifile);
		cscomment.Format(_T("Processing file [%i / %i] %s"), ifile+1, nfiles, (LPCTSTR) sourceFile);
		dlg.SetStatus(cscomment);
		
		// get destination directory
		int ipath = uiIDnewpath.GetAt(ifile);
		CString path = csDestPath.GetAt(ipath);
		// create directory if necessary
		BOOL flag = CreateDirectories(path);		// create root destination directory if necessary
		if (!flag)
			return FALSE;

		// copy file
		CString destFile = path+ _T("\\") + csnewNames.GetAt(ifile);
		BinaryFileCopy(sourceFile, destFile);

		// update count
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}

	//-------------------------------------------------------
	// remove non-relevant files from the new database
	if (pNew->m_pDB->m_mainTableSet.GetEditMode() != dbEditNone)
		pNew->m_pDB->m_mainTableSet.Update();
	pNew->m_pDB->m_mainTableSet.Close();				// close dynaset and open as datatable

	try {pNew->m_pDB->m_mainTableSet.Open(dbOpenTable, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return FALSE;}
	
	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	pNew->m_pDB->m_mainTableSet.MoveFirst();
	BOOL bDatFile;
	BOOL bSpkFile;
	CFileStatus rStatus;

	if (!pNew->m_pDB->m_mainTableSet.IsBOF())
	{
		while(!pNew->m_pDB->m_mainTableSet.IsEOF()) 
		{
			pNew->m_pDB->GetFilenamesFromCurrentRecord();
			bDatFile = CFile::GetStatus(pNew->m_currentDatafileName, rStatus);	// check if data file is present
			bSpkFile = CFile::GetStatus(pNew->m_currentSpikefileName, rStatus);	// check if spike file is present
			if (!bDatFile && !bSpkFile)											// if none of them found, remove record
				pNew->m_pDB->m_mainTableSet.Delete();
			pNew->m_pDB->m_mainTableSet.MoveNext();
		}
		pNew->m_pDB->m_mainTableSet.MoveFirst();
	}

	// close database and document
	delete pNew;
	return true;
}

//This function uses CFile to copy binary files.
bool CdbWaveDoc::BinaryFileCopy(LPCTSTR pszSource, LPCTSTR pszDest)
{
   // check that destfile does not exist
	// check if same file already exists: if yes, destroy it
	CFileStatus rStatus;
	CString csNewname = pszDest;
	if (CFile::GetStatus(csNewname, rStatus))
	{
		CString prompt=csNewname;
		prompt += _T("\nThis file seems to exist already.\nDelete the old file?");
		if (AfxMessageBox(prompt, MB_YESNO) == IDYES)
			CFile::Remove(csNewname);
		else
			return false;
	}
	
	// constructing these file objects doesn't open them
   CFile sourceFile;
   CFile destFile;

   // we'll use a CFileException object to get error information
   CFileException ex;
  
   // open the source file for reading
   if (!sourceFile.Open(pszSource,
	  CFile::modeRead | CFile::shareDenyNone, &ex))
   {
	  // complain if an error happened
	  // no need to delete the ex object

	  TCHAR szError[1024];
	  ex.GetErrorMessage(szError, 1024);
	  _tprintf_s(_T("Couldn't open source file: %1024s"), szError);
	  return false;
   }
   else
   {
	  if (!destFile.Open(pszDest, CFile::modeWrite |
		 CFile::shareExclusive | CFile::modeCreate, &ex))
	  {
		 TCHAR szError[1024];
		 ex.GetErrorMessage(szError, 1024);
		 _tprintf_s(_T("Couldn't open source file: %1024s"), szError);

		 sourceFile.Close();
		 return false;
	  }

	  BYTE buffer[4096];
	  DWORD dwRead;

	  // Read in 4096-byte blocks,
	  // remember how many bytes were actually read,
	  // and try to write that many out. This loop ends
	  // when there are no more bytes to read.
	  do
	  {
		 dwRead = sourceFile.Read(buffer, 4096);
		 destFile.Write(buffer, dwRead);
	  }
	  while (dwRead > 0);

	  // Close both files

	  destFile.Close();
	  sourceFile.Close();
   }

   return true;
}

/////////////////////////////////////////////////////////////////////////////

HMENU CdbWaveDoc::GetDefaultMenu()
{
	return m_hMyMenu;    // just use original default
}

/////////////////////////////////////////////////////////////////////////////

// import description from a list of data files. The descriptions saved
// into each of the datafiles are extracted and into the database 
// in: 
// filenames	CStringArray with a list of file names (with their path)
// bOnlygenuine	if false, each file will be first evaluated to determine its format

void CdbWaveDoc::ImportDescFromFileList(CStringArray& filenames, BOOL bOnlygenuine)
{
	// exit if no data to import
	int nfiles = filenames.GetSize();
	if (nfiles == 0)
		return;

	CString cscomment;
	CString csFile_test;
	CString csDummy;
	CSharedFile* psf= ((CdbWaveApp*) AfxGetApp())->m_psf;
	SAFE_DELETE(psf)
	((CdbWaveApp*) AfxGetApp())->m_psf = nullptr;
	psf = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();
	m_pDB->m_mainTableSet.Close();				// close dynaset and open as datatable
	try { m_pDB->m_mainTableSet.Open(dbOpenTable, nullptr, 0);}
	catch(CDaoException* e) 
	{
		AfxMessageBox(_T("Cancel import: ") + e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		delete psf;
		return;
	}
	
	// browse existing database array - collect data file acquisition time and IDs already used
	int irecmax_tableSet = m_pDB->m_mainTableSet.GetNRecords();	
	m_pDB->m_mainTableSet.GetMaxIDs();
	long mID= m_pDB->m_mainTableSet.max_ID;

	//int ilastbackslashposition = filenames[0].ReverseFind('\\');
	//CString csPath = filenames[0].Left(ilastbackslashposition);
	//csPath.MakeLower();

	int nfilesok = CheckifFilesCanbeOpened (filenames, psf);

	// ---------------------------------------------file loop: read infos --------------------------------
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetStep(1);
	dlg.SetPos(0);
	int istep=0;

	for (int ifile = 0; ifile < nfilesok; ifile++)
	{
		// get file name
		CString csFilename = filenames[ifile];

		// check if user wants to stop and update progression bar
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;		
		cscomment.Format(_T("Import file [%i / %i] %s"), ifile +1, nfilesok, (LPCTSTR) csFilename);
		dlg.SetStatus(cscomment);
		if (MulDiv(ifile, 100, nfilesok) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfilesok);
		}

		// open document and read data - go to next file if not readable
		CString csDatFile;
		CString csSpkFile;
		int ilastbackslashposition = csFilename.ReverseFind('\\');
		int idotposition = csFilename.ReverseFind('.');
		int namelen = idotposition - ilastbackslashposition -1;
		CString csPath = csFilename.Left(ilastbackslashposition);
		csPath.MakeLower();
		CString csExtent = csFilename.Right(csFilename.GetLength() - idotposition -1);
		CString csRootName = csFilename.Mid(ilastbackslashposition + 1, namelen);

		BOOL bIsDatFile = IsDatFile(csFilename);
		if (bIsDatFile) {
			csDatFile = csFilename;
			csSpkFile = csFilename.Left(idotposition) + _T(".spk");
		}
		else {
			csSpkFile = csFilename;
			csDatFile = csFilename.Left(idotposition) + _T(".dat");
		}

		// test  files
		CFileStatus rStatus;
		BOOL bDatPresent = CFile::GetStatus(csDatFile, rStatus);
		BOOL bSpikPresent = CFile::GetStatus(csSpkFile, rStatus);
		if (bDatPresent)
			csFilename = csDatFile;
		else if (bSpikPresent)
			csFilename = csSpkFile;
		CWaveFormat* pWF = GetWaveFormat(csFilename, bDatPresent);
		if (pWF == nullptr)
		{
			csDummy.Format(_T("file discarded=%i:\t%s \r\n"), ifile, (LPCTSTR) csFilename);
			psf->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			continue;
		}
		
		// check data acquisition time - go to next file if already exist and if flag set
		COleDateTime oTime;
		CTime t = pWF->acqtime;
		oTime.SetDateTime( t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		if (!((CdbWaveApp*) AfxGetApp())->ivO.bImportDuplicateFiles)
		{
			if (!m_pDB->m_mainTableSet.CheckIfAcqDateTimeIsUnique(&oTime))
				continue;
		}
		
		// add new record  -- mID
		m_pDB->m_mainTableSet.AddNew();
		mID++;
		m_pDB->m_mainTableSet.m_ID = mID;  // is this necessary? there is an autonumber in the definition of the field in this table
		// but autonumber does not necessarily work when you add a new record to a table?

		// save file names	
		if (bDatPresent)
		{
			m_pDB->m_mainTableSet.m_path_ID = m_pDB->m_pathSet.GetIDorCreateIDforString(csPath);
			m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filedat), FALSE);
			int icount = csDatFile.GetLength() - idotposition - 1;
			m_pDB->m_mainTableSet.m_Filedat = csDatFile.Right(csDatFile.GetLength() - ilastbackslashposition-1);
			m_pDB->m_mainTableSet.m_datalen = m_pDat->GetDOCchanLength();
		}

		if (bSpikPresent)
		{
			m_pDB->m_mainTableSet.m_path2_ID = m_pDB->m_pathSet.GetIDorCreateIDforString(csPath);
			m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), FALSE);
			m_pDB->m_mainTableSet.m_Filespk = csSpkFile.Right(csSpkFile.GetLength() - ilastbackslashposition-1);
			if (!m_pSpk->OnOpenDocument(csSpkFile))
				break;

			m_pDB->m_mainTableSet.m_nspikes = m_pSpk->GetSpkListCurrent()->GetTotalSpikes();
			if (m_pSpk->GetSpkListCurrent()->GetNbclasses() < 0)
					m_pSpk->GetSpkListCurrent()->UpdateClassList();
			m_pDB->m_mainTableSet.m_nspikeclasses = m_pSpk->GetSpkListCurrent()->GetNbclasses();
			m_pDB->m_mainTableSet.m_datalen = m_pSpk->m_acqsize;
		}

		// transfer data to database
		m_pDB->TransferWaveFormatDataToRecord(pWF);

		// update record
		try { 
			m_pDB->m_mainTableSet.Update(); 
		}
		catch(CDaoException* e) { DisplayDaoException(e, 17); e->Delete(); }
	}

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	try { 
		m_pDB->m_mainTableSet.Open(dbOpenDynaset, nullptr, 0);
	}
	catch(CDaoException* e) {
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		delete psf;
		return;
	}
	m_pDB->m_mainTableSet.Requery();
	m_pDB->m_mainTableSet.BuildAndSortIDArrays();
	m_pDB->m_mainTableSet.MoveLast();

	// close files opened here
	SAFE_DELETE(m_pDat);
	SAFE_DELETE(m_pSpk);
	SAFE_DELETE(psf); 
}


BOOL CdbWaveDoc::IsDatFile(CString csFilename) {
	return (csFilename.Find(_T(".dat")) > 0
		|| csFilename.Find(_T(".mcid")) > 0
		|| csFilename.Find(_T(".asd")) > 0);
}


CWaveFormat* CdbWaveDoc::GetWaveFormat(CString csFilename, BOOL bIsDatFile) {

	// new data doc used to read data files
	if (m_pDat == nullptr)
	{
		m_pDat = new CAcqDataDoc;
		ASSERT(m_pDat != NULL);
	}

	// new spike doc used to read data files
	if (m_pSpk == nullptr)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}

	BOOL bIsReadOK = FALSE;
	CWaveFormat* pWF = nullptr;
	

	if (bIsDatFile)
	{
		bIsReadOK = m_pDat->OnOpenDocument(csFilename);
		if (bIsReadOK)
			pWF = m_pDat->GetpWaveFormat();
	}
	else if (csFilename.Find(_T(".spk")) > 0)
	{
		bIsReadOK = m_pSpk->OnOpenDocument(csFilename);
		if (bIsReadOK)
			pWF = &(m_pSpk->m_wformat);
	}
	return pWF;
}


// import files names from an existing database file
BOOL CdbWaveDoc::ExtractFilenamesFromDatabase (LPCSTR filename, CStringArray& filelist)
{
	// create dbWaveDoc, open database from this new document, then 
	// read all files from this and import them into current document
	BOOL flag= TRUE;
	CDaoDatabase* ptmpDB = new CDaoDatabase; // open database
	// Attempt to open the new database 
	try
	{
		CString csfilename(filename);
		ptmpDB->Open(csfilename);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 4);
		e->Delete();
		ptmpDB->Close();
		delete ptmpDB;
		return FALSE;
	}
	
	// check format of table set
	CDaoRecordset	recordSet(ptmpDB);		// CDaoDatabase
	CdbMainTable	tableSet(ptmpDB);
	CdbIndexTable	pathSet(ptmpDB);
	pathSet.SetNames(_T("path"),   _T("path"),    _T("pathID"));
	CDaoFieldInfo	fieldinfo_filename;		// 2
	CDaoFieldInfo	fieldinfo_filespk;		// 3
	CDaoFieldInfo	fieldinfo_path_ID;		// 18
	CDaoFieldInfo	fieldinfo_path2_ID;		// 19
	CString			csTable = _T("table");

	// check for the presence of Table 'table' 
	try 
	{ 
		recordSet.Open(dbOpenTable, csTable);
		// check if column "filename" is present
		recordSet.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_FILENAME].csColName, fieldinfo_filename);
		recordSet.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_FILESPK].csColName, fieldinfo_filespk);
		recordSet.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_PATH_ID].csColName, fieldinfo_path_ID);
		recordSet.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_PATH2_ID].csColName, fieldinfo_path2_ID);
	}

	catch (CDaoException* e) 
	{
		CString cs = _T("Error in a database operation:\n");
		cs += e->m_pErrorInfo->m_strDescription;
		AfxMessageBox(cs); 
		e->Delete(); 
		return FALSE;
	}
	recordSet.Close();

	// open tables
	try 
	{ 
		pathSet.Open(dbOpenTable, nullptr, 0);
		tableSet.Open(dbOpenDynaset, nullptr, 0);
	}
	catch(CDaoException* e) 
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		return FALSE;
	}
	
	// browse existing database array - collect data file acquisition time and IDs already used
	tableSet.MoveFirst();
	while(!tableSet.IsEOF()) 
	{
		tableSet.MoveLast();
		tableSet.MoveFirst();
		while(!tableSet.IsEOF()) 
		{
			// collect data file name and then spike file name
			CString currentDatafileName;
			CString currentSpikefileName;

			// set current datdocument
			if (!tableSet.IsFieldNull(&tableSet.m_Filedat))
			{
				pathSet.SeekID(tableSet.m_path_ID);
				currentDatafileName = pathSet.m_cs + _T('\\') + tableSet.m_Filedat;
				filelist.Add(currentDatafileName);
			}
			else if (!tableSet.IsFieldNull(&tableSet.m_Filespk))
			{
				// get path (assume it is in the same path as the datafile)
				if (tableSet.IsFieldNull( &tableSet.m_path2_ID ))
				{
					tableSet.Edit();
					tableSet.m_path2_ID = tableSet.m_path_ID;
					tableSet.Update();
				}
				pathSet.SeekID(tableSet.m_path2_ID);
				currentSpikefileName = pathSet.m_cs + _T('\\') + tableSet.m_Filespk;
				filelist.Add(currentSpikefileName);
			}
				
			// move to next record
			tableSet.MoveNext();
		}
	}
	tableSet.Close();
	pathSet.Close();

	ptmpDB->Close();
	delete ptmpDB;
	return flag;
}

// import all records from a database
BOOL CdbWaveDoc::ImportDatabase(CString &filename)
{
	// create dbWaveDoc, open database from this new document, then 
	// read all files from this and import them into current document
	CdbWaveDoc* pnewdoc = new CdbWaveDoc; // open database
	BOOL flag = pnewdoc->OnOpenDocument(filename);
	if (!flag)
		return FALSE;

	// browse existing database array - collect data file acquisition time and IDs already used
	CdbWdatabase* pnewDB = pnewdoc->m_pDB;
	pnewDB->m_mainTableSet.MoveFirst();
	int nrecordsAdded = 0;
	while(!pnewDB->m_mainTableSet.IsEOF()) 
	{
		m_pDB->ImportRecordfromDatabase(pnewDB);
		nrecordsAdded++;
		pnewDB->m_mainTableSet.MoveNext();
	}
	pnewDB->m_mainTableSet.Close();
	delete pnewdoc;

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	try { m_pDB->m_mainTableSet.Open(dbOpenDynaset, nullptr, 0);}
	catch(CDaoException* e) {
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		return FALSE;
	}
	m_pDB->m_mainTableSet.Requery();
	m_pDB->m_mainTableSet.BuildAndSortIDArrays();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CdbWaveDoc::SynchronizeSourceInfos(BOOL bAll)
{
	// save current index position - restore on exit
	long currentfile = DBGetCurrentRecordPosition();
	
	// make sure there are objects to read / write data and spike files	
	if (m_pDat == nullptr)		// data doc
	{
		m_pDat = new CAcqDataDoc;
		ASSERT(m_pDat != NULL);
	}	
	if (m_pSpk == nullptr)		// spike doc
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}

	// do only one iteration?
	if (!bAll)
	{
		CWaveFormat* pWF;
		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			BOOL bflag = OpenCurrentDataFile();
			ASSERT(bflag == TRUE);
			pWF = m_pDat->GetpWaveFormat();
			if (UpdateWaveFmtFromDatabase(pWF))
				m_pDat->AcqSaveDataDescriptors();
		}
		// process spike file
		if (!m_currentSpikefileName.IsEmpty())
		{
			BOOL bflag = OpenCurrentSpikeFile();
			ASSERT(bflag == TRUE);
			pWF = &(m_pSpk->m_wformat);
			if (UpdateWaveFmtFromDatabase(pWF))
				m_pSpk->OnSaveDocument(m_currentSpikefileName);
		}
		return;
	}
	
	// prepare progress dialog box
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetStep (1);
	int istep = 0;
	CString cscomment;
	long nfiles = DBGetNRecords();
	int ifile = 1;

	// got to the first record
	m_pDB->m_mainTableSet.MoveFirst();
	dlg.SetPos(0);

	while(!m_pDB->m_mainTableSet.IsEOF()) 
	{
		m_pDB->GetFilenamesFromCurrentRecord();
		// check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i]"), ifile, nfiles);
		dlg.SetStatus(cscomment);

		CWaveFormat* pWF;
		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			BOOL bflag = OpenCurrentDataFile();
			ASSERT(bflag == TRUE);
			pWF = m_pDat->GetpWaveFormat();
			if (UpdateWaveFmtFromDatabase(pWF))
				m_pDat->AcqSaveDataDescriptors();
				//m_pDat->OnSaveDocument(m_currentDatafileName);
		}
		// process spike file
		if (!m_currentSpikefileName.IsEmpty())
		{
			BOOL bflag = OpenCurrentSpikeFile();
			ASSERT(bflag == TRUE);
			pWF = &(m_pSpk->m_wformat);
			if (UpdateWaveFmtFromDatabase(pWF))
				m_pSpk->OnSaveDocument(m_currentSpikefileName);
		}
		// move to next record
		m_pDB->m_mainTableSet.MoveNext();
		ifile++;
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}
	m_pDB->m_mainTableSet.MoveFirst();
	m_pDB->GetFilenamesFromCurrentRecord();

	// restore current data position
	DBSetCurrentRecordPosition(currentfile);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CdbWaveDoc::UpdateWaveFmtFromDatabase (CWaveFormat* pWF)
{
	BOOL bChanged = FALSE;
	// long: experiment
	if (m_pDB->m_exptSet.SeekID(m_pDB->m_mainTableSet.m_expt_ID))
	{
		bChanged = (pWF->csComment.Compare(m_pDB->m_exptSet.m_cs) != 0);
		pWF->csComment = m_pDB->m_exptSet.m_cs;
	}

	// CString:	m_more;
	bChanged |= (pWF->csMoreComment.Compare(m_pDB->m_mainTableSet.m_more) != 0); 
	pWF->csMoreComment = m_pDB->m_mainTableSet.m_more;

	// long:	m_operator_ID;
	if (m_pDB->m_operatorSet.SeekID(m_pDB->m_mainTableSet.m_operator_ID))
	{
		bChanged |= (pWF->csOperator.Compare(m_pDB->m_operatorSet.m_cs) != 0);
		pWF->csOperator = m_pDB->m_operatorSet.m_cs;
	}

	// long:	m_insect_ID;
	if (m_pDB->m_insectSet.SeekID(m_pDB->m_mainTableSet.m_insect_ID))
	{
		bChanged |= (pWF->csInsectname.Compare(m_pDB->m_insectSet.m_cs) != 0);
		pWF->csInsectname= m_pDB->m_insectSet.m_cs;
	}

	// long:	m_strain_ID;
	if (m_pDB->m_strainSet.SeekID(m_pDB->m_mainTableSet.m_strain_ID))
	{
		bChanged |= (pWF->csStrain.Compare(m_pDB->m_strainSet.m_cs) != 0);
		pWF->csStrain= m_pDB->m_strainSet.m_cs;
	}

	// long:	m_expt_ID;
	if (m_pDB->m_exptSet.SeekID(m_pDB->m_mainTableSet.m_expt_ID))
	{
		bChanged |= (pWF->csComment.Compare(m_pDB->m_exptSet.m_cs) != 0);
		pWF->csComment= m_pDB->m_exptSet.m_cs;
	}


	// long:	m_sex_ID;
	if (m_pDB->m_sexSet.SeekID(m_pDB->m_mainTableSet.m_sex_ID))
	{
		bChanged |= (pWF->csSex.Compare(m_pDB->m_sexSet.m_cs) != 0);
		pWF->csSex= m_pDB->m_sexSet.m_cs;
	}

	// long	m_location_ID;
	if (m_pDB->m_locationSet.SeekID(m_pDB->m_mainTableSet.m_location_ID))
	{
		bChanged |= (pWF->csLocation.Compare(m_pDB->m_locationSet.m_cs) != 0);
		pWF->csLocation = m_pDB->m_locationSet.m_cs;
	}

	// long	m_sensillum_ID;
	if (m_pDB->m_sensillumSet.SeekID(m_pDB->m_mainTableSet.m_sensillum_ID))
	{
		bChanged |= (pWF->csSensillum.Compare(m_pDB->m_sensillumSet.m_cs) != 0);
		pWF->csSensillum = m_pDB->m_sensillumSet.m_cs;
	}

	//long	m_stim_ID;
	if (m_pDB->m_stimSet.SeekID(m_pDB->m_mainTableSet.m_stim_ID))
	{
		bChanged |= (pWF->csStimulus.Compare(m_pDB->m_stimSet.m_cs) != 0); 
		pWF->csStimulus = m_pDB->m_stimSet.m_cs;
	}

	// long	m_conc_ID;
	if (m_pDB->m_concSet.SeekID(m_pDB->m_mainTableSet.m_conc_ID))
	{
		bChanged |= (pWF->csConcentration.Compare(m_pDB->m_concSet.m_cs) != 0);
		pWF->csConcentration = m_pDB->m_concSet.m_cs;
	}

	// long	m_stim2_ID;
	if (m_pDB->m_stimSet.SeekID(m_pDB->m_mainTableSet.m_stim2_ID))
	{
		bChanged |= (pWF->csStimulus2.Compare(m_pDB->m_stimSet.m_cs) != 0); 
		pWF->csStimulus2 = m_pDB->m_stimSet.m_cs;
	}

	// long	m_conc2_ID;
	if (m_pDB->m_concSet.SeekID(m_pDB->m_mainTableSet.m_conc2_ID))
	{
		bChanged |= (pWF->csConcentration2.Compare(m_pDB->m_concSet.m_cs) != 0);
		pWF->csConcentration2 = m_pDB->m_concSet.m_cs;
	}

	bChanged |= (pWF->insectID != m_pDB->m_mainTableSet.m_IDinsect);
	// long	m_insectnumber;
	pWF->insectID = m_pDB->m_mainTableSet.m_IDinsect;

	bChanged |= (pWF->sensillumID != m_pDB->m_mainTableSet.m_IDsensillum);
	// long	m_IDsensillum;
	pWF->sensillumID = m_pDB->m_mainTableSet.m_IDsensillum;

	bChanged |= (pWF->repeat != m_pDB->m_mainTableSet.m_repeat);
	// long	m_repeat;
	pWF->repeat = m_pDB->m_mainTableSet.m_repeat;

	bChanged |= (pWF->repeat2 != m_pDB->m_mainTableSet.m_repeat2);
	// long	m_repeat2;
	pWF->repeat2 = m_pDB->m_mainTableSet.m_repeat2;

	int npercycle = (int) ((m_pSpk->m_stimIntervals.nitems / 2) 
		/ (m_pSpk->GetAcqDuration()/8.192f));
	bChanged |= (npercycle != m_pSpk->m_stimIntervals.npercycle);
	m_pSpk->m_stimIntervals.npercycle = npercycle;

	return bChanged;
}


/////////////////////////////////////////////////////////////////////////////
void CdbWaveDoc::ExportSpkDescriptors(CSharedFile* pSF, CSpikeList* pSL, int kclass)
{
	CString csDummy;
	CString csTemp;
	CString csTab = _T("\t");

	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	OPTIONS_VIEWSPIKES* pvdS = &(pApp->vdS);

	CString csFileComment = _T("\r\n") + ExportDatabaseData();
	pSF->Write(csFileComment, csFileComment.GetLength() * sizeof(TCHAR));

	// spike file additional comments
	if (pvdS->bspkcomments)
	{
		pSF->Write(csTab, csTab.GetLength() * sizeof(TCHAR));
		csTemp = m_pSpk->GetComment();
		pSF->Write(csTemp, csTemp.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	if (pvdS->btotalspikes)
	{
		csDummy.Format(_T("%s%i"), (LPCTSTR) csTab, pSL->GetTotalSpikes());
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		csDummy.Format(_T("%s%i"), (LPCTSTR) csTab, pSL->GetNbclasses());
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
		float tduration = (float) m_pSpk->GetAcqDuration();
		csDummy.Format(_T("%s%.3f"), (LPCTSTR) csTab, tduration);
		pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
	}

	// spike list item, spike class
	if (pvdS->spikeclassoption != 0)
		csDummy.Format(_T("%s%i %s%s %s%i"), (LPCTSTR) csTab, pvdS->ichan, (LPCTSTR) csTab, (LPCTSTR) pSL->GetComment(), (LPCTSTR) csTab, kclass);
	else
		csDummy.Format(_T("%s%i %s%s \t(all)"), (LPCTSTR) csTab, pvdS->ichan, (LPCTSTR) csTab, (LPCTSTR) pSL->GetComment());
	pSF->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
}

CString CdbWaveDoc::ExportDatabaseData(int ioption)
{
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	OPTIONS_VIEWSPIKES* pvdS = &(pApp->vdS);
	CString separator = _T("\t");
	if (ioption == 1)
		separator = _T(" | ");

	// export data comment into csFileComment
	CString csFileComment;
	CString csDummy;
	CString filename;

	DB_ITEMDESC desc;
	m_pDB->GetRecordItemValue(CH_PATH2_ID, &desc);
	filename = desc.csVal;
	m_pDB->GetRecordItemValue(CH_FILESPK, &desc);
	filename = filename + _T('\\') + desc.csVal;
	csFileComment.Format(_T("%i%s%s"), m_pDB->m_mainTableSet.m_ID, (LPCTSTR) separator, (LPCTSTR) filename);

	// source data file items
	if (pvdS->bacqdate)				// source data time and date
	{
		csDummy = separator + m_pDB->m_mainTableSet.m_acq_date.Format(VAR_DATEVALUEONLY);
		csFileComment += csDummy;
		csDummy = separator + m_pDB->m_mainTableSet.m_acq_date.Format(VAR_TIMEVALUEONLY);
		csFileComment += csDummy;
	}
	// source data comments
	if (pvdS->bacqcomments)
	{
		m_pDB->GetRecordItemValue(CH_EXPT_ID, &desc);
		csFileComment += separator + desc.csVal;
		
		m_pDB->GetRecordItemValue(CH_IDINSECT, &desc);
		csDummy.Format(_T("%i"), desc.lVal);
		csFileComment += separator + csDummy;
		m_pDB->GetRecordItemValue(CH_IDSENSILLUM, &desc);
		csDummy.Format(_T("%i"), desc.lVal);
		csFileComment += separator + csDummy;

		m_pDB->GetRecordItemValue(CH_INSECT_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_STRAIN_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_SEX_ID, &desc);
		csFileComment += separator + desc.csVal; 
		m_pDB->GetRecordItemValue(CH_LOCATION_ID, &desc);
		csFileComment += separator + desc.csVal;
		
		m_pDB->GetRecordItemValue(CH_OPERATOR_ID, &desc);
		csFileComment += separator + desc.csVal; 
		m_pDB->GetRecordItemValue(CH_MORE, &desc);
		csFileComment += separator + desc.csVal;

		m_pDB->GetRecordItemValue(CH_STIM_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_CONC_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_REPEAT, &desc);
		csDummy.Format(_T("%i"), desc.lVal);
		csFileComment += separator + csDummy;

		m_pDB->GetRecordItemValue(CH_STIM2_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_CONC2_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_REPEAT2, &desc);
		csDummy.Format(_T("%i"), desc.lVal);
		csFileComment += separator + csDummy;

		m_pDB->GetRecordItemValue(CH_SENSILLUM_ID, &desc);
		csFileComment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_FLAG, &desc);
		csDummy.Format(_T("%i"), desc.lVal);
		csFileComment += separator + csDummy;
	}
	return csFileComment;
}

/////////////////////////////////////////////////////////////////////////////
// exports spike data on consecutive rows (except for a few options)
// returns nb columns of comments

void CdbWaveDoc::ExportNumberofSpikes(CSharedFile* pSF)
{
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep (1);
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");

	// save current selection and export header of the table
	int ioldindex = DBGetCurrentRecordPosition();
	int nfiles = DBGetNRecords();
	if (nullptr == m_pSpk)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);	
		m_pSpk->SetSpkListCurrent(GetcurrentSpkListIndex());
	}

	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	OPTIONS_VIEWSPIKES* parms = &(pApp->vdS);

	int ioldlist = m_pSpk->GetSpkListCurrentIndex(); 
	m_pSpk->ExportTableTitle(pSF, parms, nfiles);
	m_pSpk->ExportTableColHeaders_db(pSF, parms);
	m_pSpk->ExportTableColHeaders_data(pSF, parms);		// this is for the measure

	// single file export operation: EXTREMA, AMPLIT, SPIKEPOINTS
	m_bTranspose = FALSE;
	if (parms->exportdatatype == EXPORT_INTERV)
		m_bTranspose = TRUE;

	CString csFileDesc;
	if (   parms->exportdatatype == EXPORT_EXTREMA
		|| parms->exportdatatype == EXPORT_AMPLIT
		|| parms->exportdatatype == EXPORT_SPIKEPOINTS) /*|| parms->exportdatatype == EXPORT_INTERV*/
	{
		m_pSpk->ExportSpkFileComment      (pSF, parms, 0, csFileDesc);
		m_pSpk->ExportSpkAttributesOneFile(pSF, parms, csFileDesc);
	}

	// multiple file export operations: ISI, AUTOCORR, HISTAMPL, AVERAGE, INTERV, PSTH
	else
	{
		int nbins=0;
		long* pHist0 = nullptr;
		double* pDoubl = nullptr;
		switch (parms->exportdatatype)
		{	
		case EXPORT_ISI:		// ISI
		case EXPORT_AUTOCORR:	// Autocorr
			nbins = parms->nbinsISI;
			break;
		case EXPORT_HISTAMPL:	// spike amplitude histogram
			nbins = parms->histampl_nbins+2;
			break;
		case EXPORT_AVERAGE:	// assume that all spikes have the same length
			pDoubl = new double [m_pSpk->GetSpkListCurrent()->GetSpikeLength() * 2 + 1 +2];
			*pDoubl = m_pSpk->GetSpkListCurrent()->GetSpikeLength();			
			break;
		case EXPORT_INTERV:		// feb 23, 2009
			break;
		case EXPORT_PSTH:		// PSTH
		default:
			nbins = parms->nbins;
			break;
		}
		pHist0 = new long[nbins+2]; // create array (dimension = nbins) to store results
		*pHist0 = nbins;
		ASSERT(pHist0 != NULL);
		CString	csFileComment;
		csFileComment.Empty();

		// in each spike list, loop over spike classes as defined in the options
		int iclass1 = parms->classnb;
		int iclass2 = parms->classnb2;
		if (parms->spikeclassoption == 0)
			iclass2 = iclass1;
		else
		{
			if (iclass2 < iclass1) 
			{
				int iclassd = iclass1;
				iclass1 = iclass2;
				iclass2 = iclassd;
			}
		}

		// loop (1) from file 1 to file n ---------------------------------------------
		for (int ifile1 = 0; ifile1 < nfiles; ifile1++)
		{
			// check if user wants to stop
			if(dlg.CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;
			cscomment.Format(_T("Processing file [%i / %i]"), ifile1+1, nfiles);
			dlg.SetStatus(cscomment);

			// open document
			DBSetCurrentRecordPosition(ifile1);
			if (m_currentSpikefileName.IsEmpty())
				continue;
			// check if file is still present and open it
			CFileStatus rStatus;
			if (!CFile::GetStatus(m_currentSpikefileName, rStatus)) 
			{
				csFileComment = _T("\r\n") + ExportDatabaseData();
				csFileComment += _T("\tERROR: MISSING FILE");		// next line
				pSF->Write(csFileComment, csFileComment.GetLength() * sizeof(TCHAR));
				continue;
			}
			BOOL flag= OpenCurrentSpikeFile();
			ASSERT(flag);
			BOOL bChanged = FALSE;

			// loop over the spike lists stored in that file
			int ichan1 = 0;
			int ichan2 = m_pSpk->GetSpkListSize();
			if (!parms->ballChannels)
			{
				ichan1 = ioldlist;
				ichan2 = ichan1+1;
			}


			//----------------------------------------------------------
			for (int ispikelist=ichan1; ispikelist< ichan2; ispikelist++)
			{
				CSpikeList* pSL = m_pSpk->SetSpkListCurrent(ispikelist);
				parms->ichan = ispikelist;
				for (int kclass = iclass1; kclass <= iclass2; kclass++)
				{
					ExportSpkDescriptors (pSF, pSL, kclass);
					// export data
					switch (parms->exportdatatype)
					{	
					case EXPORT_HISTAMPL:	// spike amplitude histogram
						m_pSpk->_ExportSpkAmplitHistogram(pSF, parms, pHist0, csFileComment, ispikelist, kclass);
						break;
					case EXPORT_LATENCY:	// occurence time of the first 10 spikes
						m_pSpk->_ExportSpkLatencies(pSF, parms, 10, csFileComment, ispikelist, kclass);
						break;
					case EXPORT_INTERV:		// feb 23, 2009 - occurence time of all spikes
						m_pSpk->_ExportSpkLatencies(pSF, parms, -1, csFileComment, ispikelist, kclass);
						break;
					case EXPORT_AVERAGE:	// assume that all spikes have the same length
						m_pSpk->_ExportSpkAverageWave(pSF, parms, pDoubl, csFileComment, ispikelist, kclass);
						break;
					case EXPORT_PSTH:		// PSTH
					case EXPORT_ISI:		// ISI
					case EXPORT_AUTOCORR:	// Autocorr
						m_pSpk->_ExportSpkPSTH(pSF, parms, pHist0, csFileComment, ispikelist, kclass);
						break;
					default:
						ATLTRACE2(_T("option selected not implemented: %i\n"), parms->exportdatatype);
						break;			
					}
				} // end of for: kclass
			} // end of for: spklist

			// update progress bar 
			if (MulDiv(ifile1, 100, nfiles) > istep)
			{
				dlg.StepIt();
				istep = MulDiv(ifile1, 100, nfiles);
			}
		}

		delete [] pHist0;
		if (pDoubl) delete [] pDoubl;
	}

	// transpose file
	if (m_bTranspose)
		TransposeFile(pSF);

	// restore initial file name and channel
	DBSetCurrentRecordPosition(ioldindex);
	if (OpenCurrentSpikeFile())
		m_pSpk->SetSpkListCurrent(ioldlist);
	UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
}

// called when we export data to Excel

BOOL CdbWaveDoc::TransposeFile (CSharedFile *pSF)
{
	// create dummy file on disk that duplicates the memory file
	CStdioFile dataDest;								// destination file object
	CString dummyFile1 = _T("dummyFile.txt");				// to read the clipboard
	CString dummyFile2 = _T("dummyFile_transposed.txt");	// to store the transposed data

	CFileException fe;		// trap exceptions	
	if (!dataDest.Open(dummyFile1, CFile::modeCreate | CFile::modeReadWrite|CFile::shareDenyNone | CFile::typeText,&fe))
	{
		dataDest.Abort();	// file not found
		return FALSE;		// and return
	}
	ULONGLONG ulLencharTotal = pSF->GetLength();
	ULONGLONG ulLenchar;
	pSF->SeekToBegin();
	ulLenchar = ulLencharTotal;
	char buffer[2048];
	while (ulLenchar > 0)
	{
		ULONGLONG ulLen = ulLenchar;
		if (ulLen > 1024)
			ulLen = 1024;
		pSF->Read(&buffer, (UINT) ulLen);
		dataDest.Write(&buffer, (UINT) ulLen);
		ulLenchar = ulLenchar - ulLen;
	}

	// create a transposed file
	CStdioFile dataTransposed;	// destination file object
	if (!dataTransposed.Open(dummyFile2, CFile::modeCreate | CFile::modeReadWrite|CFile::shareDenyNone | CFile::typeText,&fe))
	{
		dataTransposed.Abort();	// file not found
		return FALSE;			// and return
	}

	// files are created, now read dataDest line by line
	dataDest.SeekToBegin();
	CString csRead; 
	// first find where the header is and copy the first lines of the table to the transposed file
	BOOL bHeaderFound=FALSE;
	ULONGLONG ulPositionHeader = 0;
	while(dataDest.ReadString(csRead) && !bHeaderFound)
	{
		ULONGLONG ulPositionFirst = dataDest.GetPosition();
		if (bHeaderFound = (csRead.Find(_T("ID")) >= 0))
			break;

		dataTransposed.WriteString(csRead);
		ulPositionHeader = ulPositionFirst;
	}
	// now: ulPositionHeader saves the file position of the table header
	//		dataTransposed is ready to get the first line of data
	
	// Copy data up to the header to dataTransposed
	// --------------------------------------------------------- first line of the table
	// first pass: browse through all lines from dataDest
	int sourceCol=0;
	int nrecords = -1;
	dataDest.Seek(ulPositionHeader, CFile::begin);
	CString csTransposed;
	int iStart=0;
	int iFirst=0;
	int ifound;
	char cSep = _T('\t');
	
	while (dataDest.ReadString(csRead))
	{
		ifound = csRead.Find(_T('\t'), iStart);
		csTransposed += csRead.Mid(iFirst, ifound-iStart) + cSep;
		nrecords++;
	}
	csTransposed += _T('\n');
	dataTransposed.WriteString(csTransposed);
	
	// ---------------------------------------------------------
	// now we now how many lines are in dataDest and we will scan line by line until all lines are exhausted
	int icol_Dest = 0;
	int nFound = nrecords;

	while (nFound > 0)
	{
		dataDest.Seek(ulPositionHeader, CFile::begin);
		csTransposed.Empty();
		nFound = nrecords+1;
		icol_Dest++;

		for (int i=0; i<= nrecords; i++)
		{
			dataDest.ReadString(csRead);

			// find tab at icol_dest position
			int iFirst=0;
			for (int j=0; j<icol_Dest; j++)
			{
				iFirst = csRead.Find(cSep, iFirst+1);
				if (iFirst < 0)		// not found: exit loop and go to next line
					break;
			}

			// write dummy space if no tab found
			if (iFirst < 0)
			{
				csTransposed += cSep;
				nFound --;
				continue;
			}

			// find end of the token
			int iLast;
			iLast = csRead.Find(cSep, iFirst+1);
			
			// extract value
			if (iLast < 0)
			{
				iLast= csRead.GetLength();
				if (csRead.Find (_T('\n')) >0)
					iLast--;
				if (csRead.Find (_T('\r')) >0)
					iLast--;
			}
			
			CString csTemp = csRead.Mid(iFirst, iLast-iFirst);
			int itest = csTemp.Find(cSep);
			if (itest < 1)
				csTemp = csRead.Mid(iFirst+1, iLast-iFirst-1);
			if (csTemp.Find(cSep) < 0)
				csTemp += cSep;
			csTransposed += csTemp;
		}
		csTransposed += _T('\n');
		dataTransposed.WriteString(csTransposed);
	}
	
	// write end-of-the-file character
	buffer[0]=0;
	dataTransposed.Write(&buffer, 1);

	// now the transposed file has all data in the proper format
	// and we can copy it back to the clipboard
	ulLencharTotal = dataTransposed.GetLength(); 
	dataTransposed.SeekToBegin();
	ulLenchar = ulLencharTotal;

	// erase old clipboard data and create a new block of data
	pSF->SetLength(ulLencharTotal);
	pSF->SeekToBegin();
	
	while (ulLenchar > 0)
	{
		ULONGLONG ulLen = ulLenchar;
		if (ulLen > 1024)
			ulLen = 1024;
		dataTransposed.Read(&buffer, (UINT) ulLen);
		pSF->Write(&buffer, (UINT) ulLen);
		ulLenchar -= ulLen;
	}
	
	// now delete the 2 dummy files
	dataTransposed.Close();
	dataDest.Close();
	CFile::Remove(dummyFile1);
	CFile::Remove(dummyFile2);

	return TRUE;
}

int CdbWaveDoc::CheckifFilesCanbeOpened(CStringArray & filenames, CSharedFile* psf)
{
	// prepare progress dialog box
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetStep(1);
	int istep = 0;
	int nfilesok = 0;
	int nfiles = filenames.GetSize();
	CString cscomment;
	CString csDummy;

	for (int ifile = nfiles - 1; ifile >= 0; ifile--)
	{
		// check if filename not already defined
		filenames[ifile].MakeLower();
		CString csFilename = filenames[ifile];
		if (lstrlen(csFilename) >= _MAX_PATH)
			continue;

		cscomment.Format(_T("Checking file type and status on disk [%i / %i] %s"), nfiles - ifile, nfiles, (LPCTSTR)csFilename);
		dlg.SetStatus(cscomment);
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}

		// check if file of correct type
		CString cs_ext;
		int iExt = csFilename.ReverseFind('.');
		cs_ext = csFilename.Right(csFilename.GetLength() - iExt - 1);
		BOOL bDat = (cs_ext.Compare(_T("dat")) == 0);
		if (!bDat)
			bDat = (cs_ext.Compare(_T("mcid")) == 0);
		if (!bDat)
			bDat = (cs_ext.Compare(_T("asd")) == 0);

		if ((!bDat && cs_ext.Compare(_T("spk")) != 0) || (csFilename.Find(_T("tmp.dat")) >= 0))
		{
			csDummy.Format(_T("file discarded=%i:\t%s \t not proper file extension\r\n"), ifile, (LPCTSTR)csFilename);
			psf->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			filenames.RemoveAt(ifile);
			continue;
		}

		// open document and read data
		CFileStatus rStatus;	// file status: time creation, ..
		BOOL bflag = CFile::GetStatus(csFilename, rStatus);

		// GOTO next file if it not possible to open the file either as a spk or a dat file
		if (!bflag)
		{
			csDummy.Format(_T("file discarded=%i:\t%s\t error reading file \r\n"), ifile, (LPCTSTR)csFilename);
			psf->Write(csDummy, csDummy.GetLength() * sizeof(TCHAR));
			filenames.RemoveAt(ifile);
			continue;
		}
		nfilesok++;
	}
	return nfilesok;
}

// change file extension of files listed into table_deleted
void CdbWaveDoc::DeleteErasedFiles()
{
	// close current data file (no need to do that for spk file)(so far)
	if (m_pDat != nullptr)
		m_pDat->AcqCloseFile();

	// loop over all records
	CString csfileName;
	CString csNewname;
	int nfiles = m_csfiles_to_delete.GetSize()-1;

	for (int i=nfiles; i>= 0; i--)
	{
		// get data file name and rename file
		csfileName = m_csfiles_to_delete.GetAt(i);
		csNewname = csfileName + _T("del");

		// check if same file already exists: if yes, search if up to 10 clones exist, otherwise, destroy the last clone
		CFileStatus rStatus;
		if (CFile::GetStatus(csNewname, rStatus))
		{
			BOOL flag = TRUE;
			CString prompt;
			for (int j=0; j < 10; j++)
			{
				CString cs;
				prompt.Format(_T("%s%i"), (LPCTSTR) csNewname, j);
				flag = CFile::GetStatus(prompt, rStatus);
				if (!flag)
					break;
			}
			
			if (flag == TRUE)
			{
				prompt += _T("\nThis file seems to exist already.\nDelete it?");
				if (AfxMessageBox(prompt, MB_YESNO) == IDYES)
					CFile::Remove(prompt);
			}
			else
				csNewname = prompt;
		}

		// rename file
		try
		{
			CFile::Rename(csfileName, csNewname);
		}

		catch(CFileException* pEx )
		{
		#ifdef _DEBUG
			afxDump << _T("File ") << csfileName << _T(" not found, cause = ")
					<< pEx->m_cause << _T("\n");
			switch(pEx->m_cause)
			{
			case CFileException::fileNotFound:
				afxDump << _T("The file could not be located."); break;
			case CFileException::badPath:
				afxDump <<_T("All or part of the path is invalid. "); break;
			case CFileException::tooManyOpenFiles:
				afxDump <<_T("The permitted number of open files was exceeded. "); break;
			case CFileException::accessDenied:
				afxDump <<_T("The file could not be accessed. "); break;
			case CFileException::invalidFile:
				afxDump <<_T("There was an attempt to use an invalid file handle.  "); break;
			case CFileException::removeCurrentDir:
				afxDump <<_T("The current working directory cannot be removed.  "); break;
			case CFileException::directoryFull:
				afxDump <<_T("There are no more directory entries.  "); break;
			case CFileException::badSeek:
				afxDump <<_T("There was an error trying to set the file pointer.  "); break;
			case CFileException::hardIO:
				afxDump <<_T("There was a hardware error.  "); break;
			case CFileException::sharingViolation:
				afxDump <<_T("SHARE.EXE was not loaded, or a shared region was locked.  "); break;
			case CFileException::lockViolation:
				afxDump <<_T("There was an attempt to lock a region that was already locked.  "); break;
			case CFileException::diskFull:
				afxDump <<_T("The disk is full.  "); break;
			case CFileException::endOfFile:
				afxDump <<_T("The end of file was reached.  ");
				break;
			default: break;
			}
			afxDump << _T("\n");
		#endif
			pEx->Delete();
		}
		m_csfiles_to_delete.RemoveAt(i);
	}
}


// delete record: change data and spike file extensions and copy record to table_deleted
// remove corresponding records from table_deleted
void CdbWaveDoc::DBDeleteCurrentRecord()
{
	// save data & spike file names, together with their full access path
	m_pDB->m_pathSet.SeekID(m_pDB->m_mainTableSet.m_path_ID);
	CString cs;
	if (!m_pDB->m_mainTableSet.m_Filedat.IsEmpty())
	{
		cs =  m_pDB->m_pathSet.m_cs + _T('\\') + m_pDB->m_mainTableSet.m_Filedat;
		m_csfiles_to_delete.Add(cs);
	}
	if (!m_pDB->m_mainTableSet.m_Filespk.IsEmpty())
	{
		cs =  m_pDB->m_pathSet.m_cs + _T('\\') + m_pDB->m_mainTableSet.m_Filespk;
		m_csfiles_to_delete.Add(cs);
	}
	m_pDB->m_mainTableSet.Delete();
	m_pDB->m_mainTableSet.MoveNext();
	// exit 
	return;
}


void CdbWaveDoc::RemoveDuplicateFiles()
{
	// loop through all files
	// discard if data acquisition the same day, same time
	
	// display progress in a dialog box
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetWindowText(_T("Scan database to discard duplicate (or missing) data files..."));
	CString cscomment;
	int nfiles = DBGetNRecords();
	dlg.SetRange(0, nfiles);
	dlg.SetStep(1);
	
	CStringArray		csDeletedNames;
	CStringArray		csDeletedSpkNames;
	CStringArray		csOriginalNames;
	CStringArray		csAllNames;
	CArray <CTime, CTime> oTimeArray;
	csDeletedNames.SetSize(nfiles);
	csDeletedSpkNames.SetSize(nfiles);
	csOriginalNames.SetSize(nfiles);
	csAllNames.SetSize(nfiles);
	oTimeArray.SetSize(nfiles);

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();	

	m_pDB->m_mainTableSet.Close();				// close dynaset and open as datatable
	try {m_pDB-> m_mainTableSet.Open(dbOpenTable, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}

	// scan database to collect all file names
	// explore the whole table from record 1 to the end
	cscomment.Format(_T("Collect names from database and read data acquisition times..."));
	dlg.SetStatus(cscomment);

	int kfile = 0;				// index nb of validated records
	int nduplicates = 0;		// nb records to suppress
	int ifile = 0;				// absolute index (kfile+nduplicates)
	BOOL bOK = TRUE;			// flag FALSE= suppress record

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		while(!m_pDB->m_mainTableSet.IsEOF()) 
		{
			// load time for this record and save bookmark
			COleVariant bookmark = m_pDB->m_mainTableSet.GetBookmark();
			m_pDB->GetFilenamesFromCurrentRecord();
			m_pDB->m_mainTableSet.SetBookmark(bookmark);
			csAllNames.SetAt(kfile, m_currentDatafileName);

			// process file
			cscomment.Format(_T("Processing file [%i / %i] %s"), kfile, nfiles, (LPCTSTR) m_currentDatafileName);
			dlg.SetStatus(cscomment);
			if(dlg.CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;

			// load file and read date and time of data acquisition
			CTime oTime = 0;
			bOK = OpenCurrentDataFile();
			int ioriginalfile=-1;
			if (bOK)
			{
				CWaveFormat* pW = m_pDat->GetpWaveFormat();
				oTime = pW->acqtime;
				// loop to find if current file has a duplicate in the list of previous files stored in the array
				for (int i= 0; i < kfile; i++)
				{
					CTime oTimei = oTimeArray.GetAt(i);
					ASSERT (oTimei != 0);
					if (oTime == oTimei)
					{
						ioriginalfile = i;
						bOK = FALSE;			// duplicate found, set flag and exit loop 
						break;
					}
				}
			}

			// update parameters array
			if (!bOK)	// suppress record if data file not found or if duplicate
			{
				if (ioriginalfile >= 0)
					csOriginalNames.SetAt(nduplicates, csAllNames.GetAt(ioriginalfile));
				else
					csOriginalNames.SetAt(nduplicates, _T("(none)"));
				csDeletedNames.SetAt(nduplicates, m_currentDatafileName);
				csDeletedSpkNames.SetAt(nduplicates, m_currentSpikefileName);
				m_pDB->m_mainTableSet.Delete();
				nduplicates++;
			}
			else		// save time of this valid record into array
			{
				oTimeArray.SetAtGrow(kfile, oTime);			
				kfile++;
			}
	
			// move to next record & update dialog box
			m_pDB->m_mainTableSet.MoveNext();
			ifile++;
			dlg.StepIt();
		}
		m_pDB->m_mainTableSet.MoveFirst();
	}

	// re-open dynaset
	m_pDB->m_mainTableSet.Close();
	try { m_pDB->m_mainTableSet.Open(dbOpenDynaset, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}
	m_pDB->m_mainTableSet.Requery();

	// trace
	if (nduplicates == 0)
		AfxMessageBox(_T("No duplicate files found"));
	else
	{
		CString cs;
		cs.Format(_T("Changes were made to the data base:\nNumber of files removed= %i\n"), nduplicates);
		AfxMessageBox(cs);
		//---------------------------------------------
		csDeletedNames.SetSize(nduplicates);
		csOriginalNames.SetSize(nduplicates);
		csDeletedSpkNames.SetSize(nduplicates);
		CString csDummy;
		CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		csDummy = _T("Removed:\tOriginal:\n");
		sf.Write(csDummy, csDummy.GetLength());
		for (int i=0; i< nduplicates; i++)
		{
			csDummy.Format(_T("%s\t%s\n"), (LPCTSTR) csDeletedNames.GetAt(i), (LPCTSTR) csOriginalNames.GetAt(i));
			sf.Write(csDummy, csDummy.GetLength());
		}
		DWORD dwLen = (DWORD) sf.GetLength();
		HGLOBAL hMem = sf.Detach();
		if (!hMem)
			return;
		hMem = ::GlobalReAlloc(hMem, dwLen, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		if (!hMem)
			return;
		COleDataSource* pSource = new COleDataSource();
		pSource->CacheGlobalData(CF_TEXT, hMem);		// CF_UNICODETEXT?
		pSource->SetClipboard();
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		CMultiDocTemplate* pTempl = pApp->m_pNoteViewTemplate;
		CDocument* pdbDocExport = pTempl->OpenDocumentFile(nullptr);
		POSITION pos = pdbDocExport->GetFirstViewPosition();
		CNoteDocView* pView = (CNoteDocView*)pdbDocExport->GetNextView(pos);
		CRichEditCtrl& pEdit = pView->GetRichEditCtrl();
		pEdit.Paste();
		//---------------------------------------------
		int idResponse = AfxMessageBox(_T("Do you want to erase these files from the disk?"), MB_YESNO);
		if(idResponse == IDYES)
		{
			for (int i=0; i< nduplicates; i++)
			{
				// remove dat
				CString csFilePath = csDeletedNames.GetAt(i);
				if (!csFilePath.IsEmpty())
				{
					try
					{
					   CFile::Remove(csFilePath);
					}
					catch (CFileException* pEx)
					{
						ATLTRACE2(_T("File %20s cannot be removed\n"), csFilePath);
						pEx->Delete();
					}
				}
			
				// remove spk
				csFilePath = csDeletedSpkNames.GetAt(i);
				if (!csFilePath.IsEmpty())
				{
					try
					{
					   CFile::Remove(csFilePath);
					}
					catch (CFileException* pEx)
					{
						ATLTRACE2(_T("File %20s cannot be removed\n"), csFilePath);
						pEx->Delete();
					}
				}
			}
		}
	}
}

void CdbWaveDoc::RemoveMissingFiles()
{
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep (1);	
	CString cscomment;
	CString csfilecomment = _T("Checking database consistency - find and remove missing files: ");
	CString csDummy;

	int indexcurrent = DBGetCurrentRecordPosition();
	int nfiles = DBGetNRecords();
	int ifile = 0;
	
	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();	

	m_pDB->m_mainTableSet.Close();				// close dynaset and open as datatable
	try { m_pDB->m_mainTableSet.Open(dbOpenTable, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}
	
	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	m_pDB->m_mainTableSet.MoveFirst();
	BOOL bDatFile;
	BOOL bSpkFile;
	CFileStatus rStatus;

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		while(!m_pDB->m_mainTableSet.IsEOF()) 
		{
			csDummy.Format(_T("%i\t"),   ifile);
			// check if user wants to stop
			if(dlg.CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;
			// load file names
			m_pDB->GetFilenamesFromCurrentRecord();
			cscomment.Format(_T("Processing file [%i / %i] %s"), ifile, nfiles, (LPCTSTR) m_currentDatafileName);
			dlg.SetStatus(cscomment);
			// check if files are present
			bDatFile = CFile::GetStatus(m_currentDatafileName, rStatus);	// check if data file is present
			bSpkFile = CFile::GetStatus(m_currentSpikefileName, rStatus);	// check if spike file is present
			if (!bDatFile && !bSpkFile)										// if none of them found, remove record
				m_pDB->m_mainTableSet.Delete();

			m_pDB->m_mainTableSet.MoveNext();
			// update dialog box
			ifile++;
			if (MulDiv(ifile, 100, nfiles) > istep)
			{
				dlg.StepIt();
				istep = MulDiv(ifile, 100, nfiles);
			}
		}
		m_pDB->m_mainTableSet.MoveFirst();
	}

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	try { m_pDB->m_mainTableSet.Open(dbOpenDynaset, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}
	m_pDB->m_mainTableSet.Requery();
}

// --------------------
void CdbWaveDoc::RemoveFalseSpkFiles()
{
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep (1);	
	CString cscomment;
	CString csfilecomment = _T("Checking database consistency - find and remove missing files: ");
	CString csDummy;

	int indexcurrent = DBGetCurrentRecordPosition();
	int nfiles = DBGetNRecords();
	int ifile = 0;
	int iFilesRemoved = 0;
	int iErrorsCorrected = 0;
	
	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();	

	m_pDB->m_mainTableSet.Close();				// close dynaset and open as datatable
	try { m_pDB->m_mainTableSet.Open(dbOpenTable, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}
	
	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	m_pDB->m_mainTableSet.MoveFirst();
	BOOL bSpkFile;
	CString csSpikefileName;
	CFileStatus rStatus;

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		while(!m_pDB->m_mainTableSet.IsEOF()) 
		{
			csDummy.Format(_T("%i\t"),   ifile);
			// check if user wants to stop
			if(dlg.CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;
			m_pDB->GetFilenamesFromCurrentRecord();
			cscomment.Format(_T("Processing file [%i / %i] %s"), ifile, nfiles, (LPCTSTR) m_currentDatafileName);
			dlg.SetStatus(cscomment);
			
			// check if spike file is present
			csSpikefileName = m_currentDatafileName.Left(m_currentDatafileName.ReverseFind('.')+1) + _T("spk");
			bSpkFile = CFile::GetStatus(csSpikefileName, rStatus);

			// if spk file not found in the same directory, remove record
			if (!bSpkFile)
			{
				m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), TRUE);
				iFilesRemoved++;
			}
			else
			{
				// compare the 2 file names - if different, copy dataname & directory index into spike name & directory fields
				if (csSpikefileName.CompareNoCase(m_currentSpikefileName) != 0)
				{
					csSpikefileName = m_pDB->m_mainTableSet.m_Filedat.Left(m_pDB->m_mainTableSet.m_Filedat.ReverseFind('.')+1) + _T("spk");
					m_pDB->m_mainTableSet.m_Filespk = csSpikefileName;
					iErrorsCorrected++;
				}
			}

			m_pDB->m_mainTableSet.MoveNext();			
			ifile++;
			if (MulDiv(ifile, 100, nfiles) > istep)
			{
				dlg.StepIt();
				istep = MulDiv(ifile, 100, nfiles);
			}
		}
		m_pDB->m_mainTableSet.MoveFirst();
	}

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	try { m_pDB->m_mainTableSet.Open(dbOpenDynaset, nullptr, 0);}
	catch(CDaoException* e) {AfxMessageBox(e->m_pErrorInfo->m_strDescription);e->Delete(); return;}
	m_pDB->m_mainTableSet.Requery();

	// trace
	if (iErrorsCorrected == 0 && iFilesRemoved == 0)
		AfxMessageBox(_T("No errors found"));
	else
	{
		CString cs;
		cs.Format(_T("Changes were made to the data base:\nerrors corrected: %i, files removed: %i\n"), iErrorsCorrected, iFilesRemoved);
		AfxMessageBox(cs);
	}
}

void CdbWaveDoc::ExportDatafilesAsTXTfiles()
{
	// save current index position - restore on exit
	long currentfile = DBGetCurrentRecordPosition();
	
	// make sure there are objects to read / write data and spike files	
	if (m_pDat == nullptr)		// data doc
	{
		m_pDat = new CAcqDataDoc;
		ASSERT(m_pDat != NULL);
	}	

	// prepare clipboard to receive names of files exported
	CSharedFile* psf = ((CdbWaveApp*) AfxGetApp())->m_psf;
	SAFE_DELETE(psf);
	((CdbWaveApp*) AfxGetApp())->m_psf= nullptr;
	psf = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	
	// prepare progress dialog box
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	CString cscomment;
	long nfiles = DBGetNRecords();
	int ifile = 1;
	dlg.SetRange(0, nfiles);
	CString csFilenames;

	// got to the first record
	m_pDB->m_mainTableSet.MoveFirst();
	dlg.SetPos(0);

	while(!m_pDB->m_mainTableSet.IsEOF()) 
	{
		m_pDB->GetFilenamesFromCurrentRecord();
		// check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i]"), ifile, nfiles);
		dlg.SetStatus(cscomment);

		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			// open file
			BOOL bflag = OpenCurrentDataFile();
			ASSERT(bflag == TRUE);
			
			// create text file on disk with the same name as the data file_with dat
			CStdioFile dataDest;								// destination file object
			CString cstxtFile = m_currentDatafileName + _T("_.txt");
			CFileException fe;		// trap exceptions	
			if (!dataDest.Open(cstxtFile, CFile::modeCreate | CFile::modeReadWrite |CFile::shareDenyNone | CFile::typeText,&fe))
			{
				dataDest.Abort();	// file not found
				continue;
			}

			// export data
			m_pDat->ExportDataFile_to_TXTFile(&dataDest);	// get infos
			dataDest.Close();
			cstxtFile += _T("\n");
			psf->Write(cstxtFile, cstxtFile.GetLength() * sizeof(TCHAR));
		}

		// move to next record
		m_pDB->m_mainTableSet.MoveNext();
		ifile++;
		dlg.SetPos(ifile);

	}
	m_pDB->m_mainTableSet.MoveFirst();
	m_pDB->GetFilenamesFromCurrentRecord();

	// restore current data position
	DBSetCurrentRecordPosition(currentfile);
	((CdbWaveApp*) AfxGetApp())->m_psf =psf;
}


