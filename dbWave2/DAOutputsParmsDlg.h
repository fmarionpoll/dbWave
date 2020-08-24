#pragma once

// CDAOutputsDlg dialog

class CDAOutputParametersDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDAOutputParametersDlg)

public:
	CDAOutputParametersDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDAOutputParametersDlg();

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };
	OPTIONS_OUTPUTDATA	m_outD;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
