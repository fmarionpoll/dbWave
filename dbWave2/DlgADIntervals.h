#pragma once
#include "Editctrl.h"
#include "AcqWaveFormat.h"

class DlgADIntervals : public CDialog
{
	// Construction
public:
	DlgADIntervals(CWnd* pParent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_AD_INTERVALS };

	int m_bufferNitems;
	float m_adratechan;
	float m_acqduration;
	float m_sweepduration;
	UINT m_bufferWsize;
	UINT m_undersamplefactor;
	BOOL m_baudiblesound;
	int m_threshchan;
	int m_threshval;
	CWaveFormat* m_pwaveFormat;

	WORD m_postmessage; 
	float m_ratemax; 
	float m_ratemin; 
	UINT m_bufferWsizemax; 
	BOOL m_bchainDialog;

	CEditCtrl mm_adratechan;
	CEditCtrl mm_sweepduration;
	CEditCtrl mm_bufferWsize;
	CEditCtrl mm_bufferNitems;
	CEditCtrl mm_acqduration;

	// Implementation
protected:
	CWaveFormat m_acqdef; // acquisition parameters

	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	afx_msg void OnAdchannels();
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void OnEnChangeAdratechan();
	afx_msg void OnEnChangeDuration();
	afx_msg void OnEnChangeBuffersize();
	afx_msg void OnEnChangeNbuffers();
	afx_msg void OnEnChangeAcqduration();
	afx_msg void OnTrigthresholdOFF();
	afx_msg void OnTrigthresholdON();

	DECLARE_MESSAGE_MAP()
};
