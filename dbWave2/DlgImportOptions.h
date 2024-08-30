#pragma once

// DlgImportOptions dialog

class DlgImportOptions : public CDialogEx
{
	DECLARE_DYNAMIC(DlgImportOptions)

public:
	DlgImportOptions(CWnd* p_parent = nullptr); // standard constructor
	~DlgImportOptions() override;

	// Dialog Data
	enum { IDD = IDD_IMPORTOPTIONS };

	BOOL m_b_new_i_ds = false;
	BOOL m_b_allow_duplicate_files = false;
	BOOL m_b_read_columns = false;
	BOOL m_b_header = false;

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
