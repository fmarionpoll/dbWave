#pragma once
#include "IntervalsListCtrl.h"
#include "Spikedoc.h"
#include "StretchControls.h"


class DlgEditStimArray : public CDialog
{
	DECLARE_DYNAMIC(DlgEditStimArray)

	DlgEditStimArray(CWnd* pParent = nullptr);
	~DlgEditStimArray() override;

	// data passed by caller
	CTagList*	tag_list {nullptr};
	CIntervals	intervals_saved {};
	CIntervals	intervals{};
	float		m_sampling_rate = 0.f;

	enum { IDD = IDD_EDITSTIMARRAY};

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	
	CIntervalsListCtrl list_control{};
	CStretchControl m_stretch{};
	BOOL m_initialized { false };
	int m_item_index {-1};

	void make_dialog_stretchable();
	void reset_list_order();
	void transfer_intervals_array_to_control_list();
	void transfer_control_list_to_intervals_array();

public:
	DECLARE_MESSAGE_MAP()

	BOOL OnInitDialog() override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	afx_msg void OnBnClickedReOrder();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImportfromdata();
	afx_msg void OnBnClickedOk();
};
