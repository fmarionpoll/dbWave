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

	int m_buffer_N_items{ 0 };
	float m_ad_rate_channel{ 0.f };
	float m_duration_to_acquire{ 0.f };
	float m_sweep_duration{ 1.f };
	UINT m_buffer_W_size{ 0 };
	UINT m_under_sample_factor{ 1 };
	BOOL m_b_audible_sound{ false };
	int m_threshold_channel{ 0 };
	int m_threshold_value{ 0 };
	CWaveFormat* m_p_wave_format{ nullptr };

	WORD m_postmessage{ 0 };
	float m_rate_maximum{ 50000.f };
	float m_rate_minimum{ 0.1f };
	UINT m_buffer_W_size_maximum{ 0 };
	BOOL m_b_chain_dialog{ false };

	CEditCtrl mm_ad_rate_channel;
	CEditCtrl mm_sweep_duration;
	CEditCtrl mm_buffer_W_size;
	CEditCtrl mm_buffer_N_items;
	CEditCtrl mm_acquisition_duration;

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
