#pragma once

// CDAOutputsDlg dialog

class CDAOutputsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDAOutputsDlg)

public:
	CDAOutputsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDAOutputsDlg();

// Dialog Data
	enum { IDD = IDD_DAPARAMETERS };
	OPTIONS_OUTPUTDATA*	poutD;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
