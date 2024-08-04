#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "OPTIONS_DATA_MEASURE.h"

class CMeasureOptionsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureOptionsPage)

	// Construction
public:
	CMeasureOptionsPage();
	~CMeasureOptionsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE1 };

	BOOL m_bExtrema;
	BOOL m_bDiffExtrema;
	BOOL m_bDiffDatalimits;
	BOOL m_bHalfrisetime;
	BOOL m_bHalfrecovery;
	BOOL m_bDatalimits;
	BOOL m_bLimitsval;
	WORD m_wSourcechan{};
	WORD m_wStimulusthresh;
	WORD m_wStimuluschan;
	BOOL m_bAllFiles;
	int m_uiSourceChan;
	float m_fStimulusoffset;
	UINT m_uiStimulusThreshold;
	UINT m_uiStimuluschan;
	// input parameters
	OPTIONS_DATA_MEASURE* m_pMO{};
	CdbWaveDoc* m_pdbDoc{};
	AcqDataDoc* m_pdatDoc{};
	ChartData* m_pChartDataWnd{};

	// Overrides
public:
	void OnOK() override;
	BOOL OnKillActive() override;
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	void ShowLimitsParms(BOOL bShow);
	void ShowChanParm(BOOL b_show);
	void SaveOptions();

	// Generated message map functions
	afx_msg void OnAllchannels();
	afx_msg void OnSinglechannel();
	afx_msg void OnVerticaltags();
	afx_msg void OnHorizontaltags();
	afx_msg void OnStimulustag();
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
