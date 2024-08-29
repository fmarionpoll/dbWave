#pragma once

class DlgCopyAs : public CDialog
{
public:
	DlgCopyAs(CWnd* p_parent = nullptr); 

	enum { IDD = IDD_COPYAS };

	int m_n_abscissa{ 0 };
	int m_n_ordinates{ 0 };
	int m_i_option{ 0 };
	int m_i_unit{ 0 };
	BOOL b_graphics{ false };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; 

	void OnOK() override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
