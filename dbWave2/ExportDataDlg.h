#pragma once

// ExportDataDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDataDlg dialog

class CExportDataDlg : public CDialog
{
// Construction
public:
	CExportDataDlg(CWnd* pParent = NULL);   // standard constructor

	OPTIONS_IMPORT	iivO;					// import options
	CdbWaveDoc* m_dbDoc;					// (source) data document(s)	
	CString		m_filesource;
	CString		m_filedest;
	CString		m_filetemp;
	int			m_icurrentfile;				// store index currently selected file
	int			m_indexoldselectedfile;
	BOOL		m_bAllFiles;

	// parameters used by the different export routines
	long	mm_lFirst;
	long	mm_lLast;
	float	mm_timefirst;
	float	mm_timelast;
	int		mm_firstchan;
	int		mm_lastchan;
	short	mm_binzero;
	CAcqDataDoc* m_pDat;

// Dialog Data
	enum { IDD = IDD_EXPORTDATAFILE };
	CComboBox	m_ComboExportas;
	CComboBox	m_filedroplist;
	float	m_timefirst;
	float	m_timelast;
	int		m_channelnumber;
	int		m_iundersample;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDataDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateStructFromControls();
	void Export();
	BOOL ExportDataAsdbWaveFile();
	BOOL ExportDataAsTextFile();
	BOOL ExportDataAsSapidFile();
	BOOL ExportDataAsExcelFile();	
	void save_BIFF(CFile* fp, int type, int row, int col, char *data);
	void saveCString_BIFF(CFile* fp, int row, int col, CString& data);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSinglechannel();
	afx_msg void OnAllchannels();
	virtual void OnOK();
	afx_msg void OnEntirefile();
	afx_msg void OnSelchangeExportas();
	afx_msg void OnExport();
	afx_msg void OnExportall();
	DECLARE_MESSAGE_MAP()
};
