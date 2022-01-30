#include "StdAfx.h"
#include "dbWave.h"

#include <winspool.h>
#include "Fileversion.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "CViewData.h"
#include "CViewSpikes.h"
#include "CViewdbWave.h"
#include "CViewADcontinuous.h"
#include "CViewNotedoc.h"
#include "dbWaveDoc.h"
#include "NoteDoc.h"
#include "WaveBuf.h"
#include "Splash.h"
#include "DlgAbout.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CdbWaveApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CdbWaveApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, &CdbWaveApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CdbWaveApp::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

CdbWaveApp theApp;
static TCHAR sz_vds[] = _T("Default parameters");
static TCHAR sz_file_entry[] = _T("File%d");

void DisplayDaoException(CDaoException* e, int iID = 0)
{
	CString str_msg;
	if (e->m_pErrorInfo != nullptr)
	{
		str_msg.Format(
			_T("%s   (%d) at line ID %i\n\n")
			_T("Would you like to see help?"),
			(LPCTSTR)e->m_pErrorInfo->m_strDescription,
			e->m_pErrorInfo->m_lErrorCode, iID);

		if (AfxMessageBox(str_msg, MB_YESNO) == IDYES)
		{
			WinHelp(GetDesktopWindow(),
			        e->m_pErrorInfo->m_strHelpFile,
			        HELP_CONTEXT,
			        e->m_pErrorInfo->m_lHelpContext);
		}
	}
	else
	{
		str_msg.Format(
			_T("ERROR:CDaoException\n\n")
			_T("SCODE_CODE      =%d\n")
			_T("SCODE_FACILITY  =%d\n")
			_T("SCODE_SEVERITY  =%d\n")
			_T("ResultFromScode =%d\n"),
			SCODE_CODE(e->m_scode),
			SCODE_FACILITY(e->m_scode),
			SCODE_SEVERITY(e->m_scode),
			ResultFromScode(e->m_scode));
		AfxMessageBox(str_msg);
	}
}

CdbWaveApp::CdbWaveApp() : CWinAppEx(TRUE /* m_bResourceSmartUpdate */)
{
	EnableHtmlHelp();
	m_bHiColorIcons = TRUE;

	// support Restart Manager: yes
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("FMP.dbWave2.VS2019.Jan-2019"));

	m_pviewdataMemFile = nullptr;
	m_pviewspikesMemFile = nullptr;
	m_psort1spikesMemFile = nullptr;
	m_pdbWaveViewTemplate = nullptr;
	m_bADcardFound = TRUE;
}

BOOL CdbWaveApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction();

	//AfxInitRichEdit2();

	// CG: The following block was added by the Splash Screen component.
	{
		CCommandLineInfo cmd_info;
		ParseCommandLine(cmd_info);
		CSplashWnd::EnableSplashScreen(cmd_info.m_bShowSplash);
		CSplashWnd::ShowSplashScreen();
	}

	// works with debug version, fails with release statically linked
	//AfxGetModuleState()->m_dwVersion = 0x0601; // enable Access2000

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("FMP\\dbWave2"));
	SetRegistryBase(_T("Settings"));
	Defaultparameters(TRUE);
	LoadStdProfileSettings(4); // Load standard INI file options (including MRU)

	InitContextMenuManager();
	InitShellManager();
	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo tt_params;
	tt_params.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &tt_params);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pdbWaveViewTemplate = new CdbMultiDocTemplate(IDR_DBWAVETYPE,
	                                                RUNTIME_CLASS(CdbWaveDoc),
	                                                RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                                RUNTIME_CLASS(CViewdbWave));
	m_pdbWaveViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pdbWaveViewTemplate);
	m_hDBView = m_pdbWaveViewTemplate->m_hMenuShared;

	// ---------------------------------------------
	m_pdataViewTemplate = new CdbMultiDocTemplate(IDR_DBDATATYPE,
	                                              RUNTIME_CLASS(CdbWaveDoc),
	                                              RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                              RUNTIME_CLASS(CViewData));
	m_pdataViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pdataViewTemplate);
	m_hDataView = m_pdataViewTemplate->m_hMenuShared;

	// continuous A/D view with data translation card
	m_pADViewTemplate = new CdbMultiDocTemplate(IDR_DBDATATYPE,
	                                            RUNTIME_CLASS(CdbWaveDoc),
	                                            RUNTIME_CLASS(CChildFrame), // multifile MDI child frame
	                                            RUNTIME_CLASS(CADContView)); // AD view
	ASSERT(m_pADViewTemplate != NULL);
	m_pADViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pADViewTemplate);
	m_hAcqView = m_pADViewTemplate->m_hMenuShared;

	m_pspikeViewTemplate = new CdbMultiDocTemplate(IDR_DBSPIKETYPE,
	                                               RUNTIME_CLASS(CdbWaveDoc),
	                                               RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                               RUNTIME_CLASS(CViewSpikes));
	m_pspikeViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pspikeViewTemplate);
	m_hSpikeView = m_pspikeViewTemplate->m_hMenuShared;

	m_pNoteViewTemplate = new CdbMultiDocTemplate(IDR_PROJECTTYPE,
	                                              RUNTIME_CLASS(CNoteDoc),
	                                              RUNTIME_CLASS(CMDIChildWndEx), // standard MDI child frame
	                                              RUNTIME_CLASS(CViewNoteDoc));
	m_pNoteViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pNoteViewTemplate);

	// create main MDI Frame window
	auto p_main_frame = new CMainFrame;
	if (!p_main_frame || !p_main_frame->LoadFrame(IDR_MAINFRAME))
	{
		delete p_main_frame;
		return FALSE;
	}
	m_pMainWnd = p_main_frame;

	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmd_info;
	ParseCommandLine(cmd_info);
	if (cmd_info.m_nShellCommand == CCommandLineInfo::FileNew) // actually none
		cmd_info.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Enable DDE Execute open - this allow to open data files by double clicking on it
	EnableShellOpen();
	RegisterShellFileTypes(TRUE); // this works only if user has administrative rights

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmd_info))
		return FALSE;

	// The main window has been initialized, so show and update it.
	m_nCmdShow = SW_SHOWNORMAL; //SW_SHOWMAXIMIZED;  // change into THIS to start dbwave maximized
	p_main_frame->ShowWindow(m_nCmdShow);
	p_main_frame->UpdateWindow();

	return TRUE;
}

int CdbWaveApp::ExitInstance()
{
	Defaultparameters(FALSE);
	AfxOleTerm(FALSE);

	SAFE_DELETE(m_pviewdataMemFile);
	SAFE_DELETE(m_psf);

	if (viewspikesmemfile_ptr_array.GetSize() != NULL)
	{
		for (auto i = 0; i < viewspikesmemfile_ptr_array.GetSize(); i++)
			delete viewspikesmemfile_ptr_array.GetAt(i);
	}

	SAFE_DELETE(m_pviewspikesMemFile);
	SAFE_DELETE(m_psort1spikesMemFile);

	// erase temporary "mdb" files
	const auto i0 = m_tempMDBfiles.GetCount() - 1;
	for (auto i = i0; i >= 0; i--)
	{
		auto cs = m_tempMDBfiles.GetAt(i);
		CFile::Remove(cs);
	}

	return CWinAppEx::ExitInstance();
}

void CdbWaveApp::OnAppAbout()
{
	DlgAbout about_dlg;
	about_dlg.DoModal();
}

BOOL DlgAbout::OnInitDialog()
{
	CDialog::OnInitDialog();

	CDaoWorkspace ws;
	try
	{
		ws.Create(_T("VersionWorkspace"), _T("Admin"), _T(""));
		GetDlgItem(IDC_VERSION)->SetWindowText(ws.GetVersion());
	}
	catch (CDaoException* e)
	{
		e->ReportError();
		e->Delete();
	}

	CFileVersion c_fv;
	const CString cs_app_name = _T("dbwave2.exe");
	c_fv.Open(cs_app_name);
	GetDlgItem(IDC_DBWAVEVERSION)->SetWindowText(c_fv.GetProductVersion());
	GetDlgItem(IDC_STATIC7)->SetWindowText(c_fv.GetLegalCopyright());

	return TRUE;
}

void CdbWaveApp::PreLoadState()
{
	CString str_name;
	auto b_name_valid = str_name.LoadString(IDS_EDIT_MENU);
	ASSERT(b_name_valid);
	GetContextMenuManager()->AddMenu(str_name, IDR_POPUP_EDIT);
	b_name_valid = str_name.LoadString(IDS_EXPLORER);
	ASSERT(b_name_valid);
	GetContextMenuManager()->AddMenu(str_name, IDR_POPUP_EXPLORER);
}

void CdbWaveApp::LoadCustomState()
{
}

void CdbWaveApp::SaveCustomState()
{
}

BOOL CdbWaveApp::PreTranslateMessage(MSG* pMsg)
{
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;
	return CWinAppEx::PreTranslateMessage(pMsg);
}

void CdbWaveApp::Defaultparameters(BOOL b_read)
{
	TCHAR sz_path[MAX_PATH];
	CString cspath;
	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		sz_path)))
	{
		// check that directory is present - otherwise create...
		cspath = sz_path;
		cspath += _T("\\My dbWave Files");
		// test if directory already present
		const auto b_exist = PathFileExists(cspath);
		if (!b_exist)
		{
			if (!CreateDirectory(cspath, nullptr))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}

	TCHAR szEntry[MAX_PATH];
	unsigned long len = MAX_PATH;
	GetUserName(&szEntry[0], &len);
	CString cs_ext(szEntry);
	if (cs_ext.IsEmpty())
		cs_ext = _T("dbWave2");
	cs_ext += _T(".prefs2");
	const auto cs_default_parmfile = cspath + "\\" + cs_ext;

	// read data and copy into vdP
	if (b_read)
	{
		// get list of parameter files (others)
		m_csParmFiles.RemoveAll();
		auto i = 0;
		while (i >= 0)
		{
			wsprintf(&szEntry[0], sz_file_entry, i + 1);
			auto dummy = GetProfileString(sz_vds, &szEntry[0]);
			if (dummy.IsEmpty())
				break;
			if (dummy.Find(cs_ext) > 0)
				m_csParmFiles.Add(dummy);
			i++;
		}
		// get default parameter file and load data
		if (m_csParmFiles.GetSize() <= 0)
			m_csParmFiles.Add(cs_default_parmfile);
		ParmFile(m_csParmFiles[0], b_read);
	}
	// Save informations
	else
	{
		// save default parameter file
		if (m_csParmFiles.GetSize() <= 0)
			m_csParmFiles.Add(cs_default_parmfile);
		if (!ParmFile(m_csParmFiles[0], b_read))
		{
			m_csParmFiles[0] = cs_default_parmfile;
			ParmFile(m_csParmFiles[0], b_read);
		}
		// save profile with locations of parmfiles
		for (auto i = 0; i < m_csParmFiles.GetSize(); i++)
		{
			wsprintf(&szEntry[0], sz_file_entry, i + 1);
			WriteProfileString(sz_vds, &szEntry[0], m_csParmFiles[i]);
		}
	}
}

BOOL CdbWaveApp::ParmFile(CString& csParmfile, BOOL b_read)
{
	CFile f; // file object
	CFileException fe; // trap exceptions
	auto bsuccess = TRUE;

	if (b_read) // read informations ...........................
	{
		if (f.Open(csParmfile, CFile::modeReadWrite | CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::load);
			WORD m;
			ar >> m; // nb items to load
			int n = m;
			ar >> m_comment; // comment
			n--;
			if (n > 0) stiD.Serialize(ar); // STIMDETECT
			n--;
			if (n > 0) spkDA.Serialize(ar); // SPKDETECTARRAY
			n--;
			if (n > 0) options_viewdata.Serialize(ar); // OPTIONS_VIEWDATA
			n--;
			if (n > 0) options_viewspikes.Serialize(ar); // OPTIONS_VIEWSPIKES
			n--;
			if (n > 0) spkC.Serialize(ar); // SPKCLASSIF
			n--;
			if (n > 0) options_viewdata_measure.Serialize(ar); // OPTIONS_VIEWDATAMEASURE
			n--;
			if (n > 0) options_import.Serialize(ar); // OPTIONS_IMPORT
			n--;
			if (n > 0) options_acqdata.Serialize(ar); // OPTIONS_ACQDATA
			n--;
			if (n > 0) options_outputdata.Serialize(ar); //OPTIONS_OUTPUTDATA

			ar.Close(); // close archive
			f.Close(); // close file
		}
	}
	else // Save informations .............................
	{
		if (f.Open(csParmfile, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::store);
			ar << static_cast<WORD>(10); // nb items
			ar << m_comment; // 1 comment
			stiD.Serialize(ar); // 2 STIMDETECT
			spkDA.Serialize(ar); // 3 SPKDETECTARRAY
			options_viewdata.Serialize(ar); // 4 OPTIONS_VIEWDATA
			options_viewspikes.Serialize(ar); // 5 OPTIONS_VIEWSPIKES
			spkC.Serialize(ar); // 6 SPKCLASSIF
			options_viewdata_measure.Serialize(ar); // 7 OPTIONS_VIEWDATAMEASURE
			options_import.Serialize(ar); // 8 OPTIONS_IMPORT
			options_acqdata.Serialize(ar); // 9 OPTIONS_ACQDATA
			options_outputdata.Serialize(ar); // 10 OPTIONS_OUTPUTDATA
			ar.Close();
			f.Close();
		}
		else
		{
			AfxMessageBox(IDS_PARAMETERFILE_FAILEDTOSAVE);
			bsuccess = FALSE;
		}
	}
	return bsuccess;
}

void CdbWaveApp::SetPrinterOrientation()
{
	if (options_viewdata.horzRes <= 0 || options_viewdata.vertRes <= 0)
		return;

	// Get default printer settings.
	PRINTDLG pd;
	pd.lStructSize = static_cast<DWORD>(sizeof(PRINTDLG));
	if (GetPrinterDeviceDefaults(&pd))
	{
		// Lock memory handle.
		auto FAR* p_dev_mode = static_cast<DEVMODE FAR*>(GlobalLock(m_hDevMode));
		HANDLE h_printer;

		if (p_dev_mode)
		{
			// Change printer settings in here.
			if (options_viewdata.horzRes > options_viewdata.vertRes)
				p_dev_mode->dmOrientation = DMORIENT_LANDSCAPE;
			else
				p_dev_mode->dmOrientation = DMORIENT_PORTRAIT;
			// Unlock memory handle.
			const auto lp_dev_names = static_cast<LPDEVNAMES>(GlobalLock(pd.hDevNames));
			if (nullptr != lp_dev_names)
			{
				const auto lpszDeviceName = reinterpret_cast<LPTSTR>(lp_dev_names) + lp_dev_names->wDeviceOffset;
				//auto lpsz_driver_name = reinterpret_cast<LPTSTR>(lp_dev_names) + lp_dev_names->wDriverOffset;
				//auto lpsz_port_name = reinterpret_cast<LPTSTR>(lp_dev_names) + lp_dev_names->wOutputOffset;

				// functions defined in winspool.h
				OpenPrinter(lpszDeviceName, &h_printer, nullptr);
				DocumentProperties(nullptr, h_printer, lpszDeviceName, p_dev_mode, p_dev_mode,
				                   DM_IN_BUFFER | DM_OUT_BUFFER);

				// Sync the pDevMode.
				// See SDK help for DocumentProperties for more info.
				ClosePrinter(h_printer);
				GlobalUnlock(m_hDevNames);
			}
			GlobalUnlock(m_hDevMode);
		}
	}
}

BOOL CdbWaveApp::GetFilenamesDlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames)
{
	//---------------------------    open dialog
	const DWORD buffer_size = 16384 * sizeof(TCHAR); // buffer / file names
	const auto h_text = GlobalAlloc(GHND, buffer_size); // allocate memory for this buffer
	if (!h_text) // memory low conditions detection
	{
		// exit if buffer not be allocated
		AfxMessageBox(IDS_AWAVEERR01, MB_ICONEXCLAMATION);
		GlobalFree(h_text); // liberate memory allocated
		return FALSE; // exit import procedure
	}

	const auto lp_text = static_cast<LPTSTR>(GlobalLock(h_text)); // lock this buffer and get a pointer to it
	if (nullptr != lp_text)
	{
		const DWORD wflag = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		// prepare dialog box
		CString filedesc; // load file description from resources
		if (!filedesc.LoadString(iIDS))
			AfxMessageBox(_T("error loading the title of message box"));

		// call file open common dialog box
		CFileDialog fd(TRUE, nullptr, nullptr, wflag, filedesc, nullptr);
		fd.m_ofn.lpstrFile = lp_text; // attach local buffer to get file nameslp_texyt
		fd.m_ofn.nMaxFile = buffer_size / sizeof(TCHAR); // declare max size of buffer
		fd.m_ofn.nFilterIndex = *iFilterIndex; // select filter item
		const CString cs_title(szTitle);
		fd.m_ofn.lpstrTitle = cs_title; // new title

		// call dialog box and store filenames into cstring array
		if ((IDOK == fd.DoModal()) && (0 != *lp_text)) // dialog box returned by CANCEL or no filename?
		{
			*iFilterIndex = fd.m_ofn.nFilterIndex; // get filter item
			auto pos = fd.GetStartPosition(); // get position of first name
			while (pos != nullptr) // if "CMultidoc", load other names
			{
				// while name are in, read & store
				CString file_a = fd.GetNextPathName(pos);
				file_a.MakeUpper(); // convert into uppercase characters
				filenames->Add(file_a); // add name to the list
			}
		}
	}
	// liberate memory used
	GlobalUnlock(h_text);
	GlobalFree(h_text);

	return TRUE;
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

CString CdbWaveApp::Get_MyDocuments_MydbWavePath()
{
	TCHAR sz_path[MAX_PATH];
	CString cspath;
	// get the path of My Documents (current user)
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, sz_path)))
	{
		// check that directory is present - otherwise create...
		cspath = sz_path;
		cspath += _T("\\My dbWave Files");

		// create directory if necessary
		CFileFind cf;
		if (!cspath.IsEmpty() && !cf.FindFile(cspath))
		{
			if (!CreateDirectory(cspath, nullptr))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}
	return cspath;
}

void CdbWaveApp::OnFileOpen()
{
	CStringArray filenames; // store file names in this array
	// IDS_FILEDESCRIPT:
	//dbWave database (*.mdb)|*.mdb|
	//Acq. data (*.dat;*.asd;*.mcid)|*.dat;*.asd;*.mcid|
	//Spikes (*.spk)|*.spk|
	//Text (*.txt)|*.txt|
	//Project (*.prj)|*.prj|
	GetFilenamesDlg(IDS_FILEDESCRIP, nullptr, &options_viewdata.nfilterindex, &filenames);
	if (filenames.GetSize() == 0)
		return;

	switch (options_viewdata.nfilterindex)
	{
	case 1: // mdb
		OpenDocumentFile(filenames[0]);
		break;
	case 2: // dat
	case 3: // spk
		{
			auto p_dbwave_doc = static_cast<CdbWaveDoc*>(m_pdataViewTemplate->CreateNewDocument());
			if (p_dbwave_doc != nullptr)
			{
				p_dbwave_doc->SetClearMdbOnExit(TRUE);
				if (p_dbwave_doc->OnNewDocument()) // create table
				{
					p_dbwave_doc->ImportFileList(filenames);
					auto p_wave_format = static_cast<CMDIFrameWnd*>(m_pdbWaveViewTemplate->CreateNewFrame(
						p_dbwave_doc, nullptr));
					ASSERT(p_wave_format != NULL);
					m_pdbWaveViewTemplate->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
				}
			}
		}
		break;
	//case 4:	// txt
	//case 5: // prj
	default:
		OpenDocumentFile(filenames[0]);
		break;
	}
}

void CdbWaveApp::FilePrintSetup()
{
	OnFilePrintSetup();
}

/*
In the current implementation of the CWinApp::OnFileNew(), this function checks the number of CDocTemplate
objects registered with the framework. If more than one, a dialog box will be displayed with a ListBox
containing the names of the document types (from the string table). Once the user has selected a template
type, the framework calls OpenDocumentFile(NULL) on the selected template to create a new empty document
of the required type. By default, if you only have one CDocTemplate object registered with the system,
then it will automatically create a document of that type.

If you need to create new empty documents of a specific type without the framework displaying the selection
dialog box, you are going to have to call CDocTemplate::OpenDocumentFile(NULL) on the correct CDocTemplate
object yourself.
*/
#include "DlgFileNew1.h"

void CdbWaveApp::OnFileNew()
{
	// the standard way is to call CWinAppEx
	//	CWinAppEx::OnFileNew();

	// here we provide a custom dialog box with a listbox with 2 choices and manually create the document and the frames.
	CDlgFileNew1 dlg;
	if (dlg.DoModal() == IDOK)
	{
		const auto iselect = dlg.m_icursel;
		switch (iselect)
		{
		case 1: // ---------------------------------------create notebook document
			{
				auto* p_dbwave_doc = static_cast<CNoteDoc*>(m_pNoteViewTemplate->CreateNewDocument());
				if (p_dbwave_doc != nullptr)
				{
					if (p_dbwave_doc->OnNewDocument()) // create table
					{
						auto p_wave_format = static_cast<CMDIFrameWnd*>(m_pNoteViewTemplate->CreateNewFrame(
							p_dbwave_doc, nullptr));
						ASSERT(p_wave_format != NULL);
						m_pNoteViewTemplate->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
					}
				}
			}
			break;
		default: // -------------------------------------- create database document
			{
				auto* p_dbwave_doc = static_cast<CdbWaveDoc*>(m_pdataViewTemplate->CreateNewDocument());
				if (p_dbwave_doc != nullptr)
				{
					p_dbwave_doc->SetClearMdbOnExit(FALSE); // keep file on exit
					if (p_dbwave_doc->OnNewDocument()) // create table
					{
						auto* p_wave_format = static_cast<CMDIFrameWnd*>(m_pdbWaveViewTemplate->CreateNewFrame(
							p_dbwave_doc, nullptr));
						ASSERT(p_wave_format != NULL);
						m_pdbWaveViewTemplate->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
					}
				}
			}
			break;
		}
	}
}
