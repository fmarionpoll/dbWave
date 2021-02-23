#pragma once

// CDAOutputsDlg dialog

class CDlgDAOutputParameters : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDAOutputParameters)

public:
	CDlgDAOutputParameters(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgDAOutputParameters();

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };
	OPTIONS_OUTPUTDATA	m_outD;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
