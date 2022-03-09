#pragma once
#include <afxcmn.h>


class CIntervalsListCtrl : public CListCtrl
{
    DECLARE_DYNCREATE(CIntervalsListCtrl)

    ~CIntervalsListCtrl() override;


    CImageList* m_image_list{ nullptr };
    CEdit* m_p_edit = nullptr;
    bool mode_edit = false;

    int HitTestEx(const CPoint& point, int* col) const;
    CEdit* EditSubLabel(int nItem, int nCol);

public:
    void init_listbox(const CString header1, const int size1, const CString header2, const int size2);
    void add_new_item(int index, float time_interval);
    int get_index_item_selected() const;
	int select_item(int index);

    void set_item(int index, float time_interval);
    void set_item_index(int index);
    void set_item_value(int index, float time_interval);

    float get_item_value(int item) const;
    int   get_item_index(int item) const;

    void enable_edit_overlay();
    void disable_edit_overlay();
    void validate_edit_overlay();

protected:
    void set_edit_overlay_value(int item_selected);
    void move_edit_overlay_over_selected_item(int item_selected);
    void init_edit_overlay();
    float get_edit_value();

    void set_sub_item_1_value(LVITEM& lvi, int iItem, float time_interval, CString& cs);
    void set_sub_item_0_value(LVITEM& lvi, const int item, CString& cs);

public:
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()
};

