// adinterv.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ADIntervalsDlg dialog

#pragma once
#include "Editctrl.h"

class ADIntervalsDlg : public CDialog
{
	// Construction
public:
	ADIntervalsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_AD_INTERVALS };
	int		m_buffer_nitems;
	float	m_adratechan;
	float	m_acqduration;
	float	m_sweepduration;
	int		m_buffer_wsize;
	int		m_undersamplefactor;
	BOOL	m_baudiblesound;
	int		m_threshchan;
	int		m_threshval;
	CWaveFormat* m_pwave_format;

	UINT	m_postmessage;		// parameter passed on exit to activate another dialog
	float	m_ratemax;			// max sampling rate (per chan)
	float	m_ratemin;			// min sampling rate (per chan)
	int		m_buffer_wsizemax;	// maximum buffer size
	BOOL	m_bchain_dialog;

	CEditCtrl mm_adratechan;
	CEditCtrl mm_sweepduration;
	CEditCtrl mm_buffer_wsize;
	CEditCtrl mm_buffer_nitems;
	CEditCtrl mm_acqduration;

	// Implementation
protected:
	CWaveFormat m_acqdef;	// acquisition parameters

	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

	// Generated message map functions
	afx_msg void OnAdchannels();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeAdratechan();
	afx_msg void OnEnChangeDuration();
	afx_msg void OnEnChangeBuffersize();
	afx_msg void OnEnChangeNbuffers();
	afx_msg void OnEnChangeAcqduration();
	afx_msg void OnTrigthresholdOFF();
	afx_msg void OnTrigthresholdON();

	DECLARE_MESSAGE_MAP()
};
