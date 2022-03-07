#pragma once
#include "CIntervalsListCtrl.h"
#include "Spikedoc.h"
#include "StretchControls.h"


class DlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(DlgEditStimArray)

	DlgEditStimArray(CWnd* pParent = nullptr);
	~DlgEditStimArray() override;

	// data passed by caller
	CTagList*		tag_list {nullptr};
	CIntervals*		intervals_saved { nullptr };
	CArray<CIntervals*, CIntervals*> intervals_array{};
	float			m_sampling_rate = 0.f;

	enum { IDD = IDD_DLGEDITSTIMARRAY};

protected:
	CIntervals* intervals {nullptr} ;

	void DoDataExchange(CDataExchange* pDX) override;
	CEdit m_edit_control{};
	bool mode_edit = false;
	CIntervalsListCtrl list_control{};

	CStretchControl m_stretch{};
	BOOL m_initialized { false };
	float m_item_value {0};
	int m_item_index {-1};

	void make_dialog_stretchable();

	void reset_list_order();

	void transfer_intervals_array_to_dialog_list();
	void transfer_dialog_list_to_intervals_array();

	void set_edit_value();
	void get_edit_value();
	void set_active_edit_overlay();
	void set_inactive_edit_overlay();

public:
	BOOL OnInitDialog() override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedEditButton();
	afx_msg void OnEnKillfocusReOrder();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	afx_msg void OnBnClickedReOrder();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImportfromdata();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
};
