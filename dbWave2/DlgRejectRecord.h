#pragma once


class CDlgRejectRecord : public CDialog
{
	// Construction
public:
	CDlgRejectRecord(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DEFINEARTEFACTSLIMITS };

	BOOL m_bconsecutivepoints;
	int m_Nconsecutivepoints;
	int m_jitter;
	int m_flag;

	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
