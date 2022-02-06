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

	int m_bufferNitems{ 0 };
	float m_adratechan{ 0.f };
	float m_acqduration{ 0.f };
	float m_sweepduration{ 1.f };
	UINT m_bufferWsize{ 0 };
	UINT m_undersamplefactor{ 1 };
	BOOL m_baudiblesound{ false };
	int m_threshchan{ 0 };
	int m_threshval{ 0 };
	CWaveFormat* m_pwaveFormat{ nullptr };

	WORD m_postmessage{ 0 };
	float m_ratemax{ 50000.f };
	float m_ratemin{ 0.1f };
	UINT m_bufferWsizemax{ 0 };
	BOOL m_bchainDialog{ false };

	CEditCtrl mm_adratechan;
	CEditCtrl mm_sweepduration;
	CEditCtrl mm_bufferWsize;
	CEditCtrl mm_bufferNitems;
	CEditCtrl mm_acqduration;

	// Implementation
protected:
	CWaveFormat m_acqdef;

	void DoDataExchange(CDataExchange* pDX) override;

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
