#include "StdAfx.h"
#include "dbWave.h"

#include <winspool.h>
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ViewData.h"
#include "ViewSpikes.h"
#include "ViewdbWave.h"
#include "ViewADcontinuous.h"
#include "ViewNotedoc.h"
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

CdbWaveApp the_app;
static TCHAR sz_vds[] = _T("Default parameters");
static TCHAR sz_file_entry[] = _T("File%d");

void DisplayDaoException(CDaoException* e, int i_id = 0)
{
	CString str_msg;
	if (e->m_pErrorInfo != nullptr)
	{
		str_msg.Format(
			_T("%s   (%d) at line ID %i\n\n")
			_T("Would you like to see help?"),
			(LPCTSTR)e->m_pErrorInfo->m_strDescription,
			e->m_pErrorInfo->m_lErrorCode, i_id);

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

CdbWaveApp::CdbWaveApp()  noexcept
{

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
	EnableHtmlHelp();

#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("FMP.dbWave2.VS2019.May-2022"));
}

BOOL CdbWaveApp::InitInstance()
{
	INITCOMMONCONTROLSEX init_controls;
	init_controls.dwSize = sizeof(init_controls);
	init_controls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&init_controls);
	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction();

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("FMP"));
	SetRegistryBase(_T("Settings"));
	default_parameters(TRUE);
	LoadStdProfileSettings(4); 

	InitContextMenuManager();
	InitShellManager();
	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo tt_params;
	tt_params.m_bVislManagerTheme = TRUE;
	the_app.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &tt_params);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_db_wave_view_template = new CdbMultiDocTemplate(IDR_DBWAVETYPE,
	                                                RUNTIME_CLASS(CdbWaveDoc),
	                                                RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                                RUNTIME_CLASS(ViewdbWave));
	m_db_wave_view_template->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_db_wave_view_template);
	h_menu_db_view = m_db_wave_view_template->m_hMenuShared;

	// ---------------------------------------------
	m_data_view_template = new CdbMultiDocTemplate(IDR_DBDATATYPE,
	                                              RUNTIME_CLASS(CdbWaveDoc),
	                                              RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                              RUNTIME_CLASS(ViewData));
	m_data_view_template->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_data_view_template);
	h_menu_data_view = m_data_view_template->m_hMenuShared;

	// continuous A/D view with data translation card
	m_ad_view_template = new CdbMultiDocTemplate(IDR_DBDATATYPE,
	                                            RUNTIME_CLASS(CdbWaveDoc),
	                                            RUNTIME_CLASS(CChildFrame), // multi-file MDI child frame
	                                            RUNTIME_CLASS(ViewADcontinuous)); // AD view
	ASSERT(m_ad_view_template != NULL);
	m_ad_view_template->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_ad_view_template);
	h_menu_acq_view = m_ad_view_template->m_hMenuShared;

	m_spike_view_template = new CdbMultiDocTemplate(IDR_DBSPIKETYPE,
	                                               RUNTIME_CLASS(CdbWaveDoc),
	                                               RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	                                               RUNTIME_CLASS(ViewSpikes));
	m_spike_view_template->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_spike_view_template);
	h_menu_spike_view = m_spike_view_template->m_hMenuShared;

	m_note_view_template = new CdbMultiDocTemplate(IDR_PROJECTTYPE,
	                                              RUNTIME_CLASS(CNoteDoc),
	                                              RUNTIME_CLASS(CMDIChildWndEx), // standard MDI child frame
	                                              RUNTIME_CLASS(ViewNoteDoc));
	m_note_view_template->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_note_view_template);

	// create main MDI Frame window
	const auto p_main_frame = new CMainFrame;
	constexpr DWORD dw_default_style = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE;
	CWnd* p_parent_wnd = nullptr;
	CCreateContext* p_context = nullptr;
	if (!p_main_frame || !p_main_frame->LoadFrame(IDR_MAINFRAME, dw_default_style, p_parent_wnd, p_context))
	{
		delete p_main_frame;
		return FALSE;
	}
	m_pMainWnd = p_main_frame;

	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmd_info;
	ParseCommandLine(cmd_info);

	CSplashWnd::enable_splash_screen(cmd_info.m_bShowSplash);
	CSplashWnd::show_splash_screen();

	if (cmd_info.m_nShellCommand == CCommandLineInfo::FileNew) // actually none
		cmd_info.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Enable DDE Execute open - this allow to open data files by double-click on it
	EnableShellOpen();
	RegisterShellFileTypes(TRUE); // this works only if user has administrative rights

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Un-reg-server or /Un-register.
	if (!ProcessShellCommand(cmd_info))
		return FALSE;

	// The main window has been initialized, so show and update it.
	m_nCmdShow = SW_SHOWNORMAL;
	p_main_frame->ShowWindow(m_nCmdShow);
	p_main_frame->UpdateWindow();

	return TRUE;
}

int CdbWaveApp::ExitInstance()
{
	default_parameters(FALSE);
	AfxOleTerm(FALSE);

	SAFE_DELETE(m_p_view_data_memory_file)
	SAFE_DELETE(m_psf)

	if (view_spikes_memory_file_ptr_array.GetSize() != NULL)
	{
		for (auto i = 0; i < view_spikes_memory_file_ptr_array.GetSize(); i++)
			delete view_spikes_memory_file_ptr_array.GetAt(i);
	}

	SAFE_DELETE(m_p_view_spikes_memory_file)
	SAFE_DELETE(m_p_sort1_spikes_memory_file)

	// erase temporary "mdb" files
	const auto i0 = m_temporary_mdb_files.GetCount() - 1;
	for (auto i = i0; i >= 0; i--)
	{
		auto cs = m_temporary_mdb_files.GetAt(i);
		CFile::Remove(cs);
	}

	return CWinAppEx::ExitInstance();
}

void CdbWaveApp::OnAppAbout()
{
	DlgAbout about_dlg;
	about_dlg.DoModal();
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

BOOL CdbWaveApp::PreTranslateMessage(MSG* p_msg)
{
	if (CSplashWnd::PreTranslateAppMessage(p_msg))
		return TRUE;
	return CWinAppEx::PreTranslateMessage(p_msg);
}

void CdbWaveApp::default_parameters(BOOL b_read)
{
	TCHAR sz_path[MAX_PATH];
	CString cs_path;
	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		sz_path)))
	{
		// check that directory is present - otherwise create...
		cs_path = sz_path;
		cs_path += _T("\\My dbWave Files");
		// test if directory already present
		const auto b_exist = PathFileExists(cs_path);
		if (!b_exist)
		{
			if (!CreateDirectory(cs_path, nullptr))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}

	TCHAR sz_entry[MAX_PATH];
	unsigned long len = MAX_PATH;
	GetUserName(&sz_entry[0], &len);
	CString cs_ext(sz_entry);
	if (cs_ext.IsEmpty())
		cs_ext = _T("dbWave2");
	cs_ext += _T(".prefs2");
	const auto cs_default_parameters_file = cs_path + "\\" + cs_ext;

	// read data and copy into vdP
	if (b_read)
	{
		// get list of parameter files (others)
		m_cs_parameter_files.RemoveAll();
		auto i = 0;
		while (i >= 0)
		{
			wsprintf(&sz_entry[0], sz_file_entry, i + 1);
			auto dummy = GetProfileString(sz_vds, &sz_entry[0]);
			if (dummy.IsEmpty())
				break;
			if (dummy.Find(cs_ext) > 0)
				m_cs_parameter_files.Add(dummy);
			i++;
		}
		// get default parameter file and load data
		if (m_cs_parameter_files.GetSize() <= 0)
			m_cs_parameter_files.Add(cs_default_parameters_file);
		archive_parameter_files(m_cs_parameter_files[0], b_read);
	}
	// Save information
	else
	{
		// save default parameter file
		if (m_cs_parameter_files.GetSize() <= 0)
			m_cs_parameter_files.Add(cs_default_parameters_file);
		if (!archive_parameter_files(m_cs_parameter_files[0], b_read))
		{
			m_cs_parameter_files[0] = cs_default_parameters_file;
			archive_parameter_files(m_cs_parameter_files[0], b_read);
		}
		// save profile with locations of parameter files
		for (auto i = 0; i < m_cs_parameter_files.GetSize(); i++)
		{
			wsprintf(&sz_entry[0], sz_file_entry, i + 1);
			WriteProfileString(sz_vds, &sz_entry[0], m_cs_parameter_files[i]);
		}
	}
}

BOOL CdbWaveApp::archive_parameter_files(const CString& filename, const BOOL b_read)
{
	CFile f;
	CFileException fe; 
	auto is_success = TRUE;

	if (b_read)
	{
		if (f.Open(filename, CFile::modeReadWrite | CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::load);
			WORD m;
			ar >> m; 
			ar >> m_comment; 

			const int n = m;
			serialize_parameters(n, ar);

			ar.Close(); 
			f.Close(); 
		}
	}
	else 
	{
		if (f.Open(filename, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::store);
			constexpr int n = 10;
			ar << static_cast<WORD>(10);
			ar << m_comment; 

			serialize_parameters( n, ar);

			ar.Close();
			f.Close();
		}
		else
		{
			AfxMessageBox(IDS_PARAMETERFILE_FAILEDTOSAVE);
			is_success = FALSE;
		}
	}
	return is_success;
}

void CdbWaveApp::serialize_parameters(int n, CArchive& ar)
{
	n--; if (n > 0) stimulus_detect.Serialize(ar);
	n--; if (n > 0) spk_detect_array.Serialize(ar);
	n--; if (n > 0) options_view_data.Serialize(ar);
	n--; if (n > 0) options_view_spikes.Serialize(ar);
	n--; if (n > 0) spk_classification.Serialize(ar);
	n--; if (n > 0) options_view_data_measure.Serialize(ar);
	n--; if (n > 0) options_import.Serialize(ar);
	n--; if (n > 0) options_acq_data.Serialize(ar);
	n--; if (n > 0) options_output_data.Serialize(ar);
}

void CdbWaveApp::SetPrinterOrientation()
{
	if (options_view_data.horizontal_resolution <= 0 || options_view_data.vertical_resolution <= 0)
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
			if (options_view_data.horizontal_resolution > options_view_data.vertical_resolution)
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

				// functions defined in win_spool.h
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

BOOL CdbWaveApp::get_file_names_dlg(int i_ids, LPCSTR sz_title, int* i_filter_index, CStringArray* filenames)
{
	//---------------------------    open dialog
	constexpr DWORD buffer_size = 16384 * sizeof(TCHAR); // buffer / file names
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
		constexpr DWORD w_flag = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		// prepare dialog box
		CString file_desc; // load file description from resources
		if (!file_desc.LoadString(i_ids))
			AfxMessageBox(_T("error loading the title of message box"));

		// call file open common dialog box
		CFileDialog fd(TRUE, nullptr, nullptr, w_flag, file_desc, nullptr);
		fd.m_ofn.lpstrFile = lp_text; 
		fd.m_ofn.nMaxFile = buffer_size / sizeof(TCHAR); 
		fd.m_ofn.nFilterIndex = *i_filter_index; 
		const CString cs_title(sz_title);
		fd.m_ofn.lpstrTitle = cs_title; 

		// call dialog box and store filenames into string array
		if ((IDOK == fd.DoModal()) && (0 != *lp_text)) 
		{
			*i_filter_index = static_cast<int>(fd.m_ofn.nFilterIndex); 
			auto pos = fd.GetStartPosition(); 
			while (pos != nullptr) 
			{
				// while name are in, read & store
				CString file_a = fd.GetNextPathName(pos);
				file_a.MakeUpper();
				filenames->Add(file_a); 
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

CString CdbWaveApp::get_my_documents_my_dbwave_path()
{
	TCHAR sz_path[MAX_PATH];
	CString cs_path;
	// get the path of My Documents (current user)
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, sz_path)))
	{
		// check that directory is present - otherwise create...
		cs_path = sz_path;
		cs_path += _T("\\My dbWave Files");

		// create directory if necessary
		CFileFind cf;
		if (!cs_path.IsEmpty() && !cf.FindFile(cs_path))
		{
			if (!CreateDirectory(cs_path, nullptr))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}
	return cs_path;
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
	get_file_names_dlg(IDS_FILEDESCRIP, nullptr, &options_view_data.n_filter_index, &filenames);
	if (filenames.GetSize() == 0)
		return;

	switch (options_view_data.n_filter_index)
	{
	case 1: // mdb
		OpenDocumentFile(filenames[0]);
		break;
	case 2: // dat
	case 3: // spk
		{
			const auto p_dbwave_doc = static_cast<CdbWaveDoc*>(m_data_view_template->CreateNewDocument());
			if (p_dbwave_doc != nullptr)
			{
				p_dbwave_doc->set_clean_db_on_exit(TRUE);
				if (p_dbwave_doc->OnNewDocument()) // create table
				{
					p_dbwave_doc->import_file_list(filenames);
					auto p_wave_format = static_cast<CMDIFrameWnd*>(m_db_wave_view_template->CreateNewFrame(
						p_dbwave_doc, nullptr));
					ASSERT(p_wave_format != NULL);
					m_db_wave_view_template->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
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
	DlgFileNew1 dlg;
	if (dlg.DoModal() == IDOK)
	{
		const auto i_cur_sel = dlg.m_i_cur_sel;
		if (i_cur_sel == 1)
		{
			auto* p_dbwave_doc = static_cast<CNoteDoc*>(m_note_view_template->CreateNewDocument());
			if (p_dbwave_doc != nullptr)
			{
				if (p_dbwave_doc->OnNewDocument()) // create table
				{
					auto p_wave_format = static_cast<CMDIFrameWnd*>(m_note_view_template->CreateNewFrame(
						p_dbwave_doc, nullptr));
					ASSERT(p_wave_format != NULL);
					m_note_view_template->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
				}
			}
		}
		else // -------------------------------------- create database document
		{
			auto* p_dbwave_doc = static_cast<CdbWaveDoc*>(m_data_view_template->CreateNewDocument());
			if (p_dbwave_doc != nullptr)
			{
				p_dbwave_doc->set_clean_db_on_exit(FALSE); // keep file on exit
				if (p_dbwave_doc->OnNewDocument()) // create table
				{
					auto* p_wave_format = static_cast<CMDIFrameWnd*>(m_db_wave_view_template->CreateNewFrame(
						p_dbwave_doc, nullptr));
					ASSERT(p_wave_format != NULL);
					m_db_wave_view_template->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
				}
			}
		}

	}
}
