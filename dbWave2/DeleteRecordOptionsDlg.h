#pragma once


// CDeleteRecordOptionsDlg dialog

class CDeleteRecordOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeleteRecordOptionsDlg)

public:
	CDeleteRecordOptionsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDeleteRecordOptionsDlg();

// Dialog Data
	enum { IDD = IDD_DELRECORDOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bKeepChoice;
	BOOL m_bDeleteFile;
};
