#pragma once
#include <afxcmn.h>


class CIntervalsListCtrl : public CListCtrl
{
    DECLARE_DYNCREATE(CIntervalsListCtrl)

    ~CIntervalsListCtrl() override;


    CImageList* m_image_list{ nullptr };
    CEdit* m_p_edit = nullptr;
    bool mode_edit = false;

    int hit_test_ex(const CPoint& point_to_be_tested, int* column) const;
    CEdit* edit_sub_label(int n_item, int n_col);

    void init_listbox(const CString& header1, const int size1, const CString& header2, const int size2);
    void add_new_item(int index, float time_interval);
    int get_index_item_selected() const;
	int select_item(int index);

    void set_item(int index, float time_interval);
    void set_item_index(int index);
    void set_item_value(int index, float time_interval);

    float get_item_value(int item) const;
    int   get_item_index(int item) const;

protected:
    static void set_sub_item_1_value(LVITEM& lvi, int i_item, float time_interval, CString& cs);
    static void set_sub_item_0_value(LVITEM& lvi, const int item, CString& cs);

public:
    afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
    afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
    afx_msg void OnLButtonDown(UINT n_flags, CPoint point);
    afx_msg void on_end_label_edit(NMHDR* p_nmhdr, LRESULT* p_result);
    DECLARE_MESSAGE_MAP()
};

