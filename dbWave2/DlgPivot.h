#pragma once

#include "GridCtrl/GridCtrl.h"

class CDlgPivot : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPivot)

public:
	CDlgPivot(CWnd* pParent = nullptr); // standard constructor
	~CDlgPivot() override;

	// Dialog Data
	enum { IDD = IDD_PIVOTDLG };

	int m_nFixCols;
	int m_nFixRows;
	int m_nCols;
	int m_nRows;

	CGridCtrl m_Grid;
	CSize m_OldSize;
	static bool VirtualCompare(int c1, int c2);

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

public:
	BOOL OnInitDialog() override;
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
