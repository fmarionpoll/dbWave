#pragma once

// CEditSpikeClassDlg dialog

class CEditSpikeClassDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditSpikeClassDlg)

public:
	CEditSpikeClassDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditSpikeClassDlg();

// Dialog Data
	enum { IDD = IDD_EDITSPIKECLASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// input and output value
	int m_iClass;
};
