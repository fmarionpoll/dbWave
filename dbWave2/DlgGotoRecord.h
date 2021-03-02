#pragma once


class CDlgGotoRecord : public CDialog
{
	// Construction
public:
	CDlgGotoRecord(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	enum { IDD = IDD_GOTORECORDID };
	long	m_recordID;
	int		m_recordPos;
	BOOL	m_bGotoRecordID;

	// Overrides)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	void SetOptions();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedPosition();
	afx_msg void OnClickedID();

	DECLARE_MESSAGE_MAP()
};
