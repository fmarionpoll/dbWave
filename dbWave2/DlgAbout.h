#pragma once

class DlgAbout : public CDialogEx
{
public:
	DlgAbout();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOXDLG };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
