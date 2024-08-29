#pragma once

class DlgAbout : public CDialogEx
{
public:
	DlgAbout();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
