#pragma once

// CDAChannelsDlg dialog

class CDAChannelsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDAChannelsDlg)

public:
	CDAChannelsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDAChannelsDlg();

// Dialog Data
	enum { IDD = IDD_DACHANNELS };
	OPTIONS_OUTPUTDATA*	poutD;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
