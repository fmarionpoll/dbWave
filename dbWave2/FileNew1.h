#pragma once
#include "afxwin.h"

// CFileNew1 dialog

class CFileNew1 : public CDialogEx
{
	DECLARE_DYNAMIC(CFileNew1)

public:
	CFileNew1(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFileNew1();

	// Dialog Data
	enum { IDD = IDD_FILE_NEW1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_list;
	int		m_icursel;
	afx_msg void OnBnClickedOk();
};
