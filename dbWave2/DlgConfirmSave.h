#pragma once

// CConfirmSaveDlg dialog

class CDlgConfirmSave : public CDialog
{
	DECLARE_DYNAMIC(CDlgConfirmSave)

public:
	CDlgConfirmSave(CWnd* pParent = nullptr); // standard constructor
	~CDlgConfirmSave() override;

	// Dialog Data
	enum { IDD = IDD_CONFIRMDIALOG };

public:
	BOOL OnInitDialog() override;
	CString m_cstimeleft;
	CString m_csfilename;

private:
	int m_timeleft;
public:
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
