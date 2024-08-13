#pragma once

#include "Editctrl.h"
#include "ChartData.h"
#include "AcqDataDoc.h"
#include "options_import.h"

class DlgImportGenericData : public CDialog
{
	friend class AcqDataDoc;

	// Construction
public:
	DlgImportGenericData(CWnd* pParent = nullptr);

	options_import* piivO{ nullptr };
	BOOL bConvert{ false };
	CStringArray* m_pfilenameArray{ nullptr };
	CEditCtrl mm_nb_AD_channels;
	CEditCtrl mm_adChannelChan;
	CEditCtrl mm_skipNbytes;
	AcqDataDoc m_AcqDataFile;
	ChartData m_ChartDataWnd;
	CString m_filesource;
	CString m_filedest;
	CString m_fileold{ _T("")};
	BOOL m_bChanged{ false };
	BOOL m_bimportall{ false };

	// Dialog Data
	enum { IDD = IDD_IMPORTGENERICDATAFILE };

	CComboBox m_filedroplist;
	CComboBox m_ComboPrecision;
	CString m_adChannelComment{ _T("")};
	float m_adChannelGain{ 0.f };
	UINT m_adChannelChan{ 0 };
	UINT m_nb_AD_channels{ 0 };
	UINT m_nbRuns{ 0 };
	float m_samplingrate{ 0.f };
	UINT m_skipNbytes{ 0 };
	float m_voltageMax{ 0.f };
	float m_voltageMin{ 0.f };
	CString m_csFileTitle{ _T("")};
	BOOL m_bpreviewON{ false };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV supportTUAL

	// Implementation
protected:
	void UpdateControlsFromStruct();
	void UpdateStructFromControls();
	void EnableRunParameters();
	void UpdatePreview();
	void UpdateWaveDescriptors(AcqDataDoc* pDF);
	void SetFileNames(int index);

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
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
