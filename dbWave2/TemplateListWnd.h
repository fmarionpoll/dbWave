#pragma once

#include "TemplateWnd.h"

class CTemplateListWnd : public CListCtrl
{
	DECLARE_SERIAL(CTemplateListWnd)

	// Construction
public:
	CTemplateListWnd();
	~CTemplateListWnd() override;
	CTemplateListWnd& operator =(const CTemplateListWnd& arg); // operator redefinition
	void Serialize(CArchive& ar) override;

	// Attributes
	CImageList m_image_list;
	double m_global_std = 0.;
	double m_global_dist = 0.;
protected:
	CImageList* m_p_image_list_drag_ = nullptr;
	CArray<CTemplateWnd*, CTemplateWnd*> template_wnd_ptr_array_;
	CTemplateWnd m_tpl0_{};
	int m_tpl_len_ = 60;
	int m_tpl_left_ = 0;
	int m_tpl_right_ = 1;
	float m_k_tolerance_ = 1.96f;
	int m_hit_rate_ = 75;
	int m_y_extent_ = 0;
	int m_y_zero_ = 0;
	BOOL m_b_dragging_ = false;
	int m_i_item_drag_ = 0;
	int m_i_item_drop_ = 0;
	CPoint m_pt_hot_spot_{};
	CPoint m_pt_origin_{};
	CSize m_size_delta_{};

public:
	int insert_template(int i, int class_id);
	int insert_template_data(int i, int class_id);
	void transfer_template_data();
	void delete_all_templates();
	BOOL delete_item(int item_index);
	void sort_templates_by_class(BOOL b_up);
	void sort_templates_by_number_of_spikes(BOOL b_up, BOOL b_update_classes, int min_class_nb);
	int get_template_data_size() const { return template_wnd_ptr_array_.GetSize(); }

	BOOL t_init(int i);
	BOOL t_add(int* p_source);
	BOOL t_add(int i, int* p_source);
	BOOL t_power(int i, double* power);
	BOOL t_within(int i, int* p_source);
	BOOL t_min_dist(const int i, int* p_source, int* offset_min, double* dist_min);
	void t_global_stats();

	void set_template_length(int spk_len, int tp_left, int tp_right);
	void set_hit_rate_tolerance(const int* p_hit_rate, const float* p_tolerance);
	void set_y_w_ext_org(int extent, int zero);
	void update_template_legends(LPCSTR psz_type);
	void update_template_base_class_id(int i_new_lowest_class);
	void set_template_class_id(int item, LPCTSTR psz_type, int class_id);

	CTemplateWnd* get_template_wnd(const int i) const { return template_wnd_ptr_array_.GetAt(i); }
	static CTemplateWnd* get_template_wnd_for_class(int i_class);
	int get_template_length() const { return m_tpl_len_; }
	int get_n_templates() const { return GetItemCount(); }
	int get_template_class_id(int item) const { return template_wnd_ptr_array_.GetAt(item)->m_class_id; }

	void OnButtonUp(CPoint point);

	// Generated message map functions
protected:
	afx_msg void on_get_disp_info(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void on_begin_drag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
