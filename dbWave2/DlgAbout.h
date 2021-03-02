#pragma once

class CDlgAbout : public CDialogEx
{
public:
	CDlgAbout();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOXDLG };

protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};