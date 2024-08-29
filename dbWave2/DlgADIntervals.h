#pragma once
#include "Editctrl.h"
#include "AcqWaveFormat.h"

class DlgADIntervals : public CDialog
{
	// Construction
public:
	DlgADIntervals(CWnd* p_parent = nullptr);

	// Dialog Data
	enum { IDD = IDD_AD_INTERVALS };

	int m_buffer_n_items{ 0 };
	float m_ad_rate_channel{ 0.f };
	float m_acquisition_duration{ 0.f };
	float m_sweep_duration{ 1.f };
	UINT m_buffer_w_size{ 0 };
	UINT m_under_sample_factor{ 1 };
	BOOL m_b_audible_sound{ false };
	int m_threshold_channel{ 0 };
	int m_threshold_value{ 0 };
	CWaveFormat* m_p_wave_format{ nullptr };

	WORD m_postmessage{ 0 };
	float m_rate_maximum{ 50000.f };
	float m_rate_minimum{ 0.1f };
	UINT m_buffer_w_size_maximum{ 0 };
	BOOL m_b_chain_dialog{ false };

	CEditCtrl mm_ad_rate_channel;
	CEditCtrl mm_sweep_duration;
	CEditCtrl mm_buffer_w_size;
	CEditCtrl mm_buffer_n_items;
	CEditCtrl mm_acquisition_duration;

	// Implementation
protected:
	CWaveFormat wave_format_;

	void DoDataExchange(CDataExchange* p_dx) override;

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;

	afx_msg void on_ad_channels();
	afx_msg void on_en_change_ad_rate_per_channel();
	afx_msg void on_en_change_duration();
	afx_msg void on_en_change_buffer_size();
	afx_msg void on_en_change_n_buffers();
	afx_msg void on_en_change_acquisition_duration();
	afx_msg void on_trigger_threshold_off();
	afx_msg void on_trigger_threshold_on();

	DECLARE_MESSAGE_MAP()
};
