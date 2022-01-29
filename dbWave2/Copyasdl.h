#pragma once

class CCopyAsDlg : public CDialog
{
	// Construction
public:
	CCopyAsDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_COPYAS };

	int m_nabcissa;
	int m_nordinates;
	int m_ioption;
	int m_iunit;
	BOOL m_bgraphics;

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
