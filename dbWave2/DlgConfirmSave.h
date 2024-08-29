#pragma once

class DlgConfirmSave : public CDialog
{
	DECLARE_DYNAMIC(DlgConfirmSave)

public:
	DlgConfirmSave(CWnd* p_parent = nullptr); // standard constructor
	~DlgConfirmSave() override;

	// Dialog Data
	enum { IDD = IDD_CONFIRM };

public:
	BOOL OnInitDialog() override;
	CString m_cs_time_left;
	CString m_cs_file_name;

private:
	int m_time_left_;
public:
	afx_msg void OnTimer(UINT n_id_event);

	DECLARE_MESSAGE_MAP()
};
