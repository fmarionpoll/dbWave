#pragma once
#include <afxpropertygridctrl.h>

#include "ChartWnd.h"

class CMFCMyPropertyGridCtrl :
    public CMFCPropertyGridCtrl
{

public:
    CMFCMyPropertyGridCtrl()
    {
        m_nLeftColumnWidth = 10;
    }


    void make_fixed_header()
    {
        HDITEM hd_item = { 0 };
        hd_item.mask = HDI_FORMAT;
        GetHeaderCtrl().GetItem(0, &hd_item);
        hd_item.fmt |= HDF_FIXEDWIDTH | HDF_CENTER;
        GetHeaderCtrl().SetItem(0, &hd_item);
    }

    void set_left_column_width(const int cx)
    {
        m_nLeftColumnWidth = cx;
        AdjustLayout();
    }


    afx_msg void OnSize(UINT n_type, int cx, int cy);

    DECLARE_MESSAGE_MAP()

public:
	//int OnDrawProperty(CDC* p_dc, CMFCPropertyGridProperty* p_prop) const override;
};




