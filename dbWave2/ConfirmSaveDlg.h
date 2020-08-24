#pragma once

// CConfirmSaveDlg dialog

class CConfirmSaveDlg : public CDialog
{
	DECLARE_DYNAMIC(CConfirmSaveDlg)

public:
	CConfirmSaveDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CConfirmSaveDlg();

	// Dialog Data
	enum { IDD = IDD_CONFIRMDIALOG };

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

private:
	int m_timeleft;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	CString m_cstimeleft;
	CString m_csfilename;
};
