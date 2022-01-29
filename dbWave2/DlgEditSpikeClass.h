#pragma once

// CEditSpikeClassDlg dialog

class CDlgEditSpikeClass : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditSpikeClass)

public:
	CDlgEditSpikeClass(CWnd* pParent = nullptr); // standard constructor
	~CDlgEditSpikeClass() override;

	enum { IDD = IDD_EDITSPIKECLASS };

	// input and output value
	int m_iClass;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
