#pragma once
#include "afxwin.h"

class DlgFileNew1 : public CDialogEx
{
	DECLARE_DYNAMIC(DlgFileNew1)

public:
	DlgFileNew1(CWnd* p_parent = nullptr); // standard constructor
	~DlgFileNew1() override;

	// Dialog Data
	enum { IDD = IDD_FILE_NEW1 };

	CListBox m_list;
	int m_i_cur_sel{ 0 };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

public:
	BOOL OnInitDialog() override;
	afx_msg void on_bn_clicked_ok();

	DECLARE_MESSAGE_MAP()
};
