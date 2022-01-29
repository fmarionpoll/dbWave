#pragma once
#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "./Controls/cdxCRotBevelLine.h"
#include "CViewDao.h"

class CViewSpikeDetection : public CViewDAO
{
protected:
	DECLARE_DYNCREATE(CViewSpikeDetection)
	CViewSpikeDetection();

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKDETECTION };

	CRulerBar m_xspkdscale;
	CComboBox m_CBdetectWhat;
	CComboBox m_CBdetectChan;
	CComboBox m_CBtransform;
	CComboBox m_CBtransform2;
	float m_timefirst = 0.f;
	float m_timelast = 0.f;
	CString m_datacomments{};
	int m_spikeno = -1;
	BOOL m_bartefact = false;
	float m_thresholdval = 0.f;
	int m_ichanselected = 0;
	int m_ichanselected2 = 0;
	cdxCRotBevelLine m_bevel1;
	cdxCRotBevelLine m_bevel2;
	cdxCRotBevelLine m_bevel3;

	// form variables
protected:
	int m_scancount_doc = -1; // number of channels in the data document
	CDWordArray m_DWintervals; // intervals to draw detected spikes

	ChartData m_ChartDataWnd_Detect;
	ChartData m_ChartDataWnd_Source;
	CChartSpikeBarWnd m_ChartSpkWnd_Bar;
	CChartSpikeShapeWnd m_ChartSpkWnd_Shape;

	CScrollBarEx m_filescroll; // data position within file
	SCROLLINFO m_filescroll_infos{}; // infos for scrollbar

	CEditCtrl mm_spikeno; // spike selected
	CEditCtrl mm_thresholdval; // value of threshold
	CEditCtrl mm_timefirst; // first abcissa value
	CEditCtrl mm_timelast; // last abcissa value
	CEditCtrl mm_spkWndDuration; // duration of the spike shape window
	CEditCtrl mm_spkWndAmplitude; // amplitude of the spike shape window
	CEditCtrl mm_ichanselected; // channel selected
	CEditCtrl mm_ichanselected2;

	int m_zoominteger = 0; // zoom length (nb data acq points)
	SPKDETECTARRAY* m_pArrayFromApp = nullptr;
	CSpkDetectArray m_parmsCurrent;
	SPKDETECTPARM* m_pDetectParms = nullptr;
	int m_iDetectParms = 0; // index spk detect parm currently selected / array

	OPTIONS_VIEWDATA* options_viewdata = nullptr; // browse options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr; // measure options

	float m_samplingRate = 0.f; // data sampling rate
	BOOL m_bValidThreshold = false; // flag: TRUE=threshold should be evaluated
	BOOL m_bDetected = false;
	TCHAR m_szbuf[64]{};

	int m_cursorstate = 0;
public:
	void SetViewMouseCursor(int cursormode)
	{
		m_ChartSpkWnd_Bar.SetMouseCursorType(cursormode);
		m_ChartSpkWnd_Shape.SetMouseCursorType(cursormode);
		m_ChartDataWnd_Detect.SetMouseCursorType(cursormode);
		m_ChartDataWnd_Source.SetMouseCursorType(cursormode);
	}

	// Implementation
protected:
	void detectAll(BOOL bAll); // detect from current set of parms or from all
	int detectMethod1(WORD schan); // spike detection, method 1 / m_spkD chan schan
	int detectStim1(int i); // stimulus detection
	void serializeWindowsState(BOOL bSave, int itab = -1);
	void alignDisplayToCurrentSpike();
	void highlightSpikes(BOOL flag = TRUE);
	void scrollFile(UINT nSBCode, UINT nPos);
	void selectSpikeNo(int spikeno, BOOL bMultipleSelection);
	BOOL checkDetectionSettings();
	void updateCB();
	void updateCombosDetectChanAndTransforms();
	void defineStretchParameters();
	void defineSubClassedItems();
	void updateDetectionParameters();
	void updateDetectionControls();
	void updateDetectionSettings(int iSelParms);
	void updateFileParameters(BOOL bUpdateInterface = TRUE);
	void updateDataFile(BOOL bUpdateInterface);
	void updateSpikeFile(BOOL bUpdateInterface = TRUE);
	void updateFileScroll();
	void updateLegends();
	void updateLegendDetectionWnd();
	void updateSpkShapeWndScale(BOOL bSetFromControls = TRUE);
	void updateVTtags();
	void updateSpikeDisplay();

	// public interface to view
public:
	// Overrides
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	// Implementation
public:
	~CViewSpikeDetection() override;
	// print view
protected:
	void PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString PrintConvertFileIndex(long l_first, long l_last);
	void PrintComputePageSize();

	CString PrintGetFileInfos();
	CString PrintDataBars(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect);
	CString PrintSpkShapeBars(CDC* p_dc, CRect* prect, BOOL bAll);
	void PrintCreateFont();
	BOOL PrintGetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first);
	BOOL PrintGetNextRow(int& filenumber, long& l_first, long& very_last);
	int PrintGetNPages();
	void PrintDataCartridge(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect, BOOL bComments, BOOL bBars);

	// gain and bias setting: data and functions
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	CScrollBar m_scrolly; // V scrollbar
	float m_yscaleFactor = 0.f; // div factor for y bar
	int m_VBarMode = 0; // flag V scrollbar state

	HICON m_hBias2 = nullptr;
	HICON m_hZoom2 = nullptr;
	CScrollBar m_scrolly2; // V scrollbar
	float m_yscaleFactor2 = 0.f; // div factor for y bar
	int m_VBarMode2 = 0; // flag V scrollbar state

	void OnGainScroll(UINT nSBCode, UINT nPos, int iID);
	void OnBiasScroll(UINT nSBCode, UINT nPos, int iID);
	void UpdateGainScroll(int iID);
	void UpdateBiasScroll(int iID);
	void SetVBarMode(short bMode, int iID);
	void UpdateTabs();

	// Generated message map functions
public:
	afx_msg void OnMeasureAll();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFirstFrame();
	afx_msg void OnLastFrame();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeDetectchan();
	afx_msg void OnSelchangeTransform();
	afx_msg void OnSelchangeDetectMode();
	afx_msg void OnClear();
	afx_msg void OnEnChangeSpikeno();
	afx_msg void OnArtefact();
	afx_msg void OnFormatXscale();
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatYscaleCentercurve();
	afx_msg void OnFormatYscaleGainadjust();
	afx_msg void OnFormatSplitcurves();
	afx_msg void OnEnChangeThresholdval();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnToolsDetectionparameters();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnToolsDataseries();
	afx_msg void OnEditCopy();
	afx_msg void OnFileSave();
	afx_msg void OnMeasure();

	afx_msg void OnBnClickedBiasbutton();
	afx_msg void OnBnClickedGainbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeSpkWndAmplitude();
	afx_msg void OnEnChangeSpkWndLength();
	afx_msg void OnBnClickedLocatebttn();

	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedClearall();
	afx_msg void OnToolsEditstimulus();
	afx_msg void OnEnChangeChanselected();
	afx_msg void OnBnClickedGain2();
	afx_msg void OnBnClickedBias2();
	afx_msg void OnEnChangeChanselected2();
	afx_msg void OnCbnSelchangeTransform2();

	DECLARE_MESSAGE_MAP()
};
