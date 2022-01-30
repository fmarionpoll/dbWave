#pragma once
#include "OPTIONS_VIEWDATA.h"


class DlgPrintDrawArea : public CDialog
{
	// Construction
public:
	DlgPrintDrawArea(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTDRAWAREA };

	int m_HeightDoc{ 0 };
	int m_heightSeparator{ 0 };
	int m_WidthDoc{ 0 };
	int m_spkheight{ 0 };
	int m_spkwidth{ 0 };
	OPTIONS_VIEWDATA* mdPM{ nullptr };
	BOOL m_bFilterDat{ false };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
