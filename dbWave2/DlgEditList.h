#pragma once


class DlgEditList : public CDialog
{
	DECLARE_DYNAMIC(DlgEditList)

public:
	DlgEditList(CWnd* p_parent = nullptr); // standard constructor
	~DlgEditList() override;
	CComboBox* p_co {nullptr};
	CString m_cs_new_string {_T("")};
	CListBox m_cl_strings;
	CStringArray m_cs_array;
	int m_selected {0};

	// Dialog Data
	enum { IDD = IDD_EDITLIST };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

public:
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void on_bn_clicked_delete();
	afx_msg void on_bn_clicked_add_item();
	afx_msg void OnSize(UINT n_type, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
