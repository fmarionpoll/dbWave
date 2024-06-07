#pragma once
#include "OPTIONS_VIEWDATA.h"


class DlgPrintDataOptions : public CDialog
{
	// Construction
public:
	DlgPrintDataOptions(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTDATADRAWOPTIONS };

	BOOL m_bClipRect{ false };
	BOOL m_bFrameRect{ false };
	BOOL m_bTimeScaleBar{ false };
	BOOL m_bVoltageScaleBar{ false };
	BOOL m_bPrintSelection{ false };
	BOOL m_bPrintSpkBars{ false };
	OPTIONS_VIEWDATA* mdPM{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CPrintDataCommentsDlg dialog

class CPrintDataCommentsDlg : public CDialog
{
	// Construction
public:
	CPrintDataCommentsDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_OPTIONSPRINTDATACOMMENTS };

	BOOL m_bacqcomment;
	BOOL m_bacqdatetime;
	BOOL m_bchanscomment;
	BOOL m_bchansettings;
	BOOL m_bdocname;
	int m_fontsize;
	int m_textseparator;
	OPTIONS_VIEWDATA* mdPM;

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void OnOK() override;
	BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
};
