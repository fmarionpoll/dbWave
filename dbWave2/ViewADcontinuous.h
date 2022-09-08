#pragma once

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "ADAcqDataDoc.h"
#include "RulerBar.h"
#include "afxwin.h"
#include "ChartDataAD.h"
#include "StretchControls.h"
#include "DataTranslation_AD.h"
#include "DataTranslation_DA.h"
#include "USBPxxS1Ctl.h"
#include "OPTIONS_INPUTDATA.h"
#include "OPTIONS_OUTPUTDATA.h"
#include "CyberAmp.h"



class ViewADcontinuous : public CFormView
{
	//friend class CBoard;

protected:
	ViewADcontinuous();
	~ViewADcontinuous() override;
	DECLARE_DYNCREATE(ViewADcontinuous)

	enum { IDD = IDD_VIEWADCONTINUOUS };

	CdbTableMain* m_ptableSet{ nullptr };
	CString m_boardName;
	DataTranslation_AD m_Acq32_AD;
	DataTranslation_DA m_Acq32_DA;

	RulerBar m_AD_xRulerBar;
	RulerBar m_AD_yRulerBar;
	CButton m_ZoomButton;
	CButton m_BiasButton;
	CButton m_UnZoomButton;
	CButton m_Button_StartStop_DA;
	CButton m_Button_SamplingMode;
	CButton m_Button_OutputChannels;
	CButton m_Button_WriteToDisk;
	CButton m_Button_Oscilloscope;
	CComboBox m_ComboStartOutput;
	CComboBox m_ADcardCombo;
	CMFCButton m_btnStartStop_AD;
	
	BOOL m_bADwritetofile{ false };
	int m_bStartOutPutMode{ 0 };
	bool m_DA_present{ false };
	bool m_AD_present{ false };

protected:
	ChartDataAD m_chartDataAD;
	int m_cursorstate{ 0 };
	float m_sweepduration{ 2 };
	CEditCtrl mm_yupper;
	CEditCtrl mm_ylower;
	CStretchControl m_stretch;
	HICON m_hBias{ nullptr };
	HICON m_hZoom{ nullptr };
	HICON m_hUnZoom{ nullptr };
	float m_yscaleFactor{ 1 };
	int m_VBarMode{ 0 };
	CScrollBar m_scrolly;
	COLORREF m_backgroundColor = GetSysColor(COLOR_BTNFACE);
	CBrush* m_pBackgroundBrush = new CBrush(m_backgroundColor);

	void OnGainScroll(UINT nSBCode, UINT nPos);
	void OnBiasScroll(UINT nSBCode, UINT nPos);
	void UpdateGainScroll();
	void UpdateBiasScroll();
	void SetVBarMode(short bMode);

	void UpdateStartStop(BOOL bStart);
	void UpdateRadioButtons();

	// data	parameters
protected:
	BOOL m_bFoundDTOPenLayerDLL{ false };
	BOOL m_bhidesubsequent{ false };

	ADAcqDataDoc m_inputDataFile;
	//AcqDataDoc m_outputDataFile;	
	CStringArray m_csNameArray;
	BOOL m_bFileOpen{ false };
	CString m_szFileName;

	BOOL m_bAskErase{ false };
	BOOL m_bchanged{ false };
	BOOL m_bSimultaneousStart{ false };

	// DT buffer
	OPTIONS_INPUTDATA* m_pOptions_AD{ nullptr };
	OPTIONS_OUTPUTDATA* m_pOptions_DA{ nullptr };
	BOOL m_bsimultaneousStartDA{ false };

	// sweep
	long m_chsweeplength{ 0 };
	long m_sweeplength{ 1000 };
	int m_channel_sweep_start{ 0 };
	int m_channel_sweep_end{ 0 };
	int m_chsweepRefresh{ 0 };
	int m_bytesweepRefresh{ 0 };
	float m_fclockrate{ 10000.f };

	// functions for data acquisition
	BOOL FindDTOpenLayersBoards();
	BOOL SelectDTOpenLayersBoard(const CString& card_name);

	void SetCombostartoutput(int option);

	void StopAcquisition();
	void save_and_close_file();
	BOOL StartAcquisition();
	ECODE StartSimultaneousList();
	BOOL StartOutput();
	void StopOutput();
	void InitAcquisitionInputFile();
	void InitAcquisitionDisplay();

	BOOL InitCyberAmp() const;
	BOOL define_experiment();
	void TransferFilesToDatabase();
	BOOL InitOutput_DA();
	BOOL InitOutput_AD();
	void UpdateViewDataFinal();
	void display_ol_da_error_message(const CHAR* error_string) const;
	void ChainDialog(WORD iID);

	short* ADC_Transfer(short* source_data, const CWaveFormat* pWFormat);
	void under_sample_buffer(short* pRawDataBuf, short* pDTbuf0, const CWaveFormat* pWFormat, const int under_sample_factor);
	void ADC_TransferToChart(short* pRawDataBuf, const CWaveFormat* pWFormat);
	void ADC_TransferToFile(CWaveFormat* pWFormat);
	void InitializeAmplifiers();

	// Overrides
	CdbTableMain* OnGetRecordset();
	void DoDataExchange(CDataExchange* pDX) override;
	void AttachControls();
	void OnInitialUpdate() override;
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

public:
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnInputChannels();
	afx_msg void OnSamplingMode();
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
	afx_msg void OnBnClickedUnzoom();
};
