#pragma once
#include <afxwin.h>


// DlgOverwriteFileOptions dialog

class DlgOverwriteFileOptions : public CDialog
{
	DECLARE_DYNAMIC(DlgOverwriteFileOptions)

public:
	DlgOverwriteFileOptions(CWnd* pParent = nullptr);
	//~DlgOverwriteFileOptions();

public:
	BOOL m_bKeepChoice{ true };
	BOOL m_bOverwriteFile{ false };

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPTIONSOVERWRITEFILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};
