#pragma once

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOXDLG };

protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
};