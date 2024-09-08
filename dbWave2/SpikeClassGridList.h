#pragma once
#include <afxpropertygridctrl.h>
#include "SpikeClassGridProperty.h"
#include "Spikelist.h"


class SpikeClassGridList :
    public CMFCPropertyGridCtrl
{
    DECLARE_DYNAMIC(SpikeClassGridList)

    void init_header();
    void delete_all();
    SpikeClassGridProperty* find_item(int class_id) const;
    void update(SpikeList* spk_list);

    void make_fixed_header()
    {
        HDITEM hd_item = { 0 };
        hd_item.mask = HDI_FORMAT;
        GetHeaderCtrl().GetItem(0, &hd_item);
        hd_item.fmt |= HDF_FIXEDWIDTH | HDF_CENTER;
        GetHeaderCtrl().SetItem(0, &hd_item);
    }

protected:
    afx_msg void OnSize(UINT n_type, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};




