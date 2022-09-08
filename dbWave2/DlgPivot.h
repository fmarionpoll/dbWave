#pragma once

#include "GridCtrl/GridCtrl.h"

class DlgPivot : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPivot)

public:
	DlgPivot(CWnd* pParent = nullptr); // standard constructor
	~DlgPivot() override;

	// Dialog Data
	enum { IDD = IDD_PIVOTDLG };

	int m_nFixCols{ 1 };
	int m_nFixRows{ 1 };
	int m_nCols{ 3 };
	int m_nRows{ 26 };

	CGridCtrl m_GridPivot;
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
