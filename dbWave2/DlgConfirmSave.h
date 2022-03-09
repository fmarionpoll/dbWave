#pragma once

class DlgConfirmSave : public CDialog
{
	DECLARE_DYNAMIC(DlgConfirmSave)

public:
	DlgConfirmSave(CWnd* pParent = nullptr); // standard constructor
	~DlgConfirmSave() override;

	// Dialog Data
	enum { IDD = IDD_CONFIRM };

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
