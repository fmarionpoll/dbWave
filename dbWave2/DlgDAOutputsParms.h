#pragma once
#include "OPTIONS_OUTPUTDATA.h"

// CDAOutputsDlg dialog

class CDlgDAOutputParameters : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDAOutputParameters)

public:
	CDlgDAOutputParameters(CWnd* pParent = nullptr); // standard constructor
	~CDlgDAOutputParameters() override;

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };

	OPTIONS_OUTPUTDATA m_outD;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
