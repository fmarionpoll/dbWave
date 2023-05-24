#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "DlgADIntervals.h"
#include "MeasureProperties.h"
#include "RulerBar.h"
#include "DlgdbEditRecord.h"
#include "DlgADInputParms.h"
#include "ViewData.h"

#include "DlgCopyAs.h"
#include "DlgDataSeries.h"
#include "DlgDataSeriesFormat.h"
#include "DlgDataViewAbcissa.h"
#include "DlgDataViewOrdinates.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewData, dbTableView)

BEGIN_MESSAGE_MAP(ViewData, dbTableView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BIAS_button, &ViewData::OnClickedBias)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewData::OnClickedGain)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FORMAT_XSCALE, &ViewData::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &ViewData::OnFormatYscale)
	ON_COMMAND(ID_EDIT_COPY, &ViewData::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &ViewData::OnUpdateEditCopy)
	ON_COMMAND(ID_TOOLS_DATASERIES, &ViewData::OnToolsDataseries)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &ViewData::ADC_OnHardwareChannelsDlg)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &ViewData::ADC_OnHardwareIntervalsDlg)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewData::OnCenterCurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewData::OnGainAdjustCurve)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &ViewData::OnSplitCurves)
	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &ViewData::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &ViewData::OnLastFrame)
	ON_MESSAGE(WM_MYMESSAGE, &ViewData::OnMyMessage)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewData::OnViewAlldata)
	ON_COMMAND(ID_FORMAT_DATASERIESATTRIBUTES, &ViewData::OnFormatDataseriesattributes)
	ON_COMMAND(ID_TOOLS_MEASUREMODE, &ViewData::OnToolsMeasuremode)
	ON_COMMAND(ID_TOOLS_MEASURE, &ViewData::OnToolsMeasure)
	ON_COMMAND(ID_TOOLS_VERTICALTAGS, &ViewData::OnToolsVerticaltags)
	ON_COMMAND(ID_TOOLS_HORIZONTALCURSORS, &ViewData::OnToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_HORIZONTALCURSORS, &ViewData::OnUpdateToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_VERTICALTAGS, &ViewData::OnUpdateToolsVerticaltags)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &ViewData::ADC_OnHardwareDefineexperiment)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewData::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewData::OnEnChangeTimelast)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FORMAT_SETORDINATES, &ViewData::OnFormatYscale)
	ON_COMMAND(ID_FILE_PRINT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, dbTableView::OnFilePrintPreview)
	ON_CBN_SELCHANGE(IDC_COMBOCHAN, &ViewData::OnCbnSelchangeCombochan)
END_MESSAGE_MAP()

ViewData::ViewData()
	: dbTableView(IDD)
{
}

ViewData::~ViewData()
{
	m_pdatDoc->AcqCloseFile();
	DeleteObject(m_hBias);
	DeleteObject(m_hZoom);
}

void ViewData::DoDataExchange(CDataExchange* pDX)
{
	// pass values
	dbTableView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_first_Hz_cursor);
	DDX_Text(pDX, IDC_EDIT2, m_second_Hz_cursor);
	DDX_Text(pDX, IDC_EDIT3, m_difference_second_minus_first);
	DDX_Text(pDX, IDC_TIMEFIRST, m_time_first_abcissa);
	DDX_Text(pDX, IDC_TIMELAST, m_time_last_abcissa);
	DDX_Control(pDX, IDC_FILESCROLL, m_filescroll);
	DDX_Control(pDX, IDC_COMBOCHAN, m_comboSelectChan);
}

void ViewData::DefineSubClassedItems()
{
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// bitmap buttons: load icons & set buttons
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hBias)));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hZoom)));

	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAY, this));
	VERIFY(mm_time_first_abcissa.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_abcissa.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(m_ADC_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ADC_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
}

void ViewData::DefineStretchParameters()
{
	// save coordinates and properties of "always visible" controls
	m_stretch.AttachParent(this); // attach formview pointer
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

void ViewData::OnInitialUpdate()
{
	DefineSubClassedItems();

	m_ADC_yRulerBar.AttachScopeWnd(&m_ChartDataWnd, FALSE);
	m_ADC_xRulerBar.AttachScopeWnd(&m_ChartDataWnd, TRUE);
	m_ChartDataWnd.AttachExternalXRuler(&m_ADC_xRulerBar);
	m_ChartDataWnd.AttachExternalYRuler(&m_ADC_yRulerBar);
	m_ChartDataWnd.m_bNiceGrid = TRUE;

	DefineStretchParameters();

	// init relation with document, display data, adjust parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_viewdata = &(p_app->options_viewdata);
	mdMO = &(p_app->options_viewdata_measure);

	// set data file
	dbTableView::OnInitialUpdate();
	UpdateFileParameters(TRUE); 

	m_ChartDataWnd.SetScopeParameters(&(options_viewdata->viewdata));
	constexpr int legends_options = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
	m_bCommonScale = TRUE;
	m_comboSelectChan.SetCurSel(m_ChartDataWnd.GetChanlistSize());
	UpdateLegends(legends_options);
}

void ViewData::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	auto i_update = NULL;
	switch (LOWORD(lHint))
	{
	case HINT_REPLACEVIEW:
		return;
	case HINT_CLOSEFILEMODIFIED: 
		SaveModifiedFile();
		break;
	case HINT_DOCHASCHANGED: 
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
	m_ChartDataWnd.Invalidate();
	SetVBarMode(m_VBarMode);
}

void ViewData::UpdateLegends(int legends_options)
{
	if (!m_ChartDataWnd.IsDefined() && !m_bvalidDoc)
		return;
	if (legends_options & UPD_ABCISSA)
		UpdateFileScroll();
	if (legends_options & CHG_YSCALE)
	{
		UpdateHZtagsVal();
		legends_options |= CHG_YBAR;
	}
	if (legends_options & UPD_YSCALE)
		legends_options |= CHG_YBAR;
	if (legends_options & CHG_YBAR)
		UpdateYZero(m_channel_selected, m_ChartDataWnd.GetChanlistItem(m_channel_selected)->GetYzero());

	UpdateData(FALSE);
}

void ViewData::OnClickedBias()
{
	// set bias down and set gain up CButton
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void ViewData::OnClickedGain()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void ViewData::UpdateChannel(int channel)
{
	m_channel_selected = channel;
	if (m_channel_selected > m_ChartDataWnd.GetChanlistSize() - 1) 
		m_channel_selected = m_ChartDataWnd.GetChanlistSize() - 1; 
	else if (m_channel_selected < 0) 
		m_channel_selected = 0;

	if (m_channel_selected == channel)
		UpdateData(FALSE);
	else 
	{
		if (m_cursorstate == CURSOR_CROSS && mdMO->wOption == 1
			&& m_ChartDataWnd.m_HZtags.GetNTags() > 0)
		{
			for (auto i = 0; i < m_ChartDataWnd.m_HZtags.GetNTags(); i++)
				m_ChartDataWnd.m_HZtags.SetTagChan(i, m_channel_selected);
			UpdateHZtagsVal();
			m_ChartDataWnd.Invalidate();
		}
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
}

void ViewData::OnFormatYscale()
{
	DlgDataViewOrdinates dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_Channel = m_channel_selected;
	if (IDOK == dlg.DoModal())
		UpdateLegends(UPD_ORDINATES | UPD_YSCALE | CHG_YBAR);

	m_ChartDataWnd.Invalidate();
}

void ViewData::OnToolsDataseries()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_channel_selected;
	dlg.DoModal();

	m_channel_selected = dlg.m_listindex;
	UpdateLegends(UPD_YSCALE);
}

void ViewData::OnEditCopy()
{
	DlgCopyAs dlg;
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
			m_ChartDataWnd.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			CRect old_rect;
			m_ChartDataWnd.GetWindowRect(&old_rect);

			CRect rect(0, 0, options_viewdata->hzResolution, options_viewdata->vtResolution);
			m_npixels0 = m_ChartDataWnd.GetRectWidth();

			// create metafile
			CMetaFileDC m_dc;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + m_pdatDoc->GetTitle();
			cs_title += _T("\0\0");
			CRect rect_bound(0, 0, 21000, 29700); // dimensions in HIMETRIC units (in .01-millimeter increments)
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in metafile.
			CClientDC attrib_dc(this); // Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

			const auto oldparms = new SCOPESTRUCT();
			SCOPESTRUCT* p_newparms = m_ChartDataWnd.GetScopeParameters();
			*oldparms = *p_newparms;
			p_newparms->bDrawframe = options_viewdata->bFrameRect;
			p_newparms->bClipRect = options_viewdata->bClipRect;
			m_ChartDataWnd.Print(&m_dc, &rect);
			*p_newparms = *oldparms;

			// print comments : set font
			memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
			GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_logFont);
			m_pOldFont = nullptr;
			/*BOOL flag = */
			m_fontPrint.CreateFontIndirect(&m_logFont);
			m_pOldFont = m_dc.SelectObject(&m_fontPrint);
			const int lineheight = m_logFont.lfHeight + 5;
			auto ypxrow = 0;
			const auto xcol = 10;

			CString comments = _T("Abcissa: ");
			CString content;
			content.Format(_T("%g - %g s"), m_time_first_abcissa, m_time_last_abcissa);
			comments += content;
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format(_T("Vertical bar (ch. 0) = %g mV"), m_ChartDataWnd.m_yRuler.GetScaleIncrement());
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			comments.Format(_T("Horizontal bar = %g s"), m_ChartDataWnd.m_xRuler.GetScaleIncrement());
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// bars
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			m_dc.MoveTo(0, ypxrow);
			const auto bottom = m_ChartDataWnd.m_yRuler.GetScaleUnitPixels(rect.Height());
			m_dc.LineTo(0, ypxrow - bottom);
			m_dc.MoveTo(0, ypxrow);
			const auto left = m_ChartDataWnd.m_xRuler.GetScaleUnitPixels(rect.Width());
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
				EmptyClipboard(); // prepare clipboard
				SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); // put data
				CloseClipboard(); // close clipboard
			}
			else
			{
				// Someone else has the Clipboard open...
				DeleteEnhMetaFile(h_emf_tmp); // delete data
				MessageBeep(0); // tell user something is wrong!
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
			}

			// restore initial conditions
			m_ChartDataWnd.ResizeChannels(m_npixels0, 0);
			m_ChartDataWnd.GetDataFromDoc();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void ViewData::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ChartDataWnd.IsDefined() != NULL); // if document has data
}

void ViewData::ADC_OnHardwareChannelsDlg()
{
	DlgADInputs dlg;
	dlg.m_pwFormat = m_pdatDoc->GetpWaveFormat();
	dlg.m_pchArray = m_pdatDoc->GetpWavechanArray();
	if (IDOK == dlg.DoModal())
	{
		m_pdatDoc->AcqSaveDataDescriptors();
		m_pdatDoc->SetModifiedFlag(TRUE);
	}
}

void ViewData::ADC_OnHardwareIntervalsDlg()
{
	DlgADIntervals dlg;
	dlg.m_p_wave_format = m_pdatDoc->GetpWaveFormat();
	if (IDOK == dlg.DoModal())
	{
		m_pdatDoc->AcqSaveDataDescriptors();
		m_pdatDoc->SetModifiedFlag(TRUE);
	}
}

void ViewData::ChainDialog(WORD iID)
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

void ViewData::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void ViewData::OnLastFrame()
{
	OnFileScroll(SB_RIGHT, 1L);
}

void ViewData::UpdateFileParameters(BOOL bUpdateInterface)
{
	// load parameters from document file: none yet loaded?
	const BOOL b_first_update = (m_pdatDoc == nullptr);
	const auto dbwave_doc = GetDocument();
	const auto cs_dat_file = dbwave_doc->DB_GetCurrentDatFileName();
	if ((m_bvalidDoc = cs_dat_file.IsEmpty()))
		return;

	// open data file
	if (dbwave_doc->OpenCurrentDataFile() == nullptr)
	{
		MessageBox(_T("This data file could not be opened"), _T("The file might be missing, or inaccessible..."),
		           MB_OK);
		m_bvalidDoc = FALSE;
		return;
	}
	m_pdatDoc = dbwave_doc->m_pDat;
	m_pdatDoc->ReadDataInfos();
	const auto wave_format = m_pdatDoc->GetpWaveFormat();

	if (b_first_update)
	{
		m_samplingRate = wave_format->sampling_rate_per_channel; 
		m_time_first_abcissa = 0.0f; 
		m_time_last_abcissa = (m_pdatDoc->GetDOCchanLength()) / m_samplingRate;
	}

	// load parameters from current data file
	m_ChartDataWnd.AttachDataFile(m_pdatDoc);
	m_pdatDoc->SetModifiedFlag(FALSE);

	// OPTION: display entire file	--	(inactif si multirow)
	long l_first = 0;
	long l_last = m_pdatDoc->GetDOCchanLength() - 1;

	if (!options_viewdata->bEntireRecord || options_viewdata->bMultirowDisplay && !b_first_update)
	{
		l_first = static_cast<long>(m_time_first_abcissa * m_samplingRate);
		l_last = static_cast<long>(m_time_last_abcissa * m_samplingRate);
		if (l_last > m_pdatDoc->GetDOCchanLength() - 1) // last OK?
			l_last = m_pdatDoc->GetDOCchanLength() - 1; // clip to the end of the file
	}
	m_samplingRate = wave_format->sampling_rate_per_channel; // update sampling rate

	// display all channels
	auto lnvchans = m_ChartDataWnd.GetChanlistSize();
	const int ndocchans = wave_format->scan_count;

	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	if (options_viewdata->bAllChannels || lnvchans == 0)
	{
		for (auto jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{
			auto b_present = FALSE;
			for (auto j = lnvchans - 1; j >= 0; j--)
			{
				if ((b_present = (m_ChartDataWnd.GetChanlistItem(j)->GetSourceChan() == jdocchan)))
					break;
			}
			if (!b_present)
			{
				m_ChartDataWnd.AddChanlistItem(jdocchan, 0);
				lnvchans++;
			}
			m_ChartDataWnd.GetChanlistItem(jdocchan)->SetColor(jdocchan);
		}
	}

	// load real data from file and update time parameters
	m_ChartDataWnd.GetDataFromDoc(l_first, l_last); // load data requested
	m_time_first_abcissa = m_ChartDataWnd.GetDataFirstIndex() / m_samplingRate; // update abscissa parameters
	m_time_last_abcissa = m_ChartDataWnd.GetDataLastIndex() / m_samplingRate; // first - end
	m_channel_selected = 0; // select chan 0

	if (!b_first_update)
		UpdateChannelsDisplayParameters();

	// fill combo
	m_comboSelectChan.ResetContent();
	for (auto i = 0; i < m_ChartDataWnd.GetChanlistSize(); i++)
	{
		CString cs;
		cs.Format(_T("channel %i - "), i);
		cs = cs + m_ChartDataWnd.GetChanlistItem(i)->GetComment();
		m_comboSelectChan.AddString(cs);
	}
	if (ndocchans > 1)
	{
		m_comboSelectChan.AddString(_T("all channels"));
	}
	if (!m_bCommonScale)
		m_comboSelectChan.SetCurSel(0);
	else
		m_comboSelectChan.SetCurSel(m_ChartDataWnd.GetChanlistSize());

	// done
	if (bUpdateInterface)
	{
		UpdateFileScroll();
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | CHG_YSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::UpdateChannelsDisplayParameters()
{
	const auto n_lineview_chans = m_ChartDataWnd.GetChanlistSize();
	int max;
	int min;
	if (!m_bCommonScale)
	{
		for (auto i = 0; i < n_lineview_chans; i++)
		{
			// keep final gain constant even if ampli gain changed
			CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(i);
			pchan->GetMaxMin(&max, &min);
			auto iextent = pchan->GetYextent();
			auto izero = pchan->GetYzero();

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
		CChanlistItem* pchan0 = m_ChartDataWnd.GetChanlistItem(ichan);
		auto iextent = pchan0->GetYextent();
		auto izero = pchan0->GetYzero();
		if (options_viewdata->bMaximizeGain)
		{
			float vmax = 0.;
			float vmin = 0.;
			for (auto i = 0; i < n_lineview_chans; i++)
			{
				// keep final gain constant even if ampli gain changed
				CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(i);
				pchan->GetMaxMin(&max, &min);
				const auto maxchani = pchan->ConvertDataBinsToVolts(max);
				const auto minchani = pchan->ConvertDataBinsToVolts(min);
				if (maxchani > vmax)
					vmax = maxchani;
				if (minchani < vmin)
					vmin = minchani;
			}
			max = pchan0->ConvertVoltsToDataBins(vmax);
			min = pchan0->ConvertVoltsToDataBins(vmin);
			iextent = MulDiv(max - min + 1, 10, 8);
			izero = (max + min) / 2;
		}
		UpdateYExtent(ichan, iextent);
		UpdateYZero(ichan, izero);
	}
	m_ChartDataWnd.Invalidate();
}

void ViewData::SetCursorAssociatedWindows()
{
	auto n_cmd_show = SW_HIDE;
	if (m_cursorstate == CURSOR_CROSS && mdMO->wOption == 1
		&& m_ChartDataWnd.m_HZtags.GetNTags() > 0)
		n_cmd_show = SW_SHOW;

	// change windows state: edit windows
	GetDlgItem(IDC_STATIC1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC3)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT3)->ShowWindow(n_cmd_show);

	// change cursors value
	if (m_cursorstate == CURSOR_CROSS && mdMO->wOption == 1)
		UpdateHZtagsVal();
}

void ViewData::UpdateHZtagsVal()
{
	if (m_ChartDataWnd.m_HZtags.GetNTags() <= 0)
		return;
	const auto v1 = m_ChartDataWnd.m_HZtags.GetValue(0);
	auto itag = 0;
	if (m_ChartDataWnd.m_HZtags.GetNTags() > 1)
		itag = 1;
	const auto v2 = m_ChartDataWnd.m_HZtags.GetValue(itag);
	const auto mv_per_bin = m_ChartDataWnd.GetChanlistItem(m_channel_selected)->GetVoltsperDataBin() * 1000.0f;
	m_first_Hz_cursor = static_cast<float>(v1) * mv_per_bin;
	m_second_Hz_cursor = static_cast<float>(v2) * mv_per_bin;
	m_difference_second_minus_first = m_first_Hz_cursor - m_second_Hz_cursor;

	TCHAR sz_buffer[32];
	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_first_Hz_cursor);
	GetDlgItem(IDC_EDIT1)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT1)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_second_Hz_cursor);
	GetDlgItem(IDC_EDIT2)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT2)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_difference_second_minus_first);
	GetDlgItem(IDC_EDIT3)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT3)->Invalidate();
}

LRESULT ViewData::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//int j = HIWORD(lParam);		// control ID of sender
	// code = 0: chan hit 			lowp = channel
	// code = 1: cursor change		lowp = new cursor value
	// code = 2: horiz cursor hit	lowp = cursor index
	int lowp = LOWORD(lParam); // value associated

	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		// save current cursors into document if cursorstate = 3
		if (m_cursorstate == CURSOR_CROSS)
		{
			if (mdMO->wOption == 0) // vertical cursors
			{
				auto ptaglist = m_pdatDoc->GetpVTtags();
				ptaglist->CopyTagList(&m_ChartDataWnd.m_VTtags);
				m_ChartDataWnd.m_VTtags.RemoveAllTags();
			}
			else if (mdMO->wOption == 1) // horizontal cursors
			{
				auto ptaglist = m_pdatDoc->GetpHZtags();
				ptaglist->CopyTagList(&m_ChartDataWnd.m_HZtags);
				m_ChartDataWnd.m_HZtags.RemoveAllTags();
			}
			else if (mdMO->wOption == 3) // detect stimulus
			{
				mdMO->wStimuluschan = m_ChartDataWnd.m_HZtags.GetChannel(0);
				mdMO->wStimulusthresh = m_ChartDataWnd.m_HZtags.GetValue(0);
				m_ChartDataWnd.m_HZtags.RemoveAllTags();
			}
			m_ChartDataWnd.Invalidate();
		}
	// change cursor value (+1), clip to upper cursor value
		if (lowp > CURSOR_CROSS)
			lowp = 0;
	// change cursor and tell parent that it has changed
		m_cursorstate = m_ChartDataWnd.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));

	// recall cursors from document if cursorstate = 2
		if (m_cursorstate == CURSOR_CROSS)
		{
			if (mdMO->wOption == 0)
				m_ChartDataWnd.m_VTtags.CopyTagList(m_pdatDoc->GetpVTtags());
			else if (mdMO->wOption == 1)
				m_ChartDataWnd.m_HZtags.CopyTagList(m_pdatDoc->GetpHZtags());
			else if (mdMO->wOption == 3)
				m_ChartDataWnd.m_HZtags.AddTag(mdMO->wStimulusthresh, mdMO->wStimuluschan);
			m_ChartDataWnd.Invalidate();
		}
		SetCursorAssociatedWindows();
		break;

	case HINT_HITCHANNEL: // change channel if different
		m_channel_selected = lowp;
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
		break;

	case HINT_DEFINEDRECT:
		{
			const auto rect = m_ChartDataWnd.GetDefinedRect();
			mdMO->wLimitSup = static_cast<WORD>(rect.top);
			mdMO->wLimitInf = static_cast<WORD>(rect.bottom);
			mdMO->lLimitLeft = m_ChartDataWnd.GetDataOffsetfromPixel(rect.left);
			mdMO->lLimitRight = m_ChartDataWnd.GetDataOffsetfromPixel(rect.right);
		}
	// action according to option
		switch (mdMO->wOption)
		{
		// ......................  vertical tags
		case 0: // if no VTtags, then take those of rectangle, or limits of lineview
			m_ChartDataWnd.m_VTtags.AddLTag(mdMO->lLimitLeft, 0);
			if (mdMO->lLimitRight != mdMO->lLimitLeft)
				m_ChartDataWnd.m_VTtags.AddLTag(mdMO->lLimitRight, 0);
		// store new VT tags into document
			m_pdatDoc->GetpVTtags()->CopyTagList(&m_ChartDataWnd.m_VTtags);
			break;

		// ......................  horizontal cursors
		case 1: // if no HZcursors, take those of rectangle or limits of lineview
			{
				CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(m_channel_selected);
				m_ChartDataWnd.m_HZtags.AddTag(m_ChartDataWnd.GetChanlistYPixeltoBin(m_channel_selected, mdMO->wLimitSup),
				                               m_channel_selected);
				if (mdMO->wLimitInf != mdMO->wLimitSup)
					m_ChartDataWnd.m_HZtags.AddTag(
						m_ChartDataWnd.GetChanlistYPixeltoBin(m_channel_selected, mdMO->wLimitInf), m_channel_selected);
				m_pdatDoc->GetpHZtags()->CopyTagList(&m_ChartDataWnd.m_HZtags);
				if (m_ChartDataWnd.m_HZtags.GetNTags() == 2)
					SetCursorAssociatedWindows();
				UpdateHZtagsVal();
			}
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
		m_ChartDataWnd.Invalidate();
		break;

	case HINT_CHANGEHZTAG: // horizontal tag has changed 	lowp = tag nb
		if (mdMO->wOption == 3)
			mdMO->wStimulusthresh = m_ChartDataWnd.m_HZtags.GetValue(0);
		else
			UpdateHZtagsVal();
		break;

	case HINT_VIEWSIZECHANGED: // change zoom
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE);
		m_ChartDataWnd.Invalidate();
		SetVBarMode(m_VBarMode);
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_viewdata->viewdata = *(m_ChartDataWnd.GetScopeParameters());
		break;
	default:
		break;
	}
	return 0L;
}

void ViewData::OnViewAlldata()
{
	m_ChartDataWnd.GetDataFromDoc(0, GetDocument()->DB_GetDataLen() - 1);
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	UpdateData(FALSE);
	m_ChartDataWnd.Invalidate();
	UpdateFileScroll();
}

void ViewData::OnFormatDataseriesattributes()
{
	DlgDataSeriesFormat dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_pdbDoc = m_pdatDoc;
	dlg.m_listindex = m_channel_selected;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_channel_selected = dlg.m_listindex;
	}
	UpdateLegends(UPD_YSCALE);
	m_ChartDataWnd.Invalidate();
}

void ViewData::OnToolsVerticaltags()
{
	mdMO->wOption = 0;

	// change cursor and tell parent that it has changed
	m_cursorstate = m_ChartDataWnd.SetMouseCursorType(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	//MeasureProperties(1);
}

void ViewData::OnToolsHorizontalcursors()
{
	mdMO->wOption = 1;
	// change cursor and tell parent that it has changed
	m_cursorstate = m_ChartDataWnd.SetMouseCursorType(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	//MeasureProperties(0);
}

void ViewData::OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==1  && m_cursorstate == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void ViewData::OnUpdateToolsVerticaltags(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==0 && m_cursorstate == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void ViewData::OnToolsMeasuremode()
{
	MeasureProperties(2);
}

void ViewData::OnToolsMeasure()
{
	MeasureProperties(3);
}

void ViewData::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// ViewData scroll: vertical scroll bar
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

void ViewData::SetVBarMode(int bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void ViewData::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(
			m_ChartDataWnd.GetChanlistItem(m_channel_selected)->GetYextent(),
			100,
			YEXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewData::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int yExtent = m_ChartDataWnd.GetChanlistItem(m_channel_selected)->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: yExtent = YEXTENT_MIN;
		break;
	case SB_LINELEFT: yExtent -= yExtent / 10 + 1;
		break;
	case SB_LINERIGHT: yExtent += yExtent / 10 + 1;
		break;
	case SB_PAGELEFT: yExtent -= yExtent / 2 + 1;
		break;
	case SB_PAGERIGHT: yExtent += yExtent + 1;
		break;
	case SB_RIGHT: yExtent = YEXTENT_MAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: yExtent = MulDiv(nPos - 50, YEXTENT_MAX, 100);
		break;
	default: break;
	}

	// change y extent
	if (yExtent > 0) //&& yExtent<=YEXTENT_MAX)
	{
		UpdateYExtent(m_channel_selected, yExtent);
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	// update scrollBar
	m_ChartDataWnd.Invalidate();
	UpdateGainScroll();
}

void ViewData::UpdateBiasScroll()
{
	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(m_channel_selected);
	const auto i_pos = (pchan->GetYzero() - pchan->GetDataBinZero())
		* 100 / static_cast<int>(YZERO_SPAN) + 50;
	m_scrolly.SetScrollPos(i_pos, TRUE);
	UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
}

void ViewData::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(m_channel_selected);
	auto l_size = pchan->GetYzero() - pchan->GetDataBinZero();
	const auto yextent = pchan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: l_size = YZERO_MIN;
		break;
	case SB_LINELEFT: l_size -= yextent / 100 + 1;
		break;
	case SB_LINERIGHT: l_size += yextent / 100 + 1;
		break;
	case SB_PAGELEFT: l_size -= yextent / 10 + 1;
		break;
	case SB_PAGERIGHT: l_size += yextent / 10 + 1;
		break;
	case SB_RIGHT: l_size = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: l_size = (nPos - 50) * (YZERO_SPAN / 100);
		break;
	default: break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		UpdateYZero(m_channel_selected, l_size + pchan->GetDataBinZero());
	}
	// update scrollBar
	m_ChartDataWnd.Invalidate();
	UpdateBiasScroll();
}

void ViewData::OnCenterCurve()
{
	m_ChartDataWnd.CenterChan(m_channel_selected);
	m_ChartDataWnd.Invalidate();

	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(m_channel_selected);
	const auto yextent = pchan->GetYextent();
	UpdateYExtent(m_channel_selected, yextent);
	const auto yzero = pchan->GetYzero();
	UpdateYZero(m_channel_selected, yzero);
}

void ViewData::OnGainAdjustCurve()
{
	m_ChartDataWnd.MaxgainChan(m_channel_selected);
	m_ChartDataWnd.Invalidate();

	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(m_channel_selected);
	const auto yextent = pchan->GetYextent();
	UpdateYExtent(m_channel_selected, yextent);
	const auto yzero = pchan->GetYzero();
	UpdateYZero(m_channel_selected, yzero);
	UpdateLegends(CHG_YSCALE);
}

void ViewData::OnSplitCurves()
{
	const auto nchans = m_ChartDataWnd.GetChanlistSize(); // nb of data channels
	const auto pxheight = m_ChartDataWnd.GetRectHeight(); // height of the display area
	const auto pxoffset = pxheight / nchans; // height for each channel
	auto pxzero = (pxheight - pxoffset) / 2; // center first curve at

	// split display area
	int max, min;
	for (auto i = 0; i < nchans; i++)
	{
		CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(i);
		chan->GetMaxMin(&max, &min);
		const auto iextent = MulDiv(max - min + 1, 100 * nchans, 100);
		const auto ibias = MulDiv(pxzero, iextent, pxheight); // convert pixel into bins
		const auto izero = (max + min) / 2 - ibias; // change bias
		chan->SetYextent(iextent);
		chan->SetYzero(izero);
		pxzero -= pxoffset; // update position of next curve
	}
	UpdateLegends(CHG_YSCALE);
	m_ChartDataWnd.Invalidate();
}

void ViewData::OnFileScroll(UINT nSBCode, UINT nPos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
	case SB_LINELEFT: // scroll one line left
	case SB_LINERIGHT: // scroll one line right
	case SB_PAGELEFT: // scroll one page left
	case SB_PAGERIGHT: // scroll one page right
	case SB_RIGHT: // scroll to end right
		b_result = m_ChartDataWnd.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = m_ChartDataWnd.GetDataFromDoc(
			(nPos * m_pdatDoc->GetDOCchanLength()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}

	// adjust display
	if (b_result)
	{
		UpdateLegends(UPD_ABCISSA);
		UpdateData(FALSE); // copy view object to controls
		m_ChartDataWnd.Invalidate();
	}
	UpdateFileScroll();
}

void ViewData::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from ScrollBarEx
	int l_first;
	int l_last;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		m_time_first_abcissa = static_cast<float>(l_first) / m_samplingRate;
		m_time_last_abcissa = static_cast<float>(l_last) / m_samplingRate;
		m_ChartDataWnd.GetDataFromDoc(l_first, l_last);
		m_ChartDataWnd.Invalidate();
		cs.Format(_T("%.3f"), m_time_first_abcissa);
		SetDlgItemText(IDC_TIMEFIRST, cs);
		cs.Format(_T("%.3f"), m_time_last_abcissa);
		SetDlgItemText(IDC_TIMELAST, cs);
		break;

	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		if (m_ChartDataWnd.GetDataFromDoc(l_first, l_last))
		{
			UpdateLegends(UPD_ABCISSA);
			UpdateData(FALSE); // copy view object to controls
			m_ChartDataWnd.Invalidate();
		}

		break;
	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void ViewData::MeasureProperties(int item)
{
	// make sure that cursor is ok
	if (m_cursorstate != CURSOR_CROSS)
		OnMyMessage(NULL, MAKELPARAM(CURSOR_CROSS, HINT_SETMOUSECURSOR));

	// save current data into data document
	switch (mdMO->wOption)
	{
	case 0:
		m_pdatDoc->GetpVTtags()->CopyTagList(&m_ChartDataWnd.m_VTtags);
		break;
	case 1:
		m_pdatDoc->GetpHZtags()->CopyTagList(&m_ChartDataWnd.m_HZtags);
		break;
	case 3:
		mdMO->wStimuluschan = m_ChartDataWnd.m_HZtags.GetChannel(0);
		mdMO->wStimulusthresh = m_ChartDataWnd.m_HZtags.GetValue(0);
		break;
	default: break;
	}

	// call routine and pass parameters
	CMeasureProperties dlg(nullptr, item);
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_samplingrate = m_samplingRate;
	dlg.m_pdatDoc = m_pdatDoc;
	dlg.m_pdbDoc = GetDocument();
	dlg.m_pMO = mdMO;
	dlg.m_currentchan = m_channel_selected;

	dlg.DoModal();
	m_ChartDataWnd.Invalidate();
	SetCursorAssociatedWindows();
}

void ViewData::SaveModifiedFile()
{
	// save previous file if anything has changed
	if (m_pdatDoc == nullptr)
		return;

	if (m_pdatDoc->IsModified())
	{
		CString docname = GetDocument()->DB_GetCurrentDatFileName();
		m_pdatDoc->OnSaveDocument(docname);
	}
	m_pdatDoc->SetModifiedFlag(FALSE);
}

void ViewData::ADC_OnHardwareDefineexperiment()
{
	DlgdbEditRecord dlg;
	dlg.m_pdbDoc = GetDocument();

	if (IDOK == dlg.DoModal())
	{
		auto p_dbwave_doc = GetDocument();
		const auto record_id = p_dbwave_doc->DB_GetCurrentRecordID();
		GetDocument()->UpdateAllViews_dbWave(nullptr, HINT_DOCHASCHANGED, nullptr);
		p_dbwave_doc->DB_MoveToID(record_id);
		p_dbwave_doc->UpdateAllViews_dbWave(nullptr, HINT_DOCMOVERECORD, nullptr);
	}
}

void ViewData::OnFormatXscale()
{
	// init dialog data
	DlgDataViewAbcissa dlg;
	dlg.m_firstAbcissa = m_time_first_abcissa;
	dlg.m_lastAbcissa = m_time_last_abcissa;
	dlg.m_veryLastAbcissa = m_ChartDataWnd.GetDocumentLast() / m_samplingRate;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_time_first_abcissa = dlg.m_firstAbcissa * dlg.m_abcissaScale;
		m_time_last_abcissa = dlg.m_lastAbcissa * dlg.m_abcissaScale;
		m_ChartDataWnd.GetDataFromDoc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | UPD_XSCALE | CHG_XBAR);
	}
}

void ViewData::ComputePrinterPageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC(); // to delete at the end -- see doc!
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

void ViewData::PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo)
{
	auto t = CTime::GetCurrentTime();
	TCHAR ch[256];
	wsprintf(ch, _T("  page %d:%d %d-%d-%d"), // %d:%d",
	         pInfo->m_nCurPage, pInfo->GetMaxPage(),
	         t.GetDay(), t.GetMonth(), t.GetYear());

	auto cs_dat_file = GetDocument()->DB_GetCurrentDatFileName();
	const auto icount = cs_dat_file.ReverseFind(_T('\\'));
	auto ch_date = cs_dat_file.Left(icount);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + ch;
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_viewdata->horzRes / 2, options_viewdata->vertRes - 57, ch_date);
}

CString ViewData::ConvertFileIndex(long l_first, long l_last)
{
	CString csUnit = _T(" s"); // get time,  prepare time unit

	TCHAR sz_value[64]; // buffer to receive ascii represent of values
	const auto psz_value = sz_value;
	float x_scale_factor; // scale factor returned by changeunit
	auto x = m_ChartDataWnd.ChangeUnit(static_cast<float>(l_first) / m_samplingRate, &csUnit, &x_scale_factor);
	auto fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.)); // separate fractional part
	wsprintf(psz_value, _T("time = %i.%03.3i - "), static_cast<int>(x), fraction); // print value
	CString cs_comment = psz_value; // save ascii to string

	x = l_last / (m_samplingRate * x_scale_factor); // same operations for last interval
	fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("%i.%03.3i %s"), static_cast<int>(x), fraction, static_cast<LPCTSTR>(csUnit));
	cs_comment += psz_value;
	return cs_comment;
}

BOOL ViewData::GetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first)
{
	// loop until we get all rows
	const auto totalrows = m_nbrowsperpage * (page - 1);
	l_first = m_lprintFirst;
	filenumber = 0; // file list index
	if (options_viewdata->bPrintSelection) // current file if selection only
		filenumber = m_file0;
	else
		GetDocument()->DB_MoveFirst();

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

CString ViewData::GetFileInfos()
{
	CString str_comment; // scratch pad
	const CString tab(_T("    ")); // use 4 spaces as tabulation character
	const CString rc(_T("\n")); // next line

	// document's name, date and time
	const auto pwave_format = m_pdatDoc->GetpWaveFormat();
	if (options_viewdata->bDocName || options_viewdata->bAcqDateTime) // print doc infos?
	{
		if (options_viewdata->bDocName) // print file name
		{
			str_comment += GetDocument()->DB_GetCurrentDatFileName() + tab;
		}
		if (options_viewdata->bAcqDateTime) // print data acquisition date & time
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

CString ViewData::PrintBars(CDC* p_dc, CRect* prect)
{
	CString str_comment;
	const CString rc(_T("\n"));
	const CString tab(_T("     "));

	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	TCHAR sz_value[64];
	const auto lpsz_val = sz_value;
	CString cs_unit;
	float x_scale_factor;
	CPoint bar_origin(-10, -10); // origine barre à 10,10 pts de coin inf gauche rectangle
	bar_origin.x += prect->left;
	bar_origin.y += prect->bottom;
	auto xbar_end = bar_origin;
	auto ybar_end = bar_origin;

	// same len ratio as displayed on viewdata
	const auto horz_bar = m_ChartDataWnd.m_xRuler.GetScaleUnitPixels(m_ChartDataWnd.GetRectWidth());
	ASSERT(horz_bar > 0);
	const auto vert_bar = m_ChartDataWnd.m_yRuler.GetScaleUnitPixels(m_ChartDataWnd.GetRectHeight());
	ASSERT(vert_bar > 0);

	auto cs_comment = ConvertFileIndex(m_ChartDataWnd.GetDataFirstIndex(), m_ChartDataWnd.GetDataLastIndex());
	if (options_viewdata->bTimeScaleBar)
	{
		// print horizontal bar
		xbar_end.x += horz_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(xbar_end);

		// read text from control edit
		CString cs;
		cs.Format(_T(" bar= %g"), m_ChartDataWnd.m_xRuler.GetScaleIncrement());
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
		const auto imax = m_ChartDataWnd.GetChanlistSize(); // number of data channels
		for (auto ichan = 0; ichan < imax; ichan++) // loop
		{
			// boucler sur les commentaires de chan n a chan 0...
			wsprintf(lpsz_val, _T("chan#%i "), ichan); // channel number
			cs_comment = lpsz_val;
			if (options_viewdata->bVoltageScaleBar) // bar scale value
			{
				cs_unit = _T(" V"); // provisional unit
				auto z = static_cast<float>(m_ChartDataWnd.GetRectHeight()) / 5
					* m_ChartDataWnd.GetChanlistVoltsperPixel(ichan);
				auto x = m_ChartDataWnd.ChangeUnit(z, &cs_unit, &x_scale_factor); // convert

				// approximate
				auto j = static_cast<int>(x); // get int value
				if ((static_cast<double>(x) - j) > 0.5) // increment integer if diff > 0.5
					j++;
				auto k = m_ChartDataWnd.NiceUnit(x); // compare with nice unit abs
				if (j > 750) // there is a gap between 500 and 1000
					k = 1000;
				if (MulDiv(100, abs(k - j), j) <= 1) // keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = static_cast<float>(k) * x_scale_factor;
					j = static_cast<int>(m_ChartDataWnd.ChangeUnit(z, &cs_unit, &x_scale_factor)); // convert
				}
				wsprintf(sz_value, _T("bar = %i %s "), j, static_cast<LPCTSTR>(cs_unit)); // store value into comment
				cs_comment += sz_value;
			}
			str_comment += cs_comment;

			// print chan comment
			if (options_viewdata->bChansComment)
			{
				str_comment += tab;
				str_comment += m_ChartDataWnd.GetChanlistItem(ichan)->GetComment();
			}
			str_comment += rc;

			// print amplifiers settings (gain & filter), next line
			if (options_viewdata->bChanSettings)
			{
				CString cs;
				const WORD channb = m_ChartDataWnd.GetChanlistItem(ichan)->GetSourceChan();
				const auto pchanArray = m_pdatDoc->GetpWavechanArray();
				const auto pChan = pchanArray->Get_p_channel(channb);
				cs.Format(_T("headstage=%s gain=%.0f  filter= %s - %i Hz"), (LPCTSTR)pChan->am_csheadstage,
				          pChan->am_gaintotal, (LPCTSTR)pChan->am_csInputpos, pChan->am_lowpass);
				str_comment += cs;
				str_comment += rc;
			}
		}
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL ViewData::OnPreparePrinting(CPrintInfo* pInfo)
{
	// printing margins
	if (options_viewdata->vertRes <= 0 // vertical resolution defined ?
		|| options_viewdata->horzRes <= 0 // horizontal resolution defined?
		|| options_viewdata->horzRes != pInfo->m_rectDraw.Width() // same as infos provided
		|| options_viewdata->vertRes != pInfo->m_rectDraw.Height()) // by caller?
		ComputePrinterPageSize();

	auto npages = PrintGetNPages();
	pInfo->SetMaxPage(npages); //one page printing/preview
	pInfo->m_nNumPreviewPages = 1; // preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_viewdata->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION; // set button to selection

	if (!DoPreparePrinting(pInfo))
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

int ViewData::PrintGetNPages()
{
	// how many rows per page?
	const auto size_row = options_viewdata->HeightDoc + options_viewdata->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0) // prevent zero pages
		m_nbrowsperpage = 1;

	int ntotal_rows; // number of rectangles -- or nb of rows
	auto p_dbwave_doc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag
	m_lprintFirst = m_ChartDataWnd.GetDataFirstIndex();
	m_lprintLen = m_ChartDataWnd.GetDataLastIndex() - m_lprintFirst + 1;
	m_file0 = GetDocument()->DB_GetCurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	auto ifile0 = m_file0;
	auto ifile1 = m_file0;
	if (!options_viewdata->bPrintSelection)
	{
		ifile0 = 0;
		m_nfiles = p_dbwave_doc->DB_GetNRecords();
		ifile1 = m_nfiles;
	}

	// only one row per file
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
		ntotal_rows = m_nfiles;

	// multirows per file
	else
	{
		ntotal_rows = 0;
		p_dbwave_doc->DB_SetCurrentRecordPosition(ifile0);
		for (auto i = ifile0; i < ifile1; i++, p_dbwave_doc->DB_MoveNext())
		{
			// get size of document for all files
			auto len = p_dbwave_doc->DB_GetDataLen();
			if (len <= 0)
			{
				p_dbwave_doc->OpenCurrentDataFile();
				len = m_pdatDoc->GetDOCchanLength();
				p_dbwave_doc->DB_SetDataLen(len);
			}
			len -= m_lprintFirst;
			auto nrows = len / m_lprintLen; // how many rows for this file?
			if (len > nrows * m_lprintLen) // remainder?
				nrows++;
			ntotal_rows += static_cast<int>(nrows); // update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		p_dbwave_doc->DB_SetCurrentRecordPosition(m_file0);
		p_dbwave_doc->OpenCurrentDataFile();
	}

	// npages
	int npages = ntotal_rows / m_nbrowsperpage;
	if (ntotal_rows > m_nbrowsperpage * npages)
		npages++;

	return npages;
}

void ViewData::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_ChartDataWnd.GetDataFirstIndex();
	m_lLast0 = m_ChartDataWnd.GetDataLastIndex();
	m_npixels0 = m_ChartDataWnd.GetRectWidth();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial")); // Arial font
	m_logFont.lfHeight = options_viewdata->fontsize; // font height
	m_pOldFont = nullptr;
	/*BOOL flag = */
	m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewData::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_viewdata->WidthDoc; // margins
	const auto r_height = options_viewdata->HeightDoc; // margins
	CRect r_where(m_printRect.left, // printing rectangle for data
	              m_printRect.top,
	              m_printRect.left + r_width,
	              m_printRect.top + r_height);
	//CRect RW2 = RWhere;									// printing rectangle - constant
	//RW2.OffsetRect(-RWhere.left, -RWhere.top);			// set RW2 origin = 0,0

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int filenumber; // file number and file index
	long l_first; // index first data point / first file
	auto very_last = m_lprintFirst + m_lprintLen; // index last data point / current file
	const int curpage = pInfo->m_nCurPage; // get current page number
	GetFileSeriesIndexFromPage(curpage, filenumber, l_first);
	if (l_first < GetDocument()->DB_GetDataLen() - 1)
		UpdateFileParameters();
	if (options_viewdata->bEntireRecord)
		very_last = GetDocument()->DB_GetDataLen() - 1;

	SCOPESTRUCT oldparms;
	SCOPESTRUCT* p_newparms = m_ChartDataWnd.GetScopeParameters();
	oldparms = *p_newparms;
	p_newparms->bDrawframe = options_viewdata->bFrameRect;
	p_newparms->bClipRect = options_viewdata->bClipRect;

	// loop through all files	--------------------------------------------------------
	const int old_dc = p_dc->SaveDC(); // save DC
	for (auto i = 0; i < m_nbrowsperpage; i++)
	{
		// first : set rectangle where data will be printed
		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data
		auto l_last = l_first + m_lprintLen; // compute last pt to load
		if (l_first < GetDocument()->DB_GetDataLen() - 1)
		{
			if (l_last > very_last) // check end across file length
				l_last = very_last;
			m_ChartDataWnd.GetDataFromDoc(l_first, l_last); // load data from file
			UpdateChannelsDisplayParameters();
			m_ChartDataWnd.Print(p_dc, &r_where); // print data
		}

		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + options_viewdata->heightSeparator);

		// restore DC and print comments --------------------------------------------------
		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_lprintFirst) // first row = full comment
		{
			cs_comment += GetFileInfos();
			cs_comment += PrintBars(p_dc, &comment_rect); // bars and bar legends
		}
		else // other rows: time intervals only
			cs_comment = ConvertFileIndex(m_ChartDataWnd.GetDataFirstIndex(), m_ChartDataWnd.GetDataLastIndex());

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
	p_dc->RestoreDC(old_dc); // restore Display context

	// end of file loop : restore initial conditions
	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
	*p_newparms = oldparms;
}

BOOL ViewData::PrintGetNextRow(int& filenumber, long& l_first, long& very_last)
{
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
	{
		filenumber++;
		if (filenumber >= m_nfiles)
			return FALSE;

		GetDocument()->DB_MoveNext();
		if (l_first < GetDocument()->DB_GetDataLen() - 1)
		{
			if (options_viewdata->bEntireRecord)
				very_last = GetDocument()->DB_GetDataLen() - 1;
		}
	}
	else
	{
		l_first += m_lprintLen;
		if (l_first >= very_last)
		{
			filenumber++; // next index
			if (filenumber >= m_nfiles) // last file ??
				return FALSE;

			GetDocument()->DB_MoveNext();
			very_last = GetDocument()->DB_GetDataLen() - 1;
			l_first = m_lprintFirst;
		}
	}
	return TRUE;
}

void ViewData::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;
	GetDocument()->DB_SetCurrentRecordPosition(m_file0);
	m_ChartDataWnd.ResizeChannels(m_npixels0, 0);
	m_ChartDataWnd.GetDataFromDoc(m_lFirst0, m_lLast0);
	UpdateFileParameters();
}

void ViewData::OnEnChangeTimefirst()
{
	if (mm_time_first_abcissa.m_bEntryDone)
	{
		mm_time_first_abcissa.OnEnChange(this, m_time_first_abcissa, 1.f, -1.f);
		m_ChartDataWnd.GetDataFromDoc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::OnEnChangeTimelast()
{
	if (mm_time_last_abcissa.m_bEntryDone)
	{
		mm_time_last_abcissa.OnEnChange(this, m_time_last_abcissa, 1.f, -1.f);
		m_ChartDataWnd.GetDataFromDoc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::UpdateFileScroll()
{
	m_filescroll_infos.fMask = SIF_ALL | SIF_PAGE | SIF_POS;
	m_filescroll_infos.nMin = 0;
	m_filescroll_infos.nMax = GetDocument()->DB_GetDataLen();
	m_filescroll_infos.nPos = m_ChartDataWnd.GetDataFirstIndex();
	m_filescroll_infos.nPage = m_ChartDataWnd.GetDataLastIndex() - m_ChartDataWnd.GetDataFirstIndex() + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void ViewData::OnCbnSelchangeCombochan()
{
	const auto ichan = m_comboSelectChan.GetCurSel();
	if (ichan < m_ChartDataWnd.GetChanlistSize())
	{
		m_bCommonScale = FALSE;
		UpdateChannel(ichan);
	}
	else
	{
		m_bCommonScale = TRUE;
		m_channel_selected = 0;
		CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(0);
		const auto yextent = pchan->GetYextent();
		UpdateYExtent(0, yextent);
		const auto yzero = pchan->GetYzero();
		UpdateYZero(0, yzero);
	}
}

void ViewData::UpdateYExtent(int ichan, int yextent)
{
	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(ichan);
	pchan->SetYextent(yextent);
	if (m_comboSelectChan.GetCurSel() == m_ChartDataWnd.GetChanlistSize())
	{
		const auto yVoltsextent = pchan->GetVoltsperDataBin() * yextent;
		m_ChartDataWnd.SetChanlistVoltsExtent(-1, &yVoltsextent);
	}
}

void ViewData::UpdateYZero(int ichan, int ybias)
{
	CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(ichan);
	chan->SetYzero(ybias);
	if (m_comboSelectChan.GetCurSel() == m_ChartDataWnd.GetChanlistSize())
	{
		const auto yVoltsextent = chan->GetVoltsperDataBin() * ybias;
		m_ChartDataWnd.SetChanlistVoltsZero(-1, &yVoltsextent);
	}
}
