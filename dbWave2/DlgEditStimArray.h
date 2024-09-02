#pragma once
#include "IntervalsListCtrl.h"
#include "StretchControls.h"
#include "Taglist.h"


class DlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(DlgEditStimArray)

	DlgEditStimArray(CWnd* p_parent = nullptr);
	~DlgEditStimArray() override;

	// data passed by caller
	TagList*	tag_list {nullptr};
	CIntervals	intervals_saved {};
	CIntervals	intervals{};
	float		m_sampling_rate { 0.f};

	enum { IDD = IDD_EDITSTIMARRAY};

protected:
	void DoDataExchange(CDataExchange* p_dx) override;
	
	CIntervalsListCtrl list_control_{};
	CStretchControl m_stretch_{};
	BOOL m_initialized_ { false };
	int m_item_index_ {-1};

	void make_dialog_stretchable();
	void reset_list_order();
	void transfer_intervals_array_to_control_list();
	void transfer_control_list_to_intervals_array();

public:
	DECLARE_MESSAGE_MAP()

	BOOL OnInitDialog() override;
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void on_bn_clicked_delete();
	afx_msg void on_bn_clicked_insert();
	afx_msg void on_bn_clicked_delete3();
	afx_msg void on_bn_clicked_re_order();
	afx_msg void on_bn_clicked_copy();
	afx_msg void on_bn_clicked_paste();
	afx_msg void on_bn_clicked_export();
	afx_msg void on_bn_clicked_import_from_data();
	afx_msg void on_bn_clicked_ok();
};
