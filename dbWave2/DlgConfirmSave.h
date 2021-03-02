#pragma once

// CConfirmSaveDlg dialog

class CDlgConfirmSave : public CDialog
{
	DECLARE_DYNAMIC(CDlgConfirmSave)

public:
	CDlgConfirmSave(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgConfirmSave();

	// Dialog Data
	enum { IDD = IDD_CONFIRMDIALOG };

public:
	virtual BOOL OnInitDialog();
	CString m_cstimeleft;
	CString m_csfilename;

private:
	int m_timeleft;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	
	DECLARE_MESSAGE_MAP()
};
