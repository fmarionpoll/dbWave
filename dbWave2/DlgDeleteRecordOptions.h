#pragma once
#include "resource.h"

class DlgDeleteRecordOptions final : public CDialog
{
	DECLARE_DYNAMIC(DlgDeleteRecordOptions)

public:
	DlgDeleteRecordOptions(CWnd* pParent = nullptr); // standard constructor
	~DlgDeleteRecordOptions() override;

	// Dialog Data
	enum { IDD = IDD_OPTIONSDELETEDATFILE };

public:
	BOOL m_bKeepChoice {true};
	BOOL m_bDeleteFile {false};

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
