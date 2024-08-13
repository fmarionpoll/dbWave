#pragma once


class DlgExportData : public CDialog
{
	// Construction
public:
	DlgExportData(CWnd* pParent = nullptr);
	BOOL DestroyWindow() override;

	options_import iivO;
	CdbWaveDoc* m_dbDoc {nullptr};
	CString m_filesource;
	CString m_filedest;
	CString m_filetemp;
	int m_icurrentfile{ -1 };
	int m_indexoldselectedfile {0};
	BOOL m_bAllFiles{ true };

	long mm_lFirst {0};
	long mm_lLast { 0 };
	float mm_timefirst{ 0 };
	float mm_timelast{ 0 };
	int mm_firstchan{ 0 };
	int mm_lastchan{ 0 };
	int mm_binzero{ 0 };
	AcqDataDoc* m_pDat{ nullptr };

	// Dialog Data
	enum { IDD = IDD_EXPORTDATAFILE };

	CComboBox m_ComboExportas;
	CComboBox m_filedroplist;
	float m_timefirst{ 0.f };
	float m_timelast{ 0.f };
	int m_channelnumber{ 0 };
	int m_iundersample{ 1 };


protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void UpdateStructFromControls();
	void Export();
	BOOL ExportDataAsdbWaveFile();
	BOOL ExportDataAsTextFile();
	BOOL ExportDataAsSapidFile();
	BOOL ExportDataAsExcelFile();
	void save_BIFF(CFile* fp, int type, int row, int col, char* data);
	void saveCString_BIFF(CFile* fp, int row, int col, CString& data);

	BOOL OnInitDialog() override;
	afx_msg void OnSinglechannel();
	afx_msg void OnAllchannels();
	void OnOK() override;
	afx_msg void OnEntirefile();
	afx_msg void OnSelchangeExportas();
	afx_msg void OnExport();
	afx_msg void OnExportall();

	DECLARE_MESSAGE_MAP()
};
