#pragma once

class DlgCopyAs : public CDialog
{
public:
	DlgCopyAs(CWnd* pParent = nullptr); 

	enum { IDD = IDD_COPYAS };

	int m_nabcissa{ 0 };
	int m_nordinates{ 0 };
	int m_ioption{ 0 };
	int m_iunit{ 0 };
	BOOL m_bgraphics{ false };

protected:
	void DoDataExchange(CDataExchange* pDX) override; 

	void OnOK() override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
