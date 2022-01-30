#pragma once


class DlgListBClaSize : public CDialog
{
	// Construction
public:
	DlgListBClaSize(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_LISTBCLAPARAMETERS };

	int m_rowheight{ 0 };
	int m_superpcol{ 0 };
	int m_textcol{ 0 };
	int m_intercolspace{ 0 };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
