#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "ChartData.h"
#include "Editctrl.h"

// CMeasureOptionsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeasureOptionsPage dialog

class CMeasureOptionsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureOptionsPage)

	// Construction
public:
	CMeasureOptionsPage();
	~CMeasureOptionsPage();

	// Dialog Data
	enum { IDD = IDD_PROPPAGE1 };
	BOOL	m_bExtrema;
	BOOL	m_bDiffExtrema;
	BOOL	m_bDiffDatalimits;
	BOOL	m_bHalfrisetime;
	BOOL	m_bHalfrecovery;
	BOOL	m_bDatalimits;
	BOOL	m_bLimitsval;
	WORD	m_wSourcechan{};
	WORD	m_wStimulusthresh;
	WORD	m_wStimuluschan;
	BOOL	m_bAllFiles;
	int		m_uiSourceChan;
	float	m_fStimulusoffset;
	UINT	m_uiStimulusThreshold;
	UINT	m_uiStimuluschan;
	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO{};
	CdbWaveDoc*		m_pdbDoc{};
	CAcqDataDoc*	m_pdatDoc{};
	CChartDataWnd*	m_pChartDataWnd{};

	// Overrides
public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

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
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
