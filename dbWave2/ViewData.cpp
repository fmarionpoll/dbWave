// View data displays data and allow interactive browsing through the file
// print and copy implemented

#include "StdAfx.h"
//#include <math.h>
#include "dbWave.h"
#include "dbWave_constants.h"
#include "resource.h"

//#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
//#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Vdordina.h"
#include "Vdseries.h"
#include "Adinterv.h"
#include "Dataseri.h"

#include "MeasureProperties.h"
//#include "Editctrl.h"
//#include "CyberAmp.h"
//#include "Adexperi.h"
#include "MainFrm.h"
#include "Copyasdl.h"
//#include "ChildFrm.h"
#include "Vdabciss.h"
#include "RulerBar.h"

#include "dbEditRecordDlg.h"
#include "ADInputParmsDlg.h"
#include "ViewData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewData, CDaoRecordView)

BEGIN_MESSAGE_MAP(CViewData, CDaoRecordView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BIAS_button, &CViewData::OnClickedBias)
	ON_BN_CLICKED(IDC_GAIN_button, &CViewData::OnClickedGain)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FORMAT_XSCALE, &CViewData::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &CViewData::OnFormatYscale)
	ON_COMMAND(ID_EDIT_COPY, &CViewData::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CViewData::OnUpdateEditCopy)
	ON_COMMAND(ID_TOOLS_DATASERIES, &CViewData::OnToolsDataseries)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &CViewData::ADC_OnHardwareChannelsDlg)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &CViewData::ADC_OnHardwareIntervalsDlg)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &CViewData::OnCenterCurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &CViewData::OnGainAdjustCurve)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &CViewData::OnSplitCurves)
	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &CViewData::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &CViewData::OnLastFrame)
	ON_MESSAGE(WM_MYMESSAGE, &CViewData::OnMyMessage)
	ON_COMMAND(ID_FORMAT_ALLDATA, &CViewData::OnViewAlldata)
	ON_COMMAND(ID_FORMAT_DATASERIESATTRIBUTES, &CViewData::OnFormatDataseriesattributes)
	ON_COMMAND(ID_TOOLS_MEASUREMODE, &CViewData::OnToolsMeasuremode)
	ON_COMMAND(ID_TOOLS_MEASURE, &CViewData::OnToolsMeasure)
	ON_COMMAND(ID_TOOLS_VERTICALTAGS, &CViewData::OnToolsVerticaltags)
	ON_COMMAND(ID_TOOLS_HORIZONTALCURSORS, &CViewData::OnToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_HORIZONTALCURSORS, &CViewData::OnUpdateToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_VERTICALTAGS, &CViewData::OnUpdateToolsVerticaltags)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &CViewData::ADC_OnHardwareDefineexperiment)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CViewData::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CViewData::OnEnChangeTimelast)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FORMAT_SETORDINATES, &CViewData::OnFormatYscale)
	ON_COMMAND(ID_FILE_PRINT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CDaoRecordView::OnFilePrintPreview)
	ON_CBN_SELCHANGE(IDC_COMBOCHAN, &CViewData::OnCbnSelchangeCombochan)
END_MESSAGE_MAP()

CViewData::CViewData()
	: CDaoRecordView(CViewData::IDD), m_v1(0), m_v2(0), m_diff(0), m_hBias(nullptr), m_hZoom(nullptr), scan_count(0),
	chrate(0), m_file0(0), m_lFirst0(0), m_lLast0(0), m_npixels0(0), m_nfiles(0), m_nbrowsperpage(0),
	m_lprintFirst(0), m_lprintLen(0), m_printFirst(0), m_printLast(0), m_tMetric(), m_logFont(), m_pOldFont(nullptr),
	options_viewdata(nullptr), mdMO(nullptr), m_bCommonScale(0), m_filescroll_infos(), m_VBarMode(0)
{
	m_ichanselected		= 0;
	m_timefirst			= 0.0f;
	m_timelast			= 0.0f;
	m_floatNDigits		= 1000.0f;		// display with n digits (3)
	m_samplingRate		= 1.0f;			// sampling rate (Hz)
	m_cursorstate		= 0;			// cursor state: system arrow
	m_HBarpixelratio	= 10;			// horizontalbar pixel ratio (%)
	m_VBarpixelratio	= 30;			// vertical bar pixel ratio
	m_pdatDoc			= nullptr;
	m_bIsPrinting		= FALSE;
	m_currentfileindex	= 0;
	m_bInitComment		= TRUE;

	m_bvalidDoc			= FALSE;
	m_binit				= FALSE;

	m_bEnableActiveAccessibility = FALSE;
}

CViewData::~CViewData()
{
	m_pdatDoc->AcqCloseFile();

}

void CViewData::DoDataExchange(CDataExchange* pDX)
{
	// pass values
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_v1);
	DDX_Text(pDX, IDC_EDIT2, m_v2);
	DDX_Text(pDX, IDC_EDIT3, m_diff);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);

	DDX_Control(pDX, IDC_FILESCROLL, m_filescroll);
	DDX_Control(pDX, IDC_COMBOCHAN, m_comboSelectChan);
}

BOOL CViewData::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CViewData::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto p_mainframe = (CMainFrame*)AfxGetMainWnd();
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		((CdbWaveApp*)AfxGetApp())->options_viewdata.viewdata = *(m_displayDataFile.GetScopeParameters());
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewData::DefineSubClassedItems()
{
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// bitmap buttons: load icons & set buttons
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>((HANDLE)m_hBias));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>((HANDLE)m_hZoom));

	VERIFY(m_displayDataFile.SubclassDlgItem(IDC_DISPLAY, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(m_ADC_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ADC_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
}

void CViewData::DefineStretchParameters()
{
	// save coordinates and properties of "always visible" controls
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_DISPLAY, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_COMBOCHAN, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_SOURCE, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_YSCALE, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);

	m_binit = TRUE;
}

void CViewData::OnInitialUpdate()
{
	DefineSubClassedItems();

	m_ADC_yRulerBar.AttachScopeWnd(&m_displayDataFile, FALSE);
	m_ADC_xRulerBar.AttachScopeWnd(&m_displayDataFile, TRUE);
	m_displayDataFile.AttachExternalXRuler(&m_ADC_xRulerBar);
	m_displayDataFile.AttachExternalYRuler(&m_ADC_yRulerBar);
	m_displayDataFile.m_bNiceGrid = TRUE;

	DefineStretchParameters();

	// init relation with document, display data, adjust parameters
	auto p_app = (CdbWaveApp*)AfxGetApp();
	options_viewdata = &(p_app->options_viewdata);
	mdMO = &(p_app->options_viewdata_measure);

	// set data file
	CDaoRecordView::OnInitialUpdate();
	UpdateFileParameters(TRUE);	// load file parameters

	m_displayDataFile.SetScopeParameters(&(options_viewdata->viewdata));
	int ioperation = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
	m_bCommonScale = TRUE;
	m_comboSelectChan.SetCurSel(m_displayDataFile.GetChanlistSize());
	UpdateLegends(ioperation);
}

void CViewData::OnDestroy()
{
	CDaoRecordView::OnDestroy();
	DeleteObject(m_hBias);		// bias button (handle)
	DeleteObject(m_hZoom);		// zoom button (handle)
}

void CViewData::OnSize(UINT nType, int cx, int cy)
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

#ifdef _DEBUG
void CViewData::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewData::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewData::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

CDaoRecordset* CViewData::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

BOOL CViewData::OnMove(UINT nIDMoveCommand)
{
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewData::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	auto i_update = NULL;
	switch (LOWORD(lHint))
	{
	case HINT_REPLACEVIEW:
		return;
	case HINT_CLOSEFILEMODIFIED:	// save current file parms
		SaveModifiedFile();
		break;
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		m_bInitComment = TRUE;
		UpdateFileParameters();
		i_update = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
		break;
	default:
		if (m_pdatDoc == nullptr)
			return;
		i_update = UPD_ABCISSA | UPD_XSCALE | UPD_ORDINATES | UPD_YSCALE;
		break;
	}
	UpdateLegends(i_update);
	m_displayDataFile.Invalidate();
	SetVBarMode(m_VBarMode);
}

void CViewData::UpdateLegends(int ioperation)
{
	if (!m_displayDataFile.IsDefined() && !m_bvalidDoc)
		return;
	if (ioperation & UPD_ABCISSA)
		UpdateFileScroll();
	if (ioperation & CHG_YSCALE)
	{
		UpdateHZtagsVal();
		ioperation |= CHG_YBAR;
	}
	if (ioperation & UPD_YSCALE)
		ioperation |= CHG_YBAR;
	if (ioperation & CHG_YBAR)
		UpdateYZero(m_ichanselected, m_displayDataFile.GetChanlistYzero(m_ichanselected));

	UpdateData(FALSE);
}

void CViewData::OnClickedBias()
{
	// set bias down and set gain up CButton
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CViewData::OnClickedGain()
{
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CViewData::UpdateChannel(int channel)
{
	m_ichanselected = channel;
	if (m_ichanselected > m_displayDataFile.GetChanlistSize() - 1)	// less or equal than max nb of chans?
		m_ichanselected = m_displayDataFile.GetChanlistSize() - 1;	// clip to maximum
	else if (m_ichanselected < 0)							// less than 1 channel?
		m_ichanselected = 0;								// clip to minimum (and if 0?)
	if (m_ichanselected != channel)							// new value is different than previous
	{														// change other dependent parameters
		if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption == 1
			&& m_displayDataFile.GetNHZtags() > 0)
		{
			for (auto i = 0; i < m_displayDataFile.GetNHZtags(); i++)
				m_displayDataFile.SetHZtagChan(i, m_ichanselected);
			UpdateHZtagsVal();
			m_displayDataFile.Invalidate();
		}
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	else					 			// new value is same as previous
	{									// change content of control
		UpdateData(FALSE);				// put data into edit controls
	}
}

void CViewData::OnFormatYscale()
{
	CDataViewOrdinatesDlg dlg;
	dlg.m_plinev = &m_displayDataFile;
	dlg.m_Channel = m_ichanselected;
	if (IDOK == dlg.DoModal())
		UpdateLegends(UPD_ORDINATES | UPD_YSCALE | CHG_YBAR);

	m_displayDataFile.Invalidate();
}

void CViewData::OnToolsDataseries()
{
	// init dialog data
	CDataSeriesDlg dlg;
	dlg.m_lineview = &m_displayDataFile;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_ichanselected;
	dlg.DoModal();

	m_ichanselected = dlg.m_listindex;
	UpdateLegends(UPD_YSCALE);
}

void CViewData::OnEditCopy()
{
	CCopyAsDlg dlg;
	dlg.m_nabcissa = options_viewdata->hzResolution;
	dlg.m_nordinates = options_viewdata->vtResolution;
	dlg.m_bgraphics = options_viewdata->bgraphics;
	dlg.m_ioption = options_viewdata->bcontours;
	dlg.m_iunit = options_viewdata->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_viewdata->bgraphics = dlg.m_bgraphics;
		options_viewdata->bcontours = dlg.m_ioption;
		options_viewdata->bunits = dlg.m_iunit;
		options_viewdata->hzResolution = dlg.m_nabcissa;
		options_viewdata->vtResolution = dlg.m_nordinates;

		if (!dlg.m_bgraphics)
			m_displayDataFile.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			CRect old_rect;
			m_displayDataFile.GetWindowRect(&old_rect);

			CRect rect(0, 0, options_viewdata->hzResolution, options_viewdata->vtResolution);
			m_npixels0 = m_displayDataFile.GetRectWidth();

			// create metafile
			CMetaFileDC m_dc;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + m_pdatDoc->GetTitle();
			cs_title += _T("\0\0");
			CRect rect_bound(0, 0, 21000, 29700);  // dimensions in HIMETRIC units (in .01-millimeter increments)
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in metafile.
			CClientDC attrib_dc(this);						// Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc());		// from current screen

			const auto oldparms = new SCOPESTRUCT();
			SCOPESTRUCT* p_newparms = m_displayDataFile.GetScopeParameters();
			*oldparms = *p_newparms;
			p_newparms->bDrawframe = options_viewdata->bFrameRect;
			p_newparms->bClipRect = options_viewdata->bClipRect;
			m_displayDataFile.Print(&m_dc, &rect);
			*p_newparms = *oldparms;

			// print comments : set font
			memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
			GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), reinterpret_cast<LPSTR>(&m_logFont));
			m_pOldFont = nullptr;
			/*BOOL flag = */m_fontPrint.CreateFontIndirect(&m_logFont);
			m_pOldFont = m_dc.SelectObject(&m_fontPrint);
			const int lineheight = m_logFont.lfHeight + 5;
			auto ypxrow = 0;
			const auto xcol = 10;

			CString comments = _T("Abcissa: ");
			CString content;
			content.Format(_T("%g - %g s"), m_timefirst, m_timelast);
			comments += content;
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format(_T("Vertical bar (ch. 0) = %g mV"), m_displayDataFile.m_yRuler.GetScaleIncrement());
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format(_T("Horizontal bar = %g s"), m_displayDataFile.m_xRuler.GetScaleIncrement());
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// bars
			const auto p_old_brush = (CBrush*)m_dc.SelectStockObject(BLACK_BRUSH);
			m_dc.MoveTo(0, ypxrow);
			const auto bottom = m_displayDataFile.m_yRuler.GetScaleUnitPixels(rect.Height());
			m_dc.LineTo(0, ypxrow - bottom);
			m_dc.MoveTo(0, ypxrow);
			const auto left = m_displayDataFile.m_xRuler.GetScaleUnitPixels(rect.Width());
			m_dc.LineTo(left, ypxrow);

			m_dc.SelectObject(p_old_brush);
			if (m_pOldFont != nullptr)
				m_dc.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

			// Close metafile
			ReleaseDC(p_dc_ref);
			const auto h_emf_tmp = m_dc.CloseEnhanced();
			ASSERT(h_emf_tmp != NULL);
			if (OpenClipboard())
			{
				EmptyClipboard();							// prepare clipboard
				SetClipboardData(CF_ENHMETAFILE, h_emf_tmp);	// put data
				CloseClipboard();							// close clipboard
			}
			else
			{
				// Someone else has the Clipboard open...
				DeleteEnhMetaFile(h_emf_tmp);					// delete data
				MessageBeep(0);							// tell user something is wrong!
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
			}

			// restore initial conditions
			m_displayDataFile.ResizeChannels(m_npixels0, 0);
			m_displayDataFile.GetDataFromDoc();
			m_displayDataFile.Invalidate();
		}
	}
}

void CViewData::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_displayDataFile.IsDefined() != NULL); // if document has data
}

void CViewData::ADC_OnHardwareChannelsDlg()
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

void CViewData::ADC_OnHardwareIntervalsDlg()
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

void CViewData::ChainDialog(WORD iID)
{
	WORD menu_id;
	switch (iID)
	{
	case IDC_ADINTERVALS:
		menu_id = ID_HARDWARE_ADINTERVALS;
		break;
	case IDC_ADCHANNELS:
		menu_id = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
	}
	PostMessage(WM_COMMAND, menu_id, NULL);
}

void CViewData::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void CViewData::OnLastFrame()
{
	OnFileScroll(SB_RIGHT, 1L);
}

void CViewData::UpdateFileParameters(BOOL bUpdateInterface)
{
	// load parameters from document file: none yet loaded?
	const BOOL b_first_update = (m_pdatDoc == nullptr);
	auto p_dbwave_doc = GetDocument();
	auto cs_dat_file = p_dbwave_doc->GetDB_CurrentDatFileName();
	if ((m_bvalidDoc = cs_dat_file.IsEmpty()))
		return;

	// open data file
	if (p_dbwave_doc->OpenCurrentDataFile() == nullptr)
	{
		MessageBox(_T("This data file could not be opened"), _T("The file might be missing, or inaccessible..."), MB_OK);
		m_bvalidDoc = FALSE;
		return;
	}
	m_pdatDoc = p_dbwave_doc->m_pDat;
	m_pdatDoc->ReadDataInfos();
	const auto pwave_format = m_pdatDoc->GetpWaveFormat();

	if (b_first_update)
	{
		m_samplingRate = pwave_format->chrate;	// load sampling rate
		m_timefirst = 0.0f;						// init file size
		m_timelast = (m_pdatDoc->GetDOCchanLength()) / m_samplingRate;
	}

	// load parameters from current data file
	m_displayDataFile.AttachDataFile(m_pdatDoc);
	m_pdatDoc->SetModifiedFlag(FALSE);

	// OPTION: display entire file	--	(inactif si multirow)
	long l_first = 0;
	long l_last = m_pdatDoc->GetDOCchanLength() - 1;

	if (!options_viewdata->bEntireRecord || options_viewdata->bMultirowDisplay && !b_first_update)
	{
		l_first = static_cast<long>(m_timefirst * m_samplingRate);
		l_last = static_cast<long>(m_timelast * m_samplingRate);
		if (l_last > m_pdatDoc->GetDOCchanLength() - 1)	// last OK?
			l_last = m_pdatDoc->GetDOCchanLength() - 1;	// clip to the end of the file
	}
	m_samplingRate = pwave_format->chrate;			// update sampling rate

	// display all channels
	auto lnvchans = m_displayDataFile.GetChanlistSize();
	const int ndocchans = pwave_format->scan_count;

	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	if (options_viewdata->bAllChannels || lnvchans == 0)
	{
		for (auto jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{
			auto b_present = FALSE;
			for (auto j = lnvchans - 1; j >= 0; j--)
			{
				if ((b_present = (m_displayDataFile.GetChanlistSourceChan(j) == jdocchan)))
					break;
			}
			if (!b_present) {
				m_displayDataFile.AddChanlistItem(jdocchan, 0);
				lnvchans++;
			}
			m_displayDataFile.SetChanlistColor(jdocchan, jdocchan);
		}
	}

	// load real data from file and update time parameters
	m_displayDataFile.GetDataFromDoc(l_first, l_last);					// load data requested
	m_timefirst = m_displayDataFile.GetDataFirst() / m_samplingRate;	// update abcissa parameters
	m_timelast = m_displayDataFile.GetDataLast() / m_samplingRate;	// first - end
	m_ichanselected = 0;										// select chan 0

	if (!b_first_update)
		UpdateChannelsDisplayParameters();

	// fill combo
	m_comboSelectChan.ResetContent();
	for (auto i = 0; i < m_displayDataFile.GetChanlistSize(); i++)
	{
		CString cs;
		cs.Format(_T("channel %i - "), i);
		cs = cs + m_displayDataFile.GetChanlistComment(i);
		m_comboSelectChan.AddString(cs);
	}
	if (ndocchans > 1) {
		m_comboSelectChan.AddString(_T("all channels"));
	}
	if (!m_bCommonScale)
		m_comboSelectChan.SetCurSel(0);
	else
		m_comboSelectChan.SetCurSel(m_displayDataFile.GetChanlistSize());

	// done
	if (bUpdateInterface)
	{
		UpdateFileScroll();
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | CHG_YSCALE);
		m_displayDataFile.Invalidate();
	}
}

void CViewData::UpdateChannelsDisplayParameters()
{
	const auto n_lineview_chans = m_displayDataFile.GetChanlistSize();
	int max;
	int min;
	if (!m_bCommonScale)
	{
		for (auto i = 0; i < n_lineview_chans; i++)
		{
			// keep final gain constant even if ampli gain changed
			m_displayDataFile.GetChanlistMaxMin(i, &max, &min);
			auto iextent = m_displayDataFile.GetChanlistYextent(i);
			auto izero = m_displayDataFile.GetChanlistYzero(i);

			if (options_viewdata->bMaximizeGain)
				iextent = MulDiv(max - min + 1, 11, 10);
			// center curve
			if (options_viewdata->bCenterCurves)
				izero = (max + min) / 2;

			UpdateYExtent(i, iextent);
			UpdateYZero(i, izero);
		}
	}
	else
	{
		const auto ichan = 0;
		auto iextent = m_displayDataFile.GetChanlistYextent(ichan);
		auto izero = m_displayDataFile.GetChanlistYzero(ichan);
		if (options_viewdata->bMaximizeGain)
		{
			float vmax = 0.;
			float vmin = 0.;
			for (auto i = 0; i < n_lineview_chans; i++)
			{
				// keep final gain constant even if ampli gain changed
				m_displayDataFile.GetChanlistMaxMin(i, &max, &min);
				const auto maxchani = m_displayDataFile.ConvertChanlistDataBinsToVolts(i, max);
				const auto minchani = m_displayDataFile.ConvertChanlistDataBinsToVolts(i, min);
				if (maxchani > vmax)
					vmax = maxchani;
				if (minchani < vmin)
					vmin = minchani;
			}
			max = m_displayDataFile.ConvertChanlistVoltstoDataBins(ichan, vmax);
			min = m_displayDataFile.ConvertChanlistVoltstoDataBins(ichan, vmin);
			iextent = MulDiv(max - min + 1, 10, 8);
			izero = (max + min) / 2;
		}
		UpdateYExtent(ichan, iextent);
		UpdateYZero(ichan, izero);
	}
	m_displayDataFile.Invalidate();
}

void CViewData::SetCursorAssociatedWindows()
{
	auto n_cmd_show = SW_HIDE;
	if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption == 1
		&& m_displayDataFile.GetNHZtags() > 0)
		n_cmd_show = SW_SHOW;

	// change windows state: edit windows
	GetDlgItem(IDC_STATIC1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC3)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT3)->ShowWindow(n_cmd_show);

	// change cursors val
	if (m_cursorstate == CURSOR_MEASURE && mdMO->wOption == 1)
		UpdateHZtagsVal();
}

void CViewData::UpdateHZtagsVal()
{
	if (m_displayDataFile.GetNHZtags() <= 0)
		return;
	const auto v1 = m_displayDataFile.GetHZtagVal(0);
	auto itag = 0;
	if (m_displayDataFile.GetNHZtags() > 1)
		itag = 1;
	const auto v2 = m_displayDataFile.GetHZtagVal(itag);
	const auto mv_per_bin = m_displayDataFile.GetChanlistVoltsperDataBin(m_ichanselected) * 1000.0f;
	m_v1 = static_cast<float>(v1) * mv_per_bin;
	m_v2 = static_cast<float>(v2) * mv_per_bin;
	m_diff = m_v1 - m_v2;

	TCHAR sz_buffer[32];
	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_v1);
	GetDlgItem(IDC_EDIT1)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT1)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_v2);
	GetDlgItem(IDC_EDIT2)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT2)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_diff);
	GetDlgItem(IDC_EDIT3)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT3)->Invalidate();
}

LRESULT CViewData::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//int j = HIWORD(lParam);		// control ID of sender
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
				auto ptaglist = m_pdatDoc->GetpVTtags();
				ptaglist->CopyTagList(m_displayDataFile.GetVTtagList());
				m_displayDataFile.DelAllVTtags();
			}
			else if (mdMO->wOption == 1)	// horizontal cursors
			{
				auto ptaglist = m_pdatDoc->GetpHZtags();
				ptaglist->CopyTagList(m_displayDataFile.GetHZtagList());
				m_displayDataFile.DelAllHZtags();
			}
			else if (mdMO->wOption == 3) // detect stimulus
			{
				mdMO->wStimuluschan = m_displayDataFile.GetHZtagChan(0);
				mdMO->wStimulusthresh = m_displayDataFile.GetHZtagVal(0);
				m_displayDataFile.DelAllHZtags();
			}
			m_displayDataFile.Invalidate();
		}
		// change cursor value (+1), clip to upper cursor value
		if (lowp > CURSOR_MEASURE)
			lowp = 0;
		// change cursor and tell parent that it has changed
		m_cursorstate = m_displayDataFile.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));

		// recall cursors from document if cursorstate = 2
		if (m_cursorstate == CURSOR_MEASURE)
		{
			if (mdMO->wOption == 0)
				m_displayDataFile.SetVTtagList(m_pdatDoc->GetpVTtags());
			else if (mdMO->wOption == 1)
				m_displayDataFile.SetHZtagList(m_pdatDoc->GetpHZtags());
			else if (mdMO->wOption == 3)
				m_displayDataFile.AddHZtag(mdMO->wStimulusthresh, mdMO->wStimuluschan);
			m_displayDataFile.Invalidate();
		}
		SetCursorAssociatedWindows();
		break;

	case HINT_HITCHANNEL:	// change channel if different
		m_ichanselected = lowp;
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
		break;

	case HINT_DEFINEDRECT:
	{
		const auto rect = m_displayDataFile.GetDefinedRect();
		mdMO->wLimitSup = static_cast<WORD>(rect.top);

		mdMO->wLimitInf = static_cast<WORD>(rect.bottom);
		mdMO->lLimitLeft = m_displayDataFile.GetDataOffsetfromPixel(rect.left);
		mdMO->lLimitRight = m_displayDataFile.GetDataOffsetfromPixel(rect.right);
	}
	// action according to option
	switch (mdMO->wOption)
	{
		// ......................  vertical tags
	case 0:					// if no VTtags, then take those of rectangle, or limits of lineview
		m_displayDataFile.AddVTLtag(mdMO->lLimitLeft);
		if (mdMO->lLimitRight != mdMO->lLimitLeft)
			m_displayDataFile.AddVTLtag(mdMO->lLimitRight);
		// store new VT tags into document
		m_pdatDoc->GetpVTtags()->CopyTagList(m_displayDataFile.GetVTtagList());
		break;

		// ......................  horizontal cursors
	case 1:					// if no HZcursors, take those of rectangle or limits of lineview
		m_displayDataFile.AddHZtag(m_displayDataFile.GetChanlistPixeltoBin(m_ichanselected, mdMO->wLimitSup), m_ichanselected);
		if (mdMO->wLimitInf != mdMO->wLimitSup)
			m_displayDataFile.AddHZtag(m_displayDataFile.GetChanlistPixeltoBin(m_ichanselected, mdMO->wLimitInf), m_ichanselected);
		m_pdatDoc->GetpHZtags()->CopyTagList(m_displayDataFile.GetHZtagList());
		if (m_displayDataFile.GetNHZtags() == 2)
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
	m_displayDataFile.Invalidate();
	break;

	case HINT_CHANGEHZTAG:	// horizontal tag has changed 	lowp = tag nb
		if (mdMO->wOption == 3)
			mdMO->wStimulusthresh = m_displayDataFile.GetHZtagVal(0);
		else
			UpdateHZtagsVal();
		break;

	case HINT_VIEWSIZECHANGED:       // change zoom
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE);
		m_displayDataFile.Invalidate();
		SetVBarMode(m_VBarMode);
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_viewdata->viewdata = *(m_displayDataFile.GetScopeParameters());
		break;
	default:
		break;
	}
	return 0L;
}

void CViewData::OnViewAlldata()
{
	m_displayDataFile.GetDataFromDoc(0, GetDocument()->GetDB_DataLen() - 1);
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	UpdateData(FALSE);
	m_displayDataFile.Invalidate();
	UpdateFileScroll();
}

void CViewData::OnFormatDataseriesattributes()
{
	CDataSeriesFormatDlg dlg;
	dlg.m_plineview = &m_displayDataFile;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_ichanselected;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_ichanselected = dlg.m_listindex;
	}
	UpdateLegends(UPD_YSCALE);
	m_displayDataFile.Invalidate();
}

void CViewData::OnToolsVerticaltags()
{
	mdMO->wOption = 0;

	// change cursor and tell parent that it has changed
	m_cursorstate = m_displayDataFile.SetMouseCursorType(CURSOR_MEASURE);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	//MeasureProperties(1);
}

void CViewData::OnToolsHorizontalcursors()
{
	mdMO->wOption = 1;
	// change cursor and tell parent that it has changed
	m_cursorstate = m_displayDataFile.SetMouseCursorType(CURSOR_MEASURE);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	//MeasureProperties(0);
}

void CViewData::OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==1  && m_cursorstate == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void CViewData::OnUpdateToolsVerticaltags(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==0 && m_cursorstate == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void CViewData::OnToolsMeasuremode()
{
	MeasureProperties(2);
}

void CViewData::OnToolsMeasure()
{
	MeasureProperties(3);
}

void CViewData::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// CViewData scroll: vertical scroll bar
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

void CViewData::SetVBarMode(short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void CViewData::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(
			m_displayDataFile.GetChanlistYextent(m_ichanselected),
			100,
			YEXTENT_MAX)
		+ 50,
		TRUE);
}

void CViewData::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int lSize = m_displayDataFile.GetChanlistYextent(m_ichanselected);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
	case SB_LINELEFT:	lSize -= lSize / 10 + 1; break;
	case SB_LINERIGHT:	lSize += lSize / 10 + 1; break;
	case SB_PAGELEFT:	lSize -= lSize / 2 + 1; break;
	case SB_PAGERIGHT:	lSize += lSize + 1; break;
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
	default:			break;
	}

	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		UpdateYExtent(m_ichanselected, lSize);
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll();
}

void CViewData::UpdateBiasScroll()
{
	const auto i_pos = static_cast<int>((m_displayDataFile.GetChanlistYzero(m_ichanselected) - m_displayDataFile.GetChanlistBinZero(m_ichanselected))
		* 100 / static_cast<int>(YZERO_SPAN)) + static_cast<int>(50);
	m_scrolly.SetScrollPos(i_pos, TRUE);
	UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
}

void CViewData::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	auto l_size = m_displayDataFile.GetChanlistYzero(m_ichanselected) - m_displayDataFile.GetChanlistBinZero(m_ichanselected);
	const auto yextent = m_displayDataFile.GetChanlistYextent(m_ichanselected);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			l_size = YZERO_MIN; break;
	case SB_LINELEFT:		l_size -= yextent / 100 + 1; break;
	case SB_LINERIGHT:		l_size += yextent / 100 + 1; break;
	case SB_PAGELEFT:		l_size -= yextent / 10 + 1; break;
	case SB_PAGERIGHT:		l_size += yextent / 10 + 1; break;
	case SB_RIGHT:			l_size = YZERO_MAX; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:		l_size = (nPos - 50) * (YZERO_SPAN / 100); break;
	default:				break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		UpdateYZero(m_ichanselected, l_size + m_displayDataFile.GetChanlistBinZero(m_ichanselected));
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll();
}

void CViewData::OnCenterCurve()
{
	m_displayDataFile.CenterChan(m_ichanselected);
	m_displayDataFile.Invalidate();
	const auto yextent = m_displayDataFile.GetChanlistYextent(m_ichanselected);
	UpdateYExtent(m_ichanselected, yextent);
	const auto yzero = m_displayDataFile.GetChanlistYzero(m_ichanselected);
	UpdateYZero(m_ichanselected, yzero);
}

void CViewData::OnGainAdjustCurve()
{
	m_displayDataFile.MaxgainChan(m_ichanselected);
	m_displayDataFile.Invalidate();

	const auto yextent = m_displayDataFile.GetChanlistYextent(m_ichanselected);
	UpdateYExtent(m_ichanselected, yextent);
	const auto yzero = m_displayDataFile.GetChanlistYzero(m_ichanselected);
	UpdateYZero(m_ichanselected, yzero);
	UpdateLegends(CHG_YSCALE);
}

void CViewData::OnSplitCurves()
{
	const auto nchans = m_displayDataFile.GetChanlistSize();		// nb of data channels
	const auto pxheight = m_displayDataFile.GetRectHeight();		// height of the display area
	const auto pxoffset = pxheight / nchans;					// height for each channel
	auto pxzero = (pxheight - pxoffset) / 2;					// center first curve at

	// split display area
	int  max, min;
	for (auto i = 0; i < nchans; i++)
	{
		m_displayDataFile.GetChanlistMaxMin(i, &max, &min);
		const auto iextent = MulDiv(max - min + 1, 100 * nchans, 100);
		const auto ibias = MulDiv(pxzero, iextent, pxheight);  // convert pixel into bins
		const auto izero = (max + min) / 2 - ibias;				// change bias
		m_displayDataFile.SetChanlistYextent(i, iextent);
		m_displayDataFile.SetChanlistYzero(i, izero);
		pxzero -= pxoffset;								// update position of next curve
	}
	UpdateLegends(CHG_YSCALE);
	m_displayDataFile.Invalidate();
}

void CViewData::OnFileScroll(UINT nSBCode, UINT nPos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
	case SB_LINELEFT:		// scroll one line left
	case SB_LINERIGHT:		// scroll one line right
	case SB_PAGELEFT:		// scroll one page left
	case SB_PAGERIGHT:		// scroll one page right
	case SB_RIGHT:			// scroll to end right
		b_result = m_displayDataFile.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		b_result = m_displayDataFile.GetDataFromDoc(
			(nPos * m_pdatDoc->GetDOCchanLength()) / 100L);
		break;
	default:				// NOP: set position only
		break;
	}

	// adjust display
	if (b_result)
	{
		UpdateLegends(UPD_ABCISSA);
		UpdateData(FALSE);	// copy view object to controls
		m_displayDataFile.Invalidate();
	}
	UpdateFileScroll();
}

void CViewData::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from CScrollBarEx
	int l_first;
	int l_last;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		m_timefirst = static_cast<float>(l_first) / m_samplingRate;
		m_timelast = static_cast<float>(l_last) / m_samplingRate;
		m_displayDataFile.GetDataFromDoc(l_first, l_last);
		m_displayDataFile.Invalidate();
		cs.Format(_T("%.3f"), m_timefirst);
		SetDlgItemText(IDC_TIMEFIRST, cs);
		cs.Format(_T("%.3f"), m_timelast);
		SetDlgItemText(IDC_TIMELAST, cs);
		break;

	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		if (m_displayDataFile.GetDataFromDoc(l_first, l_last))
		{
			UpdateLegends(UPD_ABCISSA);
			UpdateData(FALSE);	// copy view object to controls
			m_displayDataFile.Invalidate();
		}

		break;
	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void CViewData::MeasureProperties(int item)
{
	// make sure that cursor is ok
	if (m_cursorstate != CURSOR_MEASURE)
		OnMyMessage(NULL, MAKELPARAM(CURSOR_MEASURE, HINT_SETMOUSECURSOR));

	// save current data into data document
	switch (mdMO->wOption)
	{
	case 0:	m_pdatDoc->GetpVTtags()->CopyTagList(m_displayDataFile.GetVTtagList()); break;
	case 1: m_pdatDoc->GetpHZtags()->CopyTagList(m_displayDataFile.GetHZtagList()); break;
	case 3:
		mdMO->wStimuluschan = m_displayDataFile.GetHZtagChan(0);
		mdMO->wStimulusthresh = m_displayDataFile.GetHZtagVal(0);
		break;
	default: break;
	}

	// call routine and pass parameters
	CMeasureProperties dlg(nullptr, item);
	dlg.m_plineview = &m_displayDataFile;
	dlg.m_samplingrate = m_samplingRate;
	dlg.m_pdatDoc = m_pdatDoc;
	dlg.m_pdbDoc = GetDocument();
	dlg.m_pMO = mdMO;
	dlg.m_currentchan = m_ichanselected;

	dlg.DoModal();
	m_displayDataFile.Invalidate();
	SetCursorAssociatedWindows();
}

void CViewData::SaveModifiedFile()
{
	// save previous file if anything has changed
	if (m_pdatDoc == nullptr)
		return;

	if (m_pdatDoc->IsModified())
	{
		CString docname = GetDocument()->GetDB_CurrentDatFileName();
		m_pdatDoc->OnSaveDocument(docname);
	}
	m_pdatDoc->SetModifiedFlag(FALSE);
}

void CViewData::ADC_OnHardwareDefineexperiment()
{
	CdbEditRecordDlg dlg;
	dlg.m_pdbDoc = GetDocument();

	if (IDOK == dlg.DoModal())
	{
		auto p_dbwave_doc = GetDocument();
		const auto record_id = p_dbwave_doc->GetDB_CurrentRecordID();;
		GetDocument()->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
		p_dbwave_doc->DBMoveToID(record_id);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	}
}

void CViewData::OnFormatXscale()
{
	// init dialog data
	CDataViewAbcissaDlg dlg;
	dlg.m_firstAbcissa = m_timefirst;
	dlg.m_lastAbcissa = m_timelast;
	dlg.m_veryLastAbcissa = m_displayDataFile.GetDocumentLast() / m_samplingRate;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_timefirst = dlg.m_firstAbcissa * dlg.m_abcissaScale;
		m_timelast = dlg.m_lastAbcissa * dlg.m_abcissaScale;
		m_displayDataFile.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | UPD_XSCALE | CHG_XBAR);
	}
}

void CViewData::ComputePrinterPageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC();     // to delete at the end -- see doc!
	ASSERT(h_dc != NULL);
	dc.Attach(h_dc);

	// Get the size of the page in pixels
	options_viewdata->horzRes = dc.GetDeviceCaps(HORZRES);
	options_viewdata->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_printRect.right = options_viewdata->horzRes - options_viewdata->rightPageMargin;
	m_printRect.bottom = options_viewdata->vertRes - options_viewdata->bottomPageMargin;
	m_printRect.left = options_viewdata->leftPageMargin;
	m_printRect.top = options_viewdata->topPageMargin;
}

void CViewData::PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo)
{
	auto t = CTime::GetCurrentTime();
	TCHAR ch[256];
	wsprintf(ch, _T("  page %d:%d %d-%d-%d"), // %d:%d",
		pInfo->m_nCurPage, pInfo->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());

	auto cs_dat_file = GetDocument()->GetDB_CurrentDatFileName();
	const auto icount = cs_dat_file.ReverseFind(_T('\\'));
	auto ch_date = cs_dat_file.Left(icount);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + ch;
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_viewdata->horzRes / 2, options_viewdata->vertRes - 57, ch_date);
}

CString CViewData::ConvertFileIndex(long l_first, long l_last)
{
	CString csUnit = _T(" s");								// get time,  prepare time unit

	TCHAR sz_value[64];										// buffer to receive ascii represent of values
	const auto psz_value = sz_value;
	float x_scale_factor;										// scale factor returned by changeunit
	auto x = m_displayDataFile.ChangeUnit(static_cast<float>(l_first) / m_samplingRate, &csUnit, &x_scale_factor);
	auto fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));	// separate fractional part
	wsprintf(psz_value, _T("time = %i.%03.3i - "), static_cast<int>(x), fraction); // print value
	CString cs_comment = psz_value;							// save ascii to string

	x = l_last / (m_samplingRate * x_scale_factor);			// same operations for last interval
	fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("%i.%03.3i %s"), static_cast<int>(x), fraction, static_cast<LPCTSTR>(csUnit));
	cs_comment += psz_value;
	return cs_comment;
}

BOOL CViewData::GetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first)
{
	// loop until we get all rows
	const auto totalrows = m_nbrowsperpage * (page - 1);
	l_first = m_lprintFirst;
	filenumber = 0;						// file list index
	if (options_viewdata->bPrintSelection)			// current file if selection only
		filenumber = m_file0;
	else
		GetDocument()->DBMoveFirst();

	auto very_last = m_lprintFirst + m_lprintLen;
	if (options_viewdata->bEntireRecord)
		very_last = m_pdatDoc->GetDOCchanLength() - 1;

	for (auto row = 0; row < totalrows; row++)
	{
		if (!PrintGetNextRow(filenumber, l_first, very_last))
			break;
	}

	return TRUE;
}

CString CViewData::GetFileInfos()
{
	CString str_comment;   					// scratch pad
	const CString tab(_T("    "));			// use 4 spaces as tabulation character
	const CString rc(_T("\n"));				// next line

	// document's name, date and time
	const auto pwave_format = m_pdatDoc->GetpWaveFormat();
	if (options_viewdata->bDocName || options_viewdata->bAcqDateTime)// print doc infos?
	{
		if (options_viewdata->bDocName)					// print file name
		{
			str_comment += GetDocument()->GetDB_CurrentDatFileName() + tab;
		}
		if (options_viewdata->bAcqDateTime)				// print data acquisition date & time
		{
			const auto date = pwave_format->acqtime.Format(_T("%#d %B %Y %X")); //("%c");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_viewdata->bAcqComment)
		str_comment += pwave_format->GetComments(_T(" ")) + rc;

	return str_comment;
}

CString CViewData::PrintBars(CDC* p_dc, CRect* prect)
{
	CString str_comment;
	const CString rc(_T("\n"));
	const CString tab(_T("     "));

	const auto p_old_brush = (CBrush*)p_dc->SelectStockObject(BLACK_BRUSH);
	TCHAR sz_value[64];
	const auto lpsz_val = sz_value;
	CString cs_unit;
	float x_scale_factor;
	CPoint bar_origin(-10, -10);					// origine barre à 10,10 pts de coin inf gauche rectangle
	bar_origin.x += prect->left;
	bar_origin.y += prect->bottom;
	auto xbar_end = bar_origin;
	auto ybar_end = bar_origin;

	// same len ratio as displayed on viewdata
	const auto horz_bar = m_displayDataFile.m_xRuler.GetScaleUnitPixels(m_displayDataFile.GetRectWidth());
	ASSERT(horz_bar > 0);
	const auto vert_bar = m_displayDataFile.m_yRuler.GetScaleUnitPixels(m_displayDataFile.GetRectHeight());
	ASSERT(vert_bar > 0);

	auto cs_comment = ConvertFileIndex(m_displayDataFile.GetDataFirst(), m_displayDataFile.GetDataLast());
	if (options_viewdata->bTimeScaleBar)
	{
		// print horizontal bar
		xbar_end.x += horz_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(xbar_end);

		// read text from control edit
		CString cs;
		cs.Format(_T(" bar= %g"), m_displayDataFile.m_xRuler.GetScaleIncrement());
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	if (options_viewdata->bVoltageScaleBar)
	{
		ybar_end.y -= vert_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(ybar_end);
	}

	// comments, bar value and chan settings for each channel
	if (options_viewdata->bChansComment || options_viewdata->bVoltageScaleBar || options_viewdata->bChanSettings)
	{
		const auto imax = m_displayDataFile.GetChanlistSize();	// number of data channels
		for (auto ichan = 0; ichan < imax; ichan++)		// loop
		{
			// boucler sur les commentaires de chan n a chan 0...
			wsprintf(lpsz_val, _T("chan#%i "), ichan);	// channel number
			cs_comment = lpsz_val;
			if (options_viewdata->bVoltageScaleBar)				// bar scale value
			{
				cs_unit = _T(" V");						// provisional unit
				auto z = static_cast<float>(m_displayDataFile.GetRectHeight()) / 5 * m_displayDataFile.GetChanlistVoltsperPixel(ichan);
				auto x = m_displayDataFile.ChangeUnit(z, &cs_unit, &x_scale_factor); // convert

				// approximate
				auto j = static_cast<int>(x);					// get int val
				if ((double(x) - j) > 0.5)				// increment integer if diff > 0.5
					j++;
				auto k = m_displayDataFile.NiceUnit(x);	// compare with nice unit abs
				if (j > 750)                        // there is a gap between 500 and 1000
					k = 1000;
				if (MulDiv(100, abs(k - j), j) <= 1)	// keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = static_cast<float>(k) * x_scale_factor;
					j = static_cast<int>(m_displayDataFile.ChangeUnit(z, &cs_unit, &x_scale_factor)); // convert
				}
				wsprintf(sz_value, _T("bar = %i %s "), j, static_cast<LPCTSTR>(cs_unit));	// store val into comment
				cs_comment += sz_value;
			}
			str_comment += cs_comment;

			// print chan comment
			if (options_viewdata->bChansComment)
			{
				str_comment += tab;
				str_comment += m_displayDataFile.GetChanlistComment(ichan);
			}
			str_comment += rc;

			// print amplifiers settings (gain & filter), next line
			if (options_viewdata->bChanSettings)
			{
				CString cs;
				const WORD channb = m_displayDataFile.GetChanlistSourceChan(ichan);
				const auto pchanArray = m_pdatDoc->GetpWavechanArray();
				const auto pChan = pchanArray->get_p_channel(channb);
				cs.Format(_T("headstage=%s gain=%.0f  filter= %s - %i Hz"), static_cast<LPCTSTR>(pChan->am_csheadstage),
					pChan->am_gaintotal, static_cast<LPCTSTR>(pChan->am_csInputpos), pChan->am_lowpass);
				str_comment += cs;
				str_comment += rc;
			}
		}
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL CViewData::OnPreparePrinting(CPrintInfo* pInfo)
{
	// printing margins
	if (options_viewdata->vertRes <= 0						// vertical resolution defined ?
		|| options_viewdata->horzRes <= 0						// horizontal resolution defined?
		|| options_viewdata->horzRes != pInfo->m_rectDraw.Width()	// same as infos provided
		|| options_viewdata->vertRes != pInfo->m_rectDraw.Height())	// by caller?
		ComputePrinterPageSize();

	auto npages = PrintGetNPages();
	pInfo->SetMaxPage(npages);						//one page printing/preview
	pInfo->m_nNumPreviewPages = 1;  				// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;	// allow print only selection

	if (options_viewdata->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;	// set button to selection

	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	if (options_viewdata->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		options_viewdata->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = PrintGetNPages();
		pInfo->SetMaxPage(npages);
	}

	return TRUE;
}

int	CViewData::PrintGetNPages()
{
	// how many rows per page?
	const auto size_row = options_viewdata->HeightDoc + options_viewdata->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0)					// prevent zero pages
		m_nbrowsperpage = 1;

	int ntotal_rows;								// number of rectangles -- or nb of rows
	auto p_dbwave_doc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag
	m_lprintFirst = m_displayDataFile.GetDataFirst();
	m_lprintLen = m_displayDataFile.GetDataLast() - m_lprintFirst + 1;
	m_file0 = GetDocument()->GetDB_CurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	auto ifile0 = m_file0;
	auto ifile1 = m_file0;
	if (!options_viewdata->bPrintSelection)
	{
		ifile0 = 0;
		m_nfiles = p_dbwave_doc->GetDB_NRecords();
		ifile1 = m_nfiles;
	}

	// only one row per file
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
		ntotal_rows = m_nfiles;

	// multirows per file
	else
	{
		ntotal_rows = 0;
		p_dbwave_doc->SetDB_CurrentRecordPosition(ifile0);
		for (auto i = ifile0; i < ifile1; i++, p_dbwave_doc->DBMoveNext())
		{
			// get size of document for all files
			auto len = p_dbwave_doc->GetDB_DataLen();
			if (len <= 0)
			{
				p_dbwave_doc->OpenCurrentDataFile();
				len = m_pdatDoc->GetDOCchanLength();
				p_dbwave_doc->SetDB_DataLen(len);
			}
			len -= m_lprintFirst;
			auto nrows = len / m_lprintLen;			// how many rows for this file?
			if (len > nrows * m_lprintLen)			// remainder?
				nrows++;
			ntotal_rows += static_cast<int>(nrows);				// update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		try
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(m_file0);
			p_dbwave_doc->OpenCurrentDataFile();
		}
		catch (CDaoException* e) { DisplayDaoException(e, 2); e->Delete(); }
	}

	// npages
	int npages = ntotal_rows / m_nbrowsperpage;
	if (ntotal_rows > m_nbrowsperpage * npages)
		npages++;

	return npages;
}

void CViewData::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_displayDataFile.GetDataFirst();
	m_lLast0 = m_displayDataFile.GetDataLast();
	m_npixels0 = m_displayDataFile.GetRectWidth();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));			// Arial font
	m_logFont.lfHeight = options_viewdata->fontsize;			// font height
	m_pOldFont = nullptr;
	/*BOOL flag = */m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void CViewData::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_viewdata->WidthDoc;					// margins
	const auto r_height = options_viewdata->HeightDoc;					// margins
	CRect r_where(m_printRect.left, 				// printing rectangle for data
		m_printRect.top,
		m_printRect.left + r_width,
		m_printRect.top + r_height);
	//CRect RW2 = RWhere;								// printing rectangle - constant
	//RW2.OffsetRect(-RWhere.left, -RWhere.top);		// set RW2 origin = 0,0

	p_dc->SetMapMode(MM_TEXT);						// change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo);				// print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int filenumber;    								// file number and file index
	long l_first;									// index first data point / first file
	auto very_last = m_lprintFirst + m_lprintLen;		// index last data point / current file
	const int curpage = pInfo->m_nCurPage;					// get current page number
	GetFileSeriesIndexFromPage(curpage, filenumber, l_first);
	if (l_first < GetDocument()->GetDB_DataLen() - 1)
		UpdateFileParameters();
	if (options_viewdata->bEntireRecord)
		very_last = GetDocument()->GetDB_DataLen() - 1;

	SCOPESTRUCT oldparms;
	SCOPESTRUCT* p_newparms = m_displayDataFile.GetScopeParameters();
	oldparms = *p_newparms;
	p_newparms->bDrawframe = options_viewdata->bFrameRect;
	p_newparms->bClipRect = options_viewdata->bClipRect;

	// loop through all files	--------------------------------------------------------
	const int old_dc = p_dc->SaveDC();						// save DC
	for (auto i = 0; i < m_nbrowsperpage; i++)
	{
		// first : set rectangle where data will be printed
		auto comment_rect = r_where;					// save RWhere for comments
		p_dc->SetMapMode(MM_TEXT);					// 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); 				// set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data
		auto l_last = l_first + m_lprintLen;			// compute last pt to load
		if (l_first < GetDocument()->GetDB_DataLen() - 1)
		{
			if (l_last > very_last)					// check end across file length
				l_last = very_last;
			m_displayDataFile.GetDataFromDoc(l_first, l_last);	// load data from file
			UpdateChannelsDisplayParameters();
			m_displayDataFile.Print(p_dc, &r_where);			// print data
		}

		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + options_viewdata->heightSeparator);

		// restore DC and print comments --------------------------------------------------
		p_dc->SetMapMode(MM_TEXT);				// 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr);				// no more clipping
		p_dc->SetViewportOrg(0, 0);				// org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_lprintFirst)			// first row = full comment
		{
			cs_comment += GetFileInfos();
			cs_comment += PrintBars(p_dc, &comment_rect);// bars and bar legends
		}
		else									// other rows: time intervals only
			cs_comment = ConvertFileIndex(m_displayDataFile.GetDataFirst(), m_displayDataFile.GetDataLast());

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_viewdata->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
			DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		const auto ifile = filenumber;
		if (!PrintGetNextRow(filenumber, l_first, very_last))
		{
			i = m_nbrowsperpage;
			break;
		}
		if (ifile != filenumber)
			UpdateFileParameters(FALSE);
	}
	p_dc->RestoreDC(old_dc);					// restore Display context

	// end of file loop : restore initial conditions
	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
	*p_newparms = oldparms;
}

BOOL CViewData::PrintGetNextRow(int& filenumber, long& l_first, long& very_last)
{
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
	{
		filenumber++;
		if (filenumber >= m_nfiles)
			return FALSE;

		GetDocument()->DBMoveNext();
		if (l_first < GetDocument()->GetDB_DataLen() - 1)
		{
			if (options_viewdata->bEntireRecord)
				very_last = GetDocument()->GetDB_DataLen() - 1;
		}
	}
	else
	{
		l_first += m_lprintLen;
		if (l_first >= very_last)
		{
			filenumber++;						// next index
			if (filenumber >= m_nfiles)		// last file ??
				return FALSE;

			GetDocument()->DBMoveNext();
			very_last = GetDocument()->GetDB_DataLen() - 1;
			l_first = m_lprintFirst;
		}
	}
	return TRUE;
}

void CViewData::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;
	GetDocument()->SetDB_CurrentRecordPosition(m_file0);
	m_displayDataFile.ResizeChannels(m_npixels0, 0);
	m_displayDataFile.GetDataFromDoc(m_lFirst0, m_lLast0);
	UpdateFileParameters();
}

void CViewData::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
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
		default:;
		}
		m_displayDataFile.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate), static_cast<long>(m_timelast * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_displayDataFile.Invalidate();
		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1);
	}
}

void CViewData::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone) {
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
		default:;
		}
		m_displayDataFile.GetDataFromDoc((long)(m_timefirst * m_samplingRate), (long)(m_timelast * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_displayDataFile.Invalidate();
		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1);
	}
}

void CViewData::UpdateFileScroll()
{
	m_filescroll_infos.fMask = SIF_ALL | SIF_PAGE | SIF_POS;
	m_filescroll_infos.nMin = 0;
	m_filescroll_infos.nMax = GetDocument()->GetDB_DataLen();
	m_filescroll_infos.nPos = m_displayDataFile.GetDataFirst();
	m_filescroll_infos.nPage = m_displayDataFile.GetDataLast() - m_displayDataFile.GetDataFirst() + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void CViewData::OnCbnSelchangeCombochan()
{
	const auto ichan = m_comboSelectChan.GetCurSel();
	if (ichan < m_displayDataFile.GetChanlistSize())
	{
		m_bCommonScale = FALSE;
		UpdateChannel(ichan);
	}
	else
	{
		m_bCommonScale = TRUE;
		m_ichanselected = 0;
		const auto yextent = m_displayDataFile.GetChanlistYextent(0);
		UpdateYExtent(0, yextent);
		const auto yzero = m_displayDataFile.GetChanlistYzero(0);
		UpdateYZero(0, yzero);
	}
}

void CViewData::UpdateYExtent(int ichan, int yextent)
{
	m_displayDataFile.SetChanlistYextent(ichan, yextent);
	if (m_comboSelectChan.GetCurSel() == m_displayDataFile.GetChanlistSize())
	{
		const auto yVoltsextent = m_displayDataFile.GetChanlistVoltsperDataBin(ichan) * yextent;
		m_displayDataFile.SetChanlistVoltsExtent(-1, &yVoltsextent);
	}
}

void CViewData::UpdateYZero(int ichan, int ybias)
{
	m_displayDataFile.SetChanlistYzero(ichan, ybias);
	if (m_comboSelectChan.GetCurSel() == m_displayDataFile.GetChanlistSize())
	{
		const auto yVoltsextent = m_displayDataFile.GetChanlistVoltsperDataBin(ichan) * ybias;
		m_displayDataFile.SetChanlistVoltsZero(-1, &yVoltsextent);
	}
}