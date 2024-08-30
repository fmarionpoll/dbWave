#pragma once
#include "resource.h"

class DlgDeleteRecordOptions final : public CDialog
{
	DECLARE_DYNAMIC(DlgDeleteRecordOptions)

public:
	DlgDeleteRecordOptions(CWnd* p_parent = nullptr); // standard constructor
	~DlgDeleteRecordOptions() override;

	// Dialog Data
	enum { IDD = IDD_OPTIONSDELETEDATFILE };

public:
	BOOL m_b_keep_choice{ true };
	BOOL m_b_delete_file{ false };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
