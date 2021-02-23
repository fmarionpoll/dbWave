#pragma once


class CDlgRejectRecord : public CDialog
{
	// Construction
public:
	CDlgRejectRecord(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DEFINEARTEFACTSLIMITS };
	BOOL	m_bconsecutivepoints;
	int		m_Nconsecutivepoints;
	int		m_jitter;
	int		m_flag;

	// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
