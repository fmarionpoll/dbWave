#pragma once
#include "options_output.h"

class DlgDAOutputParameters : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDAOutputParameters)

public:
	DlgDAOutputParameters(CWnd* p_parent = nullptr); 
	~DlgDAOutputParameters() override;

	// Dialog Data
	enum { IDD = IDD_DA_OUTPUTPARMS };

	options_output m_out_d;

protected:
	void DoDataExchange(CDataExchange* p_dx) override; 

	DECLARE_MESSAGE_MAP()
};
