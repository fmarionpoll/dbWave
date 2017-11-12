#pragma once

// adcontvi.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CADContView form view


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "afxwin.h"
#include "dtacq32.h"
#include <oltypes.h>
#include <olerrors.h>
#include <Olmem.h>

// DT Openlayer board

#define STRLEN 80        // string size 

///////////////////////////////////////////////////////////////////////////////
class CADContView : public CFormView
{
friend class CBoard;

protected:
	CADContView();		// protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CADContView)

// Form Data
public:
	enum { IDD = IDD_VIEWADCONTINUOUS };
	CdbMainTable*		m_ptableSet;
	float				m_sweepduration;
	CString				m_boardName;
	CDTAcq32			m_AnalogIN;
	CDTAcq32			m_AnalogOUT;

	float				m_yupper;
	float				m_ylower;
	CRulerBar			m_adxscale;
	CRulerBar			m_adyscale;
	BOOL				m_bOutputsEnabled;
	CComboBox			m_ADcardCombo;
	CMFCButton			m_btnStartStop;
	BOOL				m_bADwritetofile;

protected:
	// display data button and associated procedures
	CLineViewWnd		m_ADsourceView;		// source data display button
	int 				m_cursorstate;		// source data cursor state	
	CEditCtrl			mm_sweepduration;	// edit control for data acq duration	
	CEditCtrl			mm_samplingrate;	// edit control for sampling rate
	CEditCtrl			mm_yupper;			// edit control for data acq duration	
	CEditCtrl			mm_ylower;			// edit control for sampling rate
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
	void UpdateHorizontalRulerBar();
	void UpdateChanVerticalRulerBar(int ichan);
	void UpdateStartStop(BOOL bStart);
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
	BOOL				m_DAC_inprogress;	// D/A in progress
	HBUF				m_DAC_bufhandle;
	long				m_DAC_buflen;		// nb of acq sample per DT buffer
	long				m_DAC_chbuflen;
	BOOL				m_bsimultaneousStartDA;
	long				m_DAC_nBuffersFilledSinceStart;
	double				m_DAC_frequency;
	CArray <OUTPUTPARMS, OUTPUTPARMS> m_DAC_chanList;

	// sweep
	long				m_chsweeplength;	// sweep length (per channel)
	long				m_sweeplength;		// sweep length (all channels)
	int					m_chsweep1;			// indexes
	int					m_chsweep2;
	int					m_chsweepRefresh;
	int					m_bytesweepRefresh;
	float				m_fclockrate;		// apparent clock rate

// functions for data acquisition
	BOOL FindDTOpenLayersBoards();
	BOOL SelectDTOpenLayersBoard(CString cardName);
	
	BOOL ADC_OpenSubSystem(CString cardName);
	BOOL ADC_InitSubSystem();
	void ADC_DeleteBuffers();
	void ADC_DeclareBuffers();
	void ADC_Transfer(short* pDTbuf);
	void ADC_Stop();
	
	BOOL DAC_OpenSubSystem(CString cardName);
	BOOL DAC_ClearAllOutputs();
	BOOL DAC_InitSubSystem();
	void DAC_DeleteBuffers();
	void DAC_DeclareAndFillBuffers();
	void DAC_FillBufferWith_SINUSOID(short * pDTbuf, int chan);
	void DAC_FillBufferWith_SQUARE(short * pDTbuf, int chan);
	void DAC_FillBufferWith_TRIANGLE(short * pDTbuf, int chan);
	void DAC_FillBufferWith_RAMP(short * pDTbuf, int chan);
	void DAC_FillBufferWith_CONSTANT(short * pDTbuf, int chan);
	void DAC_FillBufferWith_ONOFFSeq(short * pDTbuf, int chan);
	double DAC_MSequence(BOOL start, OUTPUTPARMS * parmsChan);
	void DAC_FillBufferWith_MSEQ(short * pDTbuf, int chan);
	void DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan);
	void DAC_FillBuffer(short* pDTbuf);
	void DAC_ConvertOptionsIntoChanList();
	void DAC_Stop();
	
	long VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain);
	float ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain);

	void StopAcquisition(BOOL bDisplayErrorMsg);
	void SaveAndCloseFile();
	BOOL StartAcquisition();

	BOOL InitCyberAmp();
	BOOL Defineexperiment() ;
	void TransferFilesToDatabase();
	void UpdateViewDataFinal();
	void displayolDaErrorMessage(CHAR* errstr);

// Overrides
protected:
	virtual CDaoRecordset* OnGetRecordset();
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void	OnInitialUpdate();
	virtual void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual			~CADContView();
	virtual	void	OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	void			ChainDialog(WORD iID);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifndef _DEBUG  // debug version in dbWaveView.cpp
inline CdbWaveDoc* CADContView::GetDocument()
   { return (CdbWaveDoc*)m_pDocument; }
#else
	CdbWaveDoc*	GetDocument();
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnChangeDuration();
	afx_msg void OnHardwareAdchannels();
	afx_msg void OnHardwareAdintervals();
	afx_msg void OnHardwareDefineexperiment();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
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
	afx_msg void OnBnClickedEnableoutput();
	afx_msg void OnBnClickedDaparameters();
	afx_msg void OnBnClickedDaparameters2();
	afx_msg void OnCbnSelchangeComboboard();
	afx_msg void OnBnClickedStartstop();
	afx_msg void OnEnChangeYlower();
	afx_msg void OnEnChangeYupper();
	afx_msg void OnBnClickedWriteToDisk();
	afx_msg void OnBnClickedOscilloscope();
	afx_msg void OnBnClickedCardfeatures();
};


