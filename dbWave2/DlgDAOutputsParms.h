#pragma once
#include "options_output.h"

class DlgDAOutputParameters : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDAOutputParameters)

public:
	DlgDAOutputParameters(CWnd* pParent = nullptr); // standard constructor
	~DlgDAOutputParameters() override;

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };

	options_output m_outD;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
