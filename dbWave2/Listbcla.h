#pragma once


class CListBClaSizeDlg : public CDialog
{
	// Construction
public:
	CListBClaSizeDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_LISTBCLAPARAMETERS };

	int m_rowheight;
	int m_superpcol;
	int m_textcol;
	int m_intercolspace;

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
