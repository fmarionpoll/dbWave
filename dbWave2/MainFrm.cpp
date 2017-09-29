
/////////////////////////////////////////////////////////////////////////////
// CMainFrame
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_WINDOW_MANAGER,					&CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	
	ON_COMMAND(ID_TOOLS_OPTIONS,					&CMainFrame::OnOptions)
	
	ON_COMMAND(ID_VIEW_CUSTOMIZE,					&CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR,		&CMainFrame::OnToolbarCreateNew)
	
	ON_COMMAND(ID_PANE_PROPERTIESWND,				&CMainFrame::OnViewPropertiesWindow)
	ON_UPDATE_COMMAND_UI(ID_PANE_PROPERTIESWND,		&CMainFrame::OnUpdateViewPropertiesWindow)
	ON_COMMAND(ID_PANE_FILTERWND,					&CMainFrame::OnViewFilterWindow)
	ON_UPDATE_COMMAND_UI(ID_PANE_FILTERWND,			&CMainFrame::OnUpdateViewFilterWindow)
	ON_MESSAGE(WM_MYMESSAGE,						&CMainFrame::OnMyMessage)
	ON_WM_SETTINGCHANGE()

	ON_COMMAND(ID_CHECK_FILTERPANE,					&CMainFrame::OnCheckFilterpane)
	ON_UPDATE_COMMAND_UI(ID_CHECK_FILTERPANE,		&CMainFrame::OnUpdateCheckFilterpane)
	ON_COMMAND(ID_CHECK_PROPERTIESPANE,				&CMainFrame::OnCheckPropertiespane)
	ON_UPDATE_COMMAND_UI(ID_CHECK_PROPERTIESPANE,	&CMainFrame::OnUpdateCheckPropertiespane)

	ON_COMMAND(ID_TOOLS_COMPACTDATABASE,			&CMainFrame::OnToolsCompactdatabase)

	ON_COMMAND(ID_HELP_FINDER,	&CMDIFrameWndEx::OnHelpFinder)
	ON_COMMAND(ID_HELP,			&CMDIFrameWndEx::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWndEx::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWndEx::OnHelpFinder)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction


CMainFrame::CMainFrame()
{
	m_SecondToolBarID = 0;
	m_pSecondToolBar = NULL;
	m_bPropertiesPaneVisible = TRUE;
	m_bFilterPaneVisible = TRUE;

	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
}


CMainFrame::~CMainFrame()
{
	m_wndOutlookPane.ClearAll();
	m_wndOutlookPane.RemoveAllButtons();
	SAFE_DELETE(m_pSecondToolBar);
}


void CMainFrame::OnDestroy()
{
	CMDIFrameWndEx::OnDestroy();
}


void CMainFrame::ActivatePropertyPane(BOOL bActivate)
{
	if (bActivate != m_wndProperties.IsVisible())
		m_wndProperties.ShowPane(bActivate, FALSE, FALSE);
}


void CMainFrame::ActivateFilterPane(BOOL bActivate)
{
	if (bActivate != m_wndFilter.IsVisible())
		m_wndFilter.ShowPane(bActivate, FALSE, FALSE);
}


BOOL CMainFrame::SetSecondToolBar(UINT nIDResource)
{
	// check if same request as the previous one
	if (nIDResource == m_SecondToolBarID)
	{
		// it toolbar is not visible, make it visible
		if(!m_pSecondToolBar->IsWindowVisible())
			m_pSecondToolBar->ShowWindow(SW_SHOWNORMAL);
		return TRUE;
	}

	m_SecondToolBarID = nIDResource;
	if (m_pSecondToolBar == NULL)
	{
		m_pSecondToolBar = new CMFCToolBar;
		ASSERT(m_pSecondToolBar != NULL);

		if (!m_pSecondToolBar->CreateEx(this, 
			TBSTYLE_FLAT, 
			WS_CHILD | WS_VISIBLE | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
			CRect(1, 1, 1, 1), nIDResource)
		|| !m_pSecondToolBar->LoadToolBar(nIDResource))
			return FALSE;      // fail to create
	}
	else
	{
		m_pSecondToolBar->LoadToolBar(nIDResource);
		// make sure the toolbar is visible
		if(!m_pSecondToolBar->IsWindowVisible())
			m_pSecondToolBar->ShowWindow(SW_SHOWNORMAL);
	}

	CString strToolBarName;
	BOOL bNameValid = FALSE;
	switch (nIDResource)
	{
	case IDR_DBSPIKETYPE:
		bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_4SPIKES);
		break;
	case IDR_DBDATATYPE:
	default:
		bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_4DATA);
		break;
	}
	ASSERT(bNameValid);
	m_pSecondToolBar->SetWindowText(strToolBarName);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_pSecondToolBar->EnableDocking(CBRS_ALIGN_TOP);
	DockPane(m_pSecondToolBar);
	m_dockManager.DockPaneLeftOf(&m_wndToolBar, m_pSecondToolBar);
	
	return TRUE;
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;		// set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;					// set to TRUE to enable document icons on MDI tabs
	mdiTabParams.m_bAutoColor = TRUE;					// set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE;				// enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams);
	// Create the ribbon bar
	if (!m_wndRibbonBar.Create(this))
	{
		return -1; //Failed to create ribbon bar
	}
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// status bar
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);

	// choices bar
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);
	if (!CreateOutlookBar())
	{
		TRACE0("Failed to create navigation pane\n");
		return -1;      // fail to create
	}
	EnableDocking(CBRS_ALIGN_LEFT);											// Outlook bar is created and docking on the left side should be allowed.
	EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}
	// enable docking and attach 
	// TODO - see model VS as they have more calls here
	m_wndFilter.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndFilter);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	// tools bar
	if (!m_wndToolBar.CreateEx(this, 
		TBSTYLE_FLAT, 
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	CString strToolBarName;
	BOOL bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	//m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	//DockPane(&m_wndMenuBar);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	//// Enable toolbar and docking window menu replacement
	//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}

#endif //_DEBUG


BOOL CMainFrame::CreateDockingWindows()
{
	// Create filter view
	CString strFilterView;
	BOOL bNameValid = strFilterView.LoadString(IDS_FILTERPANE);
	ASSERT(bNameValid);
	if (!m_wndFilter.Create(strFilterView, this, CRect(0, 0, 200, 200), TRUE, 
		ID_PANE_FILTERWND,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create filter View window\n");
		return FALSE; // failed to create
	}

	// Create properties window
	CString strPropertiesView;
	bNameValid = strPropertiesView.LoadString(IDS_PROPERTIESPANE);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesView, this, CRect(0, 0, 200, 200), TRUE, 
		ID_PANE_PROPERTIESWND,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create properties View window\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}


void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFilterPaneIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(),
		MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW),
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFilter.SetIcon(hFilterPaneIcon, FALSE);

	HICON hPropertiesPaneIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(),
		MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND),
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesPaneIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

CdbWaveDoc * CMainFrame::GetMDIActiveDocument()
{
	CMDIChildWnd *pChild = MDIGetActive();
	if (pChild == NULL)
		return NULL;
	CDaoRecordView *pView = (CDaoRecordView*)pChild->GetActiveView();
	return (CdbWaveDoc*)pView->GetDocument();
}


BOOL CMainFrame::CreateOutlookBar()
{
	CMFCOutlookBarTabCtrl::EnableAnimation();
	const int nInitialWidth = 60;
	const CString strCaption = _T("Shortcuts");
	if (!m_wndOutlookBar.Create(strCaption, this, CRect(0, 0, nInitialWidth, nInitialWidth), ID_VIEW_OUTLOOKBAR, WS_CHILD | WS_VISIBLE | CBRS_LEFT))
		return FALSE;      // fail to create

						   // create the choices toolbar and fill it with images, buttons and text
	CMFCOutlookBarTabCtrl* pShortcutsBarContainer = DYNAMIC_DOWNCAST(CMFCOutlookBarTabCtrl, m_wndOutlookBar.GetUnderlyingWindow());
	if (pShortcutsBarContainer == NULL)
		return FALSE;

	// add images to this bar
	CImageList	img1;
	
	img1.Create(IDB_NAVIGATIONLARGE, 32, 0, RGB(255, 0, 255));

#define NBUTTONS 8	
	WORD dwStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	struct {
		UINT id;                            // command ID
		UINT strid;							// string ID
		UINT style;                         // button style
		UINT iImage;                        // index of image in normal/hot bitmaps
	} Buttons[NBUTTONS] = {
		// command ID              button style                        image index
		{ ID_VIEW_DATABASE,				IDS_BTTNDATABASE,	dwStyle, 0 },
		{ ID_VIEW_DATAFILE,				IDS_BTTNDATA,		dwStyle, 1 },
		{ ID_VIEW_SPIKEDETECTION,		IDS_BTTNDETECT,		dwStyle, 2 },
		{ ID_VIEW_SPIKEDISPLAY,			IDS_BTTNSPIKES,		dwStyle, 3 },
		{ ID_VIEW_SPIKESORTINGAMPLITUDE,IDS_BTTNSORT,		dwStyle, 4 },
		{ ID_VIEW_SPIKESORTINGTEMPLATES,IDS_BTTNTEMPLATES,	dwStyle, 5 },
		{ ID_VIEW_SPIKETIMESERIES,		IDS_BTTNTIMESERIES,	dwStyle, 6 },
		{ ID_VIEW_ACQUIREDATA,			IDS_BTTNACQDATA,	dwStyle, 7 }
	};

	// Create first page:
	m_wndOutlookPane.Create(&m_wndOutlookBar, AFX_DEFAULT_TOOLBAR_STYLE, ID_PANE_OUTLOOKBAR);
	m_wndOutlookPane.SetOwner(this);
	m_wndOutlookPane.EnableTextLabels();
	m_wndOutlookPane.EnableDocking(CBRS_ALIGN_ANY);

	for (int i = 0; i<NBUTTONS; i++)
	{
		CString str;
		if (!str.LoadString(Buttons[i].strid))
			str = _T("??");
		m_wndOutlookPane.AddButton(img1.ExtractIconW(i), str, Buttons[i].id);
		m_wndOutlookPane.SetButtonInfo(i,
			Buttons[i].id,                 // command id
			Buttons[i].style,              // buttons style
			Buttons[i].iImage);            // index of image in bitmap
	}
	pShortcutsBarContainer->AddTab(&m_wndOutlookPane, _T("Views"), (UINT)-1, FALSE);
	img1.Detach();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}


void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;
	theApp.m_nAppLook = id;
	BOOL bWindows7 = FALSE;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;
	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;
	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;
	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;
	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;
	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;
	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		bWindows7 = TRUE;
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;
		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;
		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;
		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	m_wndRibbonBar.SetWindows7Look(bWindows7);
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}


void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}


void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}


LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp, lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}


void CMainFrame::OnOptions()
{
	CMFCRibbonCustomizeDialog *pOptionsDlg = new CMFCRibbonCustomizeDialog(this, &m_wndRibbonBar);
	ASSERT(pOptionsDlg != NULL);

	pOptionsDlg->DoModal();
	delete pOptionsDlg;
}


BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
		return FALSE;

	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}


void CMainFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndProperties.ShowPane(TRUE, FALSE, TRUE);
	m_wndProperties.SetFocus();
	m_bPropertiesPaneVisible = TRUE;
}


void CMainFrame::OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	m_bPropertiesPaneVisible = TRUE;
}


void CMainFrame::OnViewFilterWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndFilter.ShowPane(TRUE, FALSE, TRUE);
	m_wndFilter.SetFocus();
	m_bFilterPaneVisible = TRUE;
}


void CMainFrame::OnUpdateViewFilterWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	m_bFilterPaneVisible = TRUE;
}


void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_wndProperties.OnUpdate(pSender, lHint, pHint);
	m_wndFilter.OnUpdate(pSender, lHint, pHint);
}


LRESULT CMainFrame::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// pass message to PropertiesPane
	m_wndProperties.OnMyMessage(wParam, lParam);
	m_wndFilter.OnMyMessage(wParam, lParam);
	return 0L;
}


void CMainFrame::OnCheckFilterpane()
{
	m_bFilterPaneVisible = !m_bFilterPaneVisible;
	m_wndFilter.ShowPane(m_bFilterPaneVisible, FALSE, TRUE);
}


void CMainFrame::OnUpdateCheckFilterpane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bFilterPaneVisible);
}


void CMainFrame::OnCheckPropertiespane()
{
	m_bPropertiesPaneVisible = !m_bPropertiesPaneVisible;
	m_wndProperties.ShowPane(m_bPropertiesPaneVisible, FALSE, TRUE);
}


void CMainFrame::OnUpdateCheckPropertiespane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bPropertiesPaneVisible);
}



#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

void CMainFrame::OnToolsCompactdatabase()
{
	CString fileName;
	wchar_t* p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
	CFileDialog dlg(TRUE);

	OPENFILENAME& ofn = dlg.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;
	ofn.lpstrFilter = _T("MDB Files (*.mdb)\0*.mdb\0\0");
	ofn.lpstrTitle = _T("Select MDB File");
	INT_PTR result = dlg.DoModal();
	fileName.ReleaseBuffer();

	if (result == IDOK)
	{
		fileName = dlg.GetPathName(); // return full path and filename
		int ipos = fileName.ReverseFind('.');
		CString fileName_new = fileName.Left(ipos) + _T("_new.mdb");

		// compact database and save new file
		CDaoWorkspace::CompactDatabase(fileName, fileName_new, dbLangGeneral, 0);

		CString cs;
		cs = fileName + _T(" database compacted and saved as ") + fileName_new;
		AfxMessageBox(cs);
	}
}
