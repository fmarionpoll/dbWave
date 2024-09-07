#include "stdafx.h"
#include "SpikeClassGridProperty.h"

#include "ChartWnd.h"
#include "SpikeClassProperties.h"

SpikeClassGridProperty::SpikeClassGridProperty(const CString& str_name, const COleVariant& var_value,
                                                       LPCTSTR lpsz_descr, DWORD_PTR dw_data, LPCTSTR lpsz_edit_mask, LPCTSTR lpsz_edit_template, LPCTSTR lpsz_valid_chars) :
	CMFCPropertyGridProperty(str_name, var_value, lpsz_descr, dw_data, lpsz_edit_mask, lpsz_edit_template, lpsz_valid_chars)
{
}

SpikeClassGridProperty* SpikeClassGridProperty::create(const int class_id)
{
	CString cs;
	cs.Format(_T(" %i "), class_id);

	const COleVariant var_value = _T("select/edit..");

	auto* p_prop = new SpikeClassGridProperty(cs, var_value, _T(""));
	p_prop->SetData(class_id);
	for (auto& i : SpikeClassProperties::class_descriptor)
		p_prop->AddOption(i);
	p_prop->AllowEdit(TRUE);

	const int index = class_id;
	if (index < p_prop->GetOptionCount())
		p_prop->SetValue(p_prop->GetOption(class_id));

	return p_prop;
}


void SpikeClassGridProperty::OnDrawName(CDC* p_dc, const CRect rect)
{
	const int class_index = static_cast<int>(GetData());
	const COLORREF color = ChartWnd::color_spike_class[class_index];
	p_dc->SetBkMode(OPAQUE);
	p_dc->SetBkColor(color);

	const COLORREF color_text = ChartWnd::color_spike_class_text[class_index];
	p_dc->SetTextColor(color_text);

	CMFCPropertyGridProperty::OnDrawDescription(p_dc, rect);
}
