#pragma once


class CExportDataDlg : public CDialog
{
	// Construction
public:
	CExportDataDlg(CWnd* pParent = nullptr); 
	virtual BOOL DestroyWindow() override;

	OPTIONS_IMPORT	iivO;
	CdbWaveDoc* m_dbDoc;
	CString		m_filesource;
	CString		m_filedest;
	CString		m_filetemp;
	int			m_icurrentfile;	
	int			m_indexoldselectedfile;
	BOOL		m_bAllFiles;

	long		mm_lFirst;
	long		mm_lLast;
	float		mm_timefirst;
	float		mm_timelast;
	int			mm_firstchan;
	int			mm_lastchan;
	int			mm_binzero;
	CAcqDataDoc* m_pDat;

	// Dialog Data
	enum { IDD = IDD_EXPORTDATAFILE };
	CComboBox	m_ComboExportas;
	CComboBox	m_filedroplist;
	float		m_timefirst;
	float		m_timelast;
	int			m_channelnumber;
	int			m_iundersample;


protected:
	virtual void DoDataExchange(CDataExchange* pDX) override; 
	void UpdateStructFromControls();
	void Export();
	BOOL ExportDataAsdbWaveFile();
	BOOL ExportDataAsTextFile();
	BOOL ExportDataAsSapidFile();
	BOOL ExportDataAsExcelFile();
	void save_BIFF(CFile* fp, int type, int row, int col, char* data);
	void saveCString_BIFF(CFile* fp, int row, int col, CString& data);

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
