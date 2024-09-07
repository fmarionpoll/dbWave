#pragma once
#include <afxpropertygridctrl.h>

#include "ChartWnd.h"
#include "PaneldbPropertiesToolBar.h"

class SpikeClassGrid :
    public CMFCPropertyGridCtrl
{

protected:
    //PaneldbPropertiesToolBar m_wndFormatBar;
    //int m_wnd_edit_infos_height_{ 0 };
    //CFont m_fnt_prop_list_;

    //void property_grid_toolbar_init();
    //void AdjustLayout() override;
    //void set_prop_list_font();

public:
    SpikeClassGrid()
    {
        m_nLeftColumnWidth = 10;
    }

public:

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

protected:
    afx_msg void OnSize(UINT n_type, int cx, int cy);
    //afx_msg int OnCreate(LPCREATESTRUCT lp_create_struct);


    DECLARE_MESSAGE_MAP()

public:
	//int OnDrawProperty(CDC* p_dc, CMFCPropertyGridProperty* p_prop) const override;
};




