#pragma once
#include "afxwin.h"

class CDlgFileNew1 : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFileNew1)

public:
	CDlgFileNew1(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgFileNew1();

	// Dialog Data
	enum { IDD = IDD_FILE_NEW1 };
	CListBox m_list;
	int		m_icursel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
