#include "stdafx.h"
#include "CMFCMyPropertyGridCtrl.h"


BEGIN_MESSAGE_MAP(CMFCMyPropertyGridCtrl, CMFCPropertyGridCtrl)
    ON_WM_SIZE()
END_MESSAGE_MAP()

void CMFCMyPropertyGridCtrl::OnSize(UINT n_type, const int cx, int cy)
{
    EndEditItem();
    AdjustLayout();
}

//int CMFCMyPropertyGridCtrl::OnDrawProperty(CDC* p_dc, CMFCPropertyGridProperty* p_prop) const
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

