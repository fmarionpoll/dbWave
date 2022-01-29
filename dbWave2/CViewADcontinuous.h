#pragma once

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "RulerBar.h"
#include "afxwin.h"
#include "dtacq32.h"
#include "StretchControls.h"
#include "DataTranslation_AD.h"
#include "DataTranslation_DA.h"
#include "USBPxxS1Ctl.h"
#include "OPTIONS_ACQDATA.h"
#include "OPTIONS_OUTPUTDATA.h"


class CADContView final : public CFormView
{
	friend class CBoard;

protected:
	CADContView();
	DECLARE_DYNCREATE(CADContView)
	// Form Data
	enum { IDD = IDD_VIEWADCONTINUOUS };

	CdbMainTable*		m_ptableSet {nullptr};
	CString				m_boardName;
	DataTranslation_AD	m_Acq32_AD;
	DataTranslation_DA	m_Acq32_DA;

	CRulerBar			m_AD_xRulerBar;
	CRulerBar			m_AD_yRulerBar;
	CComboBox			m_ADcardCombo;
	CMFCButton			m_btnStartStop;
	BOOL				m_bADwritetofile  { false };
	int					m_bStartOutPutMode { 0 };
	bool				m_DA_present { false };
	bool				m_AD_present { false };

protected:
	CChartDataWnd		m_ADsourceView;				// source data display button
	int 				m_cursorstate  { 0 };			// source data cursor state	
	float				m_sweepduration { 2 };
	CEditCtrl			mm_yupper;					// edit control for max amplitude displayed	
	CEditCtrl			mm_ylower;					// edit control for min amplitude displayed
	CStretchControl		m_stretch;					// array of properties associated with controls
	HICON				m_hBias { nullptr };
	HICON				m_hZoom{ nullptr };
	float				m_yscaleFactor { 1 };		// div factor for y bar 
	int					m_VBarMode{ 0 };			// flag V scrollbar state
	CScrollBar 			m_scrolly;					// V scrollbar
	COLORREF			m_backgroundColor = GetSysColor(COLOR_BTNFACE);
	CBrush*				m_pBackgroundBrush = new CBrush(m_backgroundColor);

	void	OnGainScroll(UINT nSBCode, UINT nPos);
	void	OnBiasScroll(UINT nSBCode, UINT nPos);
	void	UpdateGainScroll();
	void	UpdateBiasScroll();
	void	SetVBarMode(short bMode);
	void	UpdateChanLegends(int ichan);

	void	UpdateStartStop(BOOL bStart);
	void	UpdateRadioButtons();

	// data	parameters
protected:
	BOOL				m_bFoundDTOPenLayerDLL { false };
	BOOL				m_bhidesubsequent{ false };

	CAcqDataDoc			m_inputDataFile;			// document
	//CAcqDataDoc		m_outputDataFile;			// D/A file...
	CStringArray		m_csNameArray;
	BOOL				m_bFileOpen { false };		// flag / file open
	CString				m_szFileName;				// data filename

	BOOL 				m_bAskErase{ false };		// ask erase when data may be lost (default = FALSE)
	BOOL				m_bchanged{ false };			// flag: save data or not	
	BOOL				m_bSimultaneousStart{ false };	//TRUE if the card is capable of this

	// DT buffer
	OPTIONS_ACQDATA*	m_pOptions_AD{ nullptr };	// pointer to data acq options 
	OPTIONS_OUTPUTDATA* m_pOptions_DA{ nullptr };	// pointer to data output options
	BOOL				m_bsimultaneousStartDA{ false };

	// sweep
	long				m_chsweeplength{ 0 };		// sweep length (per channel)
	long				m_sweeplength{ 1000 };		// sweep length (all channels)
	int					m_chsweep1{ 0 };			// indexes
	int					m_chsweep2{ 0 };
	int					m_chsweepRefresh{ 0 };
	int					m_bytesweepRefresh{ 0 };
	float				m_fclockrate{ 10000.f };	// apparent clock rate

// functions for data acquisition
	BOOL	FindDTOpenLayersBoards();
	BOOL	SelectDTOpenLayersBoard(CString cardName);

	void	SetCombostartoutput(int option);

	long	VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain);
	float	ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain);

	void	StopAcquisition(BOOL bDisplayErrorMsg);
	void	SaveAndCloseFile();
	BOOL	StartAcquisition();
	BOOL	StartOutput();
	void	StopOutput();
	BOOL	InitAcquisitionSystemAndBuffers();
	void	InitAcquisitionInputFile();
	void	InitAcquisitionDisplay();

	BOOL	InitCyberAmp();
	BOOL	Defineexperiment();
	void	TransferFilesToDatabase();
	void	UpdateViewDataFinal();
	void	DisplayolDaErrorMessage(const CHAR* errstr);
	void	ChainDialog(WORD iID);

	void	ADC_Transfer(short* pDTbuf0);
	void	ADC_TransferToFile();
	void	InitializeAmplifiers();

	// Overrides
	CDaoRecordset* OnGetRecordset();
	void	DoDataExchange(CDataExchange* pDX) override; 
	void	OnInitialUpdate() override;
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
			~CADContView() override;
	void	OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

#ifdef _DEBUG
	void	AssertValid() const override;
	void	Dump(CDumpContext& dc) const override;
	CdbWaveDoc* GetDocument();
#else
	inline CdbWaveDoc* CADContView::GetDocument()
	{
		return (CdbWaveDoc*)m_pDocument;
	}
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

public:
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnHardwareAdchannels();
	afx_msg void OnHardwareAdintervals();
	afx_msg void OnHardwareDefineexperiment();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBufferDone_ADC();
	afx_msg void OnTriggerError_ADC();
	afx_msg void OnOverrunError_ADC();
	afx_msg void OnQueueDone_ADC();
	afx_msg void OnBufferDone_DAC();
	afx_msg void OnOverrunError_DAC();
	afx_msg void OnQueueDone_DAC();
	afx_msg void OnTriggerError_DAC();
	afx_msg void OnBnClickedGainbutton();
	afx_msg void OnBnClickedBiasbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedDaparameters2();
	afx_msg void OnCbnSelchangeComboboard();
	afx_msg void OnBnClickedStartstop();
	afx_msg void OnBnClickedWriteToDisk();
	afx_msg void OnBnClickedOscilloscope();
	afx_msg void OnBnClickedCardfeatures();
	afx_msg void OnCbnSelchangeCombostartoutput();
	afx_msg void OnBnClickedStartstop2();

};
