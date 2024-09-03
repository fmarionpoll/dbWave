#include "stdafx.h"
#include "CMFCMyPropertyGridProperty.h"

#include "ChartWnd.h"

CMFCMyPropertyGridProperty::CMFCMyPropertyGridProperty(const CString& strName, const COleVariant& varValue,
                                                       LPCTSTR lpszDescr, DWORD_PTR dwData, LPCTSTR lpszEditMask, LPCTSTR lpszEditTemplate, LPCTSTR lpszValidChars) :
	CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
{
}

void CMFCMyPropertyGridProperty::OnDrawName(CDC* p_dc, const CRect rect)
{
	const int class_index = GetData();
	const COLORREF color = ChartWnd::color_spike_class[class_index];
	p_dc->SetBkMode(OPAQUE);
	p_dc->SetBkColor(color);

	const COLORREF color_text = ChartWnd::color_spike_class_text[class_index];
	p_dc->SetTextColor(color_text);

	CMFCPropertyGridProperty::OnDrawDescription(p_dc, rect);
}