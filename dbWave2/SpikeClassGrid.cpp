#include "stdafx.h"
#include "SpikeClassGrid.h"

#include "dbWave.h"


BEGIN_MESSAGE_MAP(SpikeClassGrid, CMFCPropertyGridCtrl)
	//ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void SpikeClassGrid::OnSize(UINT n_type, const int cx, int cy)
{
    EndEditItem();
    AdjustLayout();
}

//int SpikeClassGrid::OnDrawProperty(CDC* p_dc, CMFCPropertyGridProperty* p_prop) const
//{
//	const int class_index = p_prop->GetData();
//	const COLORREF color = ChartWnd::color_spike_class[class_index];
//	p_dc->SetBkMode(OPAQUE);
//	p_dc->SetBkColor(color);
//
//	const COLORREF color_text = ChartWnd::color_spike_class_text[class_index];
//	p_dc->SetTextColor(color_text);
//
//	return CMFCPropertyGridCtrl::OnDrawProperty(p_dc, p_prop);
//}

//void SpikeClassGrid::property_grid_toolbar_init()
//{
//	// attach command routing to dialog window
//	m_wndFormatBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
//	m_wndFormatBar.LoadToolBar(IDR_PROPERTIES);
//	m_wndFormatBar.CleanUpLockedImages();
//
//	m_wndFormatBar.LoadBitmap(the_app.hi_color_icons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
//
//	m_wndFormatBar.SetPaneStyle(m_wndFormatBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
//	m_wndFormatBar.SetPaneStyle(
//		m_wndFormatBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
//			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
//
//	// All commands will be routed via this control, not via the parent frame:
//	m_wndFormatBar.SetOwner(this);
//	m_wndFormatBar.SetRouteCommandsViaFrame(FALSE);
//
//
//	// Calculate size of toolbar and adjust size of static control to fit size
//	AdjustLayout();
//}

//int SpikeClassGrid::OnCreate(const LPCREATESTRUCT lp_create_struct)
//{
//	if (CMFCPropertyGridCtrl::OnCreate(lp_create_struct) == -1)
//		return -1;
//
//	//const CRect rect_dummy(0, 0, 24, 24);
//	//if (!m_wnd_prop_list_.Create(WS_VISIBLE | WS_CHILD, rect_dummy, this, 2))
//	//	return -1; // fail to create
//	set_prop_list_font();
//
//	m_wndFormatBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
//	m_wndFormatBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
//
//	m_wndFormatBar.CleanUpLockedImages();
//	m_wndFormatBar.LoadBitmap(the_app.hi_color_icons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
//
//	m_wndFormatBar.SetPaneStyle(m_wndFormatBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
//	m_wndFormatBar.SetPaneStyle(
//		m_wndFormatBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
//			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
//
//	// All commands will be routed via this control, not via the parent frame:
//	m_wndFormatBar.SetOwner(this);
//	m_wndFormatBar.SetRouteCommandsViaFrame(FALSE);
//
//	AdjustLayout();
//	return 0;
//}
//
//void SpikeClassGrid::AdjustLayout()
//{
//	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
//		return;
//
//	CRect rect_client;
//	GetClientRect(rect_client);
//	const int cy_tlb = 50; // m_wndFormatBar.CalcFixedLayout(FALSE, TRUE).cy;
//
//	m_wndFormatBar.SetWindowPos(nullptr, rect_client.left,
//		rect_client.top + m_wnd_edit_infos_height_,
//		rect_client.Width(),
//		cy_tlb, SWP_NOACTIVATE | SWP_NOZORDER);
//
//	this->SetWindowPos(nullptr, rect_client.left,
//		rect_client.top + m_wnd_edit_infos_height_ + cy_tlb,
//		rect_client.Width(),
//		rect_client.Height() - m_wnd_edit_infos_height_ - cy_tlb,
//		SWP_NOACTIVATE | SWP_NOZORDER);
//}
//
//void SpikeClassGrid::set_prop_list_font()
//{
//	DeleteObject(m_fnt_prop_list_.Detach());
//
//	LOGFONT lf;
//	afxGlobalData.fontRegular.GetLogFont(&lf);
//
//	NONCLIENTMETRICS info;
//	info.cbSize = sizeof(info);
//	afxGlobalData.GetNonClientMetrics(info);
//	lf.lfHeight = info.lfMenuFont.lfHeight;
//	lf.lfWeight = info.lfMenuFont.lfWeight;
//	lf.lfItalic = info.lfMenuFont.lfItalic;
//	m_fnt_prop_list_.CreateFontIndirect(&lf);
//	this->SetFont(&m_fnt_prop_list_);
//}

