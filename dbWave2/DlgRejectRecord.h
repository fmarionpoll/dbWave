#pragma once


class DlgRejectRecord : public CDialog
{
	// Construction
public:
	DlgRejectRecord(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DEFINEARTEFACTSLIMITS };

	BOOL m_bconsecutivepoints{ false };
	int m_Nconsecutivepoints{ 0 };
	int m_jitter{ 0 };
	int m_flag{ 1 };

	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
