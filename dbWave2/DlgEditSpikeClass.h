#pragma once

// CEditSpikeClassDlg dialog

class CDlgEditSpikeClass : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditSpikeClass)

public:
	CDlgEditSpikeClass(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgEditSpikeClass();

	// Dialog Data
	enum { IDD = IDD_EDITSPIKECLASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// input and output value
	int m_iClass;
};
