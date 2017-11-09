#pragma once

class CDataTranslationBoardDlg : public CDialog
{
	DECLARE_DYNAMIC(CDataTranslationBoardDlg)

public:
	CDataTranslationBoardDlg(CWnd* pParent = NULL);
	virtual ~CDataTranslationBoardDlg();
	enum {IDD= IDD_DTBOARD_DLG};
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

