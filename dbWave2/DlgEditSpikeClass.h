#pragma once


class DlgEditSpikeClass : public CDialog
{
	DECLARE_DYNAMIC(DlgEditSpikeClass)

public:
	DlgEditSpikeClass(CWnd* pParent = nullptr); // standard constructor
	~DlgEditSpikeClass() override;

	enum { IDD = IDD_EDITSPIKECLASS };

	// input and output value
	int m_iClass {0};

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
