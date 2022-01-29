#pragma once
#include "OPTIONS_VIEWDATA.h"


class CPrintDrawAreaDlg : public CDialog
{
	// Construction
public:
	CPrintDrawAreaDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTDRAWAREA };

	int m_HeightDoc;
	int m_heightSeparator;
	int m_WidthDoc;
	int m_spkheight;
	int m_spkwidth;
	OPTIONS_VIEWDATA* mdPM;
	BOOL m_bFilterDat;

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
