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
	CDlgImportFiles(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgImportFiles();

	// Dialog Data
	enum { IDD = IDD_IMPORTFILESDLG };

	// values passed by caller
public:
	CStringArray* m_pfilenameArray;
	CStringArray* m_pconvertedFiles;
	CdbWaveDoc* m_pdbDoc;
	int				m_option;

	// Implementation
protected:
	BOOL		m_bconvert;
	CString		m_ext;
	int			m_ncurrent;	// number of current file
	int			m_nfiles;	// number of files
	CString		m_filefrom;
	CString		m_fileto;

	int			m_scan_count;
	double		m_xinstgain;
	double		m_xrate;
	double		m_dspan[16];
	double		m_dbinval[16];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void		UpdateDlgItems();
	BOOL		GetAcquisitionParameters(CAcqDataDoc* pTo);
	BOOL		GetExperimentParameters(CAcqDataDoc* pTo);
	BOOL		ImportATFFile();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCancel();
	afx_msg void ADC_OnBnClickedStartstop();
	BOOL m_bReadHeader;
};
