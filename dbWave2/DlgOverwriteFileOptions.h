#pragma once
#include <afxwin.h>


// DlgOverwriteFileOptions dialog

class DlgOverwriteFileOptions : public CDialog
{
	DECLARE_DYNAMIC(DlgOverwriteFileOptions)

public:
	DlgOverwriteFileOptions(CWnd* p_parent = nullptr);

public:
	BOOL m_b_keep_choice{ true };
	BOOL m_b_overwrite_file{ false };

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPTIONSOVERWRITEFILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* p_dx);

	DECLARE_MESSAGE_MAP()
};
