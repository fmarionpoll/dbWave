#pragma once

// listbcla.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListBClaSizeDlg dialog

class CListBClaSizeDlg : public CDialog
{
// Construction
public:
	CListBClaSizeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LISTBCLAPARAMETERS };
	int		m_rowheight;
	int		m_superpcol;
	int		m_textcol;
	int		m_intercolspace;

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:

	DECLARE_MESSAGE_MAP()
};
