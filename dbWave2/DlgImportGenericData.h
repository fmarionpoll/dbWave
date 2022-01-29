#pragma once

#include "Editctrl.h"
#include "ChartData.h"
#include "AcqDataDoc.h"
#include "OPTIONS_IMPORT.h"

class CDlgImportGenericData : public CDialog
{
	friend class CAcqDataDoc;

	// Construction
public:
	CDlgImportGenericData(CWnd* pParent = nullptr);   // standard constructor

	OPTIONS_IMPORT* piivO;				// from caller
	BOOL			bConvert;
	CStringArray*	m_pfilenameArray;
	CEditCtrl		mm_nbADchannels;
	CEditCtrl		mm_adChannelChan;
	CEditCtrl		mm_skipNbytes;
	CAcqDataDoc		m_AcqDataFile;		// data document
	CChartDataWnd	m_ChartDataWnd;			// data display
	CString			m_filesource;
	CString			m_filedest;
	CString			m_fileold;
	BOOL			m_bChanged;			// one parameter was changed
	BOOL			m_bimportall;

	// Dialog Data
	enum { IDD = IDD_IMPORTGENERICDATAFILE };
	CComboBox		m_filedroplist;
	CComboBox		m_ComboPrecision;
	CString			m_adChannelComment;
	float			m_adChannelGain;
	UINT			m_adChannelChan;
	UINT			m_nbADchannels;
	UINT			m_nbRuns;
	float			m_samplingrate;
	UINT			m_skipNbytes;
	float			m_voltageMax;
	float			m_voltageMin;
	CString			m_csFileTitle;
	BOOL			m_bpreviewON;

	// Overrides
public:
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV supportTUAL

// Implementation
protected:
	void UpdateControlsFromStruct();
	void UpdateStructFromControls();
	void EnableRunParameters();
	void UpdatePreview();
	void UpdateWaveDescriptors(CAcqDataDoc* pDF);
	void SetFileNames(int index);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangePrecision();
	afx_msg void OnEnChangeNumberofchannels();
	afx_msg void OnMultipleruns();
	afx_msg void OnSinglerun();
	afx_msg void OnEnChangeChannelno();
	afx_msg void OnDeltaposSpinNbChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoChan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnImport();
	afx_msg void OnImportall();
	afx_msg void OnSetPreview();
	afx_msg void OnSelchangeFilelist();
	afx_msg void OnOffsetbinary();
	afx_msg void OnTwoscomplement();
	afx_msg void OnEnChangeSkipnbytes();
	afx_msg void OnDeltaposSkipNbytes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSapid3_5();
	afx_msg void OnEnChangeChannelcomment();

	DECLARE_MESSAGE_MAP()
};
