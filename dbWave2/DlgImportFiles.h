#pragma once
#include "dbWaveDoc.h"

#define GERT		 0
#define ASCIISYNTECH 1
#define ATFFILE		 2

// CImportFilesDlg dialog

class CDlgImportFiles : public CDialog
{
	DECLARE_DYNAMIC(CDlgImportFiles)

public:
	CDlgImportFiles(CWnd* pParent = nullptr); // standard constructor
	~CDlgImportFiles() override;

	// Dialog Data
	enum { IDD = IDD_IMPORTFILESDLG };

	// values passed by caller
public:
	CStringArray* m_pfilenameArray = nullptr;
	CStringArray* m_pconvertedFiles = nullptr;
	CdbWaveDoc* m_pdbDoc = nullptr;
	int m_option = 0;
	BOOL m_bReadHeader = true;

	// Implementation
protected:
	BOOL m_bconvert = false;
	CString m_ext{};
	int m_ncurrent = 0; // number of current file
	int m_nfiles = 0; // number of files
	CString m_filefrom{};
	CString m_fileto{};

	int m_scan_count = 0;
	double m_xinstgain = 0.;
	double m_xrate = 0.;
	double m_dspan[16]{};
	double m_dbinval[16]{};

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void UpdateDlgItems();
	BOOL GetAcquisitionParameters(AcqDataDoc* pTo);
	BOOL GetExperimentParameters(AcqDataDoc* pTo);
	BOOL ImportATFFile();

public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCancel();
	afx_msg void ADC_OnBnClickedStartstop();

	DECLARE_MESSAGE_MAP()
};
