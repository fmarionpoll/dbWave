#pragma once


class DlgGotoRecord : public CDialog
{
	// Construction
public:
	DlgGotoRecord(CWnd* p_parent = nullptr); // standard constructor

	enum { IDD = IDD_GOTORECORDID };

	long m_record_id{ 0 };
	int m_record_pos{ 0 };
	BOOL m_b_goto_record_id{ false };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

protected:
	void set_options() const;

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void on_clicked_position();
	afx_msg void on_clicked_id();

	DECLARE_MESSAGE_MAP()
};
