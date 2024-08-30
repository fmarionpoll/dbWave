#pragma once


class DlgEditSpikeClass : public CDialog
{
	DECLARE_DYNAMIC(DlgEditSpikeClass)

public:
	DlgEditSpikeClass(CWnd* p_parent = nullptr); 
	~DlgEditSpikeClass() override;

	enum { IDD = IDD_EDITSPIKECLASS };

	// input and output value
	int m_i_class {0};

protected:
	void DoDataExchange(CDataExchange* p_dx) override; 

	DECLARE_MESSAGE_MAP()
};
