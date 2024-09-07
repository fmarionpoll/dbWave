#pragma once
#include <afxpropertygridctrl.h>

class SpikeClassGridProperty :
    public CMFCPropertyGridProperty
{
public:
    SpikeClassGridProperty(const CString& str_name, const COleVariant& var_value, LPCTSTR lpsz_descr = nullptr, DWORD_PTR dw_data = 0, LPCTSTR lpsz_edit_mask = nullptr, LPCTSTR lpsz_edit_template = nullptr, LPCTSTR lpsz_valid_chars = nullptr);

    static SpikeClassGridProperty* create(int class_id);

    void OnDrawName(CDC* p_dc, CRect rect) override;
};
