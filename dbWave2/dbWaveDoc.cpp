#include "StdAfx.h"
#include "dbWave.h"

#include "dbMainTable.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"	
#include "DlgdbNewFileDuplicate.h"
#include "DlgProgress.h"
#include "NoteDoc.h"
#include "CViewNotedoc.h"
#include "dbWave_constants.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

numberIDToText CdbWaveDoc::headers[] = {
	{0, _T("path")}, {1, _T("Expt")}, {2, _T("insectID")}, {3, _T("ssID")}, {4, _T("insect")},
	{5, _T("strain")}, {6, _T("sex")}, {7, _T("location")}, {8, _T("operator")}, {9, _T("more")},
	{10, _T("stim1")}, {11, _T("conc1")}, {12, _T("repeat1")}, {13, _T("stim2")}, {14, _T("conc2")},
	{15, _T("repeat2")},
	{16, _T("type")}, {16, _T("sensillum")},
	{17, _T("flag")}
};

IMPLEMENT_DYNCREATE(CdbWaveDoc, COleDocument)

BEGIN_MESSAGE_MAP(CdbWaveDoc, COleDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, COleDocument::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

CdbWaveDoc::CdbWaveDoc()
= default;

CdbWaveDoc::~CdbWaveDoc()
{
	SAFE_DELETE(m_pDB)
	SAFE_DELETE(m_pDat)
	SAFE_DELETE(m_pSpk)

	// store temp mdb filename to delete on exit within the application
	// (it seems it is not possible to erase the file from here)
	if (m_bClearMdbOnExit)
	{
		auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		p_app->m_tempMDBfiles.Add(m_dbFilename);
	}
}

void CdbWaveDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CDocument::UpdateAllViews(pSender, lHint, pHint);
	// passes message OnUpdate() to the mainframe and add a reference to the document that sends it
	auto pmain = static_cast<CMainFrame*>(AfxGetMainWnd());
	pmain->OnUpdate(reinterpret_cast<CView*>(this), lHint, pHint);
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;

	m_bcallnew = FALSE;
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	auto cs_path = p_app->Get_MyDocuments_MydbWavePath();
	TCHAR sz_entry[MAX_PATH];
	unsigned long len = MAX_PATH;
	GetUserName(&sz_entry[0], &len);
	CString cs_ext = sz_entry;
	cs_ext += _T("_1");
	cs_path += _T('\\') + cs_ext;
	return OnNewDocument(cs_path);
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::OnNewDocument(LPCTSTR lpszPathName)
{
	if (m_bcallnew && !COleDocument::OnNewDocument())
		return FALSE;
	m_bcallnew = TRUE;

	CString cs_name = lpszPathName;
	cs_name += _T(".mdb");
	if (cs_name.Find(_T(':')) < 0
		&& cs_name.Find(_T("\\\\")) < 0)
	{
		const auto cspath = dynamic_cast<CdbWaveApp*>(AfxGetApp())->Get_MyDocuments_MydbWavePath();
		cs_name = cspath + _T('\\') + cs_name;
	}

	// check if this file is already present, exit if not...
	CFileStatus status;
	auto b_exist = CFile::GetStatus(cs_name, status);

	while (b_exist)
	{
		DlgdbNewFileDuplicate dlg;
		dlg.m_pfilein = cs_name;
		if (IDOK == dlg.DoModal())
		{
			switch (dlg.m_option)
			{
			case 0: // open file
				m_pDB = new CdbWdatabase;
				m_pDB->Attach(&m_currentDatafileName, &m_currentSpikefileName);
				return OpenDatabase(cs_name);
			//break;
			case 1: // overwrite file
				CFile::Remove(cs_name);
				b_exist = FALSE;
				break;
			case 2: // try another name
				cs_name = dlg.m_fileout;
				b_exist = CFile::GetStatus(cs_name, status);
				break;
			case 3: // abort process
			default:
				m_validTables = FALSE;
				return FALSE;
			//break;
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
	m_pDB->Create(cs_name, dbLangGeneral, dbVersion30);
	m_dbFilename = cs_name;
	m_ProposedDataPathName = cs_name.Left(cs_name.ReverseFind('.'));
	m_pDB->CreateTables();

	// save file name
	m_validTables = m_pDB->OpenTables();
	SetTitle(cs_name);
	SetPathName(cs_name, TRUE);

	return m_validTables;
}

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

#ifdef _DEBUG
void CdbWaveDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CdbWaveDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif

BOOL CdbWaveDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// open database document
	CString csnew = lpszPathName;
	CFileStatus status;
	if (!CFile::GetStatus(lpszPathName, status))
		return FALSE;

	csnew.MakeLower();
	CString cs_ext;
	cs_ext.Empty();
	const auto iext = csnew.ReverseFind('.');
	if (iext > 0)
		cs_ext = csnew.Right(csnew.GetLength() - iext - 1);

	if ((cs_ext.Compare(_T("mdb")) == 0) || (cs_ext.Compare(_T("accdb")) == 0))
	{
		if (!COleDocument::OnOpenDocument(lpszPathName))
			return FALSE;
		return OpenDatabase(lpszPathName);
	}

	// open spike or dat documents
	if ((cs_ext.Compare(_T("dat")) == 0) || (cs_ext.Compare(_T("spk")) == 0)
		|| (cs_ext.Compare(_T("asd")) == 0) || (cs_ext.Compare(_T("smr")) == 0)
		|| (cs_ext.Compare(_T("smrx")) == 0))
	{
		if (iext > 0)
			csnew = csnew.Left(iext);
		const auto flag = OnNewDocument(csnew);
		if (flag)
		{
			CStringArray filenames;
			filenames.Add(lpszPathName);
			ImportFileList(filenames);
		}
		return flag;
	}
	return FALSE;
}

BOOL CdbWaveDoc::OpenDatabase(LPCTSTR lpszPathName)
{
	auto tmp_db = new CdbWdatabase;
	tmp_db->Attach(&m_currentDatafileName, &m_currentSpikefileName);

	// Attempt to open the new database before replacing our ptr
	try
	{
		tmp_db->Open(lpszPathName);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 4);
		e->Delete();
		tmp_db->Close();
		delete tmp_db;
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
	m_pDB = tmp_db;

	// save file name
	m_validTables = m_pDB->OpenTables();
	SetPathName(lpszPathName, TRUE);

	return m_validTables;
}

BOOL CdbWaveDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// now duplicate file
	const auto csoldname = GetPathName();
	const auto b_fail_if_exists = TRUE;

	const auto b_done = ::CopyFile(csoldname, lpszPathName, b_fail_if_exists);
	if (b_done == 0)
	{
		LPVOID lp_msg_buf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			reinterpret_cast<LPTSTR>(&lp_msg_buf),
			0,
			nullptr);

		// Process any inserts in lpMsgBuf.
		CString cs_error = static_cast<LPTSTR>(lp_msg_buf);
		cs_error = _T("Error when copying:\n") + csoldname + _T("\ninto:\n") + lpszPathName + _T("\n\n") + cs_error;

		// Display the string.
		MessageBox(nullptr, cs_error, _T("Error"), MB_OK | MB_ICONINFORMATION);
		// Free the buffer.
		LocalFree(lp_msg_buf);
		return FALSE;
	}
	return TRUE;
}

CString CdbWaveDoc::SetDB_CurrentSpikeFileName()
{
	if (!GetDB_CurrentSpkFileName(TRUE).IsEmpty())
		return m_currentSpikefileName;

	// not found? derive name from data file (same directory)
	m_currentSpikefileName = m_currentDatafileName.Left(m_currentDatafileName.ReverseFind(_T('.')) + 1) + _T("spk");

	const auto i_count = m_currentSpikefileName.ReverseFind(_T('\\'));
	const auto cs_name = m_currentSpikefileName.Right(m_currentSpikefileName.GetLength() - i_count - 1);

	try
	{
		m_pDB->m_mainTableSet.Edit();

		// change spike file name
		m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), TRUE);
		m_pDB->m_mainTableSet.m_Filespk = cs_name;

		m_pDB->m_mainTableSet.m_path2_ID = m_pDB->m_mainTableSet.m_path_ID;
		m_pDB->m_mainTableSet.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 10);
		e->Delete();
	}
	return m_currentSpikefileName;
}

CString CdbWaveDoc::GetDB_CurrentDatFileName(const BOOL b_test)
{
	m_currentDatafileName = m_pDB->GetDatFilenameFromCurrentRecord();
	auto filename = m_currentDatafileName;
	if (b_test && !IsFilePresent(filename))
		filename.Empty();
	return filename;
}

CString CdbWaveDoc::GetDB_CurrentSpkFileName(const BOOL b_test)
{
	m_currentSpikefileName = m_pDB->GetSpkFilenameFromCurrentRecord();
	auto file_name = m_currentSpikefileName;
	if (b_test && !IsFilePresent(file_name))
		file_name.Empty();
	return file_name;
}

long CdbWaveDoc::GetDB_DataLen()
{
	long datalen = 0;
	try { datalen = m_pDB->m_mainTableSet.m_datalen; }
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 11);
		e->Delete();
	}

	return datalen;
}

AcqDataDoc* CdbWaveDoc::OpenCurrentDataFile()
{
	// data doc to read data files
	if (m_pDat == nullptr)
	{
		m_pDat = new AcqDataDoc;
		ASSERT(m_pDat != NULL);
	}

	// open document; erase object if operation failed
	//GetDB_CurrentRecordPosition();
	GetDB_CurrentDatFileName(TRUE);
	if (m_currentDatafileName.IsEmpty()
		|| !m_pDat->OnOpenDocument(m_currentDatafileName))
	{
		delete m_pDat;
		m_pDat = nullptr;
	}
	else
		m_pDat->SetPathName(m_currentDatafileName, FALSE);
	return m_pDat;
}

void CdbWaveDoc::CloseCurrentDataFile()
{
	if (m_pDat != nullptr)
		m_pDat->AcqCloseFile();
}

CSpikeDoc* CdbWaveDoc::OpenCurrentSpikeFile()
{
	// spike doc to read data files
	if (m_pSpk == nullptr)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}
	// open document; erase object if operation fails
	const auto currentfile = GetDB_CurrentRecordPosition();
	GetDB_CurrentSpkFileName(TRUE);
	if (m_currentSpikefileName.IsEmpty()
		|| !m_pSpk->OnOpenDocument(m_currentSpikefileName))
	{
		delete m_pSpk;
		m_pSpk = nullptr;
	}
	else
		m_pSpk->SetPathName(m_currentSpikefileName, FALSE);
	return m_pSpk;
}

void CdbWaveDoc::GetAllSpkMaxMin(BOOL bAllFiles, BOOL bRecalc, int* max, int* min)
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (bAllFiles)
	{
		nfiles = GetDB_NRecords();
		ncurrentfile = GetDB_CurrentRecordPosition();
	}

	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (bAllFiles)
		{
			SetDB_CurrentRecordPosition(ifile);
			OpenCurrentSpikeFile();
			m_pSpk->SetSpkList_AsCurrent(0);
		}
		auto p_spk_list = m_pSpk->GetSpkList_Current();
		p_spk_list->GetTotalMaxMin(bRecalc, max, min);
	}

	if (bAllFiles)
	{
		SetDB_CurrentRecordPosition(ncurrentfile);
		OpenCurrentSpikeFile();
		m_pSpk->SetSpkList_AsCurrent(0);
	}
}

CSize CdbWaveDoc::GetSpkMaxMin_y1(BOOL bAll)
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (bAll)
	{
		nfiles = GetDB_NRecords();
		ncurrentfile = GetDB_CurrentRecordPosition();
	}

	CSize dummy(0, 0);
	BOOL initialized = false;
	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (bAll)
		{
			SetDB_CurrentRecordPosition(ifile);
			OpenCurrentSpikeFile();
			m_pSpk->SetSpkList_AsCurrent(0);
		}
		auto p_spk_list = m_pSpk->GetSpkList_Current();
		if (p_spk_list->GetTotalSpikes() == 0)
			continue;

		CSize measure = p_spk_list->Measure_Y1_MaxMin();
		if (initialized)
		{
			if (dummy.cx < measure.cx) dummy.cx = measure.cx;
			if (dummy.cy > measure.cy) dummy.cy = measure.cy;
		}
		else
		{
			initialized = true;
			dummy.cx = measure.cx;
			dummy.cy = measure.cy;
		}
	}

	if (bAll)
	{
		SetDB_CurrentRecordPosition(ncurrentfile);
		OpenCurrentSpikeFile();
		m_pSpk->SetSpkList_AsCurrent(0);
	}

	return dummy;
}

long CdbWaveDoc::GetDB_CurrentRecordPosition()
{
	long ifile = -1;
	try
	{
		ifile = m_pDB->m_mainTableSet.GetAbsolutePosition();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 14);
		e->Delete();
	}
	return ifile;
}

long CdbWaveDoc::GetDB_CurrentRecordID()
{
	return m_pDB->m_mainTableSet.m_ID;
}

void CdbWaveDoc::SetDB_CurrentRecordFlag(const int flag)
{
	m_pDB->m_mainTableSet.Edit();
	m_pDB->m_mainTableSet.m_flag = flag;
	m_pDB->m_mainTableSet.Update();
}

void CdbWaveDoc::SetDB_PathsRelative_to_DataBaseFile()
{
	m_pDB->ConvertPathTabletoRelativePath();
}

void CdbWaveDoc::SetDB_PathsAbsolute()
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

void CdbWaveDoc::SetDB_nbspikes(long nspikes)
{
	try
	{
		m_pDB->m_mainTableSet.Edit();
		m_pDB->m_mainTableSet.m_nspikes = nspikes;
		m_pDB->m_mainTableSet.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 15);
		e->Delete();
	}
}

long CdbWaveDoc::GetDB_nbspikes()
{
	return m_pDB->m_mainTableSet.m_nspikes;
}

long CdbWaveDoc::GetDB_nbspikeclasses()
{
	return m_pDB->m_mainTableSet.m_nspikeclasses;
}

void CdbWaveDoc::SetDB_nbspikeclasses(long nclasses)
{
	try
	{
		m_pDB->m_mainTableSet.Edit();
		m_pDB->m_mainTableSet.m_nspikeclasses = nclasses;
		m_pDB->m_mainTableSet.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 16);
		e->Delete();
	}
}

void CdbWaveDoc::Export_DataAsciiComments(CSharedFile* p_shared_file)
{
	DlgProgress dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep(1);
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	CString cs_dummy;
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	const auto p_view_data_options = &(p_app->options_viewdata);
	const int index_current = GetDB_CurrentRecordPosition();
	const int n_files = GetDB_NRecords();

	// memory allocated -- get pointer to it
	cs_dummy.Format(_T("n files = %i\r\n\r\n"), n_files);
	p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// loop over all files of the multi-document
	for (auto ifile = 0; ifile < n_files; ifile++)
	{
		// get ith file's comment
		SetDB_CurrentRecordPosition(ifile);
		cs_dummy.Format(_T("%i\t%i\t"), ifile + 1, m_pDB->m_mainTableSet.m_ID);
		p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		// set message for dialog box
		// check if user wants to stop
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i] %s"), ifile + 1, n_files, (LPCTSTR)m_currentDatafileName);
		dlg.SetStatus(cscomment);

		cs_dummy.Empty();
		if (OpenCurrentDataFile() != nullptr)
		{
			cs_dummy += m_pDat->GetDataFileInfos(p_view_data_options); // get infos
			if (p_view_data_options->bdatabasecols)
				cs_dummy += Export_DatabaseData();
		}
		else
			cs_dummy += _T("file not found\t");

		cs_dummy += _T("\r\n"); // next line
		p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		if (MulDiv(ifile, 100, n_files) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, n_files);
		}
	}

	SetDB_CurrentRecordPosition(index_current);
	OpenCurrentDataFile();
}

BOOL CdbWaveDoc::CreateDirectories(CString path)
{
	// first decompose the path into subdirectories
	CStringArray cs_sub_paths;
	auto cs_new_path = path;
	auto fpos = 1;
	BOOL b_exists;
	do
	{
		// test if directory already present
		b_exists = PathFileExists(cs_new_path);

		// if not, strip the chain down from one subdirectory
		if (!b_exists)
		{
			fpos = cs_new_path.ReverseFind(_T('\\'));
			if (fpos != -1)
			{
				auto subpath = cs_new_path.Right(cs_new_path.GetLength() - fpos);
				cs_sub_paths.Add(subpath);
				cs_new_path = cs_new_path.Left(fpos);
			}
		}
	}
	while (fpos > 0 && b_exists == FALSE);

	// now, newPath should be something like "c:" (if data are on a disk) or "" (if data are on a network like "\\partage" )
	for (auto i = cs_sub_paths.GetUpperBound(); i >= 0; i--)
	{
		cs_new_path = cs_new_path + cs_sub_paths.GetAt(i);
		if (!CreateDirectory(cs_new_path, nullptr))
		{
			AfxMessageBox(IDS_DIRECTORYFAILED);
			return FALSE;
		}
	}
	// then loop through array to create directories
	return TRUE;
}

BOOL CdbWaveDoc::CopyAllFilesintoDirectory(const CString& path)
{
	// prepare dialog box to tell what is going on ----------------------
	DlgProgress dlg;
	dlg.Create();
	auto istep = 0;
	dlg.SetStep(1);
	CString cs_comment;
	CString cs_file_comment = _T("Copy files");
	CString cs_dummy;

	// read all data paths and create new array --------------------------
	dlg.SetStatus(_T("Create destination directories..."));
	CStringArray cs_source_path_array;
	CStringArray cs_dest_path_array;
	CUIntArray ui_id_array;
	const auto cs_path = path + _T('\\');
	auto flag = CreateDirectories(path); // create root destination directory if necessary
	if (!flag)
		return FALSE;

	// loop over database pathSet : copy paths into 2 string arrays and create subdirectories
	m_pDB->m_pathSet.MoveFirst();
	auto nrecords = 0;
	while (!m_pDB->m_pathSet.IsEOF())
	{
		cs_source_path_array.Add(m_pDB->m_pathSet.m_cs);
		ui_id_array.Add(m_pDB->m_pathSet.m_ID);
		nrecords++;
		m_pDB->m_pathSet.MoveNext();
	}

	// find root & build new directories
	auto cs_root = cs_source_path_array[0];
	auto cs_root_length = cs_root.GetLength();
	for (auto i = 1; i < nrecords; i++)
	{
		// trim strings to the same length from the left
		auto cs_current = cs_source_path_array[i];
		if (cs_root_length > cs_current.GetLength())
		{
			cs_root = cs_root.Left(cs_current.GetLength());
			cs_root_length = cs_root.GetLength();
		}
		if (cs_current.GetLength() > cs_root_length)
			cs_current = cs_current.Left(cs_root_length);

		// loop to find the smallest common string
		for (auto j = cs_root_length; j > 1; j--)
		{
			if (0 == cs_current.CompareNoCase(cs_root))
				break;
			// not found, search next subfolder in rootstring and clip current
			const auto fpos = cs_root.ReverseFind(_T('\\'));
			if (fpos != -1)
			{
				cs_root = cs_root.Left(fpos);
				cs_root_length = fpos;
			}
			cs_current = cs_current.Left(cs_root_length);
		}
	}

	// create new paths and create directories
	if (cs_root_length <= 2)
	{
		CString cs_buf_temp;
		for (auto i = 0; i < nrecords; i++)
		{
			cs_buf_temp.Format(_T("%06.6lu"), i);
			auto csdummy = cs_path + cs_buf_temp;
			cs_dest_path_array.Add(csdummy);
		}
	}
	else
	{
		for (auto i = 0; i < nrecords; i++)
		{
			auto sub_path = cs_source_path_array.GetAt(i);
			sub_path = sub_path.Right(sub_path.GetLength() - cs_root_length - 1);
			auto csdummy = cs_path + sub_path;
			cs_dest_path_array.Add(csdummy);
		}
	}

	// copy database into new database ------------------------------------
	dlg.SetStatus(_T("Copy database ..."));
	auto olddatabase = GetPathName();
	const auto icount = olddatabase.ReverseFind(_T('\\')) + 1;
	auto newpath = path;
	auto newname = olddatabase.Right(olddatabase.GetLength() - icount);
	const auto fpos = path.ReverseFind(_T('\\'));
	if (fpos != -1)
	{
		newname = path.Right(path.GetLength() - fpos) + _T(".mdb");
		newpath = path.Left(fpos);
	}
	const auto destdatabase = newpath + newname;

	// create database and copy all records
	flag = OnSaveDocument(destdatabase);
	if (!flag)
		return FALSE;

	// read all data and spike file names and store them into an array ------------
	if (m_pDB->m_mainTableSet.GetRecordCount() < 1)
		return FALSE;

	// create new document and open it to update the paths
	auto* p_new = new CdbWaveDoc;
	flag = p_new->OnOpenDocument(destdatabase);
	if (!flag)
	{
		delete p_new;
		return FALSE;
	}

	// read fileList & copy records into target tableSet
	dlg.SetStatus(_T("Build a list of files to copy..."));

	CStringArray old_names_array;
	CStringArray new_names_array;
	CUIntArray ui_id_new_path_array;
	try
	{
		m_pDB->m_mainTableSet.MoveFirst();

		while (!m_pDB->m_mainTableSet.IsEOF())
		{
			m_pDB->GetFilenamesFromCurrentRecord();
			// data file
			if (!m_currentDatafileName.IsEmpty())
			{
				old_names_array.Add(m_currentDatafileName);

				const UINT uid = m_pDB->m_mainTableSet.m_path_ID;
				auto j = 0;
				for (auto i = 0; i < ui_id_array.GetSize(); i++, j++)
					if (ui_id_array.GetAt(i) == uid)
						break;
				ui_id_new_path_array.Add(j);
				new_names_array.Add(m_pDB->m_mainTableSet.m_Filedat);
			}
			// spike file
			if (!m_currentSpikefileName.IsEmpty())
			{
				old_names_array.Add(m_currentSpikefileName);
				const UINT uid = m_pDB->m_mainTableSet.m_path2_ID;
				auto j = 0;
				for (auto i = 0; i < ui_id_array.GetSize(); i++, j++)
					if (ui_id_array.GetAt(i) == uid)
						break;
				ui_id_new_path_array.Add(j);
				new_names_array.Add(m_pDB->m_mainTableSet.m_Filespk);
			}
			// move to next pRecord
			m_pDB->m_mainTableSet.MoveNext();
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
	}

	// replace the destination paths
	dlg.SetStatus(_T("Update directories in the destination database..."));

	// change paths
	try
	{
		p_new->m_pDB->m_pathSet.MoveFirst();
		while (!p_new->m_pDB->m_pathSet.IsEOF())
		{
			auto cs_path_local = p_new->m_pDB->m_pathSet.m_cs;
			// find corresponding path in old paths
			auto ifound = -1;
			for (auto i = 0; i < cs_source_path_array.GetSize(); i++)
			{
				if (cs_path_local.Compare(cs_source_path_array.GetAt(i)) == 0)
				{
					ifound = i;
					break;
				}
			}
			if (ifound > -1)
			{
				p_new->m_pDB->m_pathSet.Edit();
				p_new->m_pDB->m_pathSet.m_cs = cs_dest_path_array.GetAt(ifound);
				p_new->m_pDB->m_pathSet.Update();
			}
			p_new->m_pDB->m_pathSet.MoveNext();
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
	}

	//-------------------------------------------------------
	// memory allocated -- get pointer to it
	const auto nfiles = old_names_array.GetSize();
	cs_dummy.Format(_T("n files (*.dat & *.spk) = %i\r\n\r\n"), nfiles);

	// loop over all files of the multi-document
	for (auto ifile = 0; ifile < nfiles; ifile++)
	{
		// set message for dialog box and check if user wants to stop
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		// get source name
		const auto& source_file = old_names_array.GetAt(ifile);
		cs_comment.Format(_T("Processing file [%i / %i] %s"), ifile + 1, nfiles, (LPCTSTR)source_file);
		dlg.SetStatus(cs_comment);

		// get destination directory
		const int i_path = int(ui_id_new_path_array.GetAt(ifile));
		const auto& path_local = cs_dest_path_array.GetAt(i_path);
		// create directory if necessary or exit if not possible
		if (!CreateDirectories(path_local))
			return FALSE;

		// copy file
		auto destination_file = path_local + _T("\\") + new_names_array.GetAt(ifile);
		BinaryFileCopy(source_file, destination_file);

		// update count
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}

	//-------------------------------------------------------
	// remove non-relevant files from the new database
	if (p_new->m_pDB->m_mainTableSet.GetEditMode() != dbEditNone)
		p_new->m_pDB->m_mainTableSet.Update();
	p_new->m_pDB->m_mainTableSet.Close(); // close dynaset and open as datatable

	if (!p_new->m_pDB->m_mainTableSet.OpenTable(dbOpenTable, nullptr, 0))
		return FALSE;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	p_new->m_pDB->m_mainTableSet.MoveFirst();

	if (!p_new->m_pDB->m_mainTableSet.IsBOF())
	{
		while (!p_new->m_pDB->m_mainTableSet.IsEOF())
		{
			p_new->m_pDB->GetFilenamesFromCurrentRecord();
			CFileStatus status;
			const auto b_dat_file = CFile::GetStatus(p_new->m_currentDatafileName, status);
			// check if data file is present
			const auto b_spk_file = CFile::GetStatus(p_new->m_currentSpikefileName, status);
			// check if spike file is present
			if (!b_dat_file && !b_spk_file) // if none of them found, remove pRecord
				p_new->m_pDB->m_mainTableSet.Delete();
			p_new->m_pDB->m_mainTableSet.MoveNext();
		}
		p_new->m_pDB->m_mainTableSet.MoveFirst();
	}

	// close database and document
	delete p_new;
	return true;
}

bool CdbWaveDoc::BinaryFileCopy(const LPCTSTR pszSource, LPCTSTR pszDest)
{
	// check that destfile does not exist
	// check if same file already exists: if yes, destroy it
	CFileStatus status;
	const CString cs_newname = pszDest;
	if (CFile::GetStatus(cs_newname, status))
	{
		auto prompt = cs_newname;
		prompt += _T("\nThis file seems to exist already.\nDelete the old file?");
		if (AfxMessageBox(prompt, MB_YESNO) == IDYES)
			CFile::Remove(cs_newname);
		else
			return false;
	}

	// constructing these file objects doesn't open them
	CFile source_file;
	CFile destination_file;

	// we'll use a CFileException object to get error information
	CFileException ex;

	// open the source file for reading
	if (!source_file.Open(pszSource, CFile::modeRead | CFile::shareDenyNone, &ex))
	{
		// complain if an error happened
		TCHAR sz_error[1024];
		CString str_formatted = _T("Couldn't open source file:");
		if (ex.GetErrorMessage(sz_error, 1024))
			str_formatted += sz_error;
		AfxMessageBox(str_formatted);
		return false;
	}
	if (!destination_file.Open(pszDest, CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate, &ex))
	{
		TCHAR sz_error[1024];
		CString str_formatted = _T("Couldn't open source file:");
		if (ex.GetErrorMessage(sz_error, 1024))
			str_formatted += sz_error;
		AfxMessageBox(str_formatted);
		source_file.Close();
		return false;
	}

	BYTE buffer[4096];
	DWORD dw_read;

	// Read in 4096-byte blocks,
	// remember how many bytes were actually read,
	// and try to write that many out. This loop ends
	// when there are no more bytes to read.
	do
	{
		dw_read = source_file.Read(buffer, 4096);
		destination_file.Write(buffer, dw_read);
	}
	while (dw_read > 0);

	// Close both files

	destination_file.Close();
	source_file.Close();

	return true;
}

HMENU CdbWaveDoc::GetDefaultMenu()
{
	return m_hMyMenu; // just use original default
}

sourceData CdbWaveDoc::get_wave_format_from_either_file(CString cs_filename)
{
	sourceData record;

	record.i_last_backslash_position = cs_filename.ReverseFind('\\');
	const auto idotposition = cs_filename.ReverseFind('.');
	const auto namelen = idotposition - record.i_last_backslash_position - 1;
	record.cs_path = cs_filename.Left(record.i_last_backslash_position);
	record.cs_path.MakeLower();
	auto cs_extent = cs_filename.Right(cs_filename.GetLength() - idotposition - 1);
	auto cs_root_name = cs_filename.Mid(record.i_last_backslash_position + 1, namelen);

	const auto b_is_dat_file = IsExtensionRecognizedAsDataFile(cs_extent);
	if (b_is_dat_file)
	{
		record.cs_dat_file = cs_filename;
		record.cs_spk_file = cs_filename.Left(idotposition) + _T(".spk");
	}
	else
	{
		record.cs_dat_file = cs_filename.Left(idotposition) + _T(".") + cs_extent;
		record.cs_spk_file = cs_filename;
	}

	// test  files
	CFileStatus status;
	record.b_dat_present = CFile::GetStatus(record.cs_dat_file, status);
	record.b_spik_present = CFile::GetStatus(record.cs_spk_file, status);
	if (record.b_dat_present)
		cs_filename = record.cs_dat_file;
	else if (record.b_spik_present)
		cs_filename = record.cs_spk_file;
	record.p_wave_format = GetWaveFormat(cs_filename, record.b_dat_present);
	m_pDat->AcqCloseFile();

	return record;
}

int CdbWaveDoc::find_header(CString& text)
{
	int found = -1;
	for (int i = 0; i < 18; i++)
	{
		CString header = headers[i].csText;
		if (text.CompareNoCase(header) == 0)
		{
			found = headers[i].value;
			break;
		}
	}
	return found;
}

void CdbWaveDoc::getInfosFromStringArray(sourceData* pRecord, CStringArray& filenames, int irecord, int nColumns,
                                         boolean bHeader)
{
	CWaveFormat* p_wave_format = pRecord->p_wave_format;
	int index = index_2d_array(irecord, nColumns, bHeader);

	for (int i = 1; i < nColumns; i++)
	{
		int iColumn = i;
		if (bHeader)
		{
			CString text = filenames.GetAt(i);
			iColumn = find_header(text);
		}
		int index_irecord = index + i;
		CString csItem = filenames.GetAt(index_irecord);
		switch (iColumn)
		{
		case 1: p_wave_format->cs_comment = csItem;
			break;
		case 2: p_wave_format->insectID = _ttoi(csItem);
			break;
		case 3: p_wave_format->sensillumID = _ttoi(csItem);
			break;
		case 4: p_wave_format->csInsectname = csItem;
			break;
		case 5: p_wave_format->csStrain = csItem;
			break;
		case 6: p_wave_format->csSex = csItem;
			break;
		case 7: p_wave_format->csLocation = csItem;
			break;
		case 8: p_wave_format->csOperator = csItem;
			break;
		case 9: p_wave_format->csMoreComment = csItem;
			break;
		case 10: p_wave_format->csStimulus = csItem;
			break;
		case 11: p_wave_format->csConcentration = csItem;
			break;
		case 12: p_wave_format->repeat = _ttoi(csItem);
			break;
		case 13: p_wave_format->csStimulus2 = csItem;
			break;
		case 14: p_wave_format->csConcentration2 = csItem;
			break;
		case 15: p_wave_format->repeat2 = _ttoi(csItem);
			break;
		case 16: p_wave_format->csSensillum = csItem;
			break;
		case 17: p_wave_format->flag = _ttoi(csItem);
			break;
		default:
			break;
		}
	}
}

void CdbWaveDoc::set_record_file_names(sourceData* record)
{
	// save file names
	if (record->b_dat_present)
	{
		m_pDB->m_mainTableSet.m_path_ID = m_pDB->m_pathSet.GetStringInLinkedTable(record->cs_path);
		m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filedat), FALSE);
		m_pDB->m_mainTableSet.m_Filedat = record->cs_dat_file.Right(
			record->cs_dat_file.GetLength() - record->i_last_backslash_position - 1);
		m_pDB->m_mainTableSet.m_datalen = m_pDat->GetDOCchanLength();
	}

	if (record->b_spik_present)
	{
		m_pDB->m_mainTableSet.m_path2_ID = m_pDB->m_pathSet.GetStringInLinkedTable(record->cs_path);
		m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), FALSE);
		m_pDB->m_mainTableSet.m_Filespk = record->cs_spk_file.Right(
			record->cs_spk_file.GetLength() - record->i_last_backslash_position - 1);
	}
}

boolean CdbWaveDoc::set_record_spk_classes(sourceData* record)
{
	boolean flag = m_pSpk->OnOpenDocument(record->cs_spk_file);
	if (flag)
	{
		m_pDB->m_mainTableSet.m_nspikes = m_pSpk->GetSpkList_Current()->GetTotalSpikes();
		if (m_pSpk->GetSpkList_Current()->GetNbclasses() < 0)
			m_pSpk->GetSpkList_Current()->UpdateClassList();
		m_pDB->m_mainTableSet.m_nspikeclasses = m_pSpk->GetSpkList_Current()->GetNbclasses();
		m_pDB->m_mainTableSet.m_datalen = m_pSpk->m_acqsize;
	}
	return flag;
}

void CdbWaveDoc::set_record_wave_format(sourceData* record)
{
	m_pDB->TransferWaveFormatDataToRecord(record->p_wave_format);
}

void CdbWaveDoc::ImportFileList(CStringArray& fileList, int nColumns, boolean bHeader)
{
	// exit if no data to import
	int nfiles = get_size_2d_array(fileList, nColumns, bHeader);
	if (nfiles == 0)
		return;

	CSharedFile* psf = dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_psf;
	SAFE_DELETE(psf)
	dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_psf = nullptr;
	psf = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();
	m_pDB->m_mainTableSet.Close();
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenTable, nullptr, 0))
	{
		delete psf;
		return;
	}

	// browse existing database array - collect data file acquisition time and IDs already used
	m_pDB->m_mainTableSet.GetMaxIDs();
	long m_id = m_pDB->m_mainTableSet.max_ID;
	const auto nfilesok = check_files_can_be_opened(fileList, psf, nColumns, bHeader);

	// ---------------------------------------------file loop: read infos --------------------------------
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	dlg.SetPos(0);
	dlg.SetRange(0, nfilesok);

	for (auto irec = 0; irec < nfilesok; irec++)
	{
		// check if user wants to stop and update progression bar
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		// get file name
		int index = index_2d_array(irec, nColumns, bHeader);
		auto cs_filename = CString(fileList[index]);
		CString cs_comment;
		cs_comment.Format(_T("Import file [%i / %i] %s"), irec + 1, nfilesok, (LPCTSTR)cs_filename);
		dlg.SetStatus(cs_comment);
		dlg.StepIt();

		if (!import_file_single(cs_filename, m_id, irec, fileList, nColumns, bHeader))
			psf = file_discarded_message(psf, cs_filename, irec);
	}

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	if (m_pDB->m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
	{
		m_pDB->m_mainTableSet.Requery();
		m_pDB->m_mainTableSet.BuildAndSortIDArrays();
		m_pDB->m_mainTableSet.MoveLast();
	}

	// close files opened here
	SAFE_DELETE(m_pDat);
	SAFE_DELETE(m_pSpk);
	SAFE_DELETE(psf);
}

boolean CdbWaveDoc::import_file_single(CString& cs_filename, long& m_id, int irecord, CStringArray& csArray, int nColumns,
                                     boolean bHeader)
{
	// open document and read data - go to next file if not readable
	sourceData record = get_wave_format_from_either_file(cs_filename);
	if (record.p_wave_format == nullptr)
		return false;

	if (nColumns > 1)
		getInfosFromStringArray(&record, csArray, irecord, nColumns, bHeader);

	// check data acquisition time - go to next file if already exist and if flag set
	auto t = record.p_wave_format->acqtime;
	COleDateTime o_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	if (!dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_import.bDiscardDuplicateFiles)
	{
		if (!m_pDB->m_mainTableSet.CheckIfAcqDateTimeIsUnique(&o_time))
			return false;
	}

	// add new pRecord  -- mID
	m_pDB->m_mainTableSet.AddNew();
	m_id++;
	m_pDB->m_mainTableSet.m_ID = m_id;
	set_record_file_names(&record);
	set_record_spk_classes(&record);
	set_record_wave_format(&record);
	try
	{
		m_pDB->m_mainTableSet.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 17);
		e->Delete();
		return false;
	}
	return true;
}

BOOL CdbWaveDoc::IsExtensionRecognizedAsDataFile(CString string) const
{
	string.MakeLower();
	return (string.Find(_T("dat")) != -1
		|| string.Find(_T("mcid")) != -1
		|| string.Find(_T("asd")) != -1
		|| string.Find(_T("smr")) != -1
		|| string.Find(_T("smrx")) != -1);
}

CWaveFormat* CdbWaveDoc::GetWaveFormat(CString csFilename, BOOL bIsDatFile)
{
	// new data doc used to read data files
	if (m_pDat == nullptr)
	{
		m_pDat = new AcqDataDoc;
		ASSERT(m_pDat != NULL);
	}

	// new spike doc used to read data files
	if (m_pSpk == nullptr)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}

	CWaveFormat* p_wave_format = nullptr;
	if (bIsDatFile)
	{
		const auto b_is_read_ok = m_pDat->OnOpenDocument(csFilename);
		if (b_is_read_ok)
			p_wave_format = m_pDat->GetpWaveFormat();
	}
	else if (csFilename.Find(_T(".spk")) > 0)
	{
		const auto b_is_read_ok = m_pSpk->OnOpenDocument(csFilename);
		if (b_is_read_ok)
			p_wave_format = &(m_pSpk->m_wformat);
	}
	return p_wave_format;
}

BOOL CdbWaveDoc::ExtractFilenamesFromDatabase(const LPCSTR filename, CStringArray& filelist)
{
	// create dbWaveDoc, open database from this new document, then
	// read all files from this and import them into current document
	const auto flag = TRUE;
	auto* p_temp_database = new CDaoDatabase; // open database
	// Attempt to open the new database
	try
	{
		const CString csfilename(filename);
		p_temp_database->Open(csfilename);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 4);
		e->Delete();
		p_temp_database->Close();
		delete p_temp_database;
		return FALSE;
	}

	// check format of table set
	CDaoRecordset record_set(p_temp_database); // CDaoDatabase
	CdbMainTable table_set(p_temp_database);
	CdbIndexTable path_set(p_temp_database);
	path_set.SetNames(_T("path"), _T("path"), _T("pathID"));
	CDaoFieldInfo fieldinfo_filename; // 2
	CDaoFieldInfo fieldinfo_filespk; // 3
	CDaoFieldInfo fieldinfo_path_id; // 18
	CDaoFieldInfo fieldinfo_path2_id; // 19
	const CString cs_table = _T("table");

	// check for the presence of Table 'table'
	try
	{
		record_set.Open(dbOpenTable, cs_table);
		// check if column "filename" is present
		record_set.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_FILENAME].header_name, fieldinfo_filename);
		record_set.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_FILESPK].header_name, fieldinfo_filespk);
		record_set.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_PATH_ID].header_name, fieldinfo_path_id);
		record_set.GetFieldInfo(m_pDB->m_mainTableSet.m_desc[CH_PATH2_ID].header_name, fieldinfo_path2_id);
	}

	catch (CDaoException* e)
	{
		CString cs = _T("Error in a database operation:\n");
		cs += e->m_pErrorInfo->m_strDescription;
		AfxMessageBox(cs);
		e->Delete();
		return FALSE;
	}
	record_set.Close();

	// open tables
	try
	{
		path_set.Open(dbOpenTable, nullptr, 0);
		table_set.Open(dbOpenDynaset, nullptr, 0);
	}
	catch (CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// browse existing database array - collect data file acquisition time and IDs already used
	table_set.MoveFirst();
	while (!table_set.IsEOF())
	{
		table_set.MoveLast();
		table_set.MoveFirst();
		while (!table_set.IsEOF())
		{
			// collect data file name and then spike file name
			CString current_datafile_name;
			CString current_spikefile_name;

			// set current datdocument
			if (!table_set.IsFieldNull(&table_set.m_Filedat))
			{
				path_set.SeekID(table_set.m_path_ID);
				current_datafile_name = path_set.m_cs + _T('\\') + table_set.m_Filedat;
				filelist.Add(current_datafile_name);
			}
			else if (!table_set.IsFieldNull(&table_set.m_Filespk))
			{
				// get path (assume it is in the same path as the datafile)
				if (table_set.IsFieldNull(&table_set.m_path2_ID))
				{
					table_set.Edit();
					table_set.m_path2_ID = table_set.m_path_ID;
					table_set.Update();
				}
				path_set.SeekID(table_set.m_path2_ID);
				current_spikefile_name = path_set.m_cs + _T('\\') + table_set.m_Filespk;
				filelist.Add(current_spikefile_name);
			}

			// move to next pRecord
			table_set.MoveNext();
		}
	}
	table_set.Close();
	path_set.Close();

	p_temp_database->Close();
	delete p_temp_database;
	return flag;
}

BOOL CdbWaveDoc::ImportDatabase(CString& filename)
{
	// create dbWaveDoc, open database from this new document, then
	// read all files from this and import them into current document
	auto pnewdoc = new CdbWaveDoc; // open database
	if (!pnewdoc->OnOpenDocument(filename))
		return FALSE;

	// browse existing database array - collect data file acquisition time and IDs already used
	auto p_new_database = pnewdoc->m_pDB;
	p_new_database->m_mainTableSet.MoveFirst();
	auto n_added_records = 0;
	while (!p_new_database->m_mainTableSet.IsEOF())
	{
		m_pDB->ImportRecordfromDatabase(p_new_database);
		n_added_records++;
		p_new_database->m_mainTableSet.MoveNext();
	}
	p_new_database->m_mainTableSet.Close();
	delete pnewdoc;

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
		return FALSE;

	m_pDB->m_mainTableSet.Requery();
	m_pDB->m_mainTableSet.BuildAndSortIDArrays();

	return TRUE;
}

void CdbWaveDoc::SynchronizeSourceInfos(const BOOL b_all)
{
	// save current index position - restore on exit
	const auto currentfile = GetDB_CurrentRecordPosition();

	// make sure there are objects to read / write data and spike files
	if (m_pDat == nullptr) // data doc
	{
		m_pDat = new AcqDataDoc;
		ASSERT(m_pDat != NULL);
	}
	if (m_pSpk == nullptr) // spike doc
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
	}

	// do only one iteration?
	if (!b_all)
	{
		CWaveFormat* wave_format;
		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			const auto pDat = OpenCurrentDataFile();
			ASSERT(pDat != nullptr);
			wave_format = pDat->GetpWaveFormat();
			if (UpdateWaveFmtFromDatabase(wave_format))
				pDat->AcqSaveDataDescriptors();
		}
		// process spike file
		if (!m_currentSpikefileName.IsEmpty())
		{
			const auto pSpk = OpenCurrentSpikeFile();
			ASSERT(pSpk != nullptr);
			wave_format = &(m_pSpk->m_wformat);
			if (UpdateWaveFmtFromDatabase(wave_format))
				m_pSpk->OnSaveDocument(m_currentSpikefileName);
		}
		return;
	}

	// prepare progress dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	auto istep = 0;
	CString cscomment;
	const auto nfiles = GetDB_NRecords();
	auto ifile = 1;

	// got to the first pRecord
	m_pDB->m_mainTableSet.MoveFirst();
	dlg.SetPos(0);

	while (!m_pDB->m_mainTableSet.IsEOF())
	{
		m_pDB->GetFilenamesFromCurrentRecord();
		// check if user wants to stop
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i]"), ifile, nfiles);
		dlg.SetStatus(cscomment);

		CWaveFormat* p_wave_format;
		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			const auto pDat = OpenCurrentDataFile();
			ASSERT(pDat != nullptr);
			p_wave_format = m_pDat->GetpWaveFormat();
			if (UpdateWaveFmtFromDatabase(p_wave_format))
				m_pDat->AcqSaveDataDescriptors();
		}
		// process spike file
		if (!m_currentSpikefileName.IsEmpty())
		{
			const auto pSpk = OpenCurrentSpikeFile();
			ASSERT(pSpk != nullptr);
			p_wave_format = &(m_pSpk->m_wformat);
			if (UpdateWaveFmtFromDatabase(p_wave_format))
				m_pSpk->OnSaveDocument(m_currentSpikefileName);
		}
		// move to next pRecord
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
	SetDB_CurrentRecordPosition(currentfile);
}

BOOL CdbWaveDoc::UpdateWaveFmtFromDatabase(CWaveFormat* p_wave_format) const
{
	auto b_changed = FALSE;
	// long: experiment
	if (m_pDB->m_exptSet.SeekID(m_pDB->m_mainTableSet.m_expt_ID))
	{
		b_changed = (p_wave_format->cs_comment.Compare(m_pDB->m_exptSet.m_cs) != 0);
		p_wave_format->cs_comment = m_pDB->m_exptSet.m_cs;
	}

	// CString:	m_more;
	b_changed |= (p_wave_format->csMoreComment.Compare(m_pDB->m_mainTableSet.m_more) != 0);
	p_wave_format->csMoreComment = m_pDB->m_mainTableSet.m_more;

	// long:	m_operator_ID;
	if (m_pDB->m_operatorSet.SeekID(m_pDB->m_mainTableSet.m_operator_ID))
	{
		b_changed |= (p_wave_format->csOperator.Compare(m_pDB->m_operatorSet.m_cs) != 0);
		p_wave_format->csOperator = m_pDB->m_operatorSet.m_cs;
	}

	// long:	m_insect_ID;
	if (m_pDB->m_insectSet.SeekID(m_pDB->m_mainTableSet.m_insect_ID))
	{
		b_changed |= (p_wave_format->csInsectname.Compare(m_pDB->m_insectSet.m_cs) != 0);
		p_wave_format->csInsectname = m_pDB->m_insectSet.m_cs;
	}

	// long:	m_strain_ID;
	if (m_pDB->m_strainSet.SeekID(m_pDB->m_mainTableSet.m_strain_ID))
	{
		b_changed |= (p_wave_format->csStrain.Compare(m_pDB->m_strainSet.m_cs) != 0);
		p_wave_format->csStrain = m_pDB->m_strainSet.m_cs;
	}

	// long:	m_expt_ID;
	if (m_pDB->m_exptSet.SeekID(m_pDB->m_mainTableSet.m_expt_ID))
	{
		b_changed |= (p_wave_format->cs_comment.Compare(m_pDB->m_exptSet.m_cs) != 0);
		p_wave_format->cs_comment = m_pDB->m_exptSet.m_cs;
	}

	// long:	m_sex_ID;
	if (m_pDB->m_sexSet.SeekID(m_pDB->m_mainTableSet.m_sex_ID))
	{
		b_changed |= (p_wave_format->csSex.Compare(m_pDB->m_sexSet.m_cs) != 0);
		p_wave_format->csSex = m_pDB->m_sexSet.m_cs;
	}

	// long	m_location_ID;
	if (m_pDB->m_locationSet.SeekID(m_pDB->m_mainTableSet.m_location_ID))
	{
		b_changed |= (p_wave_format->csLocation.Compare(m_pDB->m_locationSet.m_cs) != 0);
		p_wave_format->csLocation = m_pDB->m_locationSet.m_cs;
	}

	// long	m_sensillum_ID;
	if (m_pDB->m_sensillumSet.SeekID(m_pDB->m_mainTableSet.m_sensillum_ID))
	{
		b_changed |= (p_wave_format->csSensillum.Compare(m_pDB->m_sensillumSet.m_cs) != 0);
		p_wave_format->csSensillum = m_pDB->m_sensillumSet.m_cs;
	}

	//long	m_stim_ID;
	if (m_pDB->m_stimSet.SeekID(m_pDB->m_mainTableSet.m_stim_ID))
	{
		b_changed |= (p_wave_format->csStimulus.Compare(m_pDB->m_stimSet.m_cs) != 0);
		p_wave_format->csStimulus = m_pDB->m_stimSet.m_cs;
	}

	// long	m_conc_ID;
	if (m_pDB->m_concSet.SeekID(m_pDB->m_mainTableSet.m_conc_ID))
	{
		b_changed |= (p_wave_format->csConcentration.Compare(m_pDB->m_concSet.m_cs) != 0);
		p_wave_format->csConcentration = m_pDB->m_concSet.m_cs;
	}

	// long	m_stim2_ID;
	if (m_pDB->m_stimSet.SeekID(m_pDB->m_mainTableSet.m_stim2_ID))
	{
		b_changed |= (p_wave_format->csStimulus2.Compare(m_pDB->m_stimSet.m_cs) != 0);
		p_wave_format->csStimulus2 = m_pDB->m_stimSet.m_cs;
	}

	// long	m_conc2_ID;
	if (m_pDB->m_concSet.SeekID(m_pDB->m_mainTableSet.m_conc2_ID))
	{
		b_changed |= (p_wave_format->csConcentration2.Compare(m_pDB->m_concSet.m_cs) != 0);
		p_wave_format->csConcentration2 = m_pDB->m_concSet.m_cs;
	}

	b_changed |= (p_wave_format->insectID != m_pDB->m_mainTableSet.m_IDinsect);
	// long	m_insectnumber;
	p_wave_format->insectID = m_pDB->m_mainTableSet.m_IDinsect;

	b_changed |= (p_wave_format->sensillumID != m_pDB->m_mainTableSet.m_IDsensillum);
	// long	m_IDsensillum;
	p_wave_format->sensillumID = m_pDB->m_mainTableSet.m_IDsensillum;

	b_changed |= (p_wave_format->repeat != m_pDB->m_mainTableSet.m_repeat);
	// long	m_repeat;
	p_wave_format->repeat = m_pDB->m_mainTableSet.m_repeat;

	b_changed |= (p_wave_format->repeat2 != m_pDB->m_mainTableSet.m_repeat2);
	// long	m_repeat2;
	p_wave_format->repeat2 = m_pDB->m_mainTableSet.m_repeat2;

	const auto npercycle = static_cast<int>(m_pSpk->m_stimIntervals.nitems / 2.f
		/ m_pSpk->GetAcqDuration() / 8.192f);
	b_changed |= (npercycle != m_pSpk->m_stimIntervals.npercycle);
	m_pSpk->m_stimIntervals.npercycle = npercycle;

	return b_changed;
}

void CdbWaveDoc::Export_SpkDescriptors(CSharedFile* pSF, CSpikeList* p_spike_list, int kclass)
{
	CString cs_dummy;
	const CString cs_tab = _T("\t");

	const auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_viewspikes = &(p_app->options_viewspikes);

	const auto cs_file_comment = _T("\r\n") + Export_DatabaseData();
	pSF->Write(cs_file_comment, cs_file_comment.GetLength() * sizeof(TCHAR));

	// spike file additional comments
	if (options_viewspikes->bspkcomments)
	{
		pSF->Write(cs_tab, cs_tab.GetLength() * sizeof(TCHAR));
		const auto cs_temp = m_pSpk->GetComment();
		pSF->Write(cs_temp, cs_temp.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	if (options_viewspikes->btotalspikes)
	{
		cs_dummy.Format(_T("%s%f"), (LPCTSTR)cs_tab, p_spike_list->GetdetectThresholdmV());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		cs_dummy.Format(_T("%s%i"), (LPCTSTR)cs_tab, p_spike_list->GetTotalSpikes());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("%s%i"), (LPCTSTR)cs_tab, p_spike_list->GetNbclasses());
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		const auto tduration = m_pSpk->GetAcqDuration();
		cs_dummy.Format(_T("%s%f"), (LPCTSTR)cs_tab, tduration);
		pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}

	// spike list iColumn, spike class
	if (options_viewspikes->spikeclassoption != 0)
		cs_dummy.Format(_T("%s%i %s%s %s%i"), (LPCTSTR)cs_tab, options_viewspikes->ichan,
			(LPCTSTR)cs_tab, (LPCTSTR)p_spike_list->GetComment(),
			(LPCTSTR)cs_tab, kclass);
	else
		cs_dummy.Format(_T("%s%i %s%s \t(all)"), (LPCTSTR)cs_tab, options_viewspikes->ichan,
			(LPCTSTR)cs_tab, (LPCTSTR)p_spike_list->GetComment());
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

CString CdbWaveDoc::Export_DatabaseData(const int ioption)
{
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_viewspikes = &(p_app->options_viewspikes);
	CString separator = _T("\t");
	if (ioption == 1)
		separator = _T(" | ");

	// export data comment into csFileComment
	CString cs_file_comment;
	CString cs_dummy;

	DB_ITEMDESC desc;
	m_pDB->GetRecordItemValue(CH_PATH2_ID, &desc);
	auto filename = desc.csVal;
	m_pDB->GetRecordItemValue(CH_FILESPK, &desc);
	filename = filename + _T('\\') + desc.csVal;
	cs_file_comment.Format(_T("%i%s%s"), m_pDB->m_mainTableSet.m_ID, (LPCTSTR)separator, (LPCTSTR)filename);

	// source data file items
	if (options_viewspikes->bacqdate) // source data time and date
	{
		cs_dummy = separator + m_pDB->m_mainTableSet.m_acq_date.Format(VAR_DATEVALUEONLY);
		cs_file_comment += cs_dummy;
		cs_dummy = separator + m_pDB->m_mainTableSet.m_acq_date.Format(VAR_TIMEVALUEONLY);
		cs_file_comment += cs_dummy;
	}
	// source data comments
	if (options_viewspikes->bacqcomments)
	{
		m_pDB->GetRecordItemValue(CH_EXPT_ID, &desc);
		cs_file_comment += separator + desc.csVal;

		m_pDB->GetRecordItemValue(CH_IDINSECT, &desc);
		cs_dummy.Format(_T("%i"), desc.lVal);
		cs_file_comment += separator + cs_dummy;
		m_pDB->GetRecordItemValue(CH_IDSENSILLUM, &desc);
		cs_dummy.Format(_T("%i"), desc.lVal);
		cs_file_comment += separator + cs_dummy;

		m_pDB->GetRecordItemValue(CH_INSECT_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_STRAIN_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_SEX_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_LOCATION_ID, &desc);
		cs_file_comment += separator + desc.csVal;

		m_pDB->GetRecordItemValue(CH_OPERATOR_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_MORE, &desc);
		cs_file_comment += separator + desc.csVal;

		m_pDB->GetRecordItemValue(CH_STIM_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_CONC_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_REPEAT, &desc);
		cs_dummy.Format(_T("%i"), desc.lVal);
		cs_file_comment += separator + cs_dummy;

		m_pDB->GetRecordItemValue(CH_STIM2_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_CONC2_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_REPEAT2, &desc);
		cs_dummy.Format(_T("%i"), desc.lVal);
		cs_file_comment += separator + cs_dummy;

		m_pDB->GetRecordItemValue(CH_SENSILLUM_ID, &desc);
		cs_file_comment += separator + desc.csVal;
		m_pDB->GetRecordItemValue(CH_FLAG, &desc);
		cs_dummy.Format(_T("%i"), desc.lVal);
		cs_file_comment += separator + cs_dummy;
	}
	return cs_file_comment;
}

void CdbWaveDoc::Export_NumberOfSpikes(CSharedFile* pSF)
{
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	CString file_comment = _T("Analyze file: ");

	// save current selection and export header of the table
	const int i_old_index = GetDB_CurrentRecordPosition();
	const int n_files = GetDB_NRecords();
	if (nullptr == m_pSpk)
	{
		m_pSpk = new CSpikeDoc;
		ASSERT(m_pSpk != NULL);
		m_pSpk->SetSpkList_AsCurrent(GetCurrent_Spk_Document()->GetSpkList_CurrentIndex());
	}

	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_viewspikes = &(p_app->options_viewspikes);

	const auto i_old_list = m_pSpk->GetSpkList_CurrentIndex();
	m_pSpk->ExportTableTitle(pSF, options_viewspikes, n_files);
	m_pSpk->ExportTableColHeaders_db(pSF, options_viewspikes);
	m_pSpk->ExportTableColHeaders_data(pSF, options_viewspikes); // this is for the measure

	// single file export operation: EXTREMA, AMPLIT, SPIKEPOINTS
	m_bTranspose = FALSE;
	if (options_viewspikes->exportdatatype == EXPORT_INTERV)
		m_bTranspose = TRUE;

	if (options_viewspikes->exportdatatype == EXPORT_EXTREMA
		|| options_viewspikes->exportdatatype == EXPORT_AMPLIT
		|| options_viewspikes->exportdatatype == EXPORT_SPIKEPOINTS) /*|| parms->exportdatatype == EXPORT_INTERV*/
	{
		const CString cs_file_desc;
		m_pSpk->ExportSpkFileComment(pSF, options_viewspikes, 0, cs_file_desc);
		m_pSpk->ExportSpkAttributesOneFile(pSF, options_viewspikes);
	}
	// multiple file export operations: ISI, AUTOCORR, HISTAMPL, AVERAGE, INTERV, PSTH
	else
	{
		auto i_step = 0;
		CString cs_comment;
		auto n_bins = 0;
		double* p_doubl = nullptr;
		switch (options_viewspikes->exportdatatype)
		{
		case EXPORT_ISI: // ISI
		case EXPORT_AUTOCORR: // Autocorr
			n_bins = options_viewspikes->nbinsISI;
			break;
		case EXPORT_HISTAMPL: // spike amplitude histogram
			n_bins = options_viewspikes->histampl_nbins + 2;
			break;
		case EXPORT_AVERAGE: // assume that all spikes have the same length
			p_doubl = new double[m_pSpk->GetSpkList_Current()->GetSpikeLength() * 2 + 1 + 2];
			*p_doubl = m_pSpk->GetSpkList_Current()->GetSpikeLength();
			break;
		case EXPORT_INTERV: // feb 23, 2009
			break;
		case EXPORT_PSTH: // PSTH
		default:
			n_bins = options_viewspikes->nbins;
			break;
		}
		const auto p_hist0 = new long[n_bins + 2]; // create array (dimension = nbins) to store results
		*p_hist0 = n_bins;
		ASSERT(p_hist0 != NULL);
		CString cs_file_comment;
		cs_file_comment.Empty();

		// in each spike list, loop over spike classes as defined in the options
		auto iclass1 = options_viewspikes->classnb;
		auto iclass2 = options_viewspikes->classnb2;
		if (options_viewspikes->spikeclassoption == 0)
			iclass2 = iclass1;
		else
		{
			if (iclass2 < iclass1)
			{
				const auto iclassd = iclass1;
				iclass1 = iclass2;
				iclass2 = iclassd;
			}
		}

		// loop (1) from file 1 to file n ---------------------------------------------
		for (auto ifile1 = 0; ifile1 < n_files; ifile1++)
		{
			// check if user wants to stop
			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), ifile1 + 1, n_files);
			dlg.SetStatus(cs_comment);

			// open document
			SetDB_CurrentRecordPosition(ifile1);
			if (m_currentSpikefileName.IsEmpty())
				continue;
			// check if file is still present and open it
			CFileStatus status;
			if (!CFile::GetStatus(m_currentSpikefileName, status))
			{
				cs_file_comment = _T("\r\n") + Export_DatabaseData();
				cs_file_comment += _T("\tERROR: MISSING FILE"); // next line
				pSF->Write(cs_file_comment, cs_file_comment.GetLength() * sizeof(TCHAR));
				continue;
			}
			const auto flag = (OpenCurrentSpikeFile() != nullptr);
			ASSERT(flag);

			// loop over the spike lists stored in that file
			auto ichan1 = 0;
			auto ichan2 = m_pSpk->GetSpkList_Size();
			if (!options_viewspikes->ballChannels)
			{
				ichan1 = i_old_list;
				ichan2 = ichan1 + 1;
			}

			//----------------------------------------------------------
			for (auto ispikelist = ichan1; ispikelist < ichan2; ispikelist++)
			{
				const auto p_spike_list = m_pSpk->SetSpkList_AsCurrent(ispikelist);
				options_viewspikes->ichan = ispikelist;
				for (auto kclass = iclass1; kclass <= iclass2; kclass++)
				{
					Export_SpkDescriptors(pSF, p_spike_list, kclass);
					// export data
					switch (options_viewspikes->exportdatatype)
					{
					case EXPORT_HISTAMPL: // spike amplitude histogram
						m_pSpk->export_spk_amplitude_histogram(pSF, options_viewspikes, p_hist0, ispikelist, kclass);
						break;
					case EXPORT_LATENCY: // occurence time of the first 10 spikes
						m_pSpk->export_spk_latencies(pSF, options_viewspikes, 10, ispikelist, kclass);
						break;
					case EXPORT_INTERV: // feb 23, 2009 - occurence time of all spikes
						m_pSpk->export_spk_latencies(pSF, options_viewspikes, -1, ispikelist, kclass);
						break;
					case EXPORT_AVERAGE: // assume that all spikes have the same length
						m_pSpk->export_spk_average_wave(pSF, options_viewspikes, p_doubl, ispikelist, kclass);
						break;
					case EXPORT_PSTH: // PSTH
					case EXPORT_ISI: // ISI
					case EXPORT_AUTOCORR: // Autocorr
						m_pSpk->export_spk_psth(pSF, options_viewspikes, p_hist0, ispikelist, kclass);
						break;
					default:
						ATLTRACE2(_T("option selected not implemented: %i /n"), options_viewspikes->exportdatatype);
						break;
					}
				} // end of for: kclass
			} // end of for: spklist

			// update progress bar
			if (MulDiv(ifile1, 100, n_files) > i_step)
			{
				dlg.StepIt();
				i_step = MulDiv(ifile1, 100, n_files);
			}
		}

		delete[] p_hist0;
		delete[] p_doubl;
	}

	// transpose file
	if (m_bTranspose)
		transpose_file_for_excel(pSF);

	// restore initial file name and channel
	SetDB_CurrentRecordPosition(i_old_index);
	if (OpenCurrentSpikeFile() != nullptr)
		m_pSpk->SetSpkList_AsCurrent(i_old_list);
	UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
}

BOOL CdbWaveDoc::transpose_file_for_excel(CSharedFile* pSF)
{
	// create dummy file on disk that duplicates the memory file
	CStdioFile data_dest; // destination file object
	const CString dummy_file1 = _T("dummyFile.txt"); // to read the clipboard
	const CString dummy_file2 = _T("dummyFile_transposed.txt"); // to store the transposed data

	CFileException fe; // trap exceptions
	if (!data_dest.Open(dummy_file1, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText,
	                    &fe))
	{
		data_dest.Abort(); // file not found
		return FALSE; // and return
	}
	auto ul_lenchar_total = pSF->GetLength();
	pSF->SeekToBegin();
	auto ul_lenchar = ul_lenchar_total;
	char buffer[2048];
	while (ul_lenchar > 0)
	{
		auto ul_len = ul_lenchar;
		if (ul_len > 1024)
			ul_len = 1024;
		pSF->Read(&buffer, static_cast<UINT>(ul_len));
		data_dest.Write(&buffer, static_cast<UINT>(ul_len));
		ul_lenchar = ul_lenchar - ul_len;
	}

	// create a transposed file
	CStdioFile data_transposed; // destination file object
	if (!data_transposed.Open(dummy_file2,
	                          CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText, &fe))
	{
		data_transposed.Abort(); // file not found
		return FALSE; // and return
	}

	// files are created, now read dataDest line by line
	data_dest.SeekToBegin();
	CString cs_read;
	// first find where the header is and copy the first lines of the table to the transposed file
	auto b_header_found = FALSE;
	ULONGLONG ul_position_header = 0;
	while (data_dest.ReadString(cs_read) && !b_header_found)
	{
		const auto ul_position_first = data_dest.GetPosition();
		if ((b_header_found = (cs_read.Find(_T("ID")) >= 0)))
			break;

		data_transposed.WriteString(cs_read);
		ul_position_header = ul_position_first;
	}
	// now: ulPositionHeader saves the file position of the table header
	//		dataTransposed is ready to get the first line of data

	// Copy data up to the header to dataTransposed
	// --------------------------------------------------------- first line of the table
	// first pass: browse through all lines from dataDest
	auto nrecords = -1;
	data_dest.Seek(ul_position_header, CFile::begin);
	CString cs_transposed;
	const auto i_start = 0;
	const auto i_first = 0;
	const char c_sep = _T('\t');

	while (data_dest.ReadString(cs_read))
	{
		const auto ifound = cs_read.Find(_T('\t'), i_start);
		cs_transposed += cs_read.Mid(i_first, ifound - i_start) + c_sep;
		nrecords++;
	}
	cs_transposed += _T('\n');
	data_transposed.WriteString(cs_transposed);

	// ---------------------------------------------------------
	// now we now how many lines are in dataDest and we will scan line by line until all lines are exhausted
	auto icol_dest = 0;
	auto n_found = nrecords;

	while (n_found > 0)
	{
		data_dest.Seek(ul_position_header, CFile::begin);
		cs_transposed.Empty();
		n_found = nrecords + 1;
		icol_dest++;

		for (auto i = 0; i <= nrecords; i++)
		{
			data_dest.ReadString(cs_read);

			// find tab at icol_dest position
			auto i_first_local = 0;
			for (auto j = 0; j < icol_dest; j++)
			{
				i_first_local = cs_read.Find(c_sep, i_first_local + 1);
				if (i_first_local < 0) // not found: exit loop and go to next line
					break;
			}

			// write dummy space if no tab found
			if (i_first_local < 0)
			{
				cs_transposed += c_sep;
				n_found--;
				continue;
			}

			auto i_last = cs_read.Find(c_sep, i_first_local + 1);
			// extract value
			if (i_last < 0)
			{
				i_last = cs_read.GetLength();
				if (cs_read.Find(_T('\n')) > 0)
					i_last--;
				if (cs_read.Find(_T('\r')) > 0)
					i_last--;
			}

			auto cs_temp = cs_read.Mid(i_first_local, i_last - i_first_local);
			const auto itest = cs_temp.Find(c_sep);
			if (itest < 1)
				cs_temp = cs_read.Mid(i_first_local + 1, i_last - i_first_local - 1);
			if (cs_temp.Find(c_sep) < 0)
				cs_temp += c_sep;
			cs_transposed += cs_temp;
		}
		cs_transposed += _T('\n');
		data_transposed.WriteString(cs_transposed);
	}

	// write end-of-the-file character
	buffer[0] = 0;
	data_transposed.Write(&buffer, 1);

	// now the transposed file has all data in the proper format
	// and we can copy it back to the clipboard
	ul_lenchar_total = data_transposed.GetLength();
	data_transposed.SeekToBegin();
	ul_lenchar = ul_lenchar_total;

	// erase old clipboard data and create a new block of data
	pSF->SetLength(ul_lenchar_total);
	pSF->SeekToBegin();

	while (ul_lenchar > 0)
	{
		auto ul_len = ul_lenchar;
		if (ul_len > 1024)
			ul_len = 1024;
		data_transposed.Read(&buffer, static_cast<UINT>(ul_len));
		pSF->Write(&buffer, static_cast<UINT>(ul_len));
		ul_lenchar -= ul_len;
	}

	// now delete the 2 dummy files
	data_transposed.Close();
	data_dest.Close();
	CFile::Remove(dummy_file1);
	CFile::Remove(dummy_file2);

	return TRUE;
}

void CdbWaveDoc::remove_row_at(CStringArray& filenames, int iRow, int nColumns, boolean bHeader)
{
	const int index = index_2d_array(iRow, nColumns, bHeader);
	for (int i = nColumns - 1; i >= 0; i--)
	{
		filenames.RemoveAt(index + i);
	}
}

int CdbWaveDoc::check_files_can_be_opened(CStringArray& filenames, CSharedFile* psf, int nColumns, boolean bHeader)
{
	// prepare progress dialog box
	auto nfilesok = 0;
	const int nfiles = get_size_2d_array(filenames, nColumns, bHeader);
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	dlg.SetRange(0, nfiles);

	for (auto irec = nfiles - 1; irec >= 0; irec--)
	{
		// check if filename not already defined
		int index = index_2d_array(irec, nColumns, bHeader);
		auto cs_filename = CString(filenames[index]);
		cs_filename.MakeLower();

		if (lstrlen(cs_filename) >= _MAX_PATH)
			continue;

		CString cscomment;
		cscomment.Format(_T("Checking file type and status on disk [%i / %i] %s"), nfiles - irec, nfiles, (LPCTSTR)cs_filename);
		dlg.SetStatus(cscomment);
		dlg.StepIt();

		// check if file of correct type
		CString cs_ext;
		const auto i_ext = cs_filename.ReverseFind('.');
		cs_ext = cs_filename.Right(cs_filename.GetLength() - i_ext - 1);
		BOOL b_dat = IsExtensionRecognizedAsDataFile(cs_ext);

		if ((!b_dat && cs_ext.Compare(_T("spk")) != 0) || (cs_filename.Find(_T("tmp.dat")) >= 0))
		{
			psf = file_discarded_message(psf, cs_filename, irec);
			remove_row_at(filenames, irec, nColumns, bHeader);
			continue;
		}

		// open document and read data
		CFileStatus status;
		const BOOL bflag = CFile::GetStatus(cs_filename, status);

		// GOTO next file if it not possible to open the file either as a spk or a dat file
		if (!bflag)
		{
			psf = file_discarded_message(psf, cs_filename, irec);
			remove_row_at(filenames, irec, nColumns, bHeader);
			continue;
		}
		nfilesok++;
	}
	return nfilesok;
}

CSharedFile* CdbWaveDoc::file_discarded_message(CSharedFile* pSF, CString cs_filename, int irec)
{
	CString cs_dummy;
	cs_dummy.Format(_T("file discarded=%i:\t%s\t error reading file \r\n"), irec, (LPCTSTR)cs_filename);
	pSF->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	return pSF;
}

void CdbWaveDoc::Delete_ErasedFiles()
{
	// close current data file (no need to do that for spk file)(so far)
	if (m_pDat != nullptr)
		m_pDat->AcqCloseFile();

	const auto nfiles = m_csfiles_to_delete.GetSize() - 1;

	for (auto i = nfiles; i >= 0; i--)
	{
		// get data file name and rename file
		auto file_name = m_csfiles_to_delete.GetAt(i);
		auto file_new_name = file_name + _T("del");

		// check if same file already exists: if yes, search if up to 10 clones exist, otherwise, destroy the last clone
		CFileStatus status;
		if (CFile::GetStatus(file_new_name, status))
		{
			auto flag = TRUE;
			CString prompt;
			for (auto j = 0; j < 10; j++)
			{
				CString cs;
				prompt.Format(_T("%s%i"), (LPCTSTR)file_new_name, j);
				flag = CFile::GetStatus(prompt, status);
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
				file_new_name = prompt;
		}

		// rename file
		try
		{
			CFile::Rename(file_name, file_new_name);
		}

		catch (CFileException* p_ex)
		{
#ifdef _DEBUG
			afxDump << _T("File ") << file_name << _T(" not found, cause = ")
				<< p_ex->m_cause << _T("\n");
			switch (p_ex->m_cause)
			{
			case CFileException::fileNotFound:
				afxDump << _T("The file could not be located.");
				break;
			case CFileException::badPath:
				afxDump << _T("All or part of the path is invalid. ");
				break;
			case CFileException::tooManyOpenFiles:
				afxDump << _T("The permitted number of open files was exceeded. ");
				break;
			case CFileException::accessDenied:
				afxDump << _T("The file could not be accessed. ");
				break;
			case CFileException::invalidFile:
				afxDump << _T("There was an attempt to use an invalid file handle.  ");
				break;
			case CFileException::removeCurrentDir:
				afxDump << _T("The current working directory cannot be removed.  ");
				break;
			case CFileException::directoryFull:
				afxDump << _T("There are no more directory entries.  ");
				break;
			case CFileException::badSeek:
				afxDump << _T("There was an error trying to set the file pointer.  ");
				break;
			case CFileException::hardIO:
				afxDump << _T("There was a hardware error.  ");
				break;
			case CFileException::sharingViolation:
				afxDump << _T("SHARE.EXE was not loaded, or a shared region was locked.  ");
				break;
			case CFileException::lockViolation:
				afxDump << _T("There was an attempt to lock a region that was already locked.  ");
				break;
			case CFileException::diskFull:
				afxDump << _T("The disk is full.  ");
				break;
			case CFileException::endOfFile:
				afxDump << _T("The end of file was reached.  ");
				break;
			default: break;
			}
			afxDump << _T("\n");
#endif
			p_ex->Delete();
		}
		m_csfiles_to_delete.RemoveAt(i);
	}
}

void CdbWaveDoc::DBDeleteCurrentRecord()
{
	// save data & spike file names, together with their full access path
	m_pDB->m_pathSet.SeekID(m_pDB->m_mainTableSet.m_path_ID);
	CString cs;
	if (!m_pDB->m_mainTableSet.m_Filedat.IsEmpty())
	{
		cs = m_pDB->m_pathSet.m_cs + _T('\\') + m_pDB->m_mainTableSet.m_Filedat;
		m_csfiles_to_delete.Add(cs);
	}
	if (!m_pDB->m_mainTableSet.m_Filespk.IsEmpty())
	{
		cs = m_pDB->m_pathSet.m_cs + _T('\\') + m_pDB->m_mainTableSet.m_Filespk;
		m_csfiles_to_delete.Add(cs);
	}
	m_pDB->m_mainTableSet.Delete();
	m_pDB->m_mainTableSet.MoveNext();
}

void CdbWaveDoc::Remove_DuplicateFiles()
{
	// loop through all files
	// discard if data acquisition the same day, same time

	DlgProgress dlg;
	dlg.Create();
	dlg.SetWindowText(_T("Scan database to discard duplicate (or missing) data files..."));

	const int n_files = GetDB_NRecords();
	dlg.SetRange(0, n_files);
	dlg.SetStep(1);

	CStringArray deleted_dat_names;
	CStringArray deleted_spk_names;
	CStringArray original_names;
	CStringArray all_names;
	CArray<CTime, CTime> o_time_array;
	deleted_dat_names.SetSize(n_files);
	deleted_spk_names.SetSize(n_files);
	original_names.SetSize(n_files);
	all_names.SetSize(n_files);
	o_time_array.SetSize(n_files);

	m_pDB->UpdateTables();
	m_pDB->m_mainTableSet.Close(); 
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenTable, nullptr, 0))
		return;

	// scan database to collect all file names
	dlg.SetStatus(_T("Collect names from database and read data acquisition times..."));

	auto nb_records_to_suppress = 0;

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		auto index_file = 0;
		auto index_valid_records = 0;
		while (!m_pDB->m_mainTableSet.IsEOF())
		{
			// load time for this pRecord and save bookmark
			const auto bookmark = m_pDB->m_mainTableSet.GetBookmark();
			m_pDB->GetFilenamesFromCurrentRecord();
			m_pDB->m_mainTableSet.SetBookmark(bookmark);
			all_names.SetAt(index_valid_records, m_currentDatafileName);

			// process file
			CString comment;
			comment.Format(_T("Processing file [%i / %i] %s"), index_valid_records, n_files, (LPCTSTR)m_currentDatafileName);
			dlg.SetStatus(comment);
			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;

			// load file and read date and time of data acquisition
			CTime o_time_file_current = 0;
			auto b_ok = (OpenCurrentDataFile() != nullptr);
			auto i_duplicate_file = -1;
			if (b_ok)
			{
				const auto wave_format = m_pDat->GetpWaveFormat();
				o_time_file_current = wave_format->acqtime;
				// loop to find if current file has a duplicate in the list of previous files stored in the array
				for (auto i = 0; i < index_valid_records; i++)
				{
					auto o_time_file_i = o_time_array.GetAt(i);
					ASSERT(o_time_file_i != 0);
					if (o_time_file_current == o_time_file_i)
					{
						i_duplicate_file = i;
						b_ok = FALSE; 
						break;
					}
				}
			}

			// update parameters array
			if (!b_ok) // suppress pRecord if data file not found or if duplicate
			{
				if (i_duplicate_file >= 0)
					original_names.SetAt(nb_records_to_suppress, all_names.GetAt(i_duplicate_file));
				else
					original_names.SetAt(nb_records_to_suppress, _T("(none)"));
				deleted_dat_names.SetAt(nb_records_to_suppress, m_currentDatafileName);
				deleted_spk_names.SetAt(nb_records_to_suppress, m_currentSpikefileName);
				m_pDB->m_mainTableSet.Delete();
				nb_records_to_suppress++;
			}
			else // save time of this valid pRecord into array
			{
				o_time_array.SetAtGrow(index_valid_records, o_time_file_current);
				index_valid_records++;
			}

			// move to next pRecord & update dialog box
			m_pDB->m_mainTableSet.MoveNext();
			index_file++;
			dlg.StepIt();
		}
		m_pDB->m_mainTableSet.MoveFirst();
	}

	// re-open dynaset
	m_pDB->m_mainTableSet.Close();
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
		return;
	m_pDB->m_mainTableSet.Requery();

	// trace
	if (nb_records_to_suppress == 0)
		AfxMessageBox(_T("No duplicate files found"));
	else
	{
		CString cs;
		cs.Format(_T("Changes were made to the data base:\nNumber of records removed= %i\n"), nb_records_to_suppress);
		AfxMessageBox(cs);
		//---------------------------------------------
		deleted_dat_names.SetSize(nb_records_to_suppress);
		original_names.SetSize(nb_records_to_suppress);
		deleted_spk_names.SetSize(nb_records_to_suppress);

		CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		CString cs_dummy = _T("Removed:\tOriginal:\n");
		sf.Write(cs_dummy, cs_dummy.GetLength());
		for (auto i = 0; i < nb_records_to_suppress; i++)
		{
			cs_dummy.Format(_T("%s\t%s\n"), (LPCTSTR)deleted_dat_names.GetAt(i), (LPCTSTR)original_names.GetAt(i));
			sf.Write(cs_dummy, cs_dummy.GetLength());
		}
		const auto dw_len = static_cast<DWORD>(sf.GetLength());
		auto h_mem = sf.Detach();
		if (!h_mem)
			return;
		h_mem = GlobalReAlloc(h_mem, dw_len, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		if (!h_mem)
			return;

		auto p_source = new COleDataSource();
		p_source->CacheGlobalData(CF_TEXT, h_mem); 
		p_source->SetClipboard();
		auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		CMultiDocTemplate* p_templ = p_app->m_pNoteViewTemplate;
		const auto pdb_doc_export = p_templ->OpenDocumentFile(nullptr);
		auto pos = pdb_doc_export->GetFirstViewPosition();
		auto* p_view = static_cast<CViewNoteDoc*>(pdb_doc_export->GetNextView(pos));
		auto& p_edit = p_view->GetRichEditCtrl();
		p_edit.Paste();

		//---------------------------------------------
		const auto id_response = AfxMessageBox(_T("Do you want to erase these files from the disk?"), MB_YESNO);
		if (id_response == IDYES)
		{
			for (auto i = 0; i < nb_records_to_suppress; i++)
			{
				remove_file_from_disk(deleted_dat_names.GetAt(i));
				remove_file_from_disk(deleted_spk_names.GetAt(i));
			}
		}
	}
}

void CdbWaveDoc::remove_file_from_disk(const CString file_name)
{
	if (!file_name.IsEmpty())
	{
		try
		{
			CFile::Remove(file_name);
		}
		catch (CFileException* p_ex)
		{
			const auto cs_local = _T("File") + file_name + _T(" %20s cannot be removed\n");
			AfxMessageBox(cs_local, MB_OK);
			p_ex->Delete();
		}
	}
}

void CdbWaveDoc::Remove_MissingFiles()
{
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	CString csfilecomment = _T("Checking database consistency - find and remove missing files: ");
	CString cs_dummy;

	//int indexcurrent = DBGetCurrentRecordPosition();
	const int nfiles = GetDB_NRecords();
	auto ifile = 0;

	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();

	m_pDB->m_mainTableSet.Close(); // close dynaset and open as datatable
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenTable, nullptr, 0))
		return;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	m_pDB->m_mainTableSet.MoveFirst();

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		CFileStatus status;
		CString cs_comment;
		auto i_step = 0;
		while (!m_pDB->m_mainTableSet.IsEOF())
		{
			cs_dummy.Format(_T("%i\t"), ifile);
			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
		
			m_pDB->GetFilenamesFromCurrentRecord();
			cs_comment.Format(_T("Processing file [%i / %i] %s"), ifile, nfiles, (LPCTSTR)m_currentDatafileName);
			dlg.SetStatus(cs_comment);
			
			const auto b_dat_file = CFile::GetStatus(m_currentDatafileName, status); 
			const auto b_spk_file = CFile::GetStatus(m_currentSpikefileName, status); 
			if (!b_dat_file && !b_spk_file) 
				m_pDB->m_mainTableSet.Delete();

			m_pDB->m_mainTableSet.MoveNext();
		
			ifile++;
			if (MulDiv(ifile, 100, nfiles) > i_step)
			{
				dlg.StepIt();
				i_step = MulDiv(ifile, 100, nfiles);
			}
		}
		m_pDB->m_mainTableSet.MoveFirst();
	}

	// open dynaset
	m_pDB->m_mainTableSet.Close();
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
		return;
	m_pDB->m_mainTableSet.Requery();
}

void CdbWaveDoc::Remove_FalseSpkFiles()
{
	DlgProgress dlg;
	dlg.Create();
	auto istep = 0;
	dlg.SetStep(1);
	CString cscomment;
	CString csfilecomment = _T("Checking database consistency - find and remove missing files: ");
	CString cs_dummy;

	const int nfiles = GetDB_NRecords();
	auto ifile = 0;
	auto i_files_removed = 0;
	auto i_errors_corrected = 0;

	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	m_pDB->UpdateTables();

	m_pDB->m_mainTableSet.Close(); // close dynaset and open as datatable
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenTable, nullptr, 0))
		return;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	m_pDB->m_mainTableSet.MoveFirst();
	CFileStatus status;

	if (!m_pDB->m_mainTableSet.IsBOF())
	{
		while (!m_pDB->m_mainTableSet.IsEOF())
		{
			cs_dummy.Format(_T("%i\t"), ifile);
			// check if user wants to stop
			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			m_pDB->GetFilenamesFromCurrentRecord();
			cscomment.Format(_T("Processing file [%i / %i] %s"), ifile, nfiles, (LPCTSTR)m_currentDatafileName);
			dlg.SetStatus(cscomment);

			// check if spike file is present
			auto cs_spikefile_name = m_currentDatafileName.Left(m_currentDatafileName.ReverseFind('.') + 1) + _T("spk");
			const auto b_spk_file = CFile::GetStatus(cs_spikefile_name, status);

			// if spk file not found in the same directory, remove pRecord
			if (!b_spk_file)
			{
				m_pDB->m_mainTableSet.SetFieldNull(&(m_pDB->m_mainTableSet.m_Filespk), TRUE);
				i_files_removed++;
			}
			else
			{
				// compare the 2 file names - if different, copy dataname & directory index into spike name & directory fields
				if (cs_spikefile_name.CompareNoCase(m_currentSpikefileName) != 0)
				{
					cs_spikefile_name = m_pDB->m_mainTableSet.m_Filedat.Left(
						m_pDB->m_mainTableSet.m_Filedat.ReverseFind('.') + 1) + _T("spk");
					m_pDB->m_mainTableSet.m_Filespk = cs_spikefile_name;
					i_errors_corrected++;
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
	if (!m_pDB->m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
		return;
	m_pDB->m_mainTableSet.Requery();

	// trace
	if (i_errors_corrected == 0 && i_files_removed == 0)
		AfxMessageBox(_T("No errors found"));
	else
	{
		CString cs;
		cs.Format(
			_T("Changes were made to the data base:\nerrors corrected: %i, files removed: %i\n"), i_errors_corrected,
			i_files_removed);
		AfxMessageBox(cs);
	}
}

void CdbWaveDoc::Export_DatafilesAsTXTfiles()
{
	// save current index position - restore on exit
	const auto currentfile = GetDB_CurrentRecordPosition();

	// make sure there are objects to read / write data and spike files
	if (m_pDat == nullptr) // data doc
	{
		m_pDat = new AcqDataDoc;
		ASSERT(m_pDat != NULL);
	}

	// prepare clipboard to receive names of files exported
	auto psf = dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_psf;
	SAFE_DELETE(psf)

	dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_psf = nullptr;
	psf = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// prepare progress dialog box
	DlgProgress dlg;
	dlg.Create();
	CString cscomment;
	const auto nfiles = GetDB_NRecords();
	auto ifile = 1;
	dlg.SetRange(0, nfiles);
	CString cs_filenames;

	// got to the first pRecord
	m_pDB->m_mainTableSet.MoveFirst();
	dlg.SetPos(0);

	while (!m_pDB->m_mainTableSet.IsEOF())
	{
		m_pDB->GetFilenamesFromCurrentRecord();
		// check if user wants to stop
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cscomment.Format(_T("Processing file [%i / %i]"), ifile, nfiles);
		dlg.SetStatus(cscomment);

		// process data file
		if (!m_currentDatafileName.IsEmpty())
		{
			// open file
			const auto pDat = OpenCurrentDataFile();
			ASSERT(pDat != nullptr);

			// create text file on disk with the same name as the data file_with dat
			CStdioFile data_dest; // destination file object
			auto cstxt_file = m_currentDatafileName + _T("_.txt");
			CFileException fe; // trap exceptions
			if (!data_dest.Open(
				cstxt_file, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText, &fe))
			{
				data_dest.Abort(); // file not found
				continue;
			}

			// export data
			m_pDat->ExportDataFile_to_TXTFile(&data_dest); // get infos
			data_dest.Close();
			cstxt_file += _T("\n");
			psf->Write(cstxt_file, cstxt_file.GetLength() * sizeof(TCHAR));
		}

		// move to next pRecord
		m_pDB->m_mainTableSet.MoveNext();
		ifile++;
		dlg.SetPos(ifile);
	}
	m_pDB->m_mainTableSet.MoveFirst();
	m_pDB->GetFilenamesFromCurrentRecord();

	// restore current data position
	SetDB_CurrentRecordPosition(currentfile);
	dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_psf = psf;
}
