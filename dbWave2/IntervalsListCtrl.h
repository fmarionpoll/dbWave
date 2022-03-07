#pragma once
#include <afxcmn.h>
class CIntervalsListCtrl :
    public CListCtrl
{
    CImageList* m_image_list{ nullptr };
    CEdit m_edit_control{};
    bool mode_edit = false;

public:
    void init_listbox(const CString header1, const int size1, const CString header2, const int size2);
    void set_sub_item_0(LVITEM& lvi, const int item, CString& cs);
    CString get_sub_item_1(int item);
    void set_sub_item_1(LVITEM& lvi, int iItem, float time_interval, CString& cs);
    void set_list_control_item(int i, float time_interval);
    void add_new_item(int i, float time_interval);
    void set_sub_item_0_value(int i);
    void set_sub_item_1_value(int index, float time_interval);
    int get_index_item_selected();
    int select_item(int i);

    void set_edit_value();
    float get_edit_value();
    void set_active_edit_overlay();
    void set_inactive_edit_overlay();
};

