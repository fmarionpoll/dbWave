#pragma once

// adcontvi.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewADContinuous form view


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "RulerBar.h"
#include "Editctrl.h"
#include "ScrollBarEx.h"
#include "afxwin.h"
#include "dtacq32.h"
#include "./include/DataTranslation/OLTYPES.H"
#include "./include/DataTranslation/OLERRORS.H"
#include "./include/DataTranslation/Olmem.h"
#include "CUSBPxxS1Ctl.h"

// DT Openlayer board

#define STRLEN 80        // string size 

///////////////////////////////////////////////////////////////////////////////
class CViewADContinuous : public CFormView
{

protected:
	CViewADContinuous();		// protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewADContinuous)

// Form Data
public:
	enum { IDD = IDD_VIEWADCONTINUOUS };
	CdbMainTable*		m_ptableSet;
	CString				m_boardName;
	CRulerBar			m_ADC_xRulerBar;
	CRulerBar			m_ADC_yRulerBar;
	CComboBox			m_ADcardCombo;
	CMFCButton			m_btnStartStop;
	BOOL				m_bADwritetofile;
	int					m_bStartOutPutMode;
	BOOL				m_bADC_IsPresent;
	BOOL				m_bDAC_IsPresent;
	// Alligator amplifier
	CArray < USBPxxPARAMETERS*, USBPxxPARAMETERS*>	alligatorparameters_ptr_array{};
	CUSBPxxS1Ctl		m_Alligator;		// DDX
	// Data Translation ActiveX
	CDTAcq32			m_ADC_DTAcq32;		// DDX
	CDTAcq32			m_DAC_DTAcq32;		// DDX


protected:
	CLineViewWnd		m_displayDataFile;			// source data display button
	int 				m_cursorstate;		// source data cursor state	
	float				m_sweepduration;
	CEditCtrl			mm_yupper;			// edit control for max amplitude displayed	
	CEditCtrl			mm_ylower;			// edit control for min amplitude displayed
	CStretchControl		m_stretch;			// array of properties associated with controls
	HICON				m_hBias;
	HICON				m_hZoom;
	float				m_yscaleFactor;		// div factor for y bar 
	int					m_VBarMode;			// flag V scrollbar state
	CScrollBar 			m_scrolly;			// V scrollbar
	CBrush*				m_pEditBkBrush;
	COLORREF			m_BkColor;

	void OnGainScroll(UINT nSBCode, UINT nPos);
	void OnBiasScroll(UINT nSBCode, UINT nPos);
	void UpdateGainScroll();
	void UpdateBiasScroll();
	void SetVBarMode(short bMode);
	void UpdateChanLegends(int ichan);

	void ADC_UpdateStartStop(BOOL bStart);
	void UpdateRadioButtons();

	// data	parameters
protected:
	BOOL				m_bFoundDTOPenLayerDLL;
	BOOL				m_bhidesubsequent;

	CAcqDataDoc			m_inputDataFile;	// document
	//CAcqDataDoc		m_outputDataFile;	// D/A file...
	CStringArray		m_csNameArray;
	BOOL				m_bFileOpen;		// flag / file open
	CString				m_szFileName;		// data filename

	BOOL 				m_bAskErase;		// ask erase when data may be lost (default = FALSE)
	BOOL				m_bchanged;			// flag: save data or not	
	double 				m_freqmax;			// maximum sampling frequency (Hz)
	int					m_numchansMAX;

	BOOL				m_bSimultaneousStart;	//TRUE if the card is capable of this
	ECODE				m_ecode;

	// DT buffer
	OPTIONS_ACQDATA*	m_pADC_options;		// pointer to data acq options 
	BOOL				m_ADC_inprogress;	// A/D is in progress (used by OnStop/OnStart)
	HBUF				m_ADC_bufhandle;
	long				m_ADC_buflen;		// nb of acq sample per DT buffer
	long				m_ADC_chbuflen;		// nb pts for one chan in DT buffer
	BOOL				m_bsimultaneousStartAD;

	OPTIONS_OUTPUTDATA*	m_pDAC_options;		// pointer to data output options
	int					m_DACdigitalchannel;
	BOOL				m_DACdigitalfirst;
	int					m_DAClistsize;
	long				m_DACmsbit;
	long				m_DAClRes;

	BOOL				m_DAC_inprogress;	// D/A in progress
	HBUF				m_DAC_bufhandle;
	long				m_DAC_buflen;		// nb of acq sample per DT buffer
	long				m_DAC_chbuflen;
	BOOL				m_bsimultaneousStartDA;
	long				m_DAC_nBuffersFilledSinceStart;
	double				m_DAC_frequency;
	
	// sweep
	long				m_chsweeplength;	// sweep length (per channel)
	long				m_sweeplength;		// sweep length (all channels)
	int					m_chsweep1;			// indexes
	int					m_chsweep2{};
	int					m_chsweepRefresh{};
	int					m_bytesweepRefresh{};
	float				m_fclockrate{};		// apparent clock rate

// functions for data acquisition
	BOOL FindDTOpenLayersBoards();
	BOOL SelectDTOpenLayersBoard(const CString& cardName);
	
	BOOL ADC_OpenSubSystem(CString cardName);
	BOOL ADC_InitSubSystem();
	void ADC_DeleteBuffers();
	void ADC_DeclareBuffers();
	void ADC_Transfer(short* pDTbuf);
	void ADC_StopAndLiberateBuffers();
	
	BOOL DAC_OpenSubSystem(const CString& cardName);
	BOOL DAC_InitSubSystem();
	void DAC_DeleteBuffers();
	void DAC_DeclareAndFillBuffers();
	void DAC_StopAndLiberateBuffers();
	void get_dt_error(ECODE ecode, BOOL b_display) const;
	BOOL DAC_ClearAllOutputs();
	int	 DAC_SetChannelList();
	int  DAC_GetNumberOfChans();
	void DAC_FillBufferWith_SINUSOID(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_FillBufferWith_SQUARE(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_FillBufferWith_TRIANGLE(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_FillBufferWith_RAMP(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_FillBufferWith_CONSTANT(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_FillBufferWith_ONOFFSeq(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	static void DAC_MSequence(OUTPUTPARMS * parmsChan);
	void DAC_FillBufferWith_MSEQ(short * pDTbuf, int chan, OUTPUTPARMS* pParms);
	void DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan) const;
	void DAC_Dig_FillBufferWith_SQUARE(short * pDTbuf, int chan, OUTPUTPARMS* pParms) const;
	void DAC_Dig_FillBufferWith_ONOFFSeq(short * pDTbuf, int chan, OUTPUTPARMS* pParms) const;
	void DAC_Dig_FillBufferWith_VAL(short * pDTbuf, int chan, OUTPUTPARMS * parmsChan, BOOL bVal) const;
	void DAC_Dig_FillBufferWith_MSEQ(short * pDTbuf, int chan, OUTPUTPARMS* pParms) const;
	void DAC_UpdateStartStop(BOOL bStart) const;
	void DAC_FillBuffer(short* pDTbuf);
	void SetCombostartoutput(int option);
	
	long VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain) const;
	float ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain) const;

	void SaveAndCloseFile();
	BOOL OnStart();
	void OnStop(BOOL bDisplayErrorMsg);
	BOOL DAC_Start();
	void DAC_Stop();

	BOOL InitConnectionWithAmplifiers();
	BOOL ADC_DefineExperimentDlg() ;
	void TransferFilesToDatabase();
	void UpdateViewDataFinal();

// Overrides
protected:
	virtual CDaoRecordset* OnGetRecordset();
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void	OnInitialUpdate();
	virtual void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual			~CViewADContinuous();
	virtual	void	OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	void			ChainDialog(WORD iID);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifndef _DEBUG  // debug version in dbWaveView.cpp
inline CdbWaveDoc* CViewADContinuous::GetDocument()
   { return (CdbWaveDoc*)m_pDocument; }
#else
	CdbWaveDoc*	GetDocument();
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
public:
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void ADC_OnHardwareChannelsDlg();
	afx_msg void ADC_OnHardwareIntervalsDlg();
	afx_msg void ADC_OnHardwareDefineexperiment();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* p_dc, CWnd* p_wnd, UINT nCtlColor);
	afx_msg void ADC_OnBufferDone();
	afx_msg void ADC_OnTriggerError();
	afx_msg void ADC_OnOverrunError();
	afx_msg void ADC_OnQueueDone();
	afx_msg void DAC_OnBufferDone();
	afx_msg void DAC_OnOverrunError();
	afx_msg void DAC_OnQueueDone();
	afx_msg void DAC_OnTriggerError();
	afx_msg void OnBnClickedGainbutton();
	afx_msg void OnBnClickedBiasbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void DAC_OnBnClickedParameters2();
	afx_msg void OnCbnSelchangeComboboard();
	afx_msg void ADC_OnBnClickedStartstop();
	afx_msg void OnBnClickedWriteToDisk();
	afx_msg void OnBnClickedOscilloscope();
	afx_msg void OnBnClickedCardFeatures();
	afx_msg void OnCbnSelchangeCombostartoutput();
	afx_msg void DAC_OnBnClickedStartStop();

	void DeviceConnectedUsbpxxs1ctl1(long Handle);
	void DeviceDisconnectedUsbpxxs1ctl1(long Handle);
};