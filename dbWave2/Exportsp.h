#pragma once
#include "OPTIONS_VIEWSPIKES.h"


class CExportSpikeInfosDlg : public CDialog
{
	// Construction
public:
	CExportSpikeInfosDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EXPORTSPIKEINFOS };
	BOOL	m_bacqchsettings;
	BOOL	m_bacqcomments;
	BOOL	m_bacqdate;
	float	m_timeend;
	float	m_timebin;
	float	m_timestart;
	//BOOL	m_bartefacts;
	int		m_classnb;
	int		m_classnb2;
	BOOL	m_btotalspikes;
	BOOL	m_bspkcomments;
	int		m_ispikeclassoptions;
	int		m_nbins;
	float	m_histampl_vmax;
	float	m_histampl_vmin;
	int		m_histampl_nbins;
	int		m_iexportoptions;
	BOOL	m_bexportzero;
	BOOL	m_bexportPivot;
	BOOL    m_bexporttoExcel;
	int		m_istimulusindex;
	BOOL	m_brelation;
	OPTIONS_VIEWSPIKES* m_pvdS;
	BOOL	m_bhist;
	BOOL	m_bhistampl;

	// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void DisplayHistAmplParms(int bdisplay);
	void DisplayHistParms(int bdisplay);

	// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnClassFilter();
	afx_msg void OnEnChangeNbins();
	afx_msg void OnEnChangeBinsize();
	afx_msg void OnclickPSTH();
	afx_msg void OnclickISI();
	afx_msg void OnclickAUTOCORR();
	afx_msg void OnclickOthers();
	afx_msg void OnclickAmplHistog();
public:
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedSpikepoints();

	DECLARE_MESSAGE_MAP()
};
