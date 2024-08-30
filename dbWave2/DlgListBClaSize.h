#pragma once


class DlgListBClaSize : public CDialog
{
	// Construction
public:
	DlgListBClaSize(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_LISTBCLAPARAMETERS };

	int m_row_height{ 0 };
	int m_super_p_col{ 0 };
	int m_text_col{ 0 };
	int m_inter_col_space{ 0 };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
