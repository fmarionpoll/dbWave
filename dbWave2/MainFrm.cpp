#include "StdAfx.h"
#include "MainFrm.h"
//#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "ColorNames.h"
#include "ViewDbTable.h"
#include "dbWave.h"
#include "dbWave_constants.h"

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

constexpr int i_max_user_toolbars = 10;
constexpr UINT ui_first_user_tool_bar_id = AFX_IDW_CONTROLBAR_FIRST + 40;
constexpr UINT ui_last_user_tool_bar_id = ui_first_user_tool_bar_id + i_max_user_toolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7,
	                           &CMainFrame::OnUpdateApplicationLook)

	ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnOptions)

	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)

	ON_COMMAND(ID_PANE_PROPERTIESWND, &CMainFrame::OnViewPropertiesWindow)
	ON_UPDATE_COMMAND_UI(ID_PANE_PROPERTIESWND, &CMainFrame::OnUpdateViewPropertiesWindow)
	ON_COMMAND(ID_PANE_FILTERWND, &CMainFrame::OnViewFilterWindow)
	ON_UPDATE_COMMAND_UI(ID_PANE_FILTERWND, &CMainFrame::OnUpdateViewFilterWindow)
	ON_MESSAGE(WM_MYMESSAGE, &CMainFrame::OnMyMessage)
	ON_WM_SETTINGCHANGE()

	ON_COMMAND(ID_CHECK_FILTER_PANE, &CMainFrame::OnCheckFilterpane)
	ON_UPDATE_COMMAND_UI(ID_CHECK_FILTER_PANE, &CMainFrame::OnUpdateCheckFilterpane)
	ON_COMMAND(ID_CHECK_PROPERTIES_PANE, &CMainFrame::OnCheckPropertiespane)
	ON_UPDATE_COMMAND_UI(ID_CHECK_PROPERTIES_PANE, &CMainFrame::OnUpdateCheckPropertiespane)

	ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWndEx::OnHelpFinder)
	ON_COMMAND(ID_HELP, &CMDIFrameWndEx::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWndEx::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWndEx::OnHelpFinder)

END_MESSAGE_MAP()


CMainFrame::CMainFrame()
{
	m_second_tool_bar_id = 0;
	m_p_second_tool_bar = nullptr;
	is_properties_panel_visible_ = TRUE;
	is_filter_pane_visible_ = TRUE;
	the_app.app_look = the_app.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
}

CMainFrame::~CMainFrame()
{
	outlook_pane_.ClearAll();
	outlook_pane_.RemoveAllButtons();
	SAFE_DELETE(m_p_second_tool_bar)
}

void CMainFrame::OnDestroy()
{
	CMDIFrameWndEx::OnDestroy();
}

void CMainFrame::ActivatePropertyPane(const BOOL b_activate)
{
	if (b_activate != panel_db_properties_.IsVisible())
		panel_db_properties_.ShowPane(b_activate, FALSE, FALSE);
}

void CMainFrame::ActivateFilterPane(const BOOL b_activate)
{
	if (b_activate != panel_db_filter_.IsVisible())
		panel_db_filter_.ShowPane(b_activate, FALSE, FALSE);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMDITabInfo mdi_tab_params;
	mdi_tab_params.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdi_tab_params.m_bActiveTabCloseButton = TRUE; // set to FALSE to place close button at right of tab area
	mdi_tab_params.m_bTabIcons = FALSE; // set to TRUE to enable document icons on MDI tabs
	mdi_tab_params.m_bAutoColor = TRUE; // set to FALSE to disable auto-coloring of MDI tabs
	mdi_tab_params.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdi_tab_params);

	// Create the ribbon bar
	if (!ribbon_bar_.Create(this))
	{
		TRACE0("Failed to create ribbon bar\n");
		return -1; //Failed to create ribbon bar
	}
	ribbon_bar_.LoadFromResource(IDR_RIBBON);

	// status bar
	if (!status_bar_.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // fail to create
	}
	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);

	// choices bar
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);
	if (!create_outlook_bar())
	{
		TRACE0("Failed to create navigation pane\n");
		return -1; // fail to create
	}
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, the_app.hi_color_icons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!create_docking_panes())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}
	// enable docking and attach
	// TODO - see model VS as they have more calls here
	panel_db_filter_.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&panel_db_filter_);

	panel_db_properties_.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&panel_db_properties_);

	// set the visual manager and style based on persisted value
	OnApplicationLook(the_app.app_look);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	//// Enable toolbar and docking window menu replacement
	//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the task_bar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWndEx::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

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

BOOL CMainFrame::create_docking_panes()
{
	// Create filter pane
	CString str_filter_view;
	auto b_name_valid = str_filter_view.LoadString(IDS_FILTERPANE);
	ASSERT(b_name_valid);
	if (!panel_db_filter_.Create(str_filter_view, this, CRect(0, 0, 200, 200), TRUE,
	                         ID_PANE_FILTERWND,
	                         WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create filter properties window\n");
		return FALSE; // failed to create
	}

	// Create properties pane
	CString str_properties_view;
	b_name_valid = str_properties_view.LoadString(IDS_PROPERTIESPANE);
	ASSERT(b_name_valid);
	if (!panel_db_properties_.Create(str_properties_view, this, CRect(0, 0, 200, 200), TRUE,
	                             ID_PANE_PROPERTIESWND,
	                             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT |
	                             CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create record properties window\n");
		return FALSE; // failed to create
	}

	set_docking_panes_icons(the_app.hi_color_icons);
	return TRUE;
}

void CMainFrame::set_docking_panes_icons(BOOL b_hi_color_icons)
{
	const auto h_filter_pane_icon = static_cast<HICON>(::LoadImage(AfxGetResourceHandle(),
	                                                               MAKEINTRESOURCE(
		                                                               b_hi_color_icons ? IDI_FILE_VIEW_HC :
		                                                               IDI_FILE_VIEW),
	                                                               IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
	                                                               GetSystemMetrics(SM_CYSMICON), 0));
	panel_db_filter_.SetIcon(h_filter_pane_icon, FALSE);

	const auto h_properties_pane_icon = static_cast<HICON>(::LoadImage(AfxGetResourceHandle(),
	                                                                   MAKEINTRESOURCE(
		                                                                   b_hi_color_icons ? IDI_PROPERTIES_WND_HC :
		                                                                   IDI_PROPERTIES_WND),
	                                                                   IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
	                                                                   GetSystemMetrics(SM_CYSMICON), 0));
	panel_db_properties_.SetIcon(h_properties_pane_icon, FALSE);

	UpdateMDITabbedBarsIcons();
}

CdbWaveDoc* CMainFrame::GetMDIActiveDocument()
{
	const auto p_child = MDIGetActive();
	if (p_child == nullptr)
		return nullptr;
	const auto db_table_view = static_cast<ViewDbTable*>(p_child->GetActiveView());
	return db_table_view->GetDocument();
}

BOOL CMainFrame::create_outlook_bar()
{
	CMFCOutlookBarTabCtrl::EnableAnimation();
	constexpr auto n_initial_width = 60;
	const CString str_caption = _T("Shortcuts");
	if (!outlook_bar_.Create(str_caption, this, CRect(0, 0, n_initial_width, n_initial_width), ID_VIEW_OUTLOOKBAR,
	                            WS_CHILD | WS_VISIBLE | CBRS_LEFT))
		return FALSE; // fail to create

	// create the choices toolbar and fill it with images, buttons and text
	auto* p_shortcuts_bar_container = DYNAMIC_DOWNCAST(CMFCOutlookBarTabCtrl, outlook_bar_.GetUnderlyingWindow());
	if (p_shortcuts_bar_container == nullptr)
		return FALSE;

	// add images to this bar
	CImageList img1;

	img1.Create(IDB_NAVIGATIONLARGE, 32, 0, col_fuchsia);

#define N_BUTTONS 8
	constexpr WORD dw_style = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	constexpr struct
	{
		UINT id;			// command ID
		UINT string_id;		// string ID
		UINT style;			// button style
		int i_image;		// index of image in normal/hot bitmaps
	} buttons[N_BUTTONS] = {
			// command ID              button style                        image index
			{ID_VIEW_DATABASE, IDS_BTTNDATABASE, dw_style, 0},
			{ID_VIEW_DATA_FILE, IDS_BTTNDATA, dw_style, 1},
			{ID_VIEW_SPIKE_DETECTION, IDS_BTTNDETECT, dw_style, 2},
			{ID_VIEW_SPIKE_DISPLAY, IDS_BTTNSPIKES, dw_style, 3},
			{ID_VIEW_SPIKE_SORTING_AMPLITUDE,IDS_BTTNSORT, dw_style, 4},
			{ID_VIEW_SPIKE_SORTING_TEMPLATES,IDS_BTTNTEMPLATES, dw_style, 5},
			{ID_VIEW_SPIKE_TIME_SERIES, IDS_BTTNTIMESERIES, dw_style, 6},
			{ID_VIEW_ACQUIRE_DATA, IDS_BTTNACQDATA, dw_style, 7}
		};

	// Create first page:
	outlook_pane_.Create(&outlook_bar_, AFX_DEFAULT_TOOLBAR_STYLE, ID_PANE_OUTLOOKBAR);
	outlook_pane_.SetOwner(this);
	outlook_pane_.EnableTextLabels();
	outlook_pane_.EnableDocking(CBRS_ALIGN_ANY);

	for (int i = 0; i < N_BUTTONS; i++)
	{
		CString str;
		if (!str.LoadString(buttons[i].string_id))
			str = _T("??");
		outlook_pane_.AddButton(img1.ExtractIconW(i), str, buttons[i].id);
		outlook_pane_.SetButtonInfo(i,
		                               buttons[i].id, // command id
		                               buttons[i].style, // buttons style
		                               buttons[i].i_image); // index of image in bitmap
	}
	p_shortcuts_bar_container->AddTab(&outlook_pane_, _T("Views"), static_cast<UINT>(-1), FALSE);
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
	the_app.app_look = id;
	auto b_windows7 = FALSE;

	switch (the_app.app_look)
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
		b_windows7 = TRUE;
		break;

	default:
		switch (the_app.app_look)
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
		default: ;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	ribbon_bar_.SetWindows7Look(b_windows7);
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
	the_app.WriteInt(_T("ApplicationLook"), static_cast<int>(the_app.app_look));
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(the_app.app_look == pCmdUI->m_nID);
}

void CMainFrame::OnViewCustomize()
{
	auto p_dlg_customize_dialog = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	p_dlg_customize_dialog->EnableUserDefinedToolbars();
	p_dlg_customize_dialog->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp)
{
	const auto l_result_create_toolbar = CMDIFrameWndEx::OnToolbarCreateNew(wp, lp);
	if (l_result_create_toolbar == 0)
		return 0;

	auto* p_user_toolbar = reinterpret_cast<CMFCToolBar*>(l_result_create_toolbar);
	ASSERT_VALID(p_user_toolbar);

	CString str_customize;
	const auto b_name_valid = str_customize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(b_name_valid);

	p_user_toolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, str_customize);
	return l_result_create_toolbar;
}

void CMainFrame::OnOptions()
{
	const auto p_options_dlg = new CMFCRibbonCustomizeDialog(this, &ribbon_bar_);
	ASSERT(p_options_dlg != NULL);

	p_options_dlg->DoModal();
	delete p_options_dlg;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
		return FALSE;

	CString str_customize;
	const auto b_name_valid = str_customize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(b_name_valid);

	for (auto i = 0; i < i_max_user_toolbars; i++)
	{
		const auto p_user_toolbar = GetUserToolBarByIndex(i);
		if (p_user_toolbar != nullptr)
		{
			p_user_toolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, str_customize);
		}
	}

	return TRUE;
}

void CMainFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	panel_db_properties_.ShowPane(TRUE, FALSE, TRUE);
	panel_db_properties_.SetFocus();
	is_properties_panel_visible_ = TRUE;
}

void CMainFrame::OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	is_properties_panel_visible_ = TRUE;
}

void CMainFrame::OnViewFilterWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	panel_db_filter_.ShowPane(TRUE, FALSE, TRUE);
	panel_db_filter_.SetFocus();
	is_filter_pane_visible_ = TRUE;
}

void CMainFrame::OnUpdateViewFilterWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	is_filter_pane_visible_ = TRUE;
}

void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	panel_db_properties_.OnUpdate(pSender, lHint, pHint);
	panel_db_filter_.OnUpdate(pSender, lHint, pHint);
}

LRESULT CMainFrame::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// pass message to PropertiesPane
	panel_db_properties_.on_my_message(wParam, lParam);
	panel_db_filter_.on_my_message(wParam, lParam);
	return 0L;
}

void CMainFrame::OnCheckFilterpane()
{
	is_filter_pane_visible_ = !is_filter_pane_visible_;
	panel_db_filter_.ShowPane(is_filter_pane_visible_, FALSE, TRUE);
}

void CMainFrame::OnUpdateCheckFilterpane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(is_filter_pane_visible_);
}

void CMainFrame::OnCheckPropertiespane()
{
	is_properties_panel_visible_ = !is_properties_panel_visible_;
	panel_db_properties_.ShowPane(is_properties_panel_visible_, FALSE, TRUE);
}

void CMainFrame::OnUpdateCheckPropertiespane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(is_properties_panel_visible_);
}
