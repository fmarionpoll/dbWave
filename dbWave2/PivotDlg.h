#pragma once


// CPivotDlg dialog

class CPivotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPivotDlg)

public:
	CPivotDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPivotDlg();

// Dialog Data
	enum { IDD = IDD_PIVOTDLG };
	int		m_nFixCols;
	int		m_nFixRows;
	int		m_nCols;
	int		m_nRows;

	CGridCtrl	m_Grid;
	CSize		m_OldSize;
	static bool VirtualCompare(int c1, int c2);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
