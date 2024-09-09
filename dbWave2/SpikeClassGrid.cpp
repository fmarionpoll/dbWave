#include "stdafx.h"
#include "SpikeClassGrid.h"

#include "dbWave.h"
#include "dbWave_constants.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(SpikeClassGrid, CWnd)

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameters

SpikeClassGrid::SpikeClassGrid()
= default;

SpikeClassGrid::~SpikeClassGrid()
= default;

BEGIN_MESSAGE_MAP(SpikeClassGrid, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_COMMAND(ID_EXPAND_ALL, on_expand_all_properties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, on_update_expand_all_properties)
	ON_COMMAND(ID_SORT_PROPERTIES, on_sort_properties)
	ON_UPDATE_COMMAND_UI(ID_SORT_PROPERTIES, on_update_sort_properties)
	ON_BN_CLICKED(IDC_EDIT_INFOS, on_bn_clicked_edit_infos)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_INFOS, on_update_bn_edit_infos)
	ON_BN_CLICKED(IDC_UPDATE_INFOS, on_bn_clicked_update_infos)
	ON_UPDATE_COMMAND_UI(IDC_UPDATE_INFOS, on_update_bn_update_infos)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, on_property_changed)
END_MESSAGE_MAP()

void SpikeClassGrid::adjust_layout()
{
	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rect;
	GetWindowRect(&rect);
	const int cy = get_vertical_size_with_all_rows();
	int rect_height = rect.Height();
	if (cy != rect_height)
	{
		GetParent()->ScreenToClient(&rect);
		SetWindowPos(nullptr, rect.left, rect.top, rect.Width(),
			cy, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect rect_client;
	GetClientRect(&rect_client);
	const int cy_tlb = property_toolbar_.CalcFixedLayout(FALSE, TRUE).cy;

	property_toolbar_.SetWindowPos(nullptr, rect_client.left,
		rect_client.top + m_wnd_edit_infos_height_,
		rect_client.Width(),
		cy_tlb, SWP_NOACTIVATE | SWP_NOZORDER);

	property_list_.SetWindowPos(nullptr, rect_client.left,
		rect_client.top + m_wnd_edit_infos_height_ + cy_tlb,
		rect_client.Width(),
		rect_client.Height() - m_wnd_edit_infos_height_ - cy_tlb,
		SWP_NOACTIVATE | SWP_NOZORDER);
}

int SpikeClassGrid::OnCreate(const LPCREATESTRUCT lp_create_struct)
{
	if (CWnd::OnCreate(lp_create_struct) == -1)
		return -1;

	create_grid();
	return 0;
}

int SpikeClassGrid::get_vertical_size_with_all_rows()
{
	const int cy_tlb = property_toolbar_.CalcFixedLayout(FALSE, TRUE).cy;
	const int n_items = property_list_.GetPropertyCount() +1;
	const int cy_header = property_list_.GetHeaderHeight();
	const int cy_row_height = property_list_.GetRowHeight();
	const int cy_total = cy_row_height * n_items  + cy_header + cy_tlb;
	return cy_total;
}

void SpikeClassGrid::update_list(SpikeList* p_spike_list)
{
	const int n_items = property_list_.GetPropertyCount();
	property_list_.update(p_spike_list);
	if (property_list_.GetPropertyCount() != n_items)
		adjust_layout();
}

void SpikeClassGrid::OnSize(const UINT n_type, const int cx, int cy)
{
	const int cy_total = get_vertical_size_with_all_rows();
	if (cy != cy_total)
		cy = cy_total;

	CWnd::OnSize(n_type, cx, cy);
	adjust_layout();
}

void SpikeClassGrid::on_expand_all_properties()
{
	property_list_.ExpandAll();
}

void SpikeClassGrid::on_update_expand_all_properties(CCmdUI* /* pCmdUI */)
{
}

void SpikeClassGrid::on_sort_properties()
{
	property_list_.SetAlphabeticMode(!property_list_.IsAlphabeticMode());
}

void SpikeClassGrid::on_update_sort_properties(CCmdUI * p_cmd_ui)
{
	p_cmd_ui->SetCheck(property_list_.IsAlphabeticMode());
}

// look at elements indexes stored into m_nCol and create property grid
// indexes are either positive or -1 (-1 is used as a stop tag)
// init all elements pointed at within m_noCol table from element i_col0 to element = -1 (stop value)
// returns next position after the stop tag

BOOL SpikeClassGrid::create_window(CWnd* p_parent)
{
	CString str_properties_view;
	BOOL b_name_valid = str_properties_view.LoadString(IDS_PROPERTIESPANE);
	ASSERT(b_name_valid);
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT |
		CBRS_FLOAT_MULTI;
	BOOL flag = Create(str_properties_view,
		_T("spike classes properties"), dwStyle,
		CRect(0, 0, 200, 200), p_parent,
		ID_SPIKE_CLASS_LIST,
		nullptr);
	if (!flag)
	{
		TRACE0("Failed to create record properties window\n");
	}
	return flag;
}

BOOL SpikeClassGrid::create_grid()
{
	const CRect rect_dummy(0, 0, 24, 24);
	const BOOL flag = property_list_.Create(WS_VISIBLE | WS_CHILD, rect_dummy, this, 2);
	if (!flag)
		return -1; // fail to create
	set_prop_list_font();

	property_toolbar_.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	property_toolbar_.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);

	property_toolbar_.CleanUpLockedImages();
	property_toolbar_.LoadBitmap(the_app.hi_color_icons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	property_toolbar_.SetPaneStyle(property_toolbar_.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	property_toolbar_.SetPaneStyle(
		property_toolbar_.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	// All commands will be routed via this control, not via the parent frame:
	property_toolbar_.SetOwner(this);
	property_toolbar_.SetRouteCommandsViaFrame(FALSE);

	adjust_layout();

	return flag;
}

void SpikeClassGrid::OnSetFocus(CWnd * p_old_wnd)
{
	CWnd::OnSetFocus(p_old_wnd);
	property_list_.SetFocus();
}

void SpikeClassGrid::OnSettingChange(const UINT u_flags, const LPCTSTR lpsz_section)
{
	CWnd::OnSettingChange(u_flags, lpsz_section);
	set_prop_list_font();
}

void SpikeClassGrid::set_prop_list_font()
{
	DeleteObject(m_fnt_prop_list_.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	afxGlobalData.GetNonClientMetrics(info);
	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;
	m_fnt_prop_list_.CreateFontIndirect(&lf);
	property_list_.SetFont(&m_fnt_prop_list_);
}

void SpikeClassGrid::on_update_bn_edit_infos(CCmdUI * p_cmd_ui)
{
}

void SpikeClassGrid::on_bn_clicked_edit_infos()
{
	//m_p_doc_->update_all_views_db_wave(nullptr, HINT_GET_SELECTED_RECORDS, nullptr);
	//DlgdbEditRecord dlg;
	//dlg.m_pdb_doc = m_p_doc_;
	//if (IDOK == dlg.DoModal())
	//{
	//	m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
	//	m_p_doc_->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
	//}
}

void SpikeClassGrid::on_update_bn_update_infos(CCmdUI * p_cmd_ui)
{
	//p_cmd_ui->Enable(m_b_changed_property_);
	p_cmd_ui->Enable(TRUE);
}

void SpikeClassGrid::on_bn_clicked_update_infos()
{
	GetParent()->SendMessage(WM_MYMESSAGE, HINT_SAVE_SPIKEFILE, MAKELONG(0, GetDlgCtrlID()));
}

LRESULT SpikeClassGrid::on_property_changed(WPARAM, LPARAM lParam)
{
	//auto p_prop = reinterpret_cast<CMFCPropertyGridProperty*>(lParam);
	m_b_changed_property_ = TRUE;
	return 0;
}


