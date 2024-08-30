#pragma once


class DlgRejectRecord : public CDialog
{
	// Construction
public:
	DlgRejectRecord(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DEFINEARTEFACTSLIMITS };

	BOOL m_b_consecutive_points{ false };
	int m_n_consecutive_points{ 0 };
	int m_jitter{ 0 };
	int m_flag{ 1 };

	// Overrides
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
