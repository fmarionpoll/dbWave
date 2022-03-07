#pragma once
#include <afxcmn.h>
class CIntervalsListCtrl :
    public CListCtrl
{
    CImageList* m_image_list{ nullptr };

public:
    void init_listbox(const CString header1, const int size1, const CString header2, const int size2);
    void set_sub_item_0(LVITEM& lvi, const int item, CString& cs);
    void set_sub_item_1(LVITEM& lvi, int iItem, float time_interval, CString& cs);
    void set_list_control_item(int i, float time_interval);
    void add_new_item(int i, float time_interval);
    void set_sub_item_0_value(int i);
    void set_sub_item_1_value(int index, float time_interval);
    int get_row_selected();
    int select_item(int i);

};

