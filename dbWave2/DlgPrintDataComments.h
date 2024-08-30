#pragma once
#include <afxwin.h>

#include "options_view_data.h"

class DlgPrintDataComment :
    public CDialog
{
};

class DlgPrintDataComments : public CDialog
{
	// Construction
public:
	DlgPrintDataComments(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_OPTIONSPRINTDATACOMMENTS };

	BOOL m_b_acq_comment;
	BOOL m_b_acq_date_time;
	BOOL m_b_channels_comment;
	BOOL m_b_channel_settings;
	BOOL m_b_doc_name;
	int m_font_size;
	int m_text_separator;
	options_view_data* options_view_data;

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void OnOK() override;
	BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
};
