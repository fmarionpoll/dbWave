// viewdata.cpp : implementation file
//
// View data displays data and allow interactive browsing through the file
// print and copy implemented

#include "stdafx.h"
#include <math.h>
#include "dbWave.h"
#include "dbWave_constants.h"
#include "resource.h"

#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "editctrl.h"
//
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "viewdata.h"

#include "vdordina.h"
#include "vdseries.h"
#include "adinterv.h"
#include "dataseri.h"
#include "MeasureOptions.h"
#include "MeasureProperties.h"
#include "editctrl.h"
#include "cyberamp.h"
#include "ADExperi.h"
#include "mainfrm.h"
#include "CopyAsdl.h"
#include "childfrm.h"
#include "vdabciss.h"
#include "RulerBar.h"

#include "dbEditRecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDataView, CDaoRecordView)

BEGIN_MESSAGE_MAP(CDataView, CDaoRecordView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BIAS_button, &CDataView::OnClickedBias)
	ON_BN_CLICKED(IDC_GAIN_button, &CDataView::OnClickedGain)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FORMAT_XSCALE, &CDataView::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &CDataView::OnFormatYscale)
	ON_COMMAND(ID_EDIT_COPY, &CDataView::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CDataView::OnUpdateEditCopy)
	ON_COMMAND(ID_TOOLS_DATASERIES, &CDataView::OnToolsDataseries)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &CDataView::OnHardwareAdchannels)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &CDataView::OnHardwareAdintervals)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &CDataView::OnCenterCurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &CDataView::OnGainAdjustCurve)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &CDataView::OnSplitCurves)
	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &CDataView::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &CDataView::OnLastFrame)
	ON_MESSAGE(WM_MYMESSAGE, &CDataView::OnMyMessage)
	ON_COMMAND(ID_FORMAT_ALLDATA, &CDataView::OnViewAlldata)
	ON_COMMAND(ID_FORMAT_DATASERIESATTRIBUTES, &CDataView::OnFormatDataseriesattributes)
	ON_COMMAND(ID_TOOLS_MEASUREMODE, &CDataView::OnToolsMeasuremode)
	ON_COMMAND(ID_TOOLS_MEASURE, &CDataView::OnToolsMeasure)
	ON_COMMAND(ID_TOOLS_VERTICALTAGS, &CDataView::OnToolsVerticaltags)
	ON_COMMAND(ID_TOOLS_HORIZONTALCURSORS, &CDataView::OnToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_HORIZONTALCURSORS, &CDataView::OnUpdateToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_VERTICALTAGS, &CDataView::OnUpdateToolsVerticaltags)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &CDataView::OnHardwareDefineexperiment)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CDataView::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CDataView::OnEnChangeTimelast)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FORMAT_SETORDINATES, &CDataView::OnFormatYscale)
	ON_COMMAND(ID_FILE_PRINT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CDaoRecordView::OnFilePrintPreview)
	ON_EN_CHANGE(IDC_YLOWER, &CDataView::OnEnChangeYlower)
	ON_EN_CHANGE(IDC_YUPPER, &CDataView::OnEnChangeYupper)
	ON_CBN_SELCHANGE(IDC_COMBOCHAN, &CDataView::OnCbnSelchangeCombochan)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataView construction, destruction
//	CDataView
//	DoDataExchange
//	~CDataView
/////////////////////////////////////////////////////////////////////////////

CDataView::CDataView()
	: CDaoRecordView(CDataView::IDD)
	, m_yupper(0)
	, m_ylower(0)
{	
	// init parameters

	m_ichanselected = 0;
	m_timefirst = 0.0f;
	m_timelast = 0.0f;
	m_floatNDigits = 1000.0f;	// display with n digits (3)
	m_samplingRate = 1.0f;		// sampling rate (Hz)	
	m_cursorstate = 0;			// cursor state: system arrow
	m_HBarpixelratio=10;		// horizontalbar pixel ratio (%)
	m_VBarpixelratio=30;		// vertical bar pixel ratio
	m_pdatDoc = NULL;
	m_bIsPrinting = FALSE;
	m_currentfileindex=0;
	m_bInitComment=TRUE;

	m_bvalidDoc = FALSE;
	m_binit = FALSE;

	m_bEnableActiveAccessibility=FALSE;
}

CDataView::~CDataView()
{
}

void CDataView::DoDataExchange(CDataExchange* pDX)
{   
	// pass values
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_v1);
	DDX_Text(pDX, IDC_EDIT2, m_v2);
	DDX_Text(pDX, IDC_EDIT3, m_diff);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);

	DDX_Control(pDX, IDC_FILESCROLL, m_filescroll);
	DDX_Text(pDX, IDC_YUPPER, m_yupper);
	DDX_Text(pDX, IDC_YLOWER, m_ylower);
	DDX_Control(pDX, IDC_COMBOCHAN, m_comboSelectChan);
}

BOOL CDataView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

//---------------------------------------------------------------------------
// OnInitial update is called once before the frame is displayed
//---------------------------------------------------------------------------

void CDataView::OnInitialUpdate()
{	
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// bitmap buttons: load icons & set buttons
	m_hBias=AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom=AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hZoom);

	VERIFY(m_VDlineview.SubclassDlgItem(IDC_DISPLAREA_button, this));	
	VERIFY(mm_yupper.SubclassDlgItem(IDC_YUPPER, this));
	VERIFY(mm_ylower.SubclassDlgItem(IDC_YLOWER, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));

	// save coordinates and properties of "always visible" controls
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_DISPLAREA_button,	XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_COMBOCHAN, 		SZEQ_XREQ, SZEQ_YTEQ);

	m_stretch.newProp(IDC_GAIN_button, 		SZEQ_XREQ, SZEQ_YTEQ);	
	m_stretch.newProp(IDC_BIAS_button,		SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar,SZEQ_XREQ, YTEQ_YBEQ);

	m_stretch.newProp(IDC_SOURCE,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMEFIRST,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST,			SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_FILESCROLL,		XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_YLOWER,			SZEQ_XLEQ, SZEQ_YBEQ);

	m_binit = TRUE;
	m_VDlineview.m_bNiceGrid = TRUE;

	// init relation with document, display data, adjust parameters    
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// load browse parameters
	mdPM = &(pApp->vdP);		// viewdata options
	mdMO = &(pApp->vdM);		// measure options
	
	// set data file
	CDaoRecordView::OnInitialUpdate();	
	UpdateFileParameters(TRUE);	// load file parameters
	
	m_VDlineview.m_parms = mdPM->viewdata;
	OnClickedBias();			// init V bar mode: bias (push button)
	OnSplitCurves();			// split curves ...	
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE);
}

// remove objects
void CDataView::OnDestroy() 
{
	CDaoRecordView::OnDestroy();

	DeleteObject(m_hBias);		// bias button (handle)
	DeleteObject(m_hZoom);		// zoom button (handle)
}

// --------------------------------------------------------------------------
// OnSize
// --------------------------------------------------------------------------

void CDataView::OnSize(UINT nType, int cx, int cy)
{	
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}

	CDaoRecordView::OnSize(nType, cx, cy);
}

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView diagnostics

#ifdef _DEBUG
void CDataView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CDataView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CDataView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView database support

CDaoRecordset* CDataView::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

/////////////////////////////////////////////////////////////////////////////
// Operations    
//	OnUpdate
/////////////////////////////////////////////////////////////////////////////

BOOL CDataView::OnMove(UINT nIDMoveCommand) 
{
	BOOL flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
	return flag;
}

//---------------------------------------------------------------------------
// OnUpdate is called whenever the document has changed and 
// the view needs to redisplay some or all of itself
//---------------------------------------------------------------------------

void CDataView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	int iUpdate = NULL;
	switch (LOWORD(lHint))
	{
	case HINT_REPLACEVIEW:
		return;
		break;
	case HINT_CLOSEFILEMODIFIED:	// save current file parms 
		SaveModifiedFile();
		break;
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		m_bInitComment=TRUE;
		UpdateFileParameters();
		iUpdate = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
		break;
	default:
		if (m_pdatDoc == NULL)
			return;
		iUpdate = UPD_ABCISSA | UPD_XSCALE | UPD_ORDINATES | UPD_YSCALE;
		break;
	}
	
	// update controls according to iupdate
	UpdateLegends(iUpdate);
	m_VDlineview.Invalidate();
	SetVBarMode(m_VBarMode);
}

/////////////////////////////////////////////////////////////////////////////
// CDataView message handlers
//	OnActivateView
/////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// OnActivateView()
// -------------------------------------------------------------------------- 

void CDataView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CMainFrame* pmF = (CMainFrame*) AfxGetMainWnd();
	if (bActivate)
	{
		BOOL flag = pmF->SetSecondToolBar(IDR_DBDATATYPE);
		ASSERT(flag);
		//((CChildFrame*)pmF->MDIGetActive())->m_cursorstate = 0;
		pmF->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// --------------------------------------------------------------------------
// Update legends
// UPD_ABCISSA
// CHG_XSCALE	CHG_YSCALE
// UPD_XSCALE	UPD_YSCALE
// CHG_XBAR		CHG_YBAR
// --------------------------------------------------------------------------

void CDataView::UpdateLegends(int ioperation)
{
	if (!m_VDlineview.IsDefined() && !m_bvalidDoc)
		return;

	// ------------------------------------------- update abcissa
	if (ioperation & UPD_ABCISSA)
	{
		m_timefirst = m_VDlineview.GetDataFirst()/m_samplingRate;
		m_timelast = (m_VDlineview.GetDataLast()+1)/m_samplingRate;	
		m_VDlineview.m_xscale.SetRange(&m_timefirst, &m_timelast);
		UpdateFileScroll();
	}

	// ------------------------------------------- update ordinates (max , min)
	/*	if (ioperation & UPD_ORDINATES) // not implemented yet
		{
		} */

	// ------------------------------------------- update y scale val
	if (ioperation & CHG_YSCALE)
	{
		UpdateHZtagsVal();
		ioperation |= CHG_YBAR;
	}	

	// ------------------------------------------- update y scale val
	if (ioperation & UPD_YSCALE)
	{
		ioperation |= CHG_YBAR;
	}	

	// ------------------------------------------- adapt vertical scale
	if (ioperation & CHG_YBAR)
	{
		int max =  m_VDlineview.FromChanlistPixeltoBin(m_ichanselected, 0);
		m_yupper = m_VDlineview.GetChanlistBintoMilliVolts(m_ichanselected, max);
		int min = m_VDlineview.FromChanlistPixeltoBin(m_ichanselected, m_VDlineview.Height());
		m_ylower = m_VDlineview.GetChanlistBintoMilliVolts(m_ichanselected, min);

		m_VDlineview.m_yscale.SetRange(&m_ylower, &m_yupper);
	}

	// -------------------------------------------
	UpdateData(FALSE);	// copy view object to controls
}

// --------------------------------------------------------------------------
// OnClickedBias
// change state of the buttons, update scroll bar (vertical)
// --------------------------------------------------------------------------
void CDataView::OnClickedBias()
{	
	// set bias down and set gain up CButton	
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

// --------------------------------------------------------------------------
// OnClickedGain
// change state of both buttons and update scroll bar (vertical)
// --------------------------------------------------------------------------
void CDataView::OnClickedGain()
{	
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
} 

void CDataView::UpdateChannel(int channel)
{
	m_ichanselected = channel;
	if (m_ichanselected > m_VDlineview.GetChanlistSize() - 1)	// less or equal than max nb of chans?
		m_ichanselected = m_VDlineview.GetChanlistSize() - 1;	// clip to maximum
	else if (m_ichanselected < 0)							// less than 1 channel?
		m_ichanselected = 0;								// clip to minimum (and if 0?)
	if (m_ichanselected != channel)							// new value is different than previous
	{														// change other dependent parameters
		if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption == 1
			&& m_VDlineview.GetNHZtags()>0)
		{
			for (int i = 0; i< m_VDlineview.GetNHZtags(); i++)
				m_VDlineview.SetHZtagChan(i, m_ichanselected);
			UpdateHZtagsVal();
			m_VDlineview.Invalidate();
		}
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	else					 			// new value is same as previous
	{									// change content of control
		UpdateData(FALSE);				// put data into edit controls		
	}
}

// --------------------------------------------------------------------------
// OnClickedYscale	call dialog box
// --------------------------------------------------------------------------
void CDataView::OnFormatYscale()
{
	// init dialog data
	CDataViewOrdinatesDlg dlg;		// dialog box
	dlg.m_plinev = &m_VDlineview;		// pointer to lineview button
	dlg.m_Channel = m_ichanselected;		// current channel

	// invoke dialog box
	if (IDOK == dlg.DoModal())
		UpdateLegends(UPD_ORDINATES | UPD_YSCALE | CHG_YBAR);		

	m_VDlineview.Invalidate();
}

// --------------------------------------------------------------------------
// OnToolsDataSeries()
// --------------------------------------------------------------------------
void CDataView::OnToolsDataseries()
{
	// init dialog data 
	CDataSeriesDlg dlg;
	dlg.m_lineview = &m_VDlineview;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_ichanselected;
	
	// invoke dialog box
	dlg.DoModal();
	m_ichanselected = dlg.m_listindex;
	UpdateLegends(UPD_YSCALE);
}

// --------------------------------------------------------------------------
// OnEditCopy
// launch dialog box
// copy Envelopes of displayed data to clipboard
// --------------------------------------------------------------------------
void CDataView::OnEditCopy()
{	
	CCopyAsDlg dlg;
	dlg.m_nabcissa	= mdPM->hzResolution;
	dlg.m_nordinates= mdPM->vtResolution;
	dlg.m_bgraphics	= mdPM->bgraphics;
	dlg.m_ioption	= mdPM->bcontours;
	dlg.m_iunit		= mdPM->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		mdPM->bgraphics		= dlg.m_bgraphics;
		mdPM->bcontours		= dlg.m_ioption;
		mdPM->bunits		= dlg.m_iunit;
		mdPM->hzResolution	= dlg.m_nabcissa;
		mdPM->vtResolution	= dlg.m_nordinates;

		if (!dlg.m_bgraphics)
			m_VDlineview.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			////////////////////////////////////////////////////////////////////
			CRect oldRect;
			m_VDlineview.GetWindowRect(&oldRect);

			CRect rect(0,0, mdPM->hzResolution, mdPM->vtResolution);
			m_npixels0 = m_VDlineview.Width();

			// create metafile
			CMetaFileDC mDC;
			CDC* pDCRef = GetDC();
			CString csTitle = _T("dbWave\0") + m_pdatDoc->GetTitle();
			csTitle +=_T("\0\0");
			CRect rectBound (0, 0, 21000, 29700);  // dimensions in HIMETRIC units (in .01-millimeter increments)
			BOOL hmDC = mDC.CreateEnhanced(pDCRef, NULL, &rectBound, csTitle);
			ASSERT (hmDC != NULL);

			// Draw document in metafile.
			CClientDC attribDC(this) ;						// Create and attach attribute DC
			mDC.SetAttribDC(attribDC.GetSafeHdc()) ;		// from current screen

			// display curves
			SCOPESTRUCT oldparms;
			oldparms = m_VDlineview.m_parms;
			m_VDlineview.m_parms.bDrawframe = mdPM->bFrameRect;
			m_VDlineview.m_parms.bClipRect	= mdPM->bClipRect;
			m_VDlineview.Print(&mDC, &rect);
			m_VDlineview.m_parms = oldparms;

			// print comments : set font
			memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
			GetObject (GetStockObject (SYSTEM_FONT), sizeof (LOGFONT), (LPSTR) &m_logFont) ;
			m_pOldFont = NULL;
			BOOL flag = m_fontPrint.CreateFontIndirect(&m_logFont);
			m_pOldFont = mDC.SelectObject(&m_fontPrint);
			int lineheight = m_logFont.lfHeight + 5;
			int ypxrow	= 0;
			int xcol	= 10;

			// title & legends
			CString comments;
			comments = _T("Abcissa: ");
			CString content;
			content.Format(_T("%g - %g s"), m_timefirst, m_timelast);
			comments += content;
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format( _T("Vertical bar (ch. 0) = %g mV"),  m_VDlineview.m_yscale.GetScaleIncrement());
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format(_T("Horizontal bar = %g s"), m_VDlineview.m_xscale.GetScaleIncrement());
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// bars
			CBrush* pOldBrush= (CBrush*) mDC.SelectStockObject(BLACK_BRUSH);
			// vertical bar
			mDC.MoveTo(0,ypxrow);
			int bottom = m_VDlineview.m_yscale.GetScaleUnitPixels(rect.Height());
			mDC.LineTo(0, ypxrow - bottom);
			// horizontal bar
			mDC.MoveTo(0,ypxrow);
			int left = m_VDlineview.m_xscale.GetScaleUnitPixels(rect.Width());
			mDC.LineTo(left, ypxrow);

			mDC.SelectObject(pOldBrush);
			if (m_pOldFont != NULL)
				mDC.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

			// Close metafile
			ReleaseDC(pDCRef);
			HENHMETAFILE hEmfTmp = mDC.CloseEnhanced();
			ASSERT (hEmfTmp != NULL);
			if (OpenClipboard())
			{				
				EmptyClipboard();							// prepare clipboard
				SetClipboardData(CF_ENHMETAFILE, hEmfTmp);	// put data
				CloseClipboard();							// close clipboard
			}
			else
			{
				// Someone else has the Clipboard open...
				DeleteEnhMetaFile(hEmfTmp);					// delete data
				MessageBeep(0) ;							// tell user something is wrong!
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL , MB_OK | MB_ICONEXCLAMATION );
			}

			// restore initial conditions
			m_VDlineview.ResizeChannels(m_npixels0, 0);
			m_VDlineview.GetDataFromDoc();
			m_VDlineview.Invalidate();
		}
	}
}

// --------------------------------------------------------------------------
// allow copy operation if document has data
// --------------------------------------------------------------------------
void CDataView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{	
	pCmdUI->Enable(m_VDlineview.IsDefined() != NULL); // if document has data
}

/////////////////////////////////////////////////////////////////////////////////////
// HARDWARE
/////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// OnHardwareAdchannels
// --------------------------------------------------------------------------
#include "ADInputParmsDlg.h"
#include ".\viewdata.h"

void CDataView::OnHardwareAdchannels()
{
	CADInputParmsDlg dlg;
	
	// init dialog data
	dlg.m_pwFormat = m_pdatDoc->GetpWaveFormat();
	dlg.m_pchArray = m_pdatDoc->GetpWavechanArray();
	
	// invoke dialog box	
	if (IDOK == dlg.DoModal())
	{
		m_pdatDoc->AcqSaveDataDescriptors();
		m_pdatDoc->SetModifiedFlag(TRUE);
	}
}

// --------------------------------------------------------------------------
// OnHardwareAdintervals
// --------------------------------------------------------------------------
void CDataView::OnHardwareAdintervals()
{
	ADIntervalsDlg dlg;
	// init dialog data	
	dlg.m_pwaveFormat = m_pdatDoc->GetpWaveFormat();

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_pdatDoc->AcqSaveDataDescriptors();
		m_pdatDoc->SetModifiedFlag(TRUE);
	}
}

// --------------------------------------------------------------------------
// Chaindialog
// some dialogs return an ID in a parameter
// (OnHardware.. adchannels, adtrigger, adintervals)
// this routine launch the dialog box corresponding to this ID
// ! this ID must be different from the menu_ID, otherwise, when the
// corresponding button is depressed, the dialog box is CALLED over the
// current one..
// --------------------------------------------------------------------------
void CDataView::ChainDialog(WORD iID)
{
	WORD menuID;
	switch (iID)
	{
	case IDC_ADINTERVALS:
		menuID = ID_HARDWARE_ADINTERVALS;
		break;
	case IDC_ADCHANNELS:
		menuID = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
		break;
	}
	PostMessage(WM_COMMAND, menuID, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// OnFormat procedures
// --------------------------------------------------------------------------
void CDataView::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void CDataView::OnLastFrame()
{	
	OnFileScroll(SB_RIGHT, 1L);
}

/////////////////////////////////////////////////////////////////////////////
// OnView procedures: multi-files document

// --------------------------------------------------------------------------
// UpdateFileParameters
// protected procedure that allows the selection of a new file
// with different parameters (length, rate, comments, etc)
// --------------------------------------------------------------------------
void CDataView::UpdateFileParameters(BOOL bUpdateInterface)
{
	// load parameters from document file: none yet loaded?
	BOOL bFirstUpdate = (m_pdatDoc == NULL);
	CdbWaveDoc* pdbDoc = GetDocument();
	CString csDatFile = pdbDoc->DBGetCurrentDatFileName();
	if (csDatFile.IsEmpty())
	{
		m_bvalidDoc = FALSE;
		return;
	}
	m_bvalidDoc = TRUE;

	// open data file
	if (!pdbDoc->OpenCurrentDataFile())
	{
		MessageBox(_T("This data file could not be opened"), _T("The file might be missing, or inaccessible..."), MB_OK);
		m_bvalidDoc = FALSE;
		return;
	}
	m_pdatDoc = pdbDoc->m_pDat;
	m_pdatDoc->ReadDataInfos();
	CWaveFormat* pwaveFormat = m_pdatDoc->GetpWaveFormat();

	if (bFirstUpdate)
	{
		m_samplingRate = pwaveFormat->chrate;	// load sampling rate
		m_timefirst=0.0f;						// init file size
		m_timelast=(m_pdatDoc->GetDOCchanLength())/m_samplingRate;
	}

	// load parameters from current data file
	m_VDlineview.AttachDataFile(m_pdatDoc, 0);	// prepare lineview
	m_pdatDoc->SetModifiedFlag(FALSE);	

	// ----------- option ------------------------------------------
	// display complete file?? ABCISSA
	// ----------- option ------------------------------------------

	// OPTION: display entire file				// (inactif si multirow)
	long lFirst=0;								// first abcissa
	long lLast = m_pdatDoc->GetDOCchanLength()-1;	// last abcissa	

	if (!mdPM->bEntireRecord || mdPM->bMultirowDisplay && !bFirstUpdate)
	{
		lFirst = (long) (m_timefirst*m_samplingRate);
		lLast = (long) (m_timelast*m_samplingRate);
		if (lLast > m_pdatDoc->GetDOCchanLength()-1)	// last OK?
			lLast = m_pdatDoc->GetDOCchanLength()-1;	// clip to the end of the file
	}    
	m_samplingRate = pwaveFormat->chrate;			// update sampling rate	

	// ----------- option -----------------------------------------
	// display all channels?: CHANNELS
	// ----------- option -----------------------------------------	

	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	int lnvchans = m_VDlineview.GetChanlistSize();
	int ndocchans = pwaveFormat->scan_count;
	
	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	if (mdPM->bAllChannels || lnvchans == 0)
	{		
		for (int jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{										// check if present in the list
			BOOL bPresent=FALSE;				// pessimistic
			for (int j = lnvchans-1; j>= 0; j--)// check all channels / display list
			{									// test if this data chan is present + no transf
				if (m_VDlineview.GetChanlistSourceChan(j) == jdocchan)
				{
					bPresent = TRUE;			// the wanted chan is present
					break;						// examine next doc channel
				}
			}
			if (bPresent == FALSE)				// no display chan contains that doc chan
			{
				m_VDlineview.AddChanlistItem(jdocchan, 0);	// add this channel
				lnvchans++;
			}
			m_VDlineview.SetChanlistColor(jdocchan, jdocchan);
		}
	}

	// load real data from file and update time parameters
	m_VDlineview.GetDataFromDoc(lFirst, lLast);	// load data requested	
	m_timefirst = m_VDlineview.GetDataFirst()/ m_samplingRate;	// update abcissa parameters
	m_timelast = m_VDlineview.GetDataLast()/ m_samplingRate;	// first - end
	m_ichanselected = 0;										// select chan 0

	// ----------- option ------------------------------------------
	// split curves??
	// ----------- option ------------------------------------------	

	if (!bFirstUpdate)
	{
		UpdateChannelsDisplayParameters();
	}

	// fill combo
	m_comboSelectChan.ResetContent();
	for (int i = 0; i < lnvchans; i++)
	{
		CString cs;
		cs.Format(_T("channel %i - "), i);
		cs = cs + m_VDlineview.GetChanlistComment(i);
		m_comboSelectChan.AddString(cs);
	}
	if (ndocchans > 1)
		m_comboSelectChan.AddString(_T("all channels"));
	m_comboSelectChan.SetCurSel(0);

	// done	
	if (bUpdateInterface)
	{
		m_VDlineview.m_xscale.SetRange(&m_timefirst, &m_timelast);
		UpdateFileScroll();
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | CHG_YSCALE);
		m_VDlineview.Invalidate();
	}
}

void CDataView::UpdateChannelsDisplayParameters()
{
	int max, min;
	int lnvchans = m_VDlineview.GetChanlistSize();
	int i, iextent, izero;
	int j = lnvchans-1;
	int iextent0 = m_VDlineview.GetChanlistYextent(0);
	int izero0	 = m_VDlineview.GetChanlistYzero(0);

	for (i=0; i<lnvchans; i++)
	{
		// keep final gain constant even if ampli gain changed
		m_VDlineview.GetChanlistMaxMin(i, &max, &min);
		iextent = m_VDlineview.GetChanlistYextent(i);
		izero = m_VDlineview.GetChanlistYzero(i);

		// split curves if requested by options
		if (mdPM->bSplitCurves)
		{
			if (mdPM->bMaximizeGain)
				iextent = MulDiv(max-min+1, 11*lnvchans, 10);
			if (mdPM->bCenterCurves)
			{				
				izero = (max+min)/2 - MulDiv(iextent, j, lnvchans*2);
				j -= 2;				
			}
		}
		// maximize gain
		else
		{
			if (mdPM->bMaximizeGain)
				iextent = MulDiv(max-min+1, 11, 10);
			// center curve
			if (mdPM->bCenterCurves)
				izero = (max+min)/2;
		}
		m_VDlineview.SetChanlistYextent(i, iextent);
		m_VDlineview.SetChanlistZero(i, izero);
	}

	if (m_VDlineview.GetChanlistYextent(0) != iextent0 || m_VDlineview.GetChanlistYzero(0) != izero0)
	{
		int i = 0;
		int max, min;
		max = m_VDlineview.FromChanlistPixeltoBin(i, 0);
		min = m_VDlineview.FromChanlistPixeltoBin(i, m_VDlineview.Height());
		float xmax = m_VDlineview.GetChanlistBintoMilliVolts(i, max);
		float xmin = m_VDlineview.GetChanlistBintoMilliVolts(i, min);
		ASSERT(xmax > xmin);
		m_VDlineview.m_yscale.SetRange(&xmin, &xmax);
	}
}
// --------------------------------------------------------------------------
// message handler to messages from lineview
// --------------------------------------------------------------------------
void CDataView::SetCursorAssociatedWindows()
{
	int nCmdShow  = SW_HIDE;
	if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption ==1
		&& m_VDlineview.GetNHZtags()>0)
		nCmdShow = SW_SHOW;

	// change windows state: edit windows
	GetDlgItem(IDC_STATIC1)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC3)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT1)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT3)->ShowWindow(nCmdShow);

	// hide other windows which occupy the same space
	if (nCmdShow == SW_SHOW)
		nCmdShow = SW_HIDE;
	else
		nCmdShow = SW_SHOW;
	// change cursors val
	if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption ==1)
		UpdateHZtagsVal();
}

void CDataView::UpdateHZtagsVal()
{
	if (m_VDlineview.GetNHZtags()<=0)
		return;
	int v1 = m_VDlineview.GetHZtagVal(0);
	int itag=0;
	if (m_VDlineview.GetNHZtags()>1)
		itag=1;
	int v2 = m_VDlineview.GetHZtagVal(itag);
	float mVperBin = m_VDlineview.GetChanlistVoltsperBin(m_ichanselected)*1000.0f;
	m_v1=((float)v1)*mVperBin;
	m_v2=((float)v2)*mVperBin;
	m_diff=m_v1-m_v2;
	
	TCHAR szBuffer[32];
	_stprintf_s(szBuffer, 32, _T("%.*g"), FLT_DIG, m_v1);	
	GetDlgItem(IDC_EDIT1)->SetWindowText(szBuffer);
	GetDlgItem(IDC_EDIT1)->Invalidate();

	_stprintf_s(szBuffer, 32, _T("%.*g"), FLT_DIG, m_v2);
	GetDlgItem(IDC_EDIT2)->SetWindowText(szBuffer);
	GetDlgItem(IDC_EDIT2)->Invalidate();

	_stprintf_s(szBuffer, 32, _T("%.*g"), FLT_DIG, m_diff);
	GetDlgItem(IDC_EDIT3)->SetWindowText(szBuffer);
	GetDlgItem(IDC_EDIT3)->Invalidate();	
}

LRESULT CDataView::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int j = HIWORD(lParam);		// control ID of sender
	// code = 0: chan hit 			lowp = channel
	// code = 1: cursor change		lowp = new cursor value
	// code = 2: horiz cursor hit	lowp = cursor index	
	int lowp = LOWORD(lParam);	// value associated

	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		// save current cursors into document if cursorstate = 3
		if (m_cursorstate == CURSOR_MEASURE)
		{
			if (mdMO->wOption == 0)	// vertical cursors
			{
				CTagList* ptaglist = m_pdatDoc->GetpVTtags();
				ptaglist->CopyTagList(m_VDlineview.GetVTtagList());
				m_VDlineview.DelAllVTtags();
			}
			else if (mdMO->wOption ==1)	// horizontal cursors
			{
				CTagList* ptaglist = m_pdatDoc->GetpHZtags();
				ptaglist->CopyTagList(m_VDlineview.GetHZtagList());
				m_VDlineview.DelAllHZtags();
			}
			else if (mdMO->wOption ==3) // detect stimulus
			{
				mdMO->wStimuluschan = m_VDlineview.GetHZtagChan(0);
				mdMO->wStimulusthresh = m_VDlineview.GetHZtagVal(0);
				m_VDlineview.DelAllHZtags();	
			}
			m_VDlineview.Invalidate();
		}
		// change cursor value (+1), clip to upper cursor value
		if (lowp > CURSOR_MEASURE) 
			lowp = 0;
		// change cursor and tell parent that it has changed
		m_cursorstate = m_VDlineview.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));

		// recall cursors from document if cursorstate = 2
		if (m_cursorstate == CURSOR_MEASURE)
		{
			if (mdMO->wOption == 0)
				m_VDlineview.SetVTtagList(m_pdatDoc->GetpVTtags());
			else if (mdMO->wOption ==1)
				m_VDlineview.SetHZtagList(m_pdatDoc->GetpHZtags());
			else if (mdMO->wOption == 3)
				m_VDlineview.AddHZtag(mdMO->wStimulusthresh, mdMO->wStimuluschan);
			m_VDlineview.Invalidate();
		}
		SetCursorAssociatedWindows();
		break;

	case HINT_HITCHANNEL:	// change channel if different
		m_ichanselected = lowp;	
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
		break;

	case HINT_DEFINEDRECT:
		{
		CRect rect = m_VDlineview.GetDefinedRect();
		mdMO->wLimitSup = (WORD) rect.top;

		mdMO->wLimitInf = (WORD) rect.bottom;
		mdMO->lLimitLeft = m_VDlineview.GetDataOffsetfromPixel(rect.left);
		mdMO->lLimitRight = m_VDlineview.GetDataOffsetfromPixel(rect.right);
		}
		// action according to option
		switch (mdMO->wOption)
		{
		// ......................  vertical tags
		case 0:					// if no VTtags, then take those of rectangle, or limits of lineview
			m_VDlineview.AddVTLtag(mdMO->lLimitLeft);
			if (mdMO->lLimitRight != mdMO->lLimitLeft)
				m_VDlineview.AddVTLtag(mdMO->lLimitRight);
			// store new VT tags into document
			m_pdatDoc->GetpVTtags()->CopyTagList(m_VDlineview.GetVTtagList());
			break;

		// ......................  horizontal cursors
		case 1:					// if no HZcursors, take those of rectangle or limits of lineview
			m_VDlineview.AddHZtag(m_VDlineview.FromChanlistPixeltoBin(m_ichanselected, mdMO->wLimitSup), m_ichanselected);
			if (mdMO->wLimitInf != mdMO->wLimitSup)
				m_VDlineview.AddHZtag(m_VDlineview.FromChanlistPixeltoBin(m_ichanselected, mdMO->wLimitInf), m_ichanselected);
			m_pdatDoc->GetpHZtags()->CopyTagList(m_VDlineview.GetHZtagList());
			if (m_VDlineview.GetNHZtags()==2)
				SetCursorAssociatedWindows();
			UpdateHZtagsVal();
			break;

		// ......................  rectangle area
		//case 2:				// parameters are already within lineview and mdMO
		//	break;
		// ......................  detect stimulus and then measure
		//case 3:				// if not displayed, plot HZ detection cursor
		//	break;
		default:
			break;
		}
		m_VDlineview.Invalidate();
		break;

	case HINT_CHANGEHZTAG:	// horizontal tag has changed 	lowp = tag nb
		if (mdMO->wOption == 3)
			mdMO->wStimulusthresh = m_VDlineview.GetHZtagVal(0);
		else
			UpdateHZtagsVal();
		break;

	case HINT_VIEWSIZECHANGED:       // change zoom
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE);
		m_VDlineview.Invalidate();
		SetVBarMode(m_VBarMode);
		break;

	case HINT_WINDOWPROPSCHANGED:
		mdPM->viewdata = m_VDlineview.m_parms;
		break;
	default:
		break;
	}	
	return 0L;
}

void CDataView::OnViewAlldata()
{
	m_VDlineview.GetDataFromDoc(0, GetDocument()->DBGetDataLen()-1);
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	UpdateData(FALSE);
	m_VDlineview.Invalidate();    
	m_VDlineview.m_xscale.SetRange(&m_timefirst, &m_timelast);
	UpdateFileScroll();
}

void CDataView::OnFormatDataseriesattributes() 
{
	CDataSeriesFormatDlg dlg;
	dlg.m_plineview = &m_VDlineview;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_ichanselected;
	
	// invoke dialog box
	if (IDOK == dlg.DoModal()) 
	{
		m_ichanselected = dlg.m_listindex;
	}	
	UpdateLegends(UPD_YSCALE);
	m_VDlineview.Invalidate();
	
}

/////////////////////////////////////////////////////////////////////////////
// measure routines

void CDataView::OnToolsVerticaltags() 
{
	MeasureProperties(1);
}

void CDataView::OnToolsHorizontalcursors() 
{
	MeasureProperties(0);
}

void CDataView::OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI) 
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	BOOL flag = (mdMO->wOption ==1  && m_cursorstate == 2);
	pCmdUI->Enable(flag);
}

void CDataView::OnUpdateToolsVerticaltags(CCmdUI* pCmdUI) 
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	BOOL flag = (mdMO->wOption ==0 && m_cursorstate == 2);
	pCmdUI->Enable(flag);
}

void CDataView::OnToolsMeasuremode() 
{
	MeasureProperties(2);
}

void CDataView::OnToolsMeasure() 
{
	MeasureProperties(3);
}

// --------------------------------------------------------------------------
// OnVScroll
// --------------------------------------------------------------------------

void CDataView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == NULL)
	{
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	
	// CDataView scroll: vertical scroll bar
	switch (m_VBarMode)
	{
	case BAR_GAIN:
		OnGainScroll(nSBCode, nPos);
		break;
	case BAR_BIAS:
		OnBiasScroll(nSBCode, nPos);
	default:		
		break;
	}
}

// --------------------------------------------------------------------------
// SetVBarMode
// --------------------------------------------------------------------------
void CDataView::SetVBarMode (short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}        

// --------------------------------------------------------------------------
// UpdateGainScroll()
// --------------------------------------------------------------------------

void CDataView::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(
			m_VDlineview.GetChanlistYextent(m_ichanselected), 
			100, 
			YEXTENT_MAX)
		+50, 
		TRUE);
}

// --------------------------------------------------------------------------
// OnGainScroll()
// --------------------------------------------------------------------------
void CDataView::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int lSize = m_VDlineview.GetChanlistYextent(m_ichanselected);
	// get corresponding data
	switch (nSBCode)
	{
	// .................scroll to the start
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
	// .................scroll one line left
	case SB_LINELEFT:	lSize -= lSize/10 +1; break;
	// .................scroll one line right
	case SB_LINERIGHT:	lSize += lSize/10 +1; break;
	// .................scroll one page left
	case SB_PAGELEFT:	lSize -= lSize/2+1; break;
	// .................scroll one page right
	case SB_PAGERIGHT:	lSize += lSize+1; break;
	// .................scroll to end right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
	// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos-50,YEXTENT_MAX,100); break;
	// .................NOP: set position only
	default:			break;			
	}

	// change y extent
	if (lSize>0 ) //&& lSize<=YEXTENT_MAX)
	{
		UpdateYExtent(m_ichanselected, lSize);
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll();
}

void CDataView::UpdateYExtent(int ichan, int yextent) 
{
	m_VDlineview.SetChanlistYextent(ichan, yextent);
	if (m_comboSelectChan.GetCurSel() == m_VDlineview.GetChanlistSize())
	{
		for (int i= 0; i < m_VDlineview.GetChanlistSize(); i++)
			m_VDlineview.SetChanlistYextent(i, yextent);
	}
	m_VDlineview.Invalidate();
}

// --------------------------------------------------------------------------
// UpdateBiasScroll()
// -- not very nice code; interface counter intuitive
// --------------------------------------------------------------------------

void CDataView::UpdateBiasScroll()
{
	int iPos = (int) ((m_VDlineview.GetChanlistYzero(m_ichanselected)
						- m_VDlineview.GetChanlistBinZero(m_ichanselected))
					*100/(int)YZERO_SPAN)+(int)50;
	m_scrolly.SetScrollPos(iPos, TRUE);
	UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
}

// --------------------------------------------------------------------------
// OnBiasScroll()
// --------------------------------------------------------------------------

void CDataView::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	int lSize =  m_VDlineview.GetChanlistYzero(m_ichanselected) - m_VDlineview.GetChanlistBinZero(m_ichanselected);
	int yextent = m_VDlineview.GetChanlistYextent(m_ichanselected);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
		lSize = YZERO_MIN;
		break;
	case SB_LINELEFT:		// scroll one line left
		lSize -= yextent/100+1;
		break;
	case SB_LINERIGHT:		// scroll one line right
		lSize += yextent/100+1;
		break;
	case SB_PAGELEFT:		// scroll one page left
		lSize -= yextent/10+1;
		break;
	case SB_PAGERIGHT:		// scroll one page right
		lSize += yextent/10+1;
		break;
	case SB_RIGHT:			// scroll to end right
		lSize = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		lSize = (nPos-50)*(YZERO_SPAN/100);
		break;
	default:				// NOP: set position only
		break;			
	}

	// try to read data with this new size
	if (lSize>YZERO_MIN && lSize<YZERO_MAX)
	{		
		UpdateYZero(m_ichanselected, lSize + m_VDlineview.GetChanlistBinZero(m_ichanselected));
		m_VDlineview.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll();
}

void CDataView::UpdateYZero(int ichan, int ybias)
{
	m_VDlineview.SetChanlistZero(ichan, ybias);
	if (m_comboSelectChan.GetCurSel() == m_VDlineview.GetChanlistSize())
	{
		for (int i = 0; i < m_VDlineview.GetChanlistSize(); i++)
			m_VDlineview.SetChanlistZero(i, ybias);
	}
	m_VDlineview.Invalidate();
}

// --------------------------------------------------------------------------
// OnCenterCurve()
// --------------------------------------------------------------------------

void CDataView::OnCenterCurve()
{
	m_VDlineview.CenterChan(m_ichanselected);
	m_VDlineview.Invalidate();
}

// --------------------------------------------------------------------------
// OnGainAdjustCurve()
// --------------------------------------------------------------------------

void CDataView::OnGainAdjustCurve()
{
	m_VDlineview.MaxgainChan(m_ichanselected);	
	m_VDlineview.Invalidate();
	UpdateLegends(CHG_YSCALE);	
}

// --------------------------------------------------------------------------
// OnSplitcurves()
// adjust position of each channel when new document is loaded for ex
// get nb of channels in the display list
// loop through all channels to get max min and center / adjust gain
// to display all signals on separate lines
// --------------------------------------------------------------------------
void CDataView::OnSplitCurves()
{
	int i;
	int nchans = m_VDlineview.GetChanlistSize();	// nb of data channels
	int pxheight = m_VDlineview.Height();				// height of the display area
	int pxoffset = pxheight/nchans;			// height for each channel
	int pxzero = (pxheight - pxoffset)/2;	// center first curve at

	// split display area	
	int  max, min;
	for (i=0; i<nchans; i++)
	{
		m_VDlineview.GetChanlistMaxMin(i, &max, &min);
		int iextent = MulDiv(max-min+1, 100*nchans, 100);
		
		int ibias = MulDiv(pxzero, iextent, pxheight);  // convert pixel into bins
		int izero = (max+min)/2 - ibias;				// change bias
		m_VDlineview.SetChanlistYextent(i, iextent);
		m_VDlineview.SetChanlistZero(i, izero);
		pxzero -= pxoffset;								// update position of next curve
	}
	UpdateLegends(CHG_YSCALE);
	m_VDlineview.Invalidate();
}

// --------------------------------------------------------------------------
// OnFileScroll()
// --------------------------------------------------------------------------

void CDataView::OnFileScroll(UINT nSBCode, UINT nPos)
{
	BOOL bResult=FALSE;	
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
	case SB_LINELEFT:		// scroll one line left
	case SB_LINERIGHT:		// scroll one line right
	case SB_PAGELEFT:		// scroll one page left
	case SB_PAGERIGHT:		// scroll one page right
	case SB_RIGHT:			// scroll to end right
		bResult = m_VDlineview.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		bResult = m_VDlineview.GetDataFromDoc(
			(nPos * m_pdatDoc->GetDOCchanLength()) / 100L);
		break;					
	default:				// NOP: set position only
		break;			
	}

	// adjust display
	if (bResult)
	{
		UpdateLegends(UPD_ABCISSA);
		UpdateData(FALSE);	// copy view object to controls
		m_VDlineview.Invalidate();
	}
	m_VDlineview.m_xscale.SetRange(&m_timefirst, &m_timelast);
	UpdateFileScroll();
}

// --------------------------------------------------------------------------
// OnHScroll()
// --------------------------------------------------------------------------

void CDataView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar == NULL)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from CScrollBarEx
	int lFirst;
	int lLast;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL );
		lFirst = m_filescroll_infos.nPos;
		lLast = lFirst + m_filescroll_infos.nPage - 1;
		m_timefirst = (float) lFirst / m_samplingRate;
		m_timelast	= (float) lLast / m_samplingRate;
		m_VDlineview.GetDataFromDoc(lFirst, lLast);
		m_VDlineview.Invalidate();
		m_VDlineview.m_xscale.SetRange(&m_timefirst, &m_timelast);
		cs.Format(_T("%.3f"), m_timefirst);
		SetDlgItemText(IDC_TIMEFIRST, cs);
		cs.Format(_T("%.3f"), m_timelast);
		SetDlgItemText(IDC_TIMELAST, cs);
		break;

	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL );
		lFirst = m_filescroll_infos.nPos;
		lLast = lFirst + m_filescroll_infos.nPage - 1;
		if (m_VDlineview.GetDataFromDoc(lFirst, lLast))
		{
			UpdateLegends(UPD_ABCISSA);
			UpdateData(FALSE);	// copy view object to controls
			m_VDlineview.Invalidate();
		}
		
		break;
	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void CDataView::MeasureProperties(int item)
{
	// make sure that cursor is ok
	if (m_cursorstate != CURSOR_MEASURE)
		OnMyMessage(NULL, MAKELPARAM(CURSOR_MEASURE, HINT_SETMOUSECURSOR));

	// save current data into data document
	switch (mdMO->wOption)
	{
	case 0:	m_pdatDoc->GetpVTtags()->CopyTagList(m_VDlineview.GetVTtagList()); break;
	case 1: m_pdatDoc->GetpHZtags()->CopyTagList(m_VDlineview.GetHZtagList()); break;
	case 3: 
		mdMO->wStimuluschan = m_VDlineview.GetHZtagChan(0);
		mdMO->wStimulusthresh = m_VDlineview.GetHZtagVal(0);
		break;
	default: break;
	}

	// call routine and pass parameters
	CMeasureProperties dlg(NULL, item);
	dlg.m_plineview = &m_VDlineview;
	dlg.m_samplingrate = m_samplingRate;
	dlg.m_pdatDoc = m_pdatDoc;
	dlg.m_pdbDoc = GetDocument();
	dlg.m_pMO = mdMO;
	dlg.m_currentchan=m_ichanselected;

	dlg.DoModal();
	m_VDlineview.Invalidate();	
	SetCursorAssociatedWindows();
}

void CDataView::SaveModifiedFile()
{
	// save previous file if anything has changed
	if (m_pdatDoc == NULL)
		return;

	if(m_pdatDoc->IsModified())
	{
		CString docname = GetDocument()->DBGetCurrentDatFileName();
		m_pdatDoc->OnSaveDocument(docname);
	}
	m_pdatDoc->SetModifiedFlag(FALSE);
}

void CDataView::OnHardwareDefineexperiment()
{
	CdbEditRecordDlg dlg;
	dlg.m_pdbDoc = GetDocument();
	
	if (IDOK == dlg.DoModal())
	{
		CdbWaveDoc* pdbDoc = GetDocument();
		long recordID = pdbDoc->DBGetCurrentRecordID();;
		GetDocument()->UpdateAllViews(NULL, HINT_DOCHASCHANGED, NULL);
		pdbDoc->DBMoveToID(recordID);
		pdbDoc->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
	}
}

// --------------------------------------------------------------------------
// OnClickedXscale	call dialog box
// --------------------------------------------------------------------------

void CDataView::OnFormatXscale()
{
	// init dialog data 
	CDataViewAbcissaDlg dlg;
	dlg.m_firstAbcissa=m_timefirst;
	dlg.m_lastAbcissa=m_timelast;
	dlg.m_veryLastAbcissa = m_VDlineview.GetDocumentLast()/m_samplingRate;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_timefirst = dlg.m_firstAbcissa * dlg.m_abcissaScale;
		m_timelast  = dlg.m_lastAbcissa * dlg.m_abcissaScale;
		m_VDlineview.GetDataFromDoc((long) (m_timefirst*m_samplingRate), 
									(long) (m_timelast*m_samplingRate));
		UpdateLegends(UPD_ABCISSA | UPD_XSCALE | CHG_XBAR);		
	}
}

//-----------------------------------------------------------------------
// compute printer's page dot resolution
// borrowed from VC++ sample\drawcli\drawdoc.cpp
//-----------------------------------------------------------------------
void CDataView::ComputePrinterPageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	HDC hDC= dlg.CreatePrinterDC();     // to delete at the end -- see doc!
	ASSERT(hDC != NULL);
	dc.Attach(hDC);

	// Get the size of the page in pixels
	mdPM->horzRes=dc.GetDeviceCaps(HORZRES);
	mdPM->vertRes=dc.GetDeviceCaps(VERTRES);	

	// margins (pixels)
	m_printRect.right = mdPM->horzRes-mdPM->rightPageMargin;
	m_printRect.bottom = mdPM->vertRes-mdPM->bottomPageMargin;
	m_printRect.left = mdPM->leftPageMargin;
	m_printRect.top = mdPM->topPageMargin;
}

/////////////////////////////////////////////////////////////////////////////
// print paging information, date on the bottom of the page
/////////////////////////////////////////////////////////////////////////////

void CDataView::PrintFileBottomPage(CDC* pDC, CPrintInfo* pInfo)
{
	CTime t= CTime::GetCurrentTime();
	TCHAR ch[256];
	wsprintf(ch,_T("  page %d:%d %d-%d-%d"), // %d:%d",
			pInfo->m_nCurPage, pInfo->GetMaxPage(),
			t.GetDay(),	t.GetMonth(),t.GetYear());

	CString csDatFile = GetDocument()->DBGetCurrentDatFileName();
	int icount = csDatFile.ReverseFind(_T('\\'));
	CString ch_date = csDatFile.Left(icount);
	ch_date = ch_date.Left(ch_date.GetLength() -1)+ ch;
	pDC->SetTextAlign(TA_CENTER);
	pDC->TextOut(mdPM->horzRes/2, mdPM->vertRes-57,	ch_date);
}

// ---------------------------------------------------------------------------------------
// ConvertFileIndex
// return ascii string
// ---------------------------------------------------------------------------------------

CString CDataView::ConvertFileIndex(long lFirst, long lLast)
{
	CString csUnit= _T(" s");								// get time,  prepare time unit

	TCHAR szValue[64];										// buffer to receive ascii represent of values
	LPTSTR pszValue = szValue;
	float xScaleFactor;										// scale factor returned by changeunit
	float x = m_VDlineview.ChangeUnit( (float) lFirst / m_samplingRate, &csUnit, &xScaleFactor);
	int fraction = (int) ((x - ((int) x)) * (float) 1000.);	// separate fractional part
	wsprintf(pszValue, _T("time = %i.%03.3i - "), (int) x, fraction); // print value
	CString csComment = pszValue;							// save ascii to string

	x = lLast / (m_samplingRate * xScaleFactor);			// same operations for last interval
	fraction = (int) ((x - ((int) x)) *  (float) 1000.);
	wsprintf(pszValue, _T("%i.%03.3i %s"), (int) x, fraction, (LPCTSTR) csUnit);
	csComment += pszValue;
	return csComment;
}

// -----------------------------------------------------------------------------------
// GetFileSeriesIndexFromPage
//
// parameters
//	page : current printer page
//	file : filelist index
// returns lFirst = index first pt to display
// assume correct parameters:
// 	m_lprintFirst
// 	m_lprintLen
//
// count how many rows by reading the length of each file of the list starting from the
// first. Stop looping through list whenever count of rows is reached or file number is
// exhausted
// -----------------------------------------------------------------------------------

BOOL CDataView::GetFileSeriesIndexFromPage(int page, int &filenumber, long &lFirst)
{
	// loop until we get all rows
	int totalrows= m_nbrowsperpage*(page-1);
	lFirst = m_lprintFirst;
	filenumber = 0;						// file list index
	if (mdPM->bPrintSelection)			// current file if selection only
		filenumber = m_file0;
	else
		GetDocument()->DBMoveFirst();

	long veryLast = m_lprintFirst + m_lprintLen;
	if (mdPM->bEntireRecord)
		veryLast= m_pdatDoc->GetDOCchanLength()-1;

	for (int row = 0; row <totalrows; row++)
	{
		if (!PrintGetNextRow(filenumber, lFirst, veryLast))
			break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// print text for a file items
// take care of user's options saved in mdPM structure
/////////////////////////////////////////////////////////////////////////////

CString CDataView::GetFileInfos()
{
	CString strComment;   					// scratch pad
	CString Tab(_T("    "));					// use 4 spaces as tabulation character
	CString RC(_T("\n"));						// next line

	// document's name, date and time
	CWaveFormat* pwaveFormat = m_pdatDoc->GetpWaveFormat();
	if (mdPM->bDocName || mdPM->bAcqDateTime)// print doc infos?
	{
		if (mdPM->bDocName)					// print file name
		{
			strComment += GetDocument()->DBGetCurrentDatFileName() + Tab;
		}
		if (mdPM->bAcqDateTime)				// print data acquisition date & time
		{
			CString date = (pwaveFormat->acqtime).Format(_T("%#d %B %Y %X")); //("%c");
			strComment +=  date;
		}
		strComment += RC;   
	}

	// document's main comment (print on multiple lines if necessary)
	if (mdPM->bAcqComment)
		strComment += pwaveFormat->GetComments(_T(" ")) + RC;

	return strComment;
}       

// ---------------------------------------------------------------------------------------
// PrintBars
// print bars and afferent comments ie value for each channel
// return comment associated with bars and individual channels
// ---------------------------------------------------------------------------------------
CString CDataView::PrintBars(CDC* pDC, CRect* prect)
{
	CString strComment;
	CString RC(_T("\n"));
	CString Tab(_T("     "));

	CBrush* pOldBrush= (CBrush*) pDC->SelectStockObject(BLACK_BRUSH);
	TCHAR szValue[64];
	LPTSTR lpszVal = szValue;	
	CString csUnit;
	CString csComment;
	float xScaleFactor;
	CPoint barOrigin(-10,-10);					// origine barre à 10,10 pts de coin inf gauche rectangle
	barOrigin.x += prect->left;
	barOrigin.y += prect->bottom;
	CPoint xbarEnd = barOrigin;
	CPoint ybarEnd = barOrigin;

	// same len ratio as displayed on viewdata
	int horzBar = m_VDlineview.m_xscale.GetScaleUnitPixels(m_VDlineview.Width());
	ASSERT(horzBar > 0);
	int vertBar = m_VDlineview.m_yscale.GetScaleUnitPixels(m_VDlineview.Height());
	ASSERT(vertBar > 0);

	///// time abcissa ///////////////////////////	
	csComment = ConvertFileIndex(m_VDlineview.GetDataFirst(), m_VDlineview.GetDataLast());

	///// horizontal time bar ///////////////////////////	
	if (mdPM->bTimeScaleBar)           
	{
		// print horizontal bar
		xbarEnd.x	+= horzBar;
		pDC->MoveTo(barOrigin);
		pDC->LineTo(xbarEnd);
		
		// read text from control edit
		CString cs;
		cs.Format(_T(" bar= %g"), m_VDlineview.m_xscale.GetScaleIncrement());
		csComment += cs;
		strComment += csComment + RC;
	}

	///// vertical voltage bars ///////////////////////////	
	if (mdPM->bVoltageScaleBar)
	{
		ybarEnd.y -= vertBar;
		pDC->MoveTo(barOrigin);
		pDC->LineTo(ybarEnd);
	}

	// comments, bar value and chan settings for each channel	
	if (mdPM->bChansComment || mdPM->bVoltageScaleBar || mdPM->bChanSettings)
	{
		int imax = m_VDlineview.GetChanlistSize();	// number of data channels
		for (int ichan=0; ichan< imax; ichan++)		// loop
		{
			// boucler sur les commentaires de chan n a chan 0...		
			wsprintf(lpszVal, _T("chan#%i "), ichan);	// channel number
			csComment = lpszVal;
			if (mdPM->bVoltageScaleBar)				// bar scale value
			{
				csUnit = _T(" V");						// provisional unit				
				float z= 	(float) m_VDlineview.Height()/5 * m_VDlineview.GetChanlistVoltsperPixel(ichan);
				float x = m_VDlineview.ChangeUnit(z, &csUnit, &xScaleFactor); // convert

				// approximate
				int j = (int) x;					// get int val				
				if (x-j > 0.5)						// increment integer if diff > 0.5
					j++;
				int k = m_VDlineview.NiceUnit(x);	// compare with nice unit abs
				if (j > 750)                        // there is a gap between 500 and 1000
					k=1000;
				if (MulDiv(100, abs(k-j),j) <= 1)	// keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = (float) k * xScaleFactor;
					j = (int) m_VDlineview.ChangeUnit(z, &csUnit, &xScaleFactor); // convert
				}
				wsprintf(szValue, _T("bar = %i %s "), j, (LPCTSTR) csUnit);	// store val into comment
				csComment += szValue;
			}
			strComment += csComment;

			// print chan comment 
			if (mdPM->bChansComment)
			{
				strComment += Tab;
				strComment += m_VDlineview.GetChanlistComment(ichan);
			}
			strComment += RC;

			// print amplifiers settings (gain & filter), next line			
			if (mdPM->bChanSettings)
			{
				CString cs;
				WORD channb = m_VDlineview.GetChanlistSourceChan(ichan);
				CWaveChanArray* pchanArray = m_pdatDoc->GetpWavechanArray();
				CWaveChan* pChan = pchanArray->GetWaveChan(channb);
				cs.Format(_T("headstage=%s gain=%.0f  filter= %s - %i Hz"), 
					(LPCTSTR) pChan->am_csheadstage, pChan->am_totalgain, (LPCTSTR) pChan->am_csInputpos, pChan->am_lowpass);
				strComment += cs;
				strComment += RC;
			}
		}
	}	
	pDC->SelectObject(pOldBrush);
	return strComment;
}

/////////////////////////////////////////////////////////////////////////////
// (1) OnPreparePrinting()
// override standard setting before calling print dialog
/////////////////////////////////////////////////////////////////////////////

BOOL CDataView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// printing margins	
	if (  mdPM->vertRes <= 0						// vertical resolution defined ?
		||mdPM->horzRes <= 0						// horizontal resolution defined?
		||mdPM->horzRes !=  pInfo->m_rectDraw.Width()	// same as infos provided
		||mdPM->vertRes != pInfo->m_rectDraw.Height())	// by caller?
		ComputePrinterPageSize();

	int npages = PrintGetNPages();
		
	//------------------------------------------------------
	pInfo->SetMaxPage(npages);						//one page printing/preview  
	pInfo->m_nNumPreviewPages = 1;  				// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;	// allow print only selection

	if(mdPM->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;	// set button to selection

	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;	
	
	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	if (mdPM->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		mdPM->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = PrintGetNPages();
		pInfo->SetMaxPage(npages);
	}

	return TRUE;
}

int	CDataView::PrintGetNPages()
{
	// how many rows per page?
	int sizeRow=mdPM->HeightDoc + mdPM->heightSeparator;
	m_nbrowsperpage = m_printRect.Height()/sizeRow;
	if (m_nbrowsperpage == 0)					// prevent zero pages
		m_nbrowsperpage = 1;

	int ntotalRows;								// number of rectangles -- or nb of rows
	int npages;									// number of pages
	CdbWaveDoc* pdbDoc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag 
	m_lprintFirst = m_VDlineview.GetDataFirst();
	m_lprintLen = m_VDlineview.GetDataLast() -m_lprintFirst + 1;
	m_file0 = GetDocument()->DBGetCurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	int ifile0 = m_file0;
	int ifile1 = m_file0;
	if (!mdPM->bPrintSelection)
	{
		ifile0 = 0;
		m_nfiles = pdbDoc->DBGetNRecords();
		ifile1 = m_nfiles;
	}

	// only one row per file
	if (!mdPM->bMultirowDisplay || !mdPM->bEntireRecord)
		ntotalRows = m_nfiles;

	// multirows per file
	else
	{
		ntotalRows=0;
		pdbDoc->DBSetCurrentRecordPosition(ifile0);
		for (int i = ifile0; i<ifile1;	i++, pdbDoc->DBMoveNext())
		{
			// get size of document for all files
			long len = pdbDoc->DBGetDataLen();
			if (len <= 0)
			{
				pdbDoc->OpenCurrentDataFile();
				len = m_pdatDoc->GetDOCchanLength();
				pdbDoc->DBSetDataLen(len);
			}
			len -= m_lprintFirst;
			long nrows = len/m_lprintLen;			// how many rows for this file?
			if (len > nrows*m_lprintLen)			// remainder?
				nrows++;
			ntotalRows+= (int) nrows;				// update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		try 
		{
			pdbDoc->DBSetCurrentRecordPosition(m_file0);
			pdbDoc->OpenCurrentDataFile();
		} 
		catch(CDaoException* e) {DisplayDaoException(e, 2); e->Delete();}
	}

	// npages
	npages = ntotalRows/m_nbrowsperpage;
	if (ntotalRows > m_nbrowsperpage*npages)
		npages++;

	return npages;
}

/////////////////////////////////////////////////////////////////////////////
// (2) OnBeginPrinting
/////////////////////////////////////////////////////////////////////////////

void CDataView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_VDlineview.GetDataFirst();
	m_lLast0 = m_VDlineview.GetDataLast();
	m_npixels0 = m_VDlineview.Width();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));			// Arial font
	m_logFont.lfHeight = mdPM->fontsize;			// font height
	m_pOldFont = NULL;
	BOOL flag = m_fontPrint.CreateFontIndirect(&m_logFont);
	pDC->SetBkMode (TRANSPARENT);
}

/////////////////////////////////////////////////////////////////////////////
//	(3) OnPrint() -- for each page
/////////////////////////////////////////////////////////////////////////////

void CDataView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	m_pOldFont = pDC->SelectObject(&m_fontPrint);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	int rWidth = mdPM->WidthDoc;					// margins
	int rHeight = mdPM->HeightDoc;					// margins
	CRect RWhere (	m_printRect.left, 				// printing rectangle for data
					m_printRect.top, 
					m_printRect.left+rWidth, 
					m_printRect.top+rHeight);
	//CRect RW2 = RWhere;								// printing rectangle - constant
	//RW2.OffsetRect(-RWhere.left, -RWhere.top);		// set RW2 origin = 0,0

	pDC->SetMapMode(MM_TEXT);						// change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(pDC, pInfo);				// print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page	
	int filenumber;    								// file number and file index
	long lFirst;									// index first data point / first file
	long veryLast= m_lprintFirst + m_lprintLen;		// index last data point / current file
	int curpage=pInfo->m_nCurPage;					// get current page number
	GetFileSeriesIndexFromPage(curpage, filenumber, lFirst);
	if (lFirst < GetDocument()->DBGetDataLen()-1)
		UpdateFileParameters();
	if (mdPM->bEntireRecord)
		veryLast = GetDocument()->DBGetDataLen()-1;

	SCOPESTRUCT oldparms;
	oldparms = m_VDlineview.m_parms;
	m_VDlineview.m_parms.bDrawframe = mdPM->bFrameRect;
	m_VDlineview.m_parms.bClipRect	= mdPM->bClipRect;

	// loop through all files	--------------------------------------------------------
	int oldDC = pDC->SaveDC();						// save DC
	for (int i = 0; i < m_nbrowsperpage; i++)
	{
		// first : set rectangle where data will be printed
		CRect CommentRect = RWhere;					// save RWhere for comments        
		pDC->SetMapMode(MM_TEXT);					// 1 pixel = 1 logical unit
		pDC->SetTextAlign(TA_LEFT); 				// set text align mode	

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data
		long lLast = lFirst + m_lprintLen;			// compute last pt to load
		if (lFirst < GetDocument()->DBGetDataLen()-1)
		{
			if (lLast > veryLast)					// check end across file length
				lLast = veryLast;
			m_VDlineview.GetDataFromDoc(lFirst, lLast);	// load data from file
			UpdateChannelsDisplayParameters();
			m_VDlineview.Print(pDC, &RWhere);			// print data	
		}

		// update display rectangle for next row
		RWhere.OffsetRect(0, rHeight+mdPM->heightSeparator);

		// restore DC and print comments --------------------------------------------------
		pDC->SetMapMode(MM_TEXT);				// 1 LP = 1 pixel
		pDC->SelectClipRgn(NULL);				// no more clipping
		pDC->SetViewportOrg(0, 0);				// org = 0,0        

		// print comments according to row within file
		CString csComment;
		if (lFirst == m_lprintFirst)			// first row = full comment
		{
			csComment += GetFileInfos();
			csComment += PrintBars(pDC, &CommentRect);// bars and bar legends
		}
		else									// other rows: time intervals only
			csComment = ConvertFileIndex(m_VDlineview.GetDataFirst(), m_VDlineview.GetDataLast());

		// print comments stored into csComment		
		CommentRect.OffsetRect(mdPM->textseparator + CommentRect.Width(), 0);
		CommentRect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		pDC->DrawText(csComment, csComment.GetLength(), CommentRect, 
				DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
		pDC->SetTextAlign(uiFlag);

		// update file parameters for next row --------------------------------------------
		int ifile = filenumber;
		if (!PrintGetNextRow(filenumber, lFirst, veryLast))
		{
			i = m_nbrowsperpage;
			break;
		}
		if (ifile != filenumber)
			UpdateFileParameters(FALSE);
	}
	pDC->RestoreDC(oldDC);					// restore Display context	

	// end of file loop : restore initial conditions
	if (m_pOldFont != NULL)
		pDC->SelectObject(m_pOldFont);
	m_VDlineview.m_parms = oldparms;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDataView::PrintGetNextRow(int &filenumber, long &lFirst, long &veryLast)
{
	if (!mdPM->bMultirowDisplay || !mdPM->bEntireRecord)
	{
		filenumber++;
		if  (filenumber >= m_nfiles)
			return FALSE;

		GetDocument()->DBMoveNext();
		if (lFirst < GetDocument()->DBGetDataLen()-1)
		{
			if (mdPM->bEntireRecord)
				veryLast=GetDocument()->DBGetDataLen()-1;
		}
	}
	else
	{
		lFirst += m_lprintLen;
		if (lFirst >= veryLast)
		{
			filenumber++;						// next index
			if  (filenumber >= m_nfiles)		// last file ??
				return FALSE;

			GetDocument()->DBMoveNext();
			veryLast = GetDocument()->DBGetDataLen()-1;
			lFirst = m_lprintFirst;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	(4) OnEndPrinting() - lastly
/////////////////////////////////////////////////////////////////////////////

void CDataView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;
	GetDocument()->DBSetCurrentRecordPosition(m_file0);
	m_VDlineview.ResizeChannels(m_npixels0, 0);
	m_VDlineview.GetDataFromDoc(m_lFirst0, m_lLast0);
	UpdateFileParameters();
}

void CDataView::OnEnChangeTimefirst() 
{
	if (!mm_timefirst.m_bEntryDone)
		return;

	switch (mm_timefirst.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timefirst++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timefirst--;
		break;
	}	
	m_VDlineview.GetDataFromDoc((long) (m_timefirst*m_samplingRate), 
								(long) (m_timelast*m_samplingRate));	
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	m_VDlineview.Invalidate();
	mm_timefirst.m_bEntryDone=FALSE;
	mm_timefirst.m_nChar=0;
	mm_timefirst.SetSel(0, -1); 	//select all text
}

void CDataView::OnEnChangeTimelast() 
{
	if (!mm_timelast.m_bEntryDone)
		return;

	switch (mm_timelast.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE);
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timelast++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timelast--;
		break;
	}	
	m_VDlineview.GetDataFromDoc((long) (m_timefirst*m_samplingRate), 
								(long) (m_timelast*m_samplingRate));	
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	m_VDlineview.Invalidate();
	mm_timelast.m_bEntryDone=FALSE;
	mm_timelast.m_nChar=0;
	mm_timelast.SetSel(0, -1); 	//select all text
}

void CDataView::UpdateFileScroll()
{
	m_filescroll_infos.fMask = SIF_ALL| SIF_PAGE | SIF_POS;
	m_filescroll_infos.nMin = 0;
	m_filescroll_infos.nMax = GetDocument()->DBGetDataLen();
	m_filescroll_infos.nPos = m_VDlineview.GetDataFirst();
	m_filescroll_infos.nPage = m_VDlineview.GetDataLast()-m_VDlineview.GetDataFirst()+1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void CDataView::OnEnChangeYlower()
{
	if (!mm_ylower.m_bEntryDone)
		return;

	switch (mm_ylower.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE);
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ylower++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ylower--;
		break;
	}	
	
	m_VDlineview.SetChanlistVoltsMaxMin(m_ichanselected, m_yupper, m_ylower);
	m_VDlineview.Invalidate();
	UpdateLegends(CHG_YBAR);

	mm_ylower.m_bEntryDone=FALSE;
	mm_ylower.m_nChar=0;
	mm_ylower.SetSel(0, -1); 	//select all text
}

void CDataView::OnEnChangeYupper()
{
	if (!mm_yupper.m_bEntryDone)
		return;

	switch (mm_yupper.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE);
		break;
	case VK_UP:
	case VK_PRIOR:
		m_yupper++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_yupper--;
		break;
	}	
	
	m_VDlineview.SetChanlistVoltsMaxMin(m_ichanselected, m_yupper, m_ylower);
	m_VDlineview.m_yscale.SetRange (&m_ylower, &m_yupper);
	m_VDlineview.Invalidate();
	UpdateLegends(CHG_YBAR);

	mm_yupper.m_bEntryDone=FALSE;
	mm_yupper.m_nChar=0;
	mm_yupper.SetSel(0, -1); 	//select all text
}

void CDataView::OnCbnSelchangeCombochan()
{
	int ichan = m_comboSelectChan.GetCurSel();
	if (ichan < m_VDlineview.GetChanlistSize()) 
	{
		UpdateChannel(ichan);
	}
	else
	{
		m_ichanselected = 0;
		int yextent = m_VDlineview.GetChanlistYextent(0);
		UpdateYExtent(0, yextent);
		int yzero = m_VDlineview.GetChanlistYzero(0);
		UpdateYZero(0, yzero);
	}
}
