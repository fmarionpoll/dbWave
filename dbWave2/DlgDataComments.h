#pragma once
#include "OPTIONS_VIEW_DATA.h"

class DlgDataComments : public CDialog
{
public:
	DlgDataComments(CWnd* pParent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_DATACOMMENTS };

	BOOL m_bacqchans{ false };
	BOOL m_bacqcomments{ false };
	BOOL m_bacqdate{ false };
	BOOL m_bacqtime{ false };
	BOOL m_bfilesize{ false };
	BOOL m_bacqchsetting{ false };
	BOOL m_bdatabasecols{ false };
	BOOL m_btoExcel{ false };
	OPTIONS_VIEW_DATA* m_pvO{ nullptr };

protected:
	void DoDataExchange(CDataExchange* pDX) override; 

	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
