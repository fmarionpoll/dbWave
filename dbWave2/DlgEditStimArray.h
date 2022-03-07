#pragma once
#include "Spikedoc.h"
#include "StretchControls.h"


class DlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(DlgEditStimArray)

	DlgEditStimArray(CWnd* pParent = nullptr);
	~DlgEditStimArray() override;

	// data passed by caller
	CTagList* tag_list {nullptr};
	CIntervalsAndLevels* intervals_and_levels_saved { nullptr };
	CArray<CIntervalsAndLevels*, CIntervalsAndLevels*> intervals_and_levels_array{};
	float m_sampling_rate = 0.f;

	enum { IDD = IDD_DLGEDITSTIMARRAY};

protected:
	CIntervalsAndLevels* intervals_and_levels {nullptr} ;

	void DoDataExchange(CDataExchange* pDX) override;
	CEdit m_edit_control{};
	bool mode_edit = false;
	CListCtrl m_stimulus_array_control{};

	CStretchControl m_stretch{};
	BOOL m_initialized { false };
	float m_item_value {0};
	int m_item_index {-1};
	CImageList* m_image_list {nullptr};

	void make_dialog_stretchable();
	void init_listbox();
	void select_item(int i);
	void resetListOrder();
	void addNewItem(int i, long lInterval);
	void transfer_intervals_array_to_dialog_list();
	void setSubItem0(LVITEM& lvi, int item, CString& cs);
	void setSubItem1(LVITEM& lvi, int iItem, long lInterval, CString& cs) const;
	void set_edit_value();
	void get_edit_value();
	void set_active_edit_overlay();
	void set_inactive_edit_overlay();
	int	 get_row_selected();

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
