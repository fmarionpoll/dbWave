#pragma once
#include <afxpropertygridctrl.h>

class CMFCMyPropertyGridProperty :
    public CMFCPropertyGridProperty
{
public:
    CMFCMyPropertyGridProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = nullptr, DWORD_PTR dwData = 0,
        LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr);

    void OnDrawName(CDC* p_dc, CRect rect) override;
    //void OnDrawValue(CDC* p_dc, CRect rect) override;
    //void OnDrawButton(CDC* p_dc, CRect rect) override;
    //void OnDrawDescription(CDC* p_dc, CRect rect) override;
};
