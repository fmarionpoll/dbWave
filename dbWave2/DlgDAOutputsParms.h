#pragma once
#include "OPTIONS_OUTPUTDATA.h"

class DlgDAOutputParameters : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDAOutputParameters)

public:
	DlgDAOutputParameters(CWnd* pParent = nullptr); // standard constructor
	~DlgDAOutputParameters() override;

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };

	OPTIONS_OUTPUTDATA m_outD;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
