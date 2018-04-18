// dbWave.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <winspool.h>		// get access to some printer functions
//#include "dbWave.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Acqparam.h"
#include "Taglines.h"
#include "datafile_X.h"
#include "wavebuf.h"
#include "awavepar.h"
#include "acqdatad.h"
#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "spikebar.h"
#include "spikeshape.h"
#include "Notedoc.h"
#include "Splash.h"
#include "editctrl.h"
#include "SpikeClassListBox.h"
#include "Fileversion.h"

#include "ViewData.h"
#include "ViewSpikes.h"
#include "ViewdbWave.h"
#include "ViewadContinuous.h"
#include "ViewNotedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbWaveApp

BEGIN_MESSAGE_MAP(CdbWaveApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CdbWaveApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, &CdbWaveApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CdbWaveApp::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CdbWaveApp construction

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
	SetAppID(_T("FMP.dbWave2.VS2017.Aug-2017"));

	m_pviewdataMemFile		= NULL;
	m_pviewspikesMemFile	= NULL;
	m_psort1spikesMemFile	= NULL;
	m_bADcardFound			= TRUE;
}


CdbWaveApp theApp;


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
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);
		CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
		CSplashWnd::ShowSplashScreen();
	}

	// works with debug version, fails with release statically linked
	//AfxGetModuleState()->m_dwVersion = 0x0601; // enable Access2000

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("FMP\\dbWave2"));
	SetRegistryBase(_T("Settings"));
	Defaultparameters(TRUE);
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	InitContextMenuManager();
	InitShellManager();
	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pdbWaveViewTemplate = new CdbMultiDocTemplate(IDR_DBWAVETYPE,
		RUNTIME_CLASS(CdbWaveDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CdbWaveView));
	m_pdbWaveViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pdbWaveViewTemplate);
	m_hDBView = m_pdbWaveViewTemplate->m_hMenuShared;
	
	// ---------------------------------------------
	m_pdataViewTemplate = new CdbMultiDocTemplate(IDR_DBDATATYPE,
		RUNTIME_CLASS(CdbWaveDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CDataView));
	m_pdataViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pdataViewTemplate);
	m_hDataView = m_pdataViewTemplate->m_hMenuShared;

	// continuous A/D view with data translation card
	m_pADViewTemplate= new CdbMultiDocTemplate(IDR_DBDATATYPE,
		RUNTIME_CLASS(CdbWaveDoc),
		RUNTIME_CLASS(CChildFrame),		// multifile MDI child frame
		RUNTIME_CLASS(CADContView));	// AD view
	ASSERT(m_pADViewTemplate != NULL);
	m_pADViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pADViewTemplate);
	m_hAcqView = m_pADViewTemplate->m_hMenuShared;

	m_pspikeViewTemplate = new CdbMultiDocTemplate(IDR_DBSPIKETYPE,
		RUNTIME_CLASS(CdbWaveDoc),
		RUNTIME_CLASS(CChildFrame),		// custom MDI child frame
		RUNTIME_CLASS(CSpikeView));
	m_pspikeViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pspikeViewTemplate);
	m_hSpikeView = m_pspikeViewTemplate->m_hMenuShared;

	m_pNoteViewTemplate = new CdbMultiDocTemplate(IDR_PROJECTTYPE,
		RUNTIME_CLASS(CNoteDoc),
		RUNTIME_CLASS(CMDIChildWndEx),	// standard MDI child frame
		RUNTIME_CLASS(CNoteDocView));
	m_pNoteViewTemplate->SetContainerInfo(IDR_DBWAVETYPE_CNTR_IP);
	AddDocTemplate(m_pNoteViewTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew) // actually none
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Enable DDE Execute open - this allow to open data files by double clicking on it
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);		// this works only if user has administrative rights

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	m_nCmdShow = SW_SHOWNORMAL; //SW_SHOWMAXIMIZED;  // change into THIS to start dbwave maximized
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CdbWaveApp::ExitInstance() 
{
	Defaultparameters(FALSE);
	AfxOleTerm(FALSE);

	SAFE_DELETE(m_pviewdataMemFile);
	SAFE_DELETE(m_psf);

	if (m_viewspkdParmsArray.GetSize() != NULL)	
	{
		for (int i=0; i< m_viewspkdParmsArray.GetSize(); i++)
			delete (CMemFile*) m_viewspkdParmsArray.GetAt(i);
	}

	SAFE_DELETE(m_pviewspikesMemFile);
	SAFE_DELETE(m_psort1spikesMemFile);

	// erase temporary "mdb" files
	int i0= m_tempMDBfiles.GetCount()-1;
	for (int i=i0; i>= 0; i--)
	{
		CString cs = m_tempMDBfiles.GetAt(i);
		CFile::Remove(cs);
	}

	return CWinAppEx::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOXDLG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CdbWaveApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog()
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
		e->ReportError(); e->Delete();
	}

	CFileVersion cFV;
	CString csAppName = _T("dbwave2.exe");
	cFV.Open(csAppName);
	GetDlgItem(IDC_DBWAVEVERSION)->SetWindowText(cFV.GetProductVersion());
	GetDlgItem(IDC_STATIC7)->SetWindowText(cFV.GetLegalCopyright());

	return TRUE;
}

// CMFCApplication1App customization load/save methods

void CdbWaveApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CdbWaveApp::LoadCustomState()
{
}

void CdbWaveApp::SaveCustomState()
{
}

/////////////////////////////////////////////////////////////////////////////
// CdbWaveApp message handlers


BOOL CdbWaveApp::PreTranslateMessage(MSG* pMsg)
{
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
	  return TRUE;
	return CWinAppEx::PreTranslateMessage(pMsg);
}


void DisplayDaoException(CDaoException* e, int iID = 0)
{
	CString strMsg;
	if (e->m_pErrorInfo!=NULL)
	{
		strMsg.Format(
			_T("%s   (%d) at line ID %i\n\n")
			_T("Would you like to see help?"),
			(LPCTSTR)e->m_pErrorInfo->m_strDescription,
			e->m_pErrorInfo->m_lErrorCode, iID);

		if (AfxMessageBox(strMsg, MB_YESNO) == IDYES)
		{
			WinHelp(GetDesktopWindow(),
					e->m_pErrorInfo->m_strHelpFile,
					HELP_CONTEXT,
					e->m_pErrorInfo->m_lHelpContext);
		}
	}
	else
	{
		strMsg.Format(
			_T("ERROR:CDaoException\n\n")
			_T("SCODE_CODE      =%d\n")
			_T("SCODE_FACILITY  =%d\n")
			_T("SCODE_SEVERITY  =%d\n")
			_T("ResultFromScode =%d\n"),
			SCODE_CODE      (e->m_scode),
			SCODE_FACILITY  (e->m_scode),
			SCODE_SEVERITY  (e->m_scode),
			ResultFromScode (e->m_scode));
		AfxMessageBox(strMsg);
	}
}


/////////////////////////////////////////////////////////////////////////////

static TCHAR szVDS[] = _T("Default parameters");
static TCHAR szFileEntry[] = _T("File%d");

void CdbWaveApp::Defaultparameters(BOOL bRead)
{	
	TCHAR szPath[MAX_PATH];
	CString cspath;
	if(SUCCEEDED(SHGetFolderPath(NULL, 
								CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
								NULL, 
								0, 
								szPath))) 
	{
		// check that directory is present - otherwise create...
		cspath = szPath; 
		cspath += _T("\\My dbWave Files");
		// test if directory already present
		BOOL bExist = PathFileExists(cspath);
		if (!bExist)
		{
			if (!CreateDirectory(cspath, NULL))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}

	TCHAR szEntry[MAX_PATH];
	unsigned long len=MAX_PATH;
	GetUserName(&szEntry[0], &len);
	CString csExt(szEntry);
	if (csExt.IsEmpty())
		csExt= _T("dbWave2");
	csExt+= _T(".prefs2");
	CString csDefaultParmfile = cspath + "\\" + csExt;

	// read data and copy into vdP
	if (bRead)
	{
		// get list of parameter files (others)
		m_csParmFiles.RemoveAll();
		int i=0;
		while (i >= 0)
		{
			wsprintf(&szEntry[0], szFileEntry, i+1);
			CString dummy = GetProfileString(szVDS, &szEntry[0]);
			if (dummy.IsEmpty())
				break;
			if (dummy.Find(csExt) > 0)
				m_csParmFiles.Add(dummy);
			i++;
		}
		// get default parameter file and load data
		if (m_csParmFiles.GetSize() <= 0)
			m_csParmFiles.Add(csDefaultParmfile);		
		ParmFile(m_csParmFiles[0], bRead);
	}
	// Save informations
	else
	{
		// save default parameter file		
		if (m_csParmFiles.GetSize() <= 0)
			m_csParmFiles.Add(csDefaultParmfile);
		if (!ParmFile(m_csParmFiles[0], bRead))
		{
			m_csParmFiles[0]=csDefaultParmfile;
			ParmFile(m_csParmFiles[0], bRead);
		}
		// save profile with locations of parmfiles
		for (int i = 0; i<m_csParmFiles.GetSize(); i++)
		{
			wsprintf(&szEntry[0], szFileEntry, i+1);
			WriteProfileString(szVDS, &szEntry[0], m_csParmFiles[i]);
		}
	}
}


BOOL CdbWaveApp::ParmFile(CString& csParmfile, BOOL bRead)
{
	CFile f;					// file object
	CFileException fe;			// trap exceptions
	BOOL bsuccess=TRUE;
	
	if (bRead)		// read informations ...........................
	{
		if (f.Open(csParmfile, CFile::modeReadWrite|CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::load);
			WORD m;	ar >> m;					// nb items to load
			int n=m;
			ar >> m_comment; // comment
			n--; if (n > 0) stiD.Serialize(ar);	// STIMDETECT
			n--; if (n > 0) spkDA.Serialize(ar);// SPKDETECTARRAY
			n--; if (n > 0) vdP.Serialize(ar);	// OPTIONS_VIEWDATA
			n--; if (n > 0) vdS.Serialize(ar);	// OPTIONS_VIEWSPIKES
			n--; if (n > 0) spkC.Serialize(ar);	// SPKCLASSIF
			n--; if (n > 0) vdM.Serialize(ar);	// OPTIONS_VIEWDATAMEASURE
			n--; if (n > 0) ivO.Serialize(ar);	// OPTIONS_IMPORT
			n--; if (n > 0) acqD.Serialize(ar);	// OPTIONS_ACQDATA
			n--; if (n > 0) outD.Serialize(ar);	//OPTIONS_OUTPUTDATA

			ar.Close();					// close archive
			f.Close();					// close file
		}
	}	
	else	// Save informations .............................
	{		
		if (f.Open(csParmfile, CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyNone, &fe))
		{
			CArchive ar(&f, CArchive::store);
			ar << (WORD) 10;		// nb items
			ar << m_comment;		// 1 comment
			stiD.Serialize(ar);		// 2 STIMDETECT
			spkDA.Serialize(ar);	// 3 SPKDETECTARRAY			
			vdP.Serialize(ar);		// 4 OPTIONS_VIEWDATA
			vdS.Serialize(ar);		// 5 OPTIONS_VIEWSPIKES
			spkC.Serialize(ar);		// 6 SPKCLASSIF
			vdM.Serialize(ar);		// 7 OPTIONS_VIEWDATAMEASURE
			ivO.Serialize(ar);		// 8 OPTIONS_IMPORT
			acqD.Serialize(ar);		// 9 OPTIONS_ACQDATA
			outD.Serialize(ar);		// 10 OPTIONS_OUTPUTDATA
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


/////////////////////////////////////////////////////////////////////////////

// ** cf How to change default printer settings in an MFC application
// Microsoft Developer Network July 1996
// PSS ID Number: Q126897  Article last modified on 01-21-1996
//
// SUMMARY
// =======
// 
// To change the default printer settings in an MFC application, you must
// retrieve the system default settings in a CWinAppEx derived object and
// modify those defaults before a print job is invoked.

void CdbWaveApp::SetPrinterOrientation()
{
	if (vdP.horzRes <= 0 || vdP.vertRes <= 0)
		return;

	// Get default printer settings.
	PRINTDLG   pd;
	pd.lStructSize = (DWORD) sizeof(PRINTDLG);
	if (GetPrinterDeviceDefaults(&pd))
	{
		// Lock memory handle.
		DEVMODE FAR* pDevMode = (DEVMODE FAR*)::GlobalLock(m_hDevMode);
		LPDEVNAMES lpDevNames = NULL;
		LPTSTR lpszDriverName, lpszDeviceName, lpszPortName;
		HANDLE hPrinter;

		if (pDevMode)
		{
			// Change printer settings in here.
			if (vdP.horzRes > vdP.vertRes)
				pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
			else
				pDevMode->dmOrientation = DMORIENT_PORTRAIT;
			// Unlock memory handle.
			lpDevNames = (LPDEVNAMES)GlobalLock(pd.hDevNames);
			if (NULL != lpDevNames)
			{
				lpszDriverName = (LPTSTR)lpDevNames + lpDevNames->wDriverOffset;
				lpszDeviceName = (LPTSTR)lpDevNames + lpDevNames->wDeviceOffset;
				lpszPortName = (LPTSTR)lpDevNames + lpDevNames->wOutputOffset;

				// functions defined in winspool.h
				OpenPrinter(lpszDeviceName, &hPrinter, NULL);
				DocumentProperties(NULL, hPrinter, lpszDeviceName, pDevMode, pDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);

				// Sync the pDevMode.
				// See SDK help for DocumentProperties for more info.
				ClosePrinter(hPrinter);
				GlobalUnlock(m_hDevNames);
			}
			GlobalUnlock(m_hDevMode);
		}
	}
}


// call file selection dialog box
// input:	int number corresponding to file filter strings
//			string with title for the dialog box
//			pointer to index of filters chosen by user (to keep track of last choices)
//			address for a string array where file names selected will be stored
// output:	string array with file names found

BOOL CdbWaveApp::GetFilenamesDlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames)
{
	//---------------------------    open dialog
	int iReturn = -1;
	DWORD bufferSize = 16384*sizeof(TCHAR);				// buffer / file names
	HGLOBAL hText = ::GlobalAlloc(GHND, bufferSize);// allocate memory for this buffer      
	if (!hText)								// memory low conditions detection
	{										// exit if buffer not be allocated
		AfxMessageBox(IDS_AWAVEERR01, MB_ICONEXCLAMATION);
		::GlobalFree(hText);				// liberate memory allocated
		return FALSE;						// exit import procedure
	}

	LPTSTR lpText = (LPTSTR) ::GlobalLock(hText);	// lock this buffer and get a pointer to it	
	if (NULL != lpText)
	{
		DWORD wflag = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		// prepare dialog box
		CString filedesc;						// load file description from resources	
		BOOL flag = filedesc.LoadString(iIDS);	// title of dialog box

		// call file open common dialog box
		CFileDialog fd(TRUE, NULL, NULL, wflag, filedesc, NULL);
		fd.m_ofn.lpstrFile = lpText;			// attach local buffer to get file names
		fd.m_ofn.nMaxFile = bufferSize /sizeof(TCHAR);	// declare max size of buffer
		fd.m_ofn.nFilterIndex = *iFilterIndex;	// select filter item
		CString csTitle(szTitle);
		fd.m_ofn.lpstrTitle = csTitle;			// new title

		// call dialog box and store filenames into cstring array
		if ((IDOK == fd.DoModal()) && (0 != *lpText))// dialog box returned by CANCEL or no filename?
		{
			*iFilterIndex = fd.m_ofn.nFilterIndex;		// get filter item
			POSITION pos = fd.GetStartPosition();		// get position of first name
			CString file_a;					// declare dummy CString to receive file names
			while (pos != NULL)				// if "CMultidoc", load other names
			{								// while name are in, read & store
				file_a = fd.GetNextPathName(pos);
				file_a.MakeUpper();			// convert into uppercase characters
				filenames->Add(file_a);		// add name to the list
			}
		}
	}
	// liberate memory used
	::GlobalUnlock(hText);
	::GlobalFree(hText);

	return TRUE;
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

CString CdbWaveApp::Get_MyDocuments_MydbWavePath()
{
	TCHAR szPath[MAX_PATH];
	CString cspath;
	// get the path of My Documents (current user)
	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, szPath))) 
	{
		// check that directory is present - otherwise create...
		cspath = szPath; 
		cspath += _T("\\My dbWave Files");

		// create directory if necessary
		CFileFind cf;
		if (!cspath.IsEmpty() && !cf.FindFile(cspath))
		{
			if (!CreateDirectory(cspath, NULL))
				AfxMessageBox(IDS_DIRECTORYFAILED);
		}
	}
	return cspath;
}

void CdbWaveApp::OnFileOpen()
{   
	CStringArray filenames;					// store file names in this array
	// IDS_FILEDESCRIPT: 
	//dbWave database (*.mdb)|*.mdb|
	//Acq. data (*.dat;*.asd;*.mcid)|*.dat;*.asd;*.mcid|
	//Spikes (*.spk)|*.spk|
	//Text (*.txt)|*.txt|
	//Project (*.prj)|*.prj|
	GetFilenamesDlg(IDS_FILEDESCRIP, NULL, &vdP.nfilterindex, &filenames);
	if (filenames.GetSize() == 0)
		return;

	switch (vdP.nfilterindex)
	{
	case 1: // mdb
		OpenDocumentFile(filenames[0]);
		break;
	case 2:	// dat
	case 3:	// spk
		{
			CdbWaveDoc* pdbDoc = (CdbWaveDoc*) m_pdataViewTemplate->CreateNewDocument();
			if (pdbDoc != NULL) 
			{
				pdbDoc->SetClearMdbOnExit(TRUE);
				if (pdbDoc->OnNewDocument())	// create table
				{
					pdbDoc->ImportDescFromFileList(filenames);
					CMDIFrameWnd* pWF = (CMDIFrameWnd*)m_pdbWaveViewTemplate->CreateNewFrame(pdbDoc, NULL);
					ASSERT(pWF != NULL);
					m_pdbWaveViewTemplate->InitialUpdateFrame(pWF, pdbDoc, TRUE);
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
#include "FileNew1.h"

void CdbWaveApp::OnFileNew()
{
	// the standard way is to call CWinAppEx
	//	CWinAppEx::OnFileNew();

	// here we provide a custom dialog box with a listbox with 2 choices and manually create the document and the frames.
	CFileNew1 dlg;
	if (dlg.DoModal() == IDOK)
	{
		int iselect = dlg.m_icursel;
		switch(iselect)
		{
		case 1: // ---------------------------------------create notebook document
			{
			CNoteDoc* pdbDoc = (CNoteDoc*) m_pNoteViewTemplate->CreateNewDocument();
			if (pdbDoc != NULL) 
				{
					if (pdbDoc->OnNewDocument())	// create table
					{
						CMDIFrameWnd* pWF = (CMDIFrameWnd*) m_pNoteViewTemplate->CreateNewFrame(pdbDoc, NULL);
						ASSERT(pWF != NULL);
						m_pNoteViewTemplate->InitialUpdateFrame(pWF, pdbDoc, TRUE);
					}
				}
			}
			break;
		default: // -------------------------------------- create database document
			{
			CdbWaveDoc* pdbDoc = (CdbWaveDoc*) m_pdataViewTemplate->CreateNewDocument();
			if (pdbDoc != NULL) 
				{
					pdbDoc->SetClearMdbOnExit(FALSE);				// keep file on exit
					if (pdbDoc->OnNewDocument())					// create table
					{
						CMDIFrameWnd* pWF = (CMDIFrameWnd*)m_pdbWaveViewTemplate->CreateNewFrame(pdbDoc, NULL);
						ASSERT(pWF != NULL);
						m_pdbWaveViewTemplate->InitialUpdateFrame(pWF, pdbDoc, TRUE);
					}
				}
			}
			break;
		}
	}
}

