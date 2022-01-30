#pragma once


class DlgGotoRecord : public CDialog
{
	// Construction
public:
	DlgGotoRecord(CWnd* pParent = nullptr); // standard constructor

	enum { IDD = IDD_GOTORECORDID };

	long m_recordID{ 0 };
	int m_recordPos{ 0 };
	BOOL m_bGotoRecordID{ false };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

protected:
	void SetOptions();

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnClickedPosition();
	afx_msg void OnClickedID();

	DECLARE_MESSAGE_MAP()
};
