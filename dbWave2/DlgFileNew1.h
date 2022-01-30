#pragma once
#include "afxwin.h"

class DlgFileNew1 : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFileNew1)

public:
	DlgFileNew1(CWnd* pParent = nullptr); // standard constructor
	~DlgFileNew1() override;

	// Dialog Data
	enum { IDD = IDD_FILE_NEW1 };

	CListBox m_list;
	int m_icursel{ 0 };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

public:
	BOOL OnInitDialog() override;
	afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
