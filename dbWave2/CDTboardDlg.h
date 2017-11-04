// CDTBoardDlg.h : header file

#pragma once

#include "resource.h"
#include "CDTBoardParameters.h"

/////////////////////////////////////////////////////////////////////////////
// CBoardDlg dialog
class CDTBoardDlg : public CDialog
{
// Construction
public:
	CDTBoardDlg(CWnd* pParent);

// Dialog Data
	enum { IDD = IDD_DTBOARD_DLG};
	CComboBox	m_cboBoard;
	CStatic		m_boardInfo;
	CStatic		m_subsystems;
	CStatic		m_subsystemCapability;
	CComboBox	m_subsystem;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSelchangeBoard();
	afx_msg void OnSelchangeSubsystem();
	DECLARE_MESSAGE_MAP()

public:
	CString		GetBoardName();
	void		SetBoardName(CString board_name);

private:
	UINT		m_atodCount;
	UINT		m_dtoaCount;
	UINT		m_dioCount;
	CWnd*		m_pParent;
	CString		m_strBoardName;
	char		m_str[STRLEN];
public:
	CStatic m_ssNumerical;
};
