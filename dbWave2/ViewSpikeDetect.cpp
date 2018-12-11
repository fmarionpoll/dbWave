
// * m_displayData   display source data (all channels)
// * m_displayDetect holds a display of source data displayed as transformed for spike detection
// * m_spkBarView display spikes detected
// * m_spkShapeView display spikes detected (superimposed)
//
// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

/////////////////////////////////////////////////////
#include "StdAfx.h"
#include <math.h>
#include "dbWave.h"
#include "dbWave_constants.h"
#include "resource.h"

#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "Spikedoc.h"
#include "Xyparame.h"
#include "Spikebar.h"
#include "spikeshape.h"

#include "Spikedetec.h"
#include "Editspik.h"
#include "Vdseries.h"
#include "Copyasdl.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "EditStimArrayDlg.h"
#include "ProgDlg.h"

#include ".\ViewSpikeDetect.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BRESTORE	0
#define BSAVE		1

/////////////////////////////////////////////////////////////////////////////
// CSpikeDetectionView

IMPLEMENT_DYNCREATE(CSpikeDetectionView, CDaoRecordView)

CSpikeDetectionView::CSpikeDetectionView()
	: CDaoRecordView(CSpikeDetectionView::IDD), m_dbDoc(nullptr), m_filescroll_infos(), m_zoominteger(0),
	  m_pArrayFromApp(nullptr), m_pDetectParms(nullptr), m_iDetectParms(0), mdPM(nullptr), mdMO(nullptr),
	  m_samplingRate(0), m_szbuf{}, m_file0(0), m_lFirst0(0), m_lLast0(0), m_npixels0(0), m_nfiles(0),
	  m_nbrowsperpage(0), m_lprintFirst(0), m_lprintLen(0), m_printFirst(0), m_printLast(0), m_bIsPrinting(0),
	  m_tMetric(), m_logFont(), m_pOldFont(nullptr), m_hBias(nullptr), m_hZoom(nullptr), m_yscaleFactor(0),
	  m_VBarMode(0), m_hBias2(nullptr), m_hZoom2(nullptr), m_yscaleFactor2(0), m_VBarMode2(0)
{
	m_timefirst = 0.f;
	m_timelast = 0.f;
	m_datacomments = "";
	m_spikeno = -1;
	m_bartefact = FALSE;
	m_thresholdval = 0.f;
	m_ichanselected = 0;
	m_ichanselected2 = 0;
	m_pspkDocVSD = nullptr;
	m_pSpkListVSD = nullptr;
	m_cursorstate = 0;
	m_bValidThreshold = FALSE;
	m_bDetected = FALSE;
	m_binit = FALSE;
	m_scancount_doc = -1;
	m_bEnableActiveAccessibility = FALSE;
}

CSpikeDetectionView::~CSpikeDetectionView()
{
	// save spkD list i	 changed
	if (m_pspkDocVSD != nullptr)
		SaveCurrentFileParms();	// save file if modified
}

BOOL CSpikeDetectionView::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CSpikeDetectionView::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Control(pDX,	IDC_COMBO1, m_CBdetectWhat);
	DDX_Control(pDX,	IDC_SOURCECHAN, m_CBdetectChan);
	DDX_Control(pDX,	IDC_TRANSFORM, m_CBtransform);
	DDX_Control(pDX,	IDC_TRANSFORM2, m_CBtransform2);
	DDX_Text(pDX,		IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX,		IDC_TIMELAST, m_timelast);
	DDX_Text(pDX,		IDC_SPIKENO, m_spikeno);
	DDX_Check(pDX,		IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX,		IDC_THRESHOLDVAL, m_thresholdval);
	DDX_Text(pDX,		IDC_CHANSELECTED, m_ichanselected);
	DDX_Text(pDX,		IDC_CHANSELECTED2, m_ichanselected2);
	DDX_Control(pDX,	IDC_XSCALE, m_xspkdscale);
	DDX_Control(pDX,	IDC_STATICDISPLAYDATA, m_bevel1);
	DDX_Control(pDX,	IDC_STATICDISPLAYDETECT, m_bevel2);
	DDX_Control(pDX,	IDC_STATICDISPLAYBARS, m_bevel3);
	DDX_Control(pDX,	IDC_TAB1, m_tabCtrl);
}


BEGIN_MESSAGE_MAP(CSpikeDetectionView, CDaoRecordView)
	ON_WM_SIZE()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)	 
	ON_COMMAND(ID_FORMAT_FIRSTFRAME, OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, OnLastFrame)
	ON_COMMAND(ID_FORMAT_SETORDINATES, OnFormatXscale)
	ON_COMMAND(ID_FORMAT_ALLDATA, OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, OnFormatYscaleCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, OnFormatYscaleGainadjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, OnFormatSplitcurves)
	ON_COMMAND(ID_TOOLS_DETECT_PARMS, OnToolsDetectionparameters)
	ON_COMMAND(ID_TOOLS_DETECT, OnMeasureAll)
	ON_COMMAND(ID_TOOLS_EDITSTIMULUS, OnToolsEditstimulus)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_DATASERIES, OnToolsDataseries)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FORMAT_XSCALE, OnFormatXscale)
	ON_CBN_SELCHANGE(IDC_SOURCECHAN, OnSelchangeDetectchan)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, OnSelchangeTransform)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeDetectMode)
	ON_EN_CHANGE(IDC_THRESHOLDVAL, OnEnChangeThresholdval)
	ON_EN_CHANGE(IDC_TIMEFIRST, OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangeSpikeno)
	ON_EN_CHANGE(IDC_SPIKEWINDOWAMPLITUDE, OnEnChangeSpkWndAmplitude)
	ON_EN_CHANGE(IDC_SPIKEWINDOWLENGTH, OnEnChangeSpkWndLength)
	ON_EN_CHANGE(IDC_CHANSELECTED, OnEnChangeChanselected)
	ON_BN_CLICKED(IDC_MEASUREALL, OnMeasureAll)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	ON_BN_CLICKED(IDC_BIAS, OnBnClickedBiasbutton)
	ON_BN_CLICKED(IDC_GAIN, OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_LOCATEBTTN, OnBnClickedLocatebttn)
	ON_BN_CLICKED(IDC_CLEARALL, OnBnClickedClearall)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()	
	ON_WM_SETFOCUS()

	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)	
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_MEASURE, &CSpikeDetectionView::OnMeasure)
	ON_BN_CLICKED(IDC_GAIN2, &CSpikeDetectionView::OnBnClickedGain2)
	ON_BN_CLICKED(IDC_BIAS2, &CSpikeDetectionView::OnBnClickedBias2)
	ON_EN_CHANGE(IDC_CHANSELECTED2, &CSpikeDetectionView::OnEnChangeChanselected2)
	ON_CBN_SELCHANGE(IDC_TRANSFORM2, &CSpikeDetectionView::OnCbnSelchangeTransform2)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CSpikeDetectionView::OnNMClickTab1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpikeDetectionView message handlers


void CSpikeDetectionView::OnFileSave() 
{
	CFile f;
	CFileDialog dlg(FALSE,
					_T("spk"),										// default filename extension
					GetDocument()->DBGetCurrentSpkFileName(),		// initial file name
					OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
					_T("Awave Spikes (*.spk) | *.spk |All Files (*.*) | *.* ||"));
					 
	if (IDOK == dlg.DoModal())
	{
		m_pspkDocVSD->OnSaveDocument(dlg.GetPathName());
		GetDocument()->Setnbspikes(m_pspkDocVSD->GetSpkListCurrent()->GetTotalSpikes());
		GetDocument()->Setnbspikeclasses(1);
		m_pspkDocVSD->SetModifiedFlag(FALSE);
	}
}

BOOL CSpikeDetectionView::OnMove(UINT nIDMoveCommand) 
{
	BOOL flag = TRUE;
	SaveCurrentFileParms();
	flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CSpikeDetectionView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_CLOSEFILEMODIFIED:	// close modified file: save
		SaveCurrentFileParms();
		break;
	case HINT_DOCMOVERECORD:
	case HINT_DOCHASCHANGED:		// file has changed?
		UpdateFileParameters();
		break;
	case HINT_REPLACEVIEW:
	default:			
		break;
	}	
}

// --------------------------------------------------------------------------
// OnActivateView()
// --------------------------------------------------------------------------

void CSpikeDetectionView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// activate view
	if (bActivate)
	{
		CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
		pmF->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	// exit view
	else
	{
		SaveCurrentFileParms();	 		// save current options so that we can restore them when we return
		SerializeWindowsState(BSAVE);	// set bincrflagonsave
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		pApp->vdS.bincrflagonsave = ((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CSpikeDetectionView::UpdateLegends()
{
	long lFirst = m_displayDetect.GetDataFirst();		// get source data time range
	long lLast = m_displayDetect.GetDataLast();
	m_displayDetect.Invalidate();
	m_displayData.GetDataFromDoc(lFirst, lLast);
	m_displayData.Invalidate();

	// draw dependent buttons
	m_spkBarView.SetTimeIntervals(lFirst, lLast);		// spike bars
	m_spkShapeView.SetTimeIntervals(lFirst, lLast);		// superimposed spikes
	UpdateSpkShapeWndScale(FALSE); 						// get parms from file/prevent spkshape draw
	
	// update text abcissa and horizontal scroll position
	m_timefirst = lFirst/m_samplingRate;
	m_timelast = (lLast+1)/m_samplingRate;
	m_spikeno = m_pSpkListVSD->m_selspike;
	if (m_spikeno > 0)
	{
		CSpikeElemt* pS = m_pSpkListVSD->GetSpikeElemt(m_spikeno);
		m_bartefact = (pS->GetSpikeClass() <0);	
	}
	else
	{
		m_bartefact = FALSE;
	}
	
	UpdateFileScroll();		// adjust scroll bar to file position	
	UpdateCB();				// update combo box
	UpdateVTtags();			// set VT tags
	
	// update data displayed
	m_spkBarView.Invalidate();
	m_displayDetect.Invalidate();
	m_spkShapeView.Invalidate();
	UpdateData(FALSE);

	// update number of spikes
	int nspikes = m_pSpkListVSD->GetTotalSpikes() ;
	if (nspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
	{
		SetDlgItemInt(IDC_NBSPIKES_NB, nspikes);
		GetDlgItem(IDC_NBSPIKES_NB)->Invalidate();
	}
}

void CSpikeDetectionView::SaveCurrentFileParms()
{
	CdbWaveDoc* pDoc = GetDocument();

	// save spike file if anything has changed
	if (m_pspkDocVSD != nullptr && (m_pspkDocVSD->IsModified() 
		|| m_bDetected))
	{
		// save file data name
		CAcqDataDoc* pdatDoc = GetDocument()->m_pDat;
		CString docname = GetDocument()->DBGetCurrentDatFileName();
		m_pspkDocVSD->m_acqfile = docname;
		m_pspkDocVSD->InitSourceDoc(pdatDoc);						// init file doc, etc
		m_pspkDocVSD->SetDetectionDate(CTime::GetCurrentTime());	// detection date

		CString filename = pDoc->DBDefineCurrentSpikeFileName();
		m_pspkDocVSD->OnSaveDocument(filename);

		// save nb spikes into database
		int nlist = m_pspkDocVSD->GetSpkListSize();
		m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(0);
		int nspikes = m_pSpkListVSD->GetTotalSpikes();
		GetDocument()->Setnbspikes(nspikes);
		if (!m_pSpkListVSD->IsClassListValid())
			m_pSpkListVSD->UpdateClassList();
		int nbclasses = m_pSpkListVSD->GetNbclasses();
		GetDocument()->Setnbspikeclasses(nbclasses);
		m_bDetected=FALSE;

		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->DBGetCurrentRecordFlag();
			flag++;
			GetDocument()->DBSetCurrentRecordFlag(flag);
		}
	}

	// save spike detection parameters
	CSpkDetectArray* parray = m_pArrayFromApp->GetChanArray(m_scancount_doc);
	*parray = m_parmsCurrent;
}

void CSpikeDetectionView::UpdateSpikeFile(BOOL bUpdateInterface)
{	
	// update spike doc and temporary spike list
	CdbWaveDoc* pdbDoc = GetDocument();
	pdbDoc->DBGetCurrentSpkFileName(FALSE);

	// open the current spike file
	if (!GetDocument()->OpenCurrentSpikeFile())
	{
		// file not found: create new object, and create file
		CSpikeDoc* pspk = new CSpikeDoc;
		ASSERT(pspk != NULL);
		pdbDoc->m_pSpk = pspk;
		m_pspkDocVSD = pspk;
		m_pspkDocVSD->OnNewDocument();
		m_pspkDocVSD->ClearData();
		if (mdPM->bDetectWhileBrowse)  
			OnMeasureAll();
	}
	else
	{
		m_pspkDocVSD = GetDocument()->m_pSpk;
		m_pspkDocVSD->SetModifiedFlag(FALSE);
		m_pspkDocVSD->SetPathName(GetDocument()->DBGetCurrentSpkFileName(FALSE), FALSE);		// init source doc name	
	}

	// init pointers and data

	// select a spikelist
	int icurspklist = GetDocument()->GetcurrentSpkListIndex();		// curr spike list at the level of the database
	m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(icurspklist);	// select this spike list in the spike document

	// invalide spikelist?
	if (m_pSpkListVSD == nullptr && m_pspkDocVSD->GetSpkListSize() > 0)	
	{
		// at least one spike list is available, select list[0]
		m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(0);
	}

	// no spikes list available, create one
	if (m_pSpkListVSD == nullptr)
	{
		// create new list here
		ASSERT(m_pspkDocVSD->GetSpkListSize()==0);
		int isize = m_parmsCurrent.GetSize();
		m_pspkDocVSD->SetSpkListSize(isize);
		for (int i=0; i < isize; i++)
		{
			CSpikeList*	pL = m_pspkDocVSD->SetSpkListCurrent(i);
			if (pL == nullptr)
			{
				m_pspkDocVSD->AddSpkList();
				pL = m_pspkDocVSD->GetSpkListCurrent();
			}
			pL->InitSpikeList(pdbDoc->m_pDat, m_parmsCurrent.GetItem(i));
		}
		icurspklist = 0;
		pdbDoc->SetcurrentSpkListIndex(icurspklist);
		m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(icurspklist);
		ASSERT(m_pSpkListVSD !=nullptr);
	}

	// 
	m_spkBarView.SetSourceData(m_pSpkListVSD, m_pspkDocVSD);		// declare this file to bars view
	m_spkBarView.SetPlotMode(PLOT_BLACK, 0);
	m_spkShapeView.SetSourceData(m_pSpkListVSD);					// declare file to shape view
	m_spkShapeView.SetPlotMode(PLOT_BLACK, 0);
	UpdateVTtags();

	// update interface elements
	m_spikeno = -1;
	if (bUpdateInterface)
	{
		UpdateTabs();
		UpdateDetectionControls();
		HighlightSpikes(TRUE);	
	}
	m_displayData.Invalidate();
	m_displayDetect.Invalidate();
	m_spkBarView.Invalidate();
	m_spkShapeView.Invalidate();

	// update nb spikes
	int nspikes = m_pSpkListVSD->GetTotalSpikes() ; 
	if (nspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, nspikes);
}

void CSpikeDetectionView::HighlightSpikes(BOOL flag)
{
	CDWordArray* pDWintervals = nullptr;	  // remove setting if no spikes of if flag is false
	if (flag && m_pSpkListVSD != nullptr && m_pSpkListVSD->GetTotalSpikes() < 1)
	{
		pDWintervals = &m_DWintervals;

		// init array
		int arraysize = m_pSpkListVSD->GetTotalSpikes() * 2 + 3;
		m_DWintervals.SetSize(arraysize);				// prepare space for all spike interv
		m_DWintervals.SetAt(0, 0);						// source channel
		m_DWintervals.SetAt(1, (DWORD)RGB(255, 0, 0));	// red color
		m_DWintervals.SetAt(2, 1);						// pen size
		int max = m_pSpkListVSD->GetTotalSpikes(); 		// nb of spikes
		int jindex = 3;									// index / DWord array
		int spklen = m_pSpkListVSD->GetSpikeLength();		// pre-trig and spike length
		int spkpretrig = m_pSpkListVSD->GetSpikePretrig();
		spklen--;

		for (int i = 0; i < max; i++)
		{
			CSpikeElemt* pS = m_pSpkListVSD->GetSpikeElemt(i);
			long lFirst = pS->GetSpikeTime() - spkpretrig;
			m_DWintervals.SetAt(jindex, lFirst);
			jindex++;
			m_DWintervals.SetAt(jindex, lFirst + spklen);
			jindex++;
		}
	}
	m_displayDetect.SetHighlightData(pDWintervals); // tell sourceview to highlight spk
}

//---------------------------------------------------------------------------
// UpdateFileParameters()
//---------------------------------------------------------------------------

void CSpikeDetectionView::UpdateFileParameters(BOOL bUpdateInterface)
{
	UpdateDataFile(bUpdateInterface);	// update data file
	UpdateSpikeFile(bUpdateInterface);	// check if corresp spike file is present and load it
	if (bUpdateInterface)
		UpdateLegends();
}

BOOL CSpikeDetectionView::CheckDetectionSettings()
{
	BOOL flag=TRUE;
	ASSERT(m_pDetectParms != NULL);
	ASSERT_VALID(m_pDetectParms);
	if (nullptr == m_pDetectParms)
	{
		m_iDetectParms = GetDocument()->GetcurrentSpkListIndex();
		m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	}

	// get infos from data file
	CAcqDataDoc* pDataFile = GetDocument()->m_pDat;
	pDataFile->ReadDataInfos();
	CWaveChanArray* pchanArray = pDataFile->GetpWavechanArray();
	CWaveFormat* pwaveFormat = pDataFile->GetpWaveFormat();

	// check detection and extraction channels
	if (m_pDetectParms->detectChan < 0 
		|| m_pDetectParms->detectChan >= pwaveFormat->scan_count)
	{
		m_pDetectParms->detectChan = 0;
		AfxMessageBox(_T("Spike detection parameters: detection channel modified"));
		flag=FALSE;
	}

	if (m_pDetectParms->extractChan < 0 
		|| m_pDetectParms->extractChan >= pwaveFormat->scan_count)
	{
		m_pDetectParms->extractChan = 0;
		AfxMessageBox(_T("Spike detection parameters: channel extracted modified"));
		flag=FALSE;
	}
	return flag;
}

void CSpikeDetectionView::UpdateDataFile(BOOL bUpdateInterface)
{
	// load method combo box with content
	// init chan list, select first detection channel
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetDocument();
	pdbDoc->DBGetCurrentDatFileName();
	if (!pdbDoc->OpenCurrentDataFile())
		return;

	// set a local pointer to the datafile into the dat document
	CAcqDataDoc* pDataFile = pdbDoc->m_pDat;
	pDataFile->ReadDataInfos();
	CWaveChanArray* pchanArray = pDataFile->GetpWavechanArray();
	CWaveFormat* pwaveFormat = pDataFile->GetpWaveFormat();

	// if the number of data channels of the data source has changed, load a new set of parameters
	// we keep one array of spike detection parameters per data acquisition configuration (ie nb of acquisition channels)
	if (m_scancount_doc != pwaveFormat->scan_count)
	{
		// save current set of parameters if scancount >= 0
		// this might not be necessary
		CSpkDetectArray* parray = m_pArrayFromApp->GetChanArray(m_scancount_doc);
		if (m_scancount_doc >= 0)
			*parray = m_parmsCurrent;

		// Get parms from the application array
		m_scancount_doc = pwaveFormat->scan_count;
		m_parmsCurrent = *(m_pArrayFromApp->GetChanArray(m_scancount_doc));
		// select by default the first set of detection parms
		m_iDetectParms = 0;
		m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	}

	// check if detection parms are valid
	CheckDetectionSettings();
	
	// update combo boxes
	if (bUpdateInterface)
	{
		// update number of available transformations?
		if (m_CBdetectChan.GetCount() != pwaveFormat->scan_count)
		{
			CString comment;
			m_CBdetectChan.ResetContent();
			int imax = pwaveFormat->scan_count;
			// load chan list into m_chan comboBox
			for (int i =0; i<imax; i++)	
			{				
				comment.Format(_T("%i: "), i);
				comment += pchanArray->get_p_channel(i)->am_csComment;
				VERIFY(m_CBdetectChan.AddString(comment) != CB_ERR);// add string to control
			}

			// load transforms
			m_CBtransform.ResetContent();					// zeroes any content
			m_CBtransform2.ResetContent();
			int jmax = pDataFile->GetTransfDataNTypes();	// ask document how many transf available
			for (int j=0; j<jmax; j++)						// loop and load ascii comment
			{
				VERIFY(m_CBtransform.AddString(pDataFile->GetTransfDataName(j)) != CB_ERR);
				VERIFY(m_CBtransform2.AddString(pDataFile->GetTransfDataName(j)) != CB_ERR);
			}
		}
		m_CBdetectChan.SetCurSel(m_pDetectParms->detectChan);
	}
	// change doc attached to lineviewbutton
	m_displayDetect.AttachDataFile(pDataFile, 0);
	m_displayData.AttachDataFile(pDataFile, 0);

	// update sourceview display
	if (m_displayDetect.GetChanlistSize() < 1)
	{
		m_displayDetect.RemoveAllChanlistItems();
		m_displayDetect.AddChanlistItem(0, 0);
		m_displayDetect.SetChanlistColor(0, 0);
		m_displayDetect.DelAllHZtags();
		m_pDetectParms->detectThreshold = m_displayDetect.ConvertChanlistVoltstoDataBins(0, m_pDetectParms->detectThresholdmV/1000.f);
		m_displayDetect.AddHZtag(m_pDetectParms->detectThreshold, 0);
	}

	//add all channels to detection window
	int lnvchans = m_displayData.GetChanlistSize();
	int ndocchans = pwaveFormat->scan_count;		
	for (int i = 0; i <ndocchans; i++)
	{										// check if present in the list
		BOOL bPresent=FALSE;				// pessimistic
		for (int j = lnvchans-1; j>= 0; j--)// check all channels / display list
		{									// test if this data chan is present + no transf
			if (m_displayData.GetChanlistSourceChan(j) == i
				&& m_displayData.GetChanlistTransformMode(j) == 0)
			{
				bPresent = TRUE;			// the wanted chan is present: stop loopint through disp list
				break;						// and examine next doc channel
			}
		}
		if (bPresent == FALSE)				// no display chan contains that doc chan
		{
			m_displayData.AddChanlistItem(i, 0);	// add this channel
			lnvchans++;
		}
		m_displayData.SetChanlistColor(i, i);
	}
	

	// if browse through another file ; keep previous display parameters & load data
	long lFirst = m_displayDetect.GetDataFirst();
	long lLast = m_displayDetect.GetDataLast();
	if (mdPM->bEntireRecord && bUpdateInterface)					// set params for data length
	{
		lFirst = 0;
		lLast = pDataFile->GetDOCchanLength()-1;
	}
	m_displayDetect.GetDataFromDoc(lFirst, lLast);
	m_displayData.GetDataFromDoc(lFirst, lLast);

	// adjust display according to options
	if (bUpdateInterface)
	{
		if (mdPM->bMaximizeGain && mdPM->bCenterCurves)
		{
			m_displayDetect.AutoZoomChan(-1);			// center & maximize gain of all curves
			m_displayData.AutoZoomChan(-1);
		}
		else if (mdPM->bMaximizeGain)
		{
			m_displayDetect.MaxgainChan(-1);			// maximize gain of all curves
			m_displayData.MaxgainChan(-1);
		}
		else if (mdPM->bCenterCurves)
		{
			m_displayDetect.CenterChan(-1);				// center all curves
			m_displayData.CenterChan(-1);
		}
		else if (mdPM->bSplitCurves)
		{
			m_displayDetect.SplitChans();
			m_displayData.SplitChans();
		}
		else
		{											// keep the same gain (in volts)
			float yvoltsextent = m_displayDetect.GetChanlistVoltsExtent(0);
			if (yvoltsextent == 0.0f)
			{
				m_displayDetect.MaxgainChan(-1);		// maximize gain of all curves
				m_displayDetect.SetChanlistVoltsExtent(0, nullptr);
				yvoltsextent = m_displayDetect.GetChanlistVoltsExtent(0);
				ASSERT(yvoltsextent != 0.0f);
			}
			m_displayDetect.SetChanlistVoltsExtent(0, &yvoltsextent);
		}
	}

	if (bUpdateInterface)
	{
		m_displayDetect.Invalidate();
		m_displayData.Invalidate();
		// adjust scroll bar (size of button and left/right limits)
		m_filescroll_infos.fMask = SIF_ALL;
		m_filescroll_infos.nMin = 0;
		m_filescroll_infos.nMax = pDataFile->GetDOCchanLength()-1;
		m_filescroll_infos.nPos = 0;
		m_filescroll_infos.nPage = m_displayDetect.GetDataLast()-m_displayDetect.GetDataFirst()+1;
		m_filescroll.SetScrollInfo(&m_filescroll_infos);	

		m_datacomments = pwaveFormat->GetComments(_T(" "));
		m_samplingRate = pwaveFormat->chrate;
		m_bValidThreshold = FALSE;
	}
}

//---------------------------------------------------------------------------
// OnInitialUpdate()
//---------------------------------------------------------------------------

void CSpikeDetectionView::OnInitialUpdate()
{
	// load spike detection parameters from .INI file
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	m_pArrayFromApp = &(pApp->spkDA);		// get address of spike detection array parms
	mdPM= &(pApp->vdP);						// get address of browse/print parms
	
	// top right ----------------------------------------
	m_stretch.AttachParent(this);				// attach formview pointer
	m_stretch.newProp(IDC_FILESCROLL,		XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_EMPTYPICTURE,	XLEQ_XREQ,YTEQ_YBEQ);
	m_stretch.newSlaveProp(IDC_DISPLAYDATA,		XLEQ_XREQ, SZPR_YTEQ, IDC_EMPTYPICTURE);
	m_stretch.newSlaveProp(IDC_DISPLAYDETECT,	XLEQ_XREQ, SZPR_YBEQ, IDC_EMPTYPICTURE);
		
	m_stretch.newSlaveProp(IDC_CHANSELECTED2,	SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA); 
	m_stretch.newSlaveProp(IDC_GAIN2, 			SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);	
	m_stretch.newSlaveProp(IDC_BIAS2,			SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_SCROLLY2,		SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_STATICDISPLAYDATA,	SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDATA);

	m_stretch.newProp(IDC_TAB1,	XLEQ_XREQ, SZEQ_YBEQ);
	
	m_stretch.newSlaveProp(IDC_CHANSELECTED,	SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_GAIN, 			SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);	
	m_stretch.newSlaveProp(IDC_BIAS,			SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_SCROLLY,			SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_STATICDISPLAYDETECT, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDETECT); 

	// bottom right  ------------------------------------
	// display spikes detected as bars, rectangle around & horizontal scroll bar
	m_stretch.newProp(IDC_DISPLAYBARS,		XLEQ_XREQ,SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATICDISPLAYBARS, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_DURATIONTEXT,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKEWINDOWLENGTH, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_MINTEXT,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKEWINDOWAMPLITUDE, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_NBSPIKES_NB,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_NBSPIKES_TEXT,	SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TRANSFORM2,		SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_STATIC3,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKENO,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_ARTEFACT,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYSPIKES,	SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_TIMEFIRST,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST,			SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SOURCE,			SZEQ_XREQ, SZEQ_YBEQ);
	
	m_stretch.newProp(IDC_XSCALE, 			XLEQ_XREQ, SZEQ_YBEQ);
	m_binit = TRUE;

	// attach controls
	VERIFY(m_filescroll.SubclassDlgItem(IDC_FILESCROLL, this));
	VERIFY(m_CBtransform.SubclassDlgItem(IDC_TRANSFORM, this));
	VERIFY(m_CBtransform2.SubclassDlgItem(IDC_TRANSFORM2, this));
	VERIFY(m_CBdetectChan.SubclassDlgItem(IDC_SOURCECHAN, this));
	VERIFY(m_CBdetectWhat.SubclassDlgItem(IDC_COMBO1, this));
	
	VERIFY(mm_spikeno.SubclassDlgItem(IDC_SPIKENO, this));
	mm_spikeno.ShowScrollBar(SB_VERT);
	VERIFY(mm_thresholdval.SubclassDlgItem(IDC_THRESHOLDVAL, this));
	mm_thresholdval.ShowScrollBar(SB_VERT);
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_spkWndDuration.SubclassDlgItem(IDC_SPIKEWINDOWLENGTH, this));
	VERIFY(mm_spkWndAmplitude.SubclassDlgItem(IDC_SPIKEWINDOWAMPLITUDE, this));
	VERIFY(mm_ichanselected.SubclassDlgItem(IDC_CHANSELECTED, this));
	VERIFY(mm_ichanselected2.SubclassDlgItem(IDC_CHANSELECTED2, this));
	
	// control derived from CScopeScreen
	VERIFY(m_spkShapeView.SubclassDlgItem(IDC_DISPLAYSPIKES, this));
	VERIFY(m_spkBarView.SubclassDlgItem(IDC_DISPLAYBARS, this));
	VERIFY(m_displayDetect.SubclassDlgItem(IDC_DISPLAYDETECT, this));
	VERIFY(m_displayData.SubclassDlgItem(IDC_DISPLAYDATA, this));

	// load left scrollbar and button
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY, this));
	m_scrolly.SetScrollRange(0, 100);
	m_hBias=AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom=AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS)->PostMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN)->PostMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hZoom);
	VERIFY(m_scrolly2.SubclassDlgItem(IDC_SCROLLY2, this));
	m_scrolly2.SetScrollRange(0, 100);
	m_hBias2=AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom2=AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS2)->PostMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hBias2);
	GetDlgItem(IDC_GAIN2)->PostMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hZoom2);

	// set bin_cr_flag_on_save
	((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(pApp->vdS.bincrflagonsave);

	// load data file parameters and build curves
	CDaoRecordView::OnInitialUpdate();
	
	// load file data
	if (m_displayDetect.GetNHZtags() < 1)
		m_displayDetect.AddHZtag(0, 0);

	UpdateFileParameters(TRUE);  
	// set display parameters according to :
	// tell mmdi parent which sourceView cursor is active
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
}

/////////////////////////////////////////////////////////////////////////////
// remove objects
void CSpikeDetectionView::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
}

// --------------------------------------------------------------------------
// OnSize
// --------------------------------------------------------------------------

void CSpikeDetectionView::OnSize(UINT nType, int cx, int cy)
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
// CSpikeDetectionView diagnostics

#ifdef _DEBUG
void CSpikeDetectionView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CSpikeDetectionView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CSpikeDetectionView::GetDocument()
{ 
	return (CdbWaveDoc*)m_pDocument; 
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView database support

CDaoRecordset* CSpikeDetectionView::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

////////////////////////////////////////////////////////////////////////////
// controls notifications

// --------------------------------------------------------------------------
// "Mymessage" handler - message sent by CScopeWnd derived controls
//	i.e. source data, spike bars, spike forms
// --------------------------------------------------------------------------

LRESULT CSpikeDetectionView::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int threshold	= LOWORD(lParam);	// value associated	
	int iID			= HIWORD(lParam);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		{
		if (threshold > CURSOR_VERTICAL)	// clip cursor shape to max 2
			threshold = 0;
		if (threshold == CURSOR_MEASURE)
			threshold = CURSOR_VERTICAL;
		m_cursorstate = threshold;	
		SetViewMouseCursor(threshold);	// change cursor val in the other button
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
		}
		break;

	// ----------------------------- move horizontal cursor / source data
	case HINT_MOVEHZTAG:
		m_pDetectParms->detectThreshold = m_displayDetect.GetHZtagVal(threshold);
		m_thresholdval = m_displayDetect.ConvertChanlistDataBinsToMilliVolts(0, m_displayDetect.GetHZtagVal(threshold));
		m_pDetectParms->detectThresholdmV = m_thresholdval;
		mm_thresholdval.m_bEntryDone = TRUE;			
		OnEnChangeThresholdval();			
		break;

	// ----------------------------- select bar/display bars or zoom		
	case HINT_CHANGEHZLIMITS:		// abcissa have changed
		m_displayDetect.GetDataFromDoc(m_spkBarView.GetTimeFirst(), m_spkBarView.GetTimeLast());
		m_displayData.GetDataFromDoc(m_spkBarView.GetTimeFirst(), m_spkBarView.GetTimeLast());
		UpdateLegends();
		break;

	case HINT_HITSPIKE:				// spike is selected or deselected
		SelectSpikeNo(threshold, FALSE);
		UpdateSpikeDisplay();
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0) 
			threshold = 0;
		m_spikeno=threshold;
		OnToolsEdittransformspikes();
		break;

	case HINT_CHANGEZOOM:
		UpdateSpkShapeWndScale(TRUE);
		m_spkShapeView.Invalidate();
		break;

	case HINT_VIEWSIZECHANGED:
		if (iID == m_displayData.GetDlgCtrlID())
		{
			long lFirst = m_displayData.GetDataFirst();		// get source data time range
			long lLast = m_displayData.GetDataLast();
			m_displayDetect.GetDataFromDoc(lFirst, lLast);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		UpdateLegends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		mdPM->viewspkdetectfiltered = m_displayDetect.m_parms;
		mdPM->viewspkdetectdata = m_displayData.m_parms;
		mdPM->viewspkdetectspk = m_spkBarView.m_parms;
		mdPM->viewspkdetectbars = m_spkShapeView.m_parms;
		break;

	case HINT_DEFINEDRECT:
		if (m_cursorstate == CURSOR_MEASURE)
		{
			CRect rect = m_displayDetect.GetDefinedRect();
			int lLimitLeft = m_displayDetect.GetDataOffsetfromPixel(rect.left);
			int lLimitRight = m_displayDetect.GetDataOffsetfromPixel(rect.right);
			if (lLimitLeft > lLimitRight)
			{
				int i = lLimitRight;
				lLimitRight = lLimitLeft;
				lLimitLeft = i;
			}
			m_pspkDocVSD->m_stimIntervals.intervalsArray.SetAtGrow(m_pspkDocVSD->m_stimIntervals.nitems, lLimitLeft);
			m_pspkDocVSD->m_stimIntervals.nitems++;
			m_pspkDocVSD->m_stimIntervals.intervalsArray.SetAtGrow(m_pspkDocVSD->m_stimIntervals.nitems, lLimitRight);
			m_pspkDocVSD->m_stimIntervals.nitems++;
			UpdateVTtags();

			m_spkBarView.Invalidate();
			m_displayDetect.Invalidate();
			m_displayData.Invalidate();
			m_pspkDocVSD->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_SELECTSPIKES:
		UpdateSpikeDisplay();
		break;

	//case HINT_HITVERTTAG:	 //11	// vertical tag hit				lowp = tag index	
	//case HINT_MOVEVERTTAG: //12	// vertical tag has moved 		lowp = new pixel / selected tag
	case HINT_CHANGEVERTTAG: //13
		{
		int lvalue = m_pspkDocVSD->m_stimIntervals.intervalsArray.GetAt(threshold);
		if(iID == m_displayDetect.GetDlgCtrlID())
			lvalue = m_displayDetect.GetVTtagLval(threshold);
		else if (iID == m_displayData.GetDlgCtrlID())
			lvalue = m_displayData.GetVTtagLval(threshold);

		m_pspkDocVSD->m_stimIntervals.intervalsArray.SetAt(threshold, lvalue);
		UpdateVTtags();

		m_spkBarView.Invalidate();
		m_displayDetect.Invalidate();
		m_displayData.Invalidate();
		m_pspkDocVSD->SetModifiedFlag(TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
	case HINT_LMOUSEBUTTONDOW_CTRL:
		{
		int cx = LOWORD(lParam);
		int cy = HIWORD(lParam);
		int lLimitLeft = m_displayDetect.GetDataOffsetfromPixel(cx);
		m_pspkDocVSD->m_stimIntervals.intervalsArray.SetAtGrow(m_pspkDocVSD->m_stimIntervals.nitems, lLimitLeft);
		m_pspkDocVSD->m_stimIntervals.nitems++;
		UpdateVTtags();

		m_spkBarView.Invalidate();
		m_displayDetect.Invalidate();
		m_displayData.Invalidate();
		m_pspkDocVSD->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_HITSPIKE_SHIFT:				// spike is selected or deselected
		{
			long lFirst;
			long lLast;
			m_pSpkListVSD->GetRangeOfSpikeFlagged(lFirst, lLast);
			long lTime = m_pSpkListVSD->GetSpikeTime(threshold);
			if (lTime < lFirst)
				lFirst = lTime;
			if (lTime > lLast)
				lLast = lTime;
			m_pSpkListVSD->FlagRangeOfSpikes(lFirst, lLast, TRUE);
			UpdateSpikeDisplay();
		}
		break;

	case HINT_HITSPIKE_CTRL:				// add/remove selected spike to/from the group of spikes selected
		SelectSpikeNo(threshold, TRUE);
		UpdateSpikeDisplay();
		break;

	case HINT_HITCHANNEL:					// change channel if different
		if(iID == m_displayDetect.GetDlgCtrlID())
		{
			if (m_ichanselected != threshold)
			{
				m_ichanselected = threshold;	// get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
			}
		}	
		else if (iID == m_displayData.GetDlgCtrlID())
		{
			if (m_ichanselected2 != threshold)
			{
				m_ichanselected2 = threshold;	// get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
			}
		}		
		break;

	default:
		break;		
	}
	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// OnFormat procedures

void CSpikeDetectionView::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void CSpikeDetectionView::OnLastFrame()
{
	OnFileScroll(SB_RIGHT, 1L);
}

void CSpikeDetectionView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// CDaoRecordView scroll bar: pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from CScrollBarEx
	long lFirst;
	long lLast;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL );
		lFirst = m_filescroll_infos.nPos;
		lLast = lFirst + m_filescroll_infos.nPage - 1;
		m_displayDetect.GetDataFromDoc(lFirst, lLast);
		m_displayData.GetDataFromDoc(lFirst, lLast);
		UpdateLegends();
		break;

	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL );
		lFirst = m_filescroll_infos.nPos;
		lLast = lFirst + m_filescroll_infos.nPage - 1;
		m_displayDetect.GetDataFromDoc(lFirst, lLast);
		m_displayData.GetDataFromDoc(lFirst, lLast);
		UpdateLegends();
		break;

	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void CSpikeDetectionView::UpdateFileScroll()
{
	m_filescroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_filescroll_infos.nPos = m_displayDetect.GetDataFirst();
	m_filescroll_infos.nPage = m_displayDetect.GetDataLast()-m_displayDetect.GetDataFirst()+1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

// --------------------------------------------------------------------------
// OnFileScroll()
// --------------------------------------------------------------------------

void CSpikeDetectionView::OnFileScroll(UINT nSBCode, UINT nPos)
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
		bResult = m_displayDetect.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		bResult = m_displayDetect.GetDataFromDoc((long)nPos*(m_displayDetect.GetDocumentLast())/(long)100);
		break;					
	default:				// NOP: set position only
		break;			
	}
	// adjust display
	if (bResult)
		UpdateLegends();
}

void CSpikeDetectionView::OnFormatYscaleCentercurve()
{
	m_displayDetect.CenterChan(0);
	m_displayDetect.Invalidate();

	for (int i=0; i < m_displayData.GetChanlistSize(); i++)
		m_displayData.CenterChan(i);
	m_displayData.Invalidate();
	
	m_spkBarView.CenterCurve();
	m_spkBarView.Invalidate();

	m_spkShapeView.SetYWExtOrg(m_spkBarView.GetYWExtent(), m_spkBarView.GetYWOrg());
	UpdateSpkShapeWndScale(TRUE);
	m_spkShapeView.Invalidate();
}

void CSpikeDetectionView::OnFormatYscaleGainadjust()
{
	m_displayDetect.MaxgainChan(0);
	m_displayDetect.SetChanlistVoltsExtent(-1, nullptr);
	m_displayDetect.Invalidate();

	for (int i=0; i < m_displayData.GetChanlistSize(); i++)
		m_displayData.MaxgainChan(i);
	m_displayData.SetChanlistVoltsExtent(-1, nullptr);
	m_displayData.Invalidate();

	m_spkBarView.MaxCenter();
	m_spkBarView.Invalidate();

	m_spkShapeView.SetYWExtOrg(m_spkBarView.GetYWExtent(), m_spkBarView.GetYWOrg());
	m_spkShapeView.SetXWExtOrg(m_pSpkListVSD->GetSpikeLength(), 0);
	UpdateSpkShapeWndScale(FALSE);
	m_spkShapeView.Invalidate();
}

// --------------------------------------------------------------------------
// OnSplitcurves()
// adjust position of each channel when new document is loaded for ex
// get nb of channels in the display list
// loop through all channels to get max min and center / adjust gain
// to display all signals on separate lines
// --------------------------------------------------------------------------

void CSpikeDetectionView::OnFormatSplitcurves()
{
	m_displayDetect.SplitChans();
	m_displayDetect.SetChanlistVoltsExtent(-1, nullptr);
	m_displayDetect.Invalidate();

	m_displayData.SplitChans();
	m_displayData.SetChanlistVoltsExtent(-1, nullptr);
	m_displayData.Invalidate();

	// center curve and display bar & spikes
	m_spkBarView.MaxCenter();
	m_spkBarView.Invalidate();

	m_spkShapeView.SetYWExtOrg(m_spkBarView.GetYWExtent(), m_spkBarView.GetYWOrg());
	UpdateSpkShapeWndScale(FALSE);	
	m_spkShapeView.Invalidate();
}

void CSpikeDetectionView::OnFormatAlldata()
{
	long lLast = GetDocument()->m_pDat->GetDOCchanLength(); 
	m_displayDetect.ResizeChannels(0, lLast);
	m_displayDetect.GetDataFromDoc(0, lLast);

	m_displayData.ResizeChannels(0, lLast);
	m_displayData.GetDataFromDoc(0, lLast);
	
	int xWE = m_pSpkListVSD->GetSpikeLength();
	if (xWE != m_spkShapeView.GetXWExtent() || 0 != m_spkShapeView.GetXWOrg())
		m_spkShapeView.SetXWExtOrg(xWE, 0);
	m_spkBarView.CenterCurve();	
	m_spkShapeView.SetYWExtOrg(m_spkBarView.GetYWExtent(), m_spkBarView.GetYWOrg());
	
	UpdateLegends();
}

// --------------------------------------------------------------------------
// OnToolsDetectionparameters()
// method 1
// --------------------------------------------------------------------------
void CSpikeDetectionView::UpdateDetectionParameters()
{
	// refresh pointer to spike detection array
	int narrays = m_parmsCurrent.GetSize();
	ASSERT(m_iDetectParms < narrays);
	m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	UpdateDetectionSettings(m_iDetectParms);

	// make sure that tabs are identical to what has been changed
	UpdateTabs();
	UpdateCB();

	// update static controls
	GetDlgItem(IDC_STATIC10)->Invalidate();
	GetDlgItem(IDC_STATIC1)->Invalidate();
	GetDlgItem(IDC_STATIC2)->Invalidate();
	GetDlgItem(IDC_STATIC6)->Invalidate();
	GetDlgItem(IDC_LOCATEBTTN)->Invalidate();

	// update CEditControls
	GetDlgItem(IDC_COMBO1)->Invalidate();
	GetDlgItem(IDC_SOURCECHAN)->Invalidate();
	GetDlgItem(IDC_TRANSFORM)->Invalidate();
	mm_thresholdval.Invalidate();
}

void CSpikeDetectionView::OnToolsDetectionparameters()
{   
	CSpikeDetectDlg dlg;
	dlg.m_dbDoc = GetDocument()->m_pDat;
	dlg.m_iDetectParmsDlg = m_iDetectParms;			// index spk detect parm currently selected / array
	dlg.m_pDetectSettingsArray = &m_parmsCurrent;	// spike detection parameters array
	dlg.mdPM = mdPM;
	dlg.m_pdisplayDetect = &m_displayDetect;
	if (IDOK == dlg.DoModal())
	{
		// copy modified parameters into array
		// update HorizontalCursorList on both sourceView & histogram
		m_iDetectParms = dlg.m_iDetectParmsDlg;
		UpdateDetectionParameters();
	}
}

void CSpikeDetectionView::OnSelchangeDetectchan()
{
	UpdateData(TRUE);
	m_pDetectParms->detectChan = m_CBdetectChan.GetCurSel(); // get new channel
	m_pDetectParms->bChanged = TRUE;			// and flag it
	m_displayDetect.SetChanlistOrdinates(0, m_pDetectParms->detectChan, m_pDetectParms->detectTransform);
	m_pDetectParms->detectThreshold = m_displayDetect.ConvertChanlistVoltstoDataBins(0, m_pDetectParms->detectThresholdmV / 1000.f);
	m_displayDetect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);

	m_displayDetect.GetDataFromDoc(); 				// load data
	m_displayDetect.AutoZoomChan(0);				// vertical position of channel
	m_displayDetect.Invalidate();
}

void CSpikeDetectionView::OnSelchangeTransform()
{
	UpdateData(TRUE);
	m_pDetectParms->detectTransform = m_CBtransform.GetCurSel();// get new method	
	m_pDetectParms->bChanged = TRUE;			// save new method and flag it
	m_displayDetect.SetChanlistTransformMode(0, m_pDetectParms->detectTransform);
	m_displayDetect.GetDataFromDoc(); 				// load data
	m_displayDetect.AutoZoomChan(0);				// vertical position of channel
	m_displayDetect.Invalidate();
}

void CSpikeDetectionView::OnMeasureAll()
{
	DetectAll(TRUE);
}

void CSpikeDetectionView::OnMeasure()
{
	DetectAll(FALSE);
}

// detect from current set of parms or from all
void CSpikeDetectionView::DetectAll(BOOL bAll)
{
	// init spike document (erase spike list data & intervals)
	m_bDetected=TRUE;									// set flag: detection = YES
	m_pspkDocVSD->SetModifiedFlag(TRUE);					// set flag: document has changed
	m_pspkDocVSD->SetDetectionDate(CTime::GetCurrentTime());		// detection date
	long loldDataFirst = m_displayDetect.GetDataFirst();	// index first pt to test
	long loldDataLast = m_displayDetect.GetDataLast();		// index last pt to test
	int ioldlist = GetDocument()->GetcurrentSpkListIndex();
	m_spikeno = -1;										// Nov 5, 2005
	
	// check if detection parameters are ok? prevent detection from a channel that does not exist
	CAcqDataDoc* pDat = GetDocument()->m_pDat;
	if (pDat == nullptr)
		return;
	CWaveFormat* pwF= pDat->GetpWaveFormat();
	int chanmax = pwF->scan_count -1;
	for (int i=0; i < m_parmsCurrent.GetSize(); i++)
	{
		SPKDETECTPARM* pspkDP = m_parmsCurrent.GetItem(i);
		if (pspkDP->extractChan > chanmax)
		{
			MessageBox(_T("Check spike detection parameters - one of the detection channel requested is not available in the source data"));
			return;
		}
	}

	// adjust size of spklist array
	if (m_parmsCurrent.GetSize() != m_pspkDocVSD->GetSpkListSize())
		m_pspkDocVSD->SetSpkListSize(m_parmsCurrent.GetSize());

	// detect spikes from all chans marked as such	
	for (int i=0; i < m_parmsCurrent.GetSize(); i++)
	{
		if (!bAll && m_iDetectParms != i)
			continue;
		// detect missing data channel
		if ((m_parmsCurrent.GetItem(i))->extractChan > chanmax)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		CSpikeList* pspklist = m_pspkDocVSD->SetSpkListCurrent(i);
		if (pspklist == nullptr)
		{
			m_pspkDocVSD->AddSpkList();
			pspklist = m_pspkDocVSD->GetSpkListCurrent();
		}

		m_pSpkListVSD = pspklist;
		if (m_pSpkListVSD->GetTotalSpikes() == 0)
		{
			SPKDETECTPARM* pFC = m_parmsCurrent.GetItem(i);
			ASSERT_VALID(pFC);
			ASSERT(pFC != NULL);
			m_pSpkListVSD->InitSpikeList(GetDocument()->m_pDat, pFC);
		}
		if ((m_parmsCurrent.GetItem(i))->detectWhat == 0)
		{
			DetectMethod1(i);							// detect spikes
		}
		else
		{
			DetectStim1(i);								// detect stimulus
			UpdateVTtags();								// display vertical bars
			m_spkBarView.Invalidate();
			m_displayDetect.Invalidate();
			m_displayData.Invalidate();
		}
	}

	// save spike file
	SaveCurrentFileParms();

	// display data	
	m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(ioldlist);
	m_spkBarView.SetSourceData(m_pSpkListVSD, m_pspkDocVSD);	// attach spike buffer
	m_spkShapeView.SetSourceData(m_pSpkListVSD);			// attach spike buffer

	// center spikes, change nb spikes and update content of draw buttons
	if (mdPM->bMaximizeGain
		|| m_spkBarView.GetYWExtent() == 0
		|| m_spkBarView.GetYWOrg() == 0
		|| m_spkShapeView.GetYWExtent() == 0
		|| m_spkShapeView.GetYWOrg() == 0)
	{
		m_spkBarView.MaxCenter();
		m_spkShapeView.SetYWExtOrg(m_spkBarView.GetYWExtent(), m_spkBarView.GetYWOrg());
		m_spkShapeView.SetXWExtOrg(m_pSpkListVSD->GetSpikeLength(), 0);
		UpdateSpkShapeWndScale(FALSE);
	}

	HighlightSpikes(TRUE);								// highlight spikes	
	UpdateLegends();
	UpdateTabs();
}

// --------------------------------------------------------------------------
// DetectStim1()
// detect stimulus; returns the nb of events detected (ON/OFF)
// --------------------------------------------------------------------------

int CSpikeDetectionView::DetectStim1(int ichan)
{
	// set parameters (copy array into local parms)	
	SPKDETECTPARM* pstiD = m_parmsCurrent.GetItem(ichan);
	int threshold	= pstiD->detectThreshold;		// threshold value
	int method		= pstiD->detectTransform;		// how source data are transformed
	int sourcechan	= pstiD->detectChan;			// source channel

	// get parameters from document
	CAcqDataDoc* pDat = GetDocument()->m_pDat;
	int nchans;										// number of data chans / source buffer
	short* pBuf = pDat->LoadRawDataParams(&nchans);	// get address of reading data buffer
	int nspan = pDat->GetTransfDataSpan(method);	// nb pts to read before transf

	// set detection mode
	// 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	BOOL bCrossUpw = TRUE;
	if (pstiD->detectMode == 1 || pstiD->detectMode == 3)
		bCrossUpw = FALSE;
	BOOL bMode = TRUE;
	if (pstiD->detectMode == 2 || pstiD->detectMode == 3)
		bMode = FALSE;

	BOOL bSaveON = FALSE;

	// get data detection limits and clip limits according to size of spikes
	long lDataFirst = m_displayDetect.GetDataFirst();	// index first pt to test
	long lDataLast = m_displayDetect.GetDataLast();		// index last pt to test

	// plot progress dialog box
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetRange(0, 100);
	long lDataLen = lDataLast - lDataFirst;
	long lDataFirst0 = lDataFirst;
	dlg.SetWindowText(_T("Detect trigger events..."));

	// loop through data defined in the lineview window    
	while (lDataFirst < lDataLast)
	{
		long lRWFirst=lDataFirst;					// index very first pt within buffers
		long lRWLast =lDataLast;					// index very last pt within buffers
		if (!pDat->LoadRawData(&lRWFirst, &lRWLast, nspan))
			break;									// exit if error reported
		if (!pDat->BuildTransfData(method, sourcechan))
			break;

		// compute initial offset (address of first point)
		long lLast = lRWLast;
		if (lLast > lDataLast)
			lLast = lDataLast;
		int iBUFFirst = lDataFirst - pDat->GetDOCchanIndexFirst();
		short* pDataFirst = pDat->GetpTransfDataElmt(iBUFFirst);

		// DETECT STIM ---------------------------------------------------------------
		// detect event if value above threshold
		for (long cx = lDataFirst; cx <= lLast; cx++)
		{
			// set message for dialog box
			// check if user wants to stop
			
	
			short* pData = pDataFirst + (cx-lDataFirst);
			int val = *pData;
			if (bCrossUpw)
			{
				if (val <= threshold)			// find first point above threshold
					continue;						// no: loop to next point

				// add element
				bCrossUpw = FALSE;
				bSaveON = !bSaveON;
				if (!bMode && !bSaveON)
					continue;
			}
			else
			{
				if (val >= threshold)			// test if a spike is present
					continue;					// no: loop to next point

				// add element
				bCrossUpw = TRUE;
				bSaveON = !bSaveON;
				if (!bMode && !bSaveON)
					continue;
			}
			// check if user does not want to continue
			int cxpos = (cx - lDataFirst0)*100 / lDataLen;
			dlg.SetPos(cxpos);
			CString cscomment;
			cscomment.Format(_T("Processing stimulus event: %i"), m_pspkDocVSD->m_stimIntervals.nitems +1 );
			dlg.SetStatus(cscomment);

			if(dlg.CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				{
					// set condition to stop detection
					lLast = lDataLast;
					// clear stimulus detected
					CIntervalsAndLevels* pSti = &(m_pspkDocVSD->m_stimIntervals);
					pSti->intervalsArray.RemoveAll();
					m_pspkDocVSD->m_stimIntervals.nitems=0;
					break;
				}

			// check if already present and insert it at the proper place
			CIntervalsAndLevels* pSti = &(m_pspkDocVSD->m_stimIntervals);
			int jitter = 2;		// allow some jitter in the detection (+-2)
			BOOL flag = TRUE;
			int i=0;
			for (i=0; i< pSti->intervalsArray.GetSize(); i++)
			{
				long lval = pSti->intervalsArray.GetAt(i);
				if (cx <= (lval +jitter) && cx >= (lval-jitter))
				{
					flag = FALSE;		// no new stim - already detected at that time
					break;
				}
				if (cx < lval-jitter)
				{
					flag=TRUE;			// new stim to insert before i
					break;
				}
					
			}
			if (flag)
			{
				pSti->intervalsArray.InsertAt(i, cx);
				m_pspkDocVSD->m_stimIntervals.nitems++;
			}
		}
		///////////////////////////////////////////////////////////////
		lDataFirst = lLast+1;					// update for next loop		
	}

	return m_pspkDocVSD->m_stimIntervals.nitems;
}

// --------------------------------------------------------------------------
// DetectMethod1()
// detect spikes using 1 threshold and add spikes to the buffer
// scan doc data from lfirst to llast
// returns the nb of spikes detected
// --------------------------------------------------------------------------

int CSpikeDetectionView::DetectMethod1(WORD schan)
{
	SPKDETECTPARM* pspkDP = m_parmsCurrent.GetItem(schan);
	if (pspkDP->extractTransform != pspkDP->detectTransform &&
		pspkDP->extractTransform != 0)
	{
		AfxMessageBox(_T("Options not implemented yet!\nd chan == extr chan or !extr chan=0\nChange detection parameters"));
		return  m_pSpkListVSD->GetTotalSpikes();
	}

	// set parameters (copy array into local parms)	
	short threshold	= pspkDP->detectThreshold;				// threshold value
	int method		= pspkDP->detectTransform;				// how source data are transformed
	int sourcechan	= pspkDP->detectChan;					// source channel
	int prethreshold= pspkDP->prethreshold;					// pts before threshold
	int refractory	= pspkDP->refractory;					// refractory period
	int postthreshold = pspkDP->extractNpoints - prethreshold;

	// get parameters from document
	CAcqDataDoc* pDat = GetDocument()->m_pDat;
	int nchans;												// number of data chans / source buffer
	short* pBuf = pDat->LoadRawDataParams(&nchans);
	int nspan = pDat->GetTransfDataSpan(method);			// nb pts to read before transf
	int nspikes = 0;										// no spikes detected yet

	// adjust detection method: if threshold lower than data zero detect lower crossing
	BOOL bCrossUpw = TRUE;
	if (threshold < 0)
		bCrossUpw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	long lDataFirst = m_displayDetect.GetDataFirst();			// index first pt to test
	long lDataLast = m_displayDetect.GetDataLast();			// index last pt to test
	if (lDataFirst < prethreshold + nspan )
		lDataFirst = (long) prethreshold + nspan;
	if (lDataLast > pDat->GetDOCchanLength()-postthreshold-nspan )
		lDataLast = pDat->GetDOCchanLength()-postthreshold-nspan ;
				
	// loop through data defined in the lineview window    
	while (lDataFirst < lDataLast)
	{
		long lRWFirst= lDataFirst - prethreshold;			// index very first pt within buffers
		long lRWLast = lDataLast;							// index very last pt within buffers
		if (!pDat->LoadRawData(&lRWFirst, &lRWLast, nspan))	// load data from file
			break;											// exit if error reported
		if (!pDat->BuildTransfData(method, sourcechan))		// transfer data into a buffer with a single channel
			break;											// exit if fail

		// load a chunk of data and see if any spikes are detected within it
		// compute initial offset (address of first point
		long lLast = lRWLast - postthreshold;
		if (lLast > lDataLast)
			lLast = lDataLast;
		int iBUFFirst = lDataFirst-pDat->GetDOCchanIndexFirst();
		short* pDataFirst = pDat->GetpTransfDataElmt(iBUFFirst);		

		// DETECT SPIKES ---------------------------------------------------------------
		// detect event if value above threshold
		long iitime = 0;
		long cx=0;
		for (cx = lDataFirst; cx <= lLast; cx++)
		{
			// ........................................ SPIKE NOT DETECTED
			short* pData = pDataFirst + (cx-lDataFirst);

			// detect > threshold ......... if found, search for max
			if (bCrossUpw)
			{
				if (*pData < threshold)		// test if a spike is present
					continue;				// no: loop to next point

				// search max and threshold crossing
				short max = *pData;			// init max
				short* pData1=pData;		// init pointer
				iitime = cx;				// init spike time
				// loop to search max
				for (long i=cx; i< cx+refractory; i++, pData1++)
				{
					if (max < *pData1)		// search max
					{
						max = *pData1;
						pData = pData1;		// pData = "center" of spike
						iitime = i;			// iitime = time of spike
					}
				}
			}
			// detect < threshold ......... if found, search for min
			else
			{
				if (*pData > threshold)		// test if a spike is present
					continue;				// no: loop to next point

				// search min and threshold crossing
				short min = *pData;
				short* pData1=pData;
				iitime = cx;
				for (int i=cx; i< cx+refractory; i++, pData1 ++)
				{
					if (min > *pData1)
					{
						min = *pData1;
						pData = pData1;			// pData = "center" of spike
						iitime = i;
					}
				}
			}

			// ........................................ SPIKE DETECTED
			if (pspkDP->extractTransform == pspkDP->detectTransform)
			{
				short* pM = pData -prethreshold;
				int ispk = m_pSpkListVSD->AddSpike(pM, 1, iitime, sourcechan, 0, TRUE); 
			}
			else  // extract from raw data
			{
				short* pM = pBuf 
						+ nchans*(iitime-prethreshold -lRWFirst +nspan)
						+ pspkDP->extractChan;
				m_pSpkListVSD->AddSpike(pM, nchans, iitime, sourcechan, 0, TRUE);
			}

			// update loop parameters
			cx = iitime + refractory;
		}

		///////////////////////////////////////////////////////////////
		lDataFirst = cx+1;		// update for next loop
	}

	return m_pSpkListVSD->GetTotalSpikes();
}

void CSpikeDetectionView::OnToolsEdittransformspikes()
{
	CSpikeEditDlg dlg;							// dialog box
	dlg.m_yextent = m_spkShapeView.GetYWExtent();// load display parameters
	dlg.m_yzero = m_spkShapeView.GetYWOrg();	// ordinates
	dlg.m_xextent = m_spkShapeView.GetXWExtent();// and
	dlg.m_xzero = m_spkShapeView.GetXWOrg();	// abcissa
	dlg.m_spikeno = m_spikeno;					// load index of selected spike
	m_pSpkListVSD->RemoveAllSpikeFlags();	
	dlg.m_dbDoc = GetDocument()->m_pDat;			// pass document dat pointer
	dlg.m_pSpkList = m_pSpkListVSD;				// pass spike list
	dlg.m_parent = this;
	
	// open dialog box and wait for response
	dlg.DoModal();
	if (m_spikeno != dlg.m_spikeno)
	{
		m_spikeno = dlg.m_spikeno;
		SelectSpikeNo(m_spikeno, FALSE);
		UpdateSpikeDisplay();
	}

	if (dlg.m_bchanged)
		m_pspkDocVSD->SetModifiedFlag(TRUE);

	UpdateLegends();
}

void CSpikeDetectionView::OnFormatXscale()
{
	XYParametersDlg dlg;
	CWnd* pFocus = GetFocus();
	
	if (pFocus != nullptr && m_displayDetect.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_displayDetect.GetChanlistYzero(m_pDetectParms->detectChan);
		dlg.m_yextent = m_displayDetect.GetChanlistYextent(m_pDetectParms->detectChan);
		dlg.m_bDisplaysource = TRUE;
	}
	else if (pFocus != nullptr && m_spkBarView.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_spkBarView.GetYWOrg();
		dlg.m_yextent = m_spkBarView.GetYWExtent();
		dlg.m_bDisplaybars = TRUE;
	}
	else
		{
		dlg.m_xzero = m_spkShapeView.GetXWOrg();
		dlg.m_xextent = m_spkShapeView.GetXWExtent();
		dlg.m_yzero = m_spkShapeView.GetYWOrg();
		dlg.m_yextent = m_spkShapeView.GetYWExtent();
		dlg.m_bDisplayspikes = TRUE;
	}
	
	if (IDOK == dlg.DoModal())	
	{
		if (dlg.m_bDisplaysource)
		{
			m_displayDetect.SetChanlistYzero(0, dlg.m_yzero);
			m_displayDetect.SetChanlistYextent(0, dlg.m_yextent);
			m_displayDetect.Invalidate();
		}
		if (dlg.m_bDisplaybars)
		{
			m_spkBarView.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_spkBarView.Invalidate();
		}
		if (dlg.m_bDisplayspikes)
		{
			m_spkShapeView.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_spkShapeView.SetXWExtOrg(dlg.m_xextent, dlg.m_xzero);
			m_spkShapeView.Invalidate();
		}
	}
}

void CSpikeDetectionView::OnBnClickedClearall()
{
	m_spikeno = -1;						// unselect spike
	m_spkBarView.SelectSpike(-1);		// deselect spike bars
	m_spkShapeView.SelectSpikeShape(-1);// deselect superimposed spikes

	// update spike list
	for (int i=0; i < m_pspkDocVSD->GetSpkListSize(); i++)
	{
		CSpikeList* pspklist = m_pspkDocVSD->SetSpkListCurrent(i);
		pspklist->InitSpikeList(GetDocument()->m_pDat, nullptr);
	}
	m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(GetDocument()->GetcurrentSpkListIndex());
	ASSERT(m_pSpkListVSD != NULL);

	HighlightSpikes(FALSE);				// remove display of spikes
	m_spkShapeView.SetSourceData(m_pSpkListVSD);
	m_pspkDocVSD->m_stimIntervals.nitems=0;		// zero stimuli
	m_pspkDocVSD->m_stimIntervals.intervalsArray.RemoveAll();

	UpdateDetectionParameters();
	UpdateVTtags();						// update display of vertical tags
	UpdateLegends();					// change legends
	m_pspkDocVSD->SetModifiedFlag(TRUE);	// mark spike document as changed
}

void CSpikeDetectionView::OnClear()
{
	m_spikeno = -1;						// unselect spike
	m_spkBarView.SelectSpike(-1);		// deselect spike bars
	m_spkShapeView.SelectSpikeShape(-1);// deselect superimposed spikes

	m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(GetDocument()->GetcurrentSpkListIndex());
	m_pSpkListVSD->InitSpikeList(GetDocument()->m_pDat, nullptr);
	HighlightSpikes(FALSE);				// remove display of spikes

	if(m_pSpkListVSD->GetdetectWhat() == 1)
	{
		m_pspkDocVSD->m_stimIntervals.nitems=0;		// zero stimuli
		m_pspkDocVSD->m_stimIntervals.intervalsArray.RemoveAll();
		UpdateVTtags();					// update display of vertical tags
	}

	UpdateLegends();					// change legends
	m_pspkDocVSD->SetModifiedFlag(TRUE);	// mark spike document as changed
}

void CSpikeDetectionView::OnEnChangeSpikeno()
{
	if (!mm_spikeno.m_bEntryDone)
		return;

	int spikeno = m_spikeno;
	switch (mm_spikeno.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE);
		spikeno = m_spikeno;
		break;
	case VK_UP:
	case VK_PRIOR:	
		spikeno++;	
		break;
	case VK_DOWN:
	case VK_NEXT:   
		spikeno--;	
		break;
	}
	// check boundaries
	if (spikeno <-1)
		spikeno = -1;
	if (spikeno >= m_pSpkListVSD->GetTotalSpikes())
		spikeno = m_pSpkListVSD->GetTotalSpikes()-1;
	
	// change display if necessary	
	mm_spikeno.m_bEntryDone=FALSE;	// clear flag
	mm_spikeno.m_nChar=0;			// empty buffer
	mm_spikeno.SetSel(0, -1);		// select all text

	// update spike num and center display on the selected spike
	SelectSpikeNo(spikeno, FALSE);
	UpdateSpikeDisplay();
	return;	
}

//-----------------------------------------------------------------------
// if artefact flag is set, modify class nb to negative value
// if artefact flag removed, modify class to positive value
// PROBL: conserve old class nb; 
// SOLUTION used here: mirror to negative value with an
// offset of -1: 0/-1; 1/-2; 2/-3
//-----------------------------------------------------------------------

void CSpikeDetectionView::OnArtefact()
{
	UpdateData(TRUE);						// load value from control
	int nspikes = m_pSpkListVSD->GetSpikeFlagArrayCount();
	if (nspikes < 1)
	{
		m_bartefact = FALSE;				// no action if spike index < 0
	}
	else
	{										// load old class nb
		ASSERT(nspikes >= 0);
		for (int i= 0; i<nspikes; i++)
		{
			int spikeno = m_pSpkListVSD->GetSpikeFlagArrayAt(i);
			int spkclass = m_pSpkListVSD->GetSpikeClass(spikeno);
			// if artefact: set class to negative value		
			if (m_bartefact && spkclass >= 0)
				spkclass = -(spkclass+1);

			// if not artefact: if spike has negative class, set to positive value
			else if (spkclass < 0)
				spkclass = -(spkclass+1);
			m_pSpkListVSD->SetSpikeClass(spikeno, spkclass);
		}
		
		m_pspkDocVSD->SetModifiedFlag(TRUE);		// set flag to save spike file
		SaveCurrentFileParms();						// save data (erase artefacts)
	}
	m_spikeno=-1;
	SelectSpikeNo(m_spikeno,FALSE);
	UpdateSpikeDisplay();

	// update nb spikes
	nspikes = m_pSpkListVSD->GetTotalSpikes() ; 
	if (nspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, nspikes);
}

//-----------------------------------------------------------------------
// adjust display horizontally to display/center spike "spikeno"

void CSpikeDetectionView::AlignDisplayToCurrentSpike()
{
	if (m_spikeno < 0)
		return;
	
	long lSpikeTime = m_pSpkListVSD->GetSpikeTime(m_spikeno);
	long lFirst = m_displayDetect.GetDataFirst();		// get source data time range
	long lLast = m_displayDetect.GetDataLast();

	if (lSpikeTime < m_displayDetect.GetDataFirst() 
		|| lSpikeTime > m_displayDetect.GetDataLast())
	{		
		long lSize = m_displayDetect.GetDataLast() - m_displayDetect.GetDataFirst();
		long lFirst = lSpikeTime - lSize/2;
		if (lFirst < 0)
			lFirst = 0;
		long lLast  = lFirst + lSize-1;
		if (lLast > m_displayDetect.GetDocumentLast())
		{
			lLast = m_displayDetect.GetDocumentLast();
			lFirst = lLast - lSize + 1;
		}
		m_displayDetect.GetDataFromDoc(lFirst, lLast);
		m_displayData.GetDataFromDoc(lFirst, lLast);
		UpdateLegends();
	}	
}

void CSpikeDetectionView::UpdateSpkShapeWndScale(BOOL bSetFromControls)
{
	// get current values	
	int ixWE=0;
	int iyWE=0;
	float x, y;

	// if set from controls, get value from the controls 
	if (bSetFromControls && m_pSpkListVSD->GetTotalSpikes() > 0)
	{
		// set time scale
		CString cs;
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->GetWindowText(cs);
		 x = (float) _ttof (cs) / 1000.0f;
		ixWE = (int) (m_pSpkListVSD->GetAcqSampRate() * x);
		if (ixWE == 0)
			ixWE = m_pSpkListVSD->GetextractNpoints() ;
		ASSERT(ixWE != 0);
		m_spkShapeView.SetXWExtOrg(ixWE, m_spkShapeView.GetXWOrg());

		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			y = (float) _ttof (cs) / 1000.0f;
			iyWE = (int) (y / m_pSpkListVSD->GetAcqVoltsperBin());
		}
		if (iyWE == 0)
			iyWE = m_spkShapeView.GetYWExtent();

		m_spkShapeView.SetYWExtOrg(iyWE, m_spkShapeView.GetYWOrg());
	}
	else
	{
		ixWE = m_spkShapeView.GetXWExtent();
		iyWE = m_spkShapeView.GetYWExtent();
	}

	if (ixWE!= NULL && iyWE != NULL)
	{
		float yunit= m_spkShapeView.GetExtent_mV() / m_spkShapeView.GetNyScaleCells();
		m_spkShapeView.SetyScaleUnitValue(yunit);
		float xunit=m_spkShapeView.GetExtent_ms() / m_spkShapeView.GetNxScaleCells();
		m_spkShapeView.SetxScaleUnitValue(xunit);
	}
	
	// output values
	_stprintf_s(m_szbuf, 64, _T("%.3lf"), m_spkShapeView.GetExtent_mV());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, m_szbuf);
	float yunit= m_spkShapeView.GetExtent_mV() / m_spkShapeView.GetNyScaleCells();
	m_spkShapeView.SetyScaleUnitValue(yunit);

	_stprintf_s(m_szbuf, 64, _T("%.3lf"), m_spkShapeView.GetExtent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, m_szbuf);
	float xunit=m_spkShapeView.GetExtent_ms() / m_spkShapeView.GetNxScaleCells();
		m_spkShapeView.SetxScaleUnitValue(xunit);

}

void CSpikeDetectionView::SelectSpikeNo(int spikeno, BOOL bMultipleSelection)
{
	// load spike parameters	
	if (spikeno >= 0)
	{	// get address of spike parms
		CSpikeElemt* pS = m_pSpkListVSD->GetSpikeElemt(spikeno);		
		m_bartefact = (pS->GetSpikeClass() <0);
		if (bMultipleSelection)	// TRUE= multiple selection
		{
			int nflaggedspikes = m_pSpkListVSD->ToggleSpikeFlag(spikeno);
			if (m_pSpkListVSD->GetSpikeFlagArrayCount() < 1)
				spikeno=-1;
			if (m_spikeno == spikeno)
				spikeno = 0;
		}
		else
		{
			m_pSpkListVSD->SetSingleSpikeFlag(spikeno);
		}
		m_spikeno = spikeno;
		AlignDisplayToCurrentSpike();
	}
	else
	{
		m_pSpkListVSD->RemoveAllSpikeFlags();
		m_bartefact = FALSE;
	}
}

void CSpikeDetectionView::UpdateSpikeDisplay()
{
	// update spike display windows
	m_spkBarView.Invalidate(TRUE);
	m_spkShapeView.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(!(m_pSpkListVSD->GetSpikeFlagArrayCount() >1));
	SetDlgItemInt(IDC_SPIKENO, m_spikeno, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
}

// this routine is called when the threshold is changed by the user (move cursor, change edit
// content

void CSpikeDetectionView::OnEnChangeThresholdval()
{
	if (!mm_thresholdval.m_bEntryDone)
		return;

	float thresholdval = m_thresholdval;
	switch (mm_thresholdval.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE);
		thresholdval = m_thresholdval;
		break;
	case VK_UP:
	case VK_PRIOR:
		thresholdval++;	
		break;
	case VK_DOWN:
	case VK_NEXT:   
		thresholdval--;	
		break;
	}

	// change display if necessary
	if (m_thresholdval != thresholdval)
	{
		m_thresholdval = thresholdval;
		m_pDetectParms->detectThresholdmV = thresholdval;
		m_pDetectParms->detectThreshold = m_displayDetect.ConvertChanlistVoltstoDataBins(0, m_thresholdval / 1000.f);
		m_displayDetect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);
	}
	
	mm_thresholdval.m_bEntryDone=FALSE;
	mm_thresholdval.m_nChar=0;
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	// change detection threshold method to manual
	mm_thresholdval.SetSel(0, -1); 		// select all text	
	return;	
}

void CSpikeDetectionView::OnEnChangeTimefirst() 
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
	m_displayDetect.GetDataFromDoc((long) (m_timefirst*m_samplingRate), (long) (m_timelast*m_samplingRate));
	m_displayData.GetDataFromDoc((long) (m_timefirst*m_samplingRate), (long) (m_timelast*m_samplingRate));
	UpdateLegends();

	mm_timefirst.m_bEntryDone=FALSE;
	mm_timefirst.m_nChar=0;
	mm_timefirst.SetSel(0, -1); 	//select all text
}

void CSpikeDetectionView::OnEnChangeTimelast() 
{
	if (!mm_timelast.m_bEntryDone)
		return;

	switch (mm_timelast.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
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
	m_displayDetect.GetDataFromDoc((long) (m_timefirst*m_samplingRate), (long) (m_timelast*m_samplingRate));
	m_displayData.GetDataFromDoc((long) (m_timefirst*m_samplingRate), (long) (m_timelast*m_samplingRate));
	UpdateLegends();

	mm_timelast.m_bEntryDone=FALSE;
	mm_timelast.m_nChar=0;
	mm_timelast.SetSel(0, -1); 	//select all text
}

void CSpikeDetectionView::OnToolsDataseries() 
{
	// init dialog data 
	CDataSeriesDlg dlg;
	dlg.m_lineview = &m_displayDetect;
	dlg.m_pdbDoc = GetDocument()->m_pDat;
	dlg.m_listindex = 0;

	// invoke dialog box
	dlg.DoModal();
	if (m_displayDetect.GetChanlistSize() <1)
	{
		m_displayDetect.RemoveAllChanlistItems();
		m_displayDetect.AddChanlistItem(m_pDetectParms->detectChan, m_pDetectParms->detectTransform); 
	}
	UpdateLegends();
}

void CSpikeDetectionView::PrintDataCartridge (CDC* pDC, CLineViewWnd* plineViewWnd, CRect* prect, BOOL bComments, BOOL bBars)
{
		BOOL bDrawF = plineViewWnd->m_parms.bDrawframe;
		plineViewWnd->m_parms.bDrawframe = TRUE;
		plineViewWnd->Print(pDC, prect, (mdPM->bcontours == 1));
		plineViewWnd->m_parms.bDrawframe = bDrawF;

		// data vertical and horizontal bars
	
		CString comments = PrintDataBars(pDC, plineViewWnd, prect);

		int xcol = prect->left;
		int ypxrow = prect->top;
		int lineheight = m_logFont.lfHeight + 5;
		pDC->TextOut(xcol, ypxrow, comments);
		ypxrow += lineheight;

		UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		UINT nFormat = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		CRect rectComment = *prect;
		rectComment.top = ypxrow;
		rectComment.right = prect->left;
}

void CSpikeDetectionView::OnEditCopy() 
{
	CCopyAsDlg dlg;
	dlg.m_nabcissa = mdPM->hzResolution;
	dlg.m_nordinates = mdPM->vtResolution;
	dlg.m_bgraphics = mdPM->bgraphics;
	dlg.m_ioption = mdPM->bcontours;
	dlg.m_iunit = mdPM->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		mdPM->bgraphics= dlg.m_bgraphics;
		mdPM->bcontours= dlg.m_ioption;
		mdPM->bunits= dlg.m_iunit;
		mdPM->hzResolution=dlg.m_nabcissa;
		mdPM->vtResolution=dlg.m_nordinates;

		if (!dlg.m_bgraphics)
			m_displayDetect.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			SerializeWindowsState(BSAVE);
			////////////////////////////////////////////////////////////////////
			CRect oldRect1;								// save size of lineview windows
			m_displayDetect.GetWindowRect(&oldRect1);
			CRect oldRect2;
			m_displayData.GetWindowRect(&oldRect2);

			CRect rect(0,0, mdPM->hzResolution, mdPM->vtResolution);
			m_npixels0 = m_displayDetect.Width();

			// create metafile
			CMetaFileDC mDC;
			CRect rectBound = rect;
			rectBound.right *= 32;
			rectBound.bottom *= 30;
			CDC* pDCRef=GetDC();
			CString csTitle = _T("dbWave\0") + GetDocument()->m_pDat->GetTitle();
			csTitle += _T("\0\0");
			BOOL hmDC = mDC.CreateEnhanced(pDCRef, nullptr, &rectBound, csTitle);
			ASSERT (hmDC != NULL);

			// Draw document in metafile.
			CPen blackPen(PS_SOLID, 0, RGB(  0,   0,   0));
			CPen* poldpen= mDC.SelectObject(&blackPen);
			CBrush* pOldBrush= (CBrush*) mDC.SelectStockObject(BLACK_BRUSH);
			CClientDC attribDC(this) ;					// Create and attach attribute DC
			mDC.SetAttribDC(attribDC.GetSafeHdc()) ;	// from current screen

			// print comments : set font
			m_pOldFont = nullptr;
			int oldsize = mdPM->fontsize;
			mdPM->fontsize = 10;
			PrintCreateFont();
			mDC.SetBkMode (TRANSPARENT);
			mdPM->fontsize = oldsize;
			m_pOldFont = mDC.SelectObject(&m_fontPrint);
			int lineheight = m_logFont.lfHeight + 5;
			int ypxrow=0;
			int xcol = 10;
			CString comments;

			// comment and descriptors
			comments = GetDocument()->ExportDatabaseData(1);
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;
			
			// abcissa
			comments = _T("Abcissa: ");
			CString content;
			GetDlgItem(IDC_TIMEFIRST)->GetWindowText(content);
			comments += content;
			comments += _T(" - ");
			GetDlgItem(IDC_TIMELAST)->GetWindowText(content);
			comments += content;
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// define display sizes - dataview & datadetect are same, spkshape & spkbar = as on screen
			CRect rectdata = rect;
			rectdata.top	-= - 3*lineheight;
			int rspkwidth	= MulDiv(m_spkShapeView.Width(), rectdata.Width(), m_spkShapeView.Width() + m_displayDetect.Width());
			int rdataheight = MulDiv(m_displayDetect.Height(), rectdata.Height(), m_displayDetect.Height()*2 + m_spkBarView.Height());
			int separator = rspkwidth / 10;

			// display curves : data
			rectdata.bottom = rect.top + rdataheight - separator/2;
			rectdata.left	= rect.left + rspkwidth + separator;
			PrintDataCartridge (&mDC, &m_displayData, &rectdata, TRUE, TRUE);

			// display curves: detect channel
			rectdata.top	= rectdata.bottom + separator;
			rectdata.bottom = rectdata.top + rdataheight;
			PrintDataCartridge (&mDC, &m_displayDetect, &rectdata, TRUE, TRUE);

			// display spike bars
			CRect rectbars = rectdata;
			rectbars.top = rectdata.bottom + separator;
			rectbars.bottom = rect.bottom -2*lineheight;
			m_spkBarView.Print(&mDC, &rectbars);

			// display spike shapes
			CRect rectspk = rect;					// compute output rectangle
			rectspk.left += separator;
			rectspk.right = rect.left + rspkwidth;
			rectspk.bottom = rect.bottom -2*lineheight;
			rectspk.top = rectspk.bottom - rectbars.Height();
			m_spkShapeView.Print(&mDC, &rectspk);
			comments = PrintSpkShapeBars(&mDC, &rectspk, TRUE);

			CRect rectComment = rect;
			rectComment.top = ypxrow;
			rectComment.right = rectdata.left;
			rectComment.top = rectspk.bottom;
			UINT nFormat = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			mDC.DrawText(comments, comments.GetLength(), rectComment, nFormat);
			mDC.SelectObject(pOldBrush);

			if (m_pOldFont != nullptr)
				mDC.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

			// restore oldpen
			mDC.SelectObject(poldpen);
			ReleaseDC(pDCRef);

			// Close metafile 
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
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION );
			}
			// restore initial conditions
			SerializeWindowsState(BRESTORE);
		}
	}
}

void CSpikeDetectionView::OnSelchangeDetectMode() 
{
	UpdateData(TRUE);
	m_pDetectParms->detectWhat = m_CBdetectWhat.GetCurSel();
	UpdateCB();	
	m_displayDetect.GetDataFromDoc(); 		// load data
	m_displayDetect.AutoZoomChan(0);		// vertical position of channel	
	m_displayDetect.Invalidate();	
}

void CSpikeDetectionView::UpdateCB()
{
	m_CBdetectChan.SetCurSel(m_pDetectParms->detectChan);
	m_CBtransform.SetCurSel(m_pDetectParms->detectTransform);
	m_displayDetect.SetChanlistOrdinates(0, m_pDetectParms->detectChan, m_pDetectParms->detectTransform);
	m_pDetectParms->detectThreshold = m_displayDetect.ConvertChanlistVoltstoDataBins(0, m_thresholdval / 1000.f);
	m_displayDetect.SetHZtagChan(0,0);
	m_displayDetect.SetHZtagVal(0, m_pDetectParms->detectThreshold);
	m_pDetectParms->detectThresholdmV = m_thresholdval;
}

void CSpikeDetectionView::UpdateVTtags()
{
	m_spkBarView.DelAllVTtags();
	m_displayDetect.DelAllVTtags();
	m_displayData.DelAllVTtags();
	if (m_pspkDocVSD->m_stimIntervals.nitems == 0)
		return;

	for (int i=0; i <m_pspkDocVSD->m_stimIntervals.intervalsArray.GetSize(); i++)
	{
		int cx = m_pspkDocVSD->m_stimIntervals.intervalsArray.GetAt(i);
		m_spkBarView.AddVTLtag(cx);
		m_displayDetect.AddVTLtag(cx);
		m_displayData.AddVTLtag(cx);
	}
}

//-----------------------------------------------------------------------
// compute printer's page dot resolution
// borrowed from VC++ sample\drawcli\drawdoc.cpp
//-----------------------------------------------------------------------
void CSpikeDetectionView::PrintComputePageSize()
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

void CSpikeDetectionView::PrintFileBottomPage(CDC* pDC, CPrintInfo* pInfo)
{
	CTime t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
		pInfo->m_nCurPage, pInfo->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	CString ch_date = GetDocument()->DBGetCurrentSpkFileName();
	pDC->SetTextAlign(TA_CENTER);
	pDC->TextOut(mdPM->horzRes / 2, mdPM->vertRes - 57, ch_date);
}

// ---------------------------------------------------------------------------------------
// ConvertFileIndex
// return ascii string
// ---------------------------------------------------------------------------------------
CString CSpikeDetectionView::PrintConvertFileIndex(long lFirst, long lLast)
{
	CString csUnit= _T(" s");			// get time,  prepare time unit
	CString csComment;
	float xScaleFactor;				// scale factor returned by changeunit
	float x1 = m_displayDetect.ChangeUnit( (float) lFirst / m_samplingRate, &csUnit, &xScaleFactor);
	int fraction1 = (int) ((x1 - ((int) x1)) * (float) 1000.);	// separate fractional part
	float x2 = lLast / (m_samplingRate * xScaleFactor);	
	int fraction2 = (int) ((x2 - ((int) x2)) *  (float) 1000.);
	csComment.Format (_T("time = %i.%03.3i - %i.%03.3i %s"), (int) x1, fraction1, (int) x2, fraction2, (LPCTSTR) csUnit);
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

BOOL CSpikeDetectionView::PrintGetFileSeriesIndexFromPage(int page, int &filenumber, long &lFirst)
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
		veryLast= GetDocument()->DBGetDataLen()-1;

	for (int row = 0; row <totalrows; row++)
	{
		if (!PrintGetNextRow(filenumber, lFirst, veryLast))
			break;
	}

	return TRUE;
}

BOOL CSpikeDetectionView::PrintGetNextRow(int &filenumber, long &lFirst, long &veryLast)
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
// print text for a file items
// take care of user's options saved in mdPM structure
/////////////////////////////////////////////////////////////////////////////

CString CSpikeDetectionView::PrintGetFileInfos()
{
	CString strComment;   					// scratch pad
	CString Tab(_T("    "));					// use 4 spaces as tabulation character
	CString RC(_T("\n"));						// next line

	// document's name, date and time
	CAcqDataDoc* pDataFile = GetDocument()->m_pDat;
	CWaveFormat* pwaveFormat = pDataFile->GetpWaveFormat();
	if (mdPM->bDocName || mdPM->bAcqDateTime)// print doc infos?
	{		
		if (mdPM->bDocName)					// print file name
			strComment += GetDocument()->DBGetCurrentDatFileName() + Tab;

		if (mdPM->bAcqDateTime)				// print data acquisition date & time
		{
			CString date = (pwaveFormat->acqtime).Format(_T("%#d %B %Y %X")); //("%c");
			strComment += date;
		}
		strComment += RC;
	}

	// document's main comment (print on multiple lines if necessary)
	if (mdPM->bAcqComment)
		strComment += GetDocument()->ExportDatabaseData(); //pwaveFormat->GetComments(" ") +RC;

	return strComment;
}

// -------------------------------------------------------------------------------
// print scale bars and print comments concerning the signal characteristics
CString CSpikeDetectionView::PrintDataBars(CDC* pDC, CLineViewWnd* pLineViewWnd, CRect* rect)
{
	CString strComment;
	CString cs;
	CString RC(_T("\r"));
	CString Tab(_T("     "));
	CBrush* pOldBrush= (CBrush*) pDC->SelectStockObject(BLACK_BRUSH);
	CString csUnit;									// string for voltage or time unit	
	CPoint	barOrigin(-10,-10);						// bar origin at 10,10 pts on the left lower corner of the rectangle
	CSize	barWidth = CSize(1,1);					// width of bars (5 pixels)
	int		ihorzBar = pLineViewWnd->Width()/10;	// initial horz bar length 1/10th of display rect
	int		ivertBar = pLineViewWnd->Height()/3;	// initial vert bar height 1/3rd  of display rect

	///// time abcissa ///////////////////////////	
	strComment = PrintConvertFileIndex(pLineViewWnd->GetDataFirst(), pLineViewWnd->GetDataLast());

	///// horizontal time bar ///////////////////////////	
	if (mdPM->bTimeScaleBar)           
	{
		// convert bar size into time units and back into pixels
		csUnit = _T(" s");											// initial time unit
		float xtperpixel = pLineViewWnd->GetTimeperPixel();
		float z =  xtperpixel* ihorzBar;							// convert 1/10 of the length of the data displayed into time
		float	xScaleFactor;
		float x = pLineViewWnd->ChangeUnit(z, &csUnit, &xScaleFactor); // convert time into a scaled time
		int k = pLineViewWnd->NiceUnit(x);							// convert the (scaled) time value into time expressed as an integral
		ihorzBar = (int) ( ((float)k * xScaleFactor) / xtperpixel); // compute how much pixels it makes
		// print out the scale and units
		cs.Format( _T("horz bar = %i %s"), k, (LPCTSTR) csUnit);
		strComment += cs + RC;
		// draw horizontal line
		ihorzBar = MulDiv(ihorzBar, rect->Width(), pLineViewWnd->Width());
		pDC->MoveTo(rect->left + barOrigin.x,			 rect->bottom -barOrigin.y);
		pDC->LineTo(rect->left + barOrigin.x + ihorzBar, rect->bottom -barOrigin.y);		
	}

	///// vertical voltage bars ///////////////////////////	
	float	yScaleFactor;											// compute a good unit for channel 0
	csUnit = _T(" V");												// initial voltage unit
	// convert bar size into voltage units and back into pixels
	float vperpixel = pLineViewWnd->GetChanlistVoltsperPixel(0);
	float zvolts = vperpixel * ivertBar;							// convert 1/3 of the height into voltage
	float zscale = pLineViewWnd->ChangeUnit(zvolts, &csUnit, &yScaleFactor);	// convert voltage into a scale voltage
	float znice = (float) pLineViewWnd->NiceUnit(zscale);			// convert the (scaled) time value into time expressed as an integral
	ivertBar = (int) (znice * yScaleFactor / vperpixel);			// compute how much pixels it makes
 
	if (mdPM->bVoltageScaleBar)
	{
		ivertBar = MulDiv(ivertBar, rect->Height(), pLineViewWnd->Height());
		pDC->MoveTo(rect->left +barOrigin.x, rect->bottom -barOrigin.y);
		pDC->LineTo(rect->left +barOrigin.x , rect->bottom -barOrigin.y - ivertBar);
	}

	// comments, bar value and chan settings for each channel	
	if (mdPM->bChansComment || mdPM->bVoltageScaleBar || mdPM->bChanSettings)
	{
		int imax = pLineViewWnd->GetChanlistSize();	// number of data channels
		for (int ichan=0; ichan< imax; ichan++)		// loop
		{
			// skip channels not printed
			if (!pLineViewWnd->GetChanlistflagPrintVisible(ichan))
				continue;
			// boucler sur les commentaires de chan n a chan 0...		
			cs.Format( _T("chan#%i "), ichan);			// channel number
			strComment += cs;
			if (mdPM->bVoltageScaleBar)				// bar scale value
			{			
				float z= (float) ivertBar* pLineViewWnd->GetChanlistVoltsperPixel(ichan);
				float x = z / yScaleFactor;
				int j = pLineViewWnd->NiceUnit(x);
				cs.Format(_T("vert bar = %i %s "), j, (LPCTSTR) csUnit);	// store val into comment
				strComment += cs;
			}
			// print chan comment 
			if (mdPM->bChansComment)
			{
				strComment += Tab;
				strComment += pLineViewWnd->GetChanlistComment(ichan);
			}
			strComment += RC;
			// print amplifiers settings (gain & filter), next line
			if (mdPM->bChanSettings)
			{
				WORD channb = pLineViewWnd->GetChanlistSourceChan(ichan);
				CWaveChanArray* pchanArray = GetDocument()->m_pDat->GetpWavechanArray();
				CWaveChan* pChan = pchanArray->get_p_channel(channb);
				cs.Format(_T("headstage=%s  g=%li LP=%i  IN+=%s  IN-=%s"), 
					(LPCTSTR) pChan->am_csheadstage, (long) (pChan->am_gaintotal), pChan->am_lowpass, (LPCTSTR) pChan->am_csInputpos, (LPCTSTR) pChan->am_csInputneg);
				strComment += cs;
				strComment += RC;
			}
		}
	}	
	pDC->SelectObject(pOldBrush);
	return strComment;
}

CString CSpikeDetectionView::PrintSpkShapeBars(CDC* pDC, CRect* rect, BOOL bAll)
{
	CString RC("\n");
	CString strComment;

	CString csComment;
	CString csUnit;				// voltage or time unit
	float z;
	int k;	

	///// vertical voltage bars ///////////////////////////	
	if (mdPM->bVoltageScaleBar && m_pSpkListVSD->GetTotalSpikes()>0)
	{
		// the following assume that spikes are higher than 1 mV...
		csUnit= _T(" mV");
		z = m_spkShapeView.GetExtent_mV()/2.0f;	// half of total height
		k = (int) z;							// clip to int
		if ((double(z) - k) > 0.5)					// make sure it is not zero
			k++;
		if (bAll)
		{
			if (k > 0)
				wsprintf(m_szbuf, _T("Vbar=%i %s"), k, (LPCTSTR) csUnit);
			else
				_stprintf_s(m_szbuf, 64, _T("Vbar=%f.3 mV"), z);
			csComment = m_szbuf;					// store val into comment
			strComment = csComment + RC;
		}
		
		// display bar
		CBrush* pOldBrush= (CBrush*) pDC->SelectStockObject(BLACK_BRUSH);
		if (k > 0)
			z = (float) (k) / z;
		int vertBar = (int) (rect->Height()* z) / 2;

		// compute coordinates of the rect
		CRect rectVertBar;		// vertical bar rectangle
		CSize barWidth = CSize(5, 5);
		rectVertBar.left = rect->left - mdPM->textseparator;
		rectVertBar.right = rectVertBar.left + barWidth.cx;
		rectVertBar.top = rect->top + (rect->Height() - vertBar)/2;
		rectVertBar.bottom = rectVertBar.top + vertBar;
		pDC->Rectangle(&rectVertBar);
		pDC->SelectObject(pOldBrush);
	}

	// spike duration 
	if (m_pSpkListVSD->GetTotalSpikes() > 0 && bAll)
	{
		z = m_spkShapeView.GetExtent_ms();
		csUnit = _T(" ms");
		k = (int) z;
		wsprintf(m_szbuf, _T("Horz=%i."), k);
		csComment = m_szbuf;		// store val into comment
		k = (int) (1000.0f * (z - (float)k));
		wsprintf(m_szbuf, _T("%i %s"), k, (LPCTSTR) csUnit);
		csComment += m_szbuf;		// store val into comment
		strComment += csComment;
		strComment += RC;
	}

	// number of spikes
	k = m_pSpkListVSD->GetTotalSpikes() ; 
	wsprintf(m_szbuf, _T("n spk= %i"), k);
	csComment = m_szbuf;
	strComment += csComment;
	strComment += RC;

	return strComment;
}

void CSpikeDetectionView::SerializeWindowsState(BOOL bSave, int itab)
{
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// pointer to list of pointers to store parameters
	if (itab < 0 || itab >= m_tabCtrl.GetItemCount())
	{
		int itab_sel = m_tabCtrl.GetCurSel();			// current selected tab
		if (itab_sel < 0)
			itab_sel = 0;
		itab = itab_sel;
	}

	// adjust size of the array
	if (pApp->m_viewspkdParmsArray.GetSize() == 0)
		pApp->m_viewspkdParmsArray.SetSize(1);
	
	if (pApp->m_viewspkdParmsArray.GetSize() < m_tabCtrl.GetItemCount())
		pApp->m_viewspkdParmsArray.SetSize(m_tabCtrl.GetItemCount());
	CMemFile* pMemFile = nullptr;
	if (pApp->m_viewspkdParmsArray.GetSize() > 0 && pApp->m_viewspkdParmsArray.GetSize() > itab)
		pMemFile = (CMemFile*) pApp->m_viewspkdParmsArray.GetAt(itab);

	// save display parameters
	if (bSave)
	{
		if (pMemFile == nullptr)
		{
			pMemFile = new CMemFile;
			ASSERT(pMemFile != NULL);
			pApp->m_viewspkdParmsArray.SetAt(itab, pMemFile);
		}
		// save data into archive
		CArchive ar(pMemFile, CArchive::store);
		pMemFile->SeekToBegin();
		m_displayData.Serialize(ar);
		m_displayDetect.Serialize(ar);
		m_spkBarView.Serialize(ar);
		m_spkShapeView.Serialize(ar);
		ar.Close();
	}

	// restore display parameters
	else
	{
		if (pMemFile != nullptr)
		{
			CArchive ar(pMemFile, CArchive::load);
			pMemFile->SeekToBegin();
			m_displayData.Serialize(ar);
			m_displayDetect.Serialize(ar);
			m_spkBarView.Serialize(ar);
			m_spkShapeView.Serialize(ar);
			ar.Close();					// close archive
		}
		else
		{
			m_displayData.m_parms = mdPM->viewspkdetectdata;
			m_displayDetect.m_parms = mdPM->viewspkdetectfiltered;
			m_spkBarView.m_parms = mdPM->viewspkdetectspk;
			m_spkShapeView.m_parms = mdPM->viewspkdetectbars;
			OnFormatSplitcurves();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// (1) OnPreparePrinting()
// override standard setting before calling print dialog
/////////////////////////////////////////////////////////////////////////////

BOOL CSpikeDetectionView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// save current state of the windows
	SerializeWindowsState(BSAVE);

	// printing margins	
	if (mdPM->vertRes <= 0 ||mdPM->horzRes <= 0
		||mdPM->horzRes !=  pInfo->m_rectDraw.Width()
		||mdPM->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// nb print pages?
	int npages = PrintGetNPages();
	pInfo->SetMaxPage(npages);			// one page printing/preview  
	pInfo->m_nNumPreviewPages = 1;  	// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	//------------------------------------------------------
	if (mdPM->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		mdPM->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = PrintGetNPages();
		pInfo->SetMaxPage(npages);
	}
	return TRUE; 
}

int	CSpikeDetectionView::PrintGetNPages()
{
	// how many rows per page?
	int sizeRow=mdPM->HeightDoc + mdPM->heightSeparator;
	m_nbrowsperpage = m_printRect.Height()/sizeRow;
	if (m_nbrowsperpage == 0)		// prevent zero pages
		m_nbrowsperpage = 1;

	int ntotalRows;						// number of rectangles -- or nb of rows
	int npages;						// number of pages
	CdbWaveDoc* pdbDoc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag 
	m_lprintFirst = m_displayDetect.GetDataFirst();
	m_lprintLen = m_displayDetect.GetDataLast() -m_lprintFirst + 1;
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
				len = pdbDoc->m_pDat->GetDOCchanLength();
				long len1 = GetDocument()->DBGetDataLen()-1;
				ASSERT(len == len1);
				pdbDoc->DBSetDataLen(len);
			}
			len -= m_lprintFirst;
			long nrows = len/m_lprintLen;	// how many rows for this file?
			if (len > nrows*m_lprintLen)	// remainder?
				nrows++;
			ntotalRows+= (int) nrows;		// update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		try 
		{
			pdbDoc->DBSetCurrentRecordPosition(m_file0);
			pdbDoc->OpenCurrentDataFile();
		} 
		catch(CDaoException* e) {DisplayDaoException(e, 3); e->Delete();}
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

void CSpikeDetectionView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_displayDetect.GetDataFirst();
	m_lLast0 = m_displayDetect.GetDataLast();
	m_npixels0 = m_displayDetect.Width();
	PrintCreateFont();
	pDC->SetBkMode (TRANSPARENT);
}	

void CSpikeDetectionView::PrintCreateFont()
{
	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
	m_logFont.lfHeight = mdPM->fontsize;			// font height
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
}

/////////////////////////////////////////////////////////////////////////////
//	(3) OnPrint() -- for each page
/////////////////////////////////////////////////////////////////////////////

void CSpikeDetectionView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	m_pOldFont = pDC->SelectObject(&m_fontPrint);

		// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	int rWidth = mdPM->WidthDoc;					// page margins
	int rHeight = mdPM->HeightDoc;					// page margins
	CRect RWhere (	m_printRect.left, 				// printing rectangle for one line of data
					m_printRect.top, 
					m_printRect.left+rWidth, 
					m_printRect.top+rHeight);
	CRect RW2 = RWhere;								// printing rectangle - constant
	RW2.OffsetRect(-RWhere.left, -RWhere.top);		// set RW2 origin = 0,0

	// define spike shape area (rect): same height as data area	
	int rSPKheight = mdPM->spkheight;
	int rSPKwidth = mdPM->spkwidth;
	if (rSPKheight == 0)
	{
		rSPKheight = rHeight - mdPM->fontsize*4;
		rSPKwidth = rSPKheight / 2;
		if (rSPKwidth < MulDiv (RWhere.Width(), 10, 100))
			rSPKwidth = MulDiv (RWhere.Width(), 10, 100);
		mdPM->spkheight= rSPKheight;
		mdPM->spkwidth = rSPKwidth;
	}

	// save current draw mode (it will be modified to print only one channel)
	WORD chan0Drawmode = 1;
	WORD chan1Drawmode = 0;
	if (!mdPM->bFilterDataSource)
		m_displayDetect.SetChanlistTransformMode(0, 0);

	pDC->SetMapMode(MM_TEXT);						// change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(pDC, pInfo);				// print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page		
	int filenumber;									// file number and file index
	long lFirst;									// index first data point / first file
	long veryLast = m_lprintFirst + m_lprintLen;	// index last data point / current file
	int curpage   = pInfo->m_nCurPage;				// get current page number
	PrintGetFileSeriesIndexFromPage(curpage, filenumber, lFirst);
	if (lFirst < GetDocument()->DBGetDataLen()-1)
		UpdateFileParameters(FALSE);
	if (mdPM->bEntireRecord)
		veryLast = GetDocument()->DBGetDataLen()-1;

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i<m_nbrowsperpage;	i++)
	{		
		int oldDC = pDC->SaveDC();					// save DC		
		
		// first : set rectangle where data will be printed
		CRect CommentRect = RWhere;					// save RWhere for comments		
		pDC->SetMapMode(MM_TEXT);					// 1 pixel = 1 logical unit
		pDC->SetTextAlign(TA_LEFT); 				// set text align mode				

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		m_rData = RWhere;								// adjust data rect/ file data rect
		if (mdPM->bPrintSpkBars)
			m_rData.bottom = m_rData.top + RWhere.Height()/2; // half vertic size = data		
		m_rData.left += (rSPKwidth + mdPM->textseparator);
		int oldsize = m_rData.Width();					// save display rect size

		// make sure enough data fit into this rectangle, otherwise clip rect
		long lLast = lFirst + m_lprintLen;				// compute last pt to load
		if (lLast > veryLast)							// check end across file length
			lLast = veryLast;
		if ((lLast-lFirst+1) <m_lprintLen)				// adjust rect to length of data
			m_rData.right = (oldsize*(lLast-lFirst)) / m_lprintLen + m_rData.left;
//--_____________________________________________________________________--------
//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// if option requested, clip output to rect
		if (mdPM->bClipRect)							// clip curve display 
			pDC->IntersectClipRect(&m_rData);			// (eventually)

		// print detected channel only data
		m_displayDetect.SetChanlistflagPrintVisible(0, chan0Drawmode);
		m_displayDetect.ResizeChannels(m_rData.Width(), 0);
		m_displayDetect.GetDataFromDoc(lFirst, lLast);	// load data from file
		m_displayDetect.Print(pDC, &m_rData);			// print data
		pDC->SelectClipRgn(nullptr);						// no more clipping

		// print spike bars ---------------------------------------------------------------
		if (mdPM->bPrintSpkBars)
		{
			CRect BarsRect = RWhere;					// adjust data rect/ file data rect
			BarsRect.top  = m_rData.bottom;				// copy coordinates
			BarsRect.left = m_rData.left;
			BarsRect.right= m_rData.right;

			m_spkBarView.SetTimeIntervals(lFirst, lLast);	// define time interval
			m_spkBarView.Print(pDC, &BarsRect);			// print data
		}

		// print spkform within a square (same width as height) ---------------------------
		m_rSpike = RWhere;								// get file rectangle
		m_rSpike.right = m_rSpike.left + rSPKwidth;
		m_rSpike.left += mdPM->textseparator;			// remove space for 4 lines of
		m_rSpike.bottom = m_rSpike.top + rSPKheight;	// legends

		m_spkShapeView.SetTimeIntervals(lFirst, lLast);
		m_spkShapeView.Print(pDC, &m_rSpike);

		// restore DC and print comments ---------------------------------------------------
		pDC->RestoreDC(oldDC);						// restore Display context	
		pDC->SetMapMode(MM_TEXT);					// 1 LP = 1 pixel
		pDC->SelectClipRgn(nullptr);					// no more clipping
		pDC->SetViewportOrg(0, 0);					// org = 0,0        

		// print data Bars & get comments according to row within file
		CString csComment;
		BOOL bAll = (lFirst == m_lprintFirst);
		if (bAll)									// first row = full comment
		{
			csComment += PrintGetFileInfos();		// describe file, intervals & comments /chan
			csComment += PrintDataBars(pDC, &m_displayDetect, &m_rData);	// bars and bar legends
		}
		else
		{	// other rows: time intervals only
			csComment = PrintConvertFileIndex(m_displayDetect.GetDataFirst(), m_displayDetect.GetDataLast());
		}

		// print comments stored into csComment				
		CommentRect.OffsetRect(mdPM->textseparator + CommentRect.Width(), 0);
		CommentRect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		UINT nFormat = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		pDC->DrawText(csComment, csComment.GetLength(), CommentRect, nFormat);

		// print comments & bar / spike shape
		csComment.Empty();
		m_rSpike.right	= m_rSpike.left + rSPKheight;
		csComment		= PrintSpkShapeBars(pDC, &m_rSpike, bAll);
		m_rSpike.right	= m_rSpike.left + rSPKwidth;
		m_rSpike.left	-= mdPM->textseparator;
		m_rSpike.top	= m_rSpike.bottom;
		m_rSpike.bottom += m_logFont.lfHeight * 3;
		pDC->DrawText(csComment, csComment.GetLength(), m_rSpike, nFormat);
		pDC->SetTextAlign(uiFlag);
//--_____________________________________________________________________--------
//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// update file parameters for next row --------------------------------------------

		RWhere.OffsetRect(0, rHeight+mdPM->heightSeparator);
		int ifile = filenumber;
		if (!PrintGetNextRow(filenumber, lFirst, veryLast))
		{
			i = m_nbrowsperpage;
			break;
		}
		if (ifile != filenumber)
		{
			UpdateFileParameters(FALSE);
			m_displayDetect.SetChanlistflagPrintVisible(0, 0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	m_displayDetect.SetChanlistflagPrintVisible(0, 1);
	if (!mdPM->bFilterDataSource)
		m_displayDetect.SetChanlistTransformMode(0, m_pDetectParms->detectTransform);

	if (m_pOldFont != nullptr)
		pDC->SelectObject(m_pOldFont);
}

/////////////////////////////////////////////////////////////////////////////
//	(4) OnEndPrinting() - lastly
/////////////////////////////////////////////////////////////////////////////

void CSpikeDetectionView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	// restore file from index and display parameters
	GetDocument()->DBSetCurrentRecordPosition(m_file0);

	m_displayDetect.ResizeChannels(m_npixels0, 0);
	m_displayDetect.GetDataFromDoc(m_lFirst0, m_lLast0);
	m_spkShapeView.SetTimeIntervals(m_lFirst0, m_lLast0);
	UpdateFileParameters(TRUE);	

	m_bIsPrinting = FALSE;	
	SerializeWindowsState(BRESTORE);
}

// trap messages
void CSpikeDetectionView::OnBnClickedBiasbutton()
{
	((CButton*) GetDlgItem(IDC_BIAS))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY);
}

void CSpikeDetectionView::OnBnClickedBias2()
{
	((CButton*) GetDlgItem(IDC_BIAS2))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN2))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY2);
}

void CSpikeDetectionView::OnBnClickedGainbutton()
{
	((CButton*) GetDlgItem(IDC_BIAS))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY);
}

void CSpikeDetectionView::OnBnClickedGain2()
{
	((CButton*) GetDlgItem(IDC_BIAS2))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN2))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY2);
}

void CSpikeDetectionView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar != nullptr)
	{
		int iID = pScrollBar->GetDlgCtrlID();
		if ((iID == IDC_SCROLLY) || (iID == IDC_SCROLLY2))
		{
			// CDataView scroll: vertical scroll bar
			switch (m_VBarMode)
			{
			case BAR_GAIN:
				OnGainScroll(nSBCode, nPos, iID);
				break;
			case BAR_BIAS:
				OnBiasScroll(nSBCode, nPos, iID);
			default:		
				break;
			}
		}
	}
	else
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CSpikeDetectionView::SetVBarMode (short bMode, int iID)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll(iID);
}        

void CSpikeDetectionView::UpdateGainScroll(int iID)
{
	if (iID == IDC_SCROLLY)
		m_scrolly.SetScrollPos(
			MulDiv(m_displayDetect.GetChanlistYextent(m_ichanselected), 100, YEXTENT_MAX) +50, TRUE);
	else
		m_scrolly2.SetScrollPos(
			MulDiv(m_displayData.GetChanlistYextent(m_ichanselected2), 100, YEXTENT_MAX) +50, TRUE);
}

void CSpikeDetectionView::OnGainScroll(UINT nSBCode, UINT nPos, int iID)
{
	CLineViewWnd* pView = &m_displayDetect;
	int ichan = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		pView = &m_displayData;
		ichan = m_ichanselected2;
	}
	int lSize = pView->GetChanlistYextent(ichan);

	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:		lSize = YEXTENT_MIN; break;		// .................scroll to the start
	case SB_LINELEFT:	lSize -= lSize/10 +1; break;	// .................scroll one line left
	case SB_LINERIGHT:	lSize += lSize/10 +1; break;	// .................scroll one line right
	case SB_PAGELEFT:	lSize -= lSize/2+1; break;		// .................scroll one page left
	case SB_PAGERIGHT:	lSize += lSize+1; break;		// .................scroll one page right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;		// .................scroll to end right
	case SB_THUMBPOSITION:			// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBTRACK:	lSize = MulDiv(nPos-50,YEXTENT_MAX,100); break;
	default:			break;							// .................NOP: set position only
	}
	// change y extent
	if (lSize>0 ) //&& lSize<=YEXTENT_MAX)
	{
		pView->SetChanlistYextent(ichan, lSize);
		UpdateLegends();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll(iID);
}

void CSpikeDetectionView::UpdateBiasScroll(int iID)
{
	if (iID == IDC_SCROLLY)
	{
		int iPos = (int) ((m_displayDetect.GetChanlistYzero(m_ichanselected) - m_displayDetect.GetChanlistBinZero(m_ichanselected))
						*100/(int)YZERO_SPAN)+(int)50;
		m_scrolly.SetScrollPos(iPos, TRUE);
	}
	else
	{
		int iPos = (int) ((m_displayData.GetChanlistYzero(m_ichanselected2) - m_displayData.GetChanlistBinZero(m_ichanselected2))
						*100/(int)YZERO_SPAN)+(int)50;
		m_scrolly2.SetScrollPos(iPos, TRUE);
	}
}

void CSpikeDetectionView::OnBiasScroll(UINT nSBCode, UINT nPos, int iID)
{
	CLineViewWnd* pView = &m_displayDetect;
	int ichan = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		pView = &m_displayData;
		ichan = m_ichanselected2;
	}
	
	int lSize = pView->GetChanlistYzero(ichan) - pView->GetChanlistBinZero(ichan);
	int yextent = pView->GetChanlistYextent(ichan);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			lSize = YZERO_MIN; break;		// scroll to the start
	case SB_LINELEFT:		lSize -= yextent/100+1; break;	// scroll one line left
	case SB_LINERIGHT:		lSize += yextent/100+1; break;	// scroll one line right
	case SB_PAGELEFT:		lSize -= yextent/10+1; break;	// scroll one page left
	case SB_PAGERIGHT:		lSize += yextent/10+1; break;	// scroll one page right
	case SB_RIGHT:			lSize = YZERO_MAX; break;		// scroll to end right
	case SB_THUMBPOSITION:	// scroll to pos = nPos	// drag scroll box -- pos = nPos
	case SB_THUMBTRACK:		lSize = (nPos-50)*(YZERO_SPAN/100); break;
	default:				break;							// NOP: set position only
	}

	// try to read data with this new size
	if (lSize>YZERO_MIN && lSize<YZERO_MAX)
	{		
		pView->SetChanlistYzero(ichan, lSize+ pView->GetChanlistBinZero(ichan));
		pView->Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll(iID);
}

void CSpikeDetectionView::OnEnChangeSpkWndAmplitude()
{
	if (!mm_spkWndAmplitude.m_bEntryDone)
		return;

	float y = m_spkShapeView.GetExtent_mV();
	float yold = y;
	CString cs;
	switch (mm_spkWndAmplitude.m_nChar)
	{		
	case VK_RETURN:			
		mm_spkWndAmplitude.GetWindowText(cs);
		y = (float) _ttof (cs);
		break;
	case VK_UP:
	case VK_PRIOR:
		y++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		y--;
		break;
	}
	
	// compute new extent and change the display
	if (y <= 0)
	{
		y= yold;
		MessageBeep(-1);
	}
	int yWE = (int) ((float) m_spkShapeView.GetYWExtent() * y / yold) ;
	m_spkShapeView.SetYWExtOrg(yWE, m_spkShapeView.GetYWOrg());
	m_spkShapeView.SetyScaleUnitValue(y/m_spkShapeView.GetNyScaleCells());
	m_spkShapeView.Invalidate();

	// update the dialog control
	mm_spkWndAmplitude.m_bEntryDone=FALSE;
	mm_spkWndAmplitude.m_nChar=0;
	mm_spkWndAmplitude.SetSel(0, -1); 	//select all text
	cs.Format(_T("%.3f"), y);
	GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
}

void CSpikeDetectionView::OnEnChangeSpkWndLength()
{
	if (!mm_spkWndDuration.m_bEntryDone)
		return;

	float x = m_spkShapeView.GetExtent_ms();
	float xold = x;
	CString cs;
	switch (mm_spkWndDuration.m_nChar)
	{		
	case VK_RETURN:			
		mm_spkWndDuration.GetWindowText(cs);
		x = (float) _ttof (cs);
		break;
	case VK_UP:
	case VK_PRIOR:
		x++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		x--;
		break;
	}
	
	// compute new extent and change the display
	if (x <= 0)
	{
		MessageBeep(-1);
		x = xold;
	}
	int xWE = (int) ((float) m_spkShapeView.GetXWExtent() * x / xold) ;
	m_spkShapeView.SetXWExtOrg(xWE, m_spkShapeView.GetXWOrg());
	m_spkShapeView.SetxScaleUnitValue(x/m_spkShapeView.GetNxScaleCells());
	m_spkShapeView.Invalidate();

	// update the dialog control
	mm_spkWndDuration.m_bEntryDone=FALSE;
	mm_spkWndDuration.m_nChar=0;
	mm_spkWndDuration.SetSel(0, -1); 	//select all text
	cs.Format(_T("%.3f"), x);
	GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
}

void CSpikeDetectionView::OnBnClickedLocatebttn()
{
	int max, min;
	m_displayDetect.GetChanlistMaxMin(0, &max, &min);
	
	// modify value
	m_pDetectParms->detectThreshold = (max + min) / 2;
	m_thresholdval = m_displayDetect.ConvertChanlistDataBinsToMilliVolts(0, m_pDetectParms->detectThreshold);
	m_pDetectParms->detectThresholdmV = m_thresholdval;
	// update user-interface: edit control and threshold bar in sourceview
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	m_displayDetect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);
	m_displayDetect.Invalidate();
}

// load detection parameters from current parameters array
// and update interface elements

// if iSelParms > m_parmsCurrent.GetSize(): TRANSFER ALL DATA FROM SPKFILE -> PARMS
// then TRANSFER ALL DATA FROM PARMS TO FILE

void CSpikeDetectionView::UpdateDetectionSettings(int iSelParms)
{
	// check size of spike detection parameters
	if (iSelParms >= m_parmsCurrent.GetSize())
	{
		// load new set of parameters from spike list
		int isize = m_pspkDocVSD->GetSpkListSize();
		m_parmsCurrent.SetSize(isize);
		for (int i=0; i < isize; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			CSpikeList* pspklist = m_pspkDocVSD->SetSpkListCurrent(i);
			ASSERT(pspklist != NULL);
			SPKDETECTPARM* psD = pspklist->GetDetectParms();
			m_parmsCurrent.SetItem(i, psD);		// copy content of spklist parm into m_parmsCurrent
		}
	}
	
	// loop over each spike detection set to update spike detection parameters
	for (int i=0; i < m_parmsCurrent.GetSize(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		CSpikeList* pspklist = m_pspkDocVSD->SetSpkListCurrent(i);
		SPKDETECTPARM* pSd = m_parmsCurrent.GetItem(i);
		if (pspklist == nullptr)
		{
			m_pspkDocVSD->AddSpkList();
			pspklist  = m_pspkDocVSD->GetSpkListCurrent();
			pspklist->InitSpikeList(GetDocument()->m_pDat, pSd);
		}
		else
			pspklist->SetDetectParms(pSd);
	}

	// set new parameters
	m_pSpkListVSD->m_selspike = m_spikeno;			// save spike selected
	m_iDetectParms = iSelParms;
	GetDocument()->SetcurrentSpkListIndex(iSelParms);
	m_pDetectParms = m_parmsCurrent.GetItem(iSelParms);
	m_pSpkListVSD = m_pspkDocVSD->SetSpkListCurrent(iSelParms);
	if (m_pSpkListVSD != nullptr)
		HighlightSpikes(TRUE);

	UpdateDetectionControls();
	UpdateLegends();
}

void CSpikeDetectionView::UpdateDetectionControls()
{
	m_CBdetectWhat.SetCurSel(m_pSpkListVSD->GetdetectWhat());
	m_CBdetectChan.SetCurSel(m_pSpkListVSD->GetdetectChan());
	m_CBtransform.SetCurSel(m_pSpkListVSD->GetdetectTransform());	// this should remains = to detection parameters
	
	// check that spike detection parameters are compatible with current data doc
	int detectchan = m_pSpkListVSD->GetdetectChan();
	int extractchan = m_pSpkListVSD->GetextractChan();
	CAcqDataDoc* pDat = m_displayDetect.GetpDataFile();
	int maxchan = pDat->GetpWaveFormat()->scan_count;
	if (detectchan >= maxchan)
		detectchan = 0;
	if (extractchan >= maxchan)
		extractchan = 0;

	m_displayDetect.SetChanlistOrdinates(0, detectchan, m_pSpkListVSD->GetdetectTransform());
	m_CBtransform2.SetCurSel(m_pSpkListVSD->GetextractTransform());
	m_displayDetect.SetChanlistColor(0, detectchan);

	m_displayDetect.GetDataFromDoc(); 		// load data
	if (mdPM->bSplitCurves)
		m_displayDetect.SplitChans();

	int ithreshold = m_pSpkListVSD->GetdetectThreshold();
	m_thresholdval = m_displayDetect.ConvertChanlistDataBinsToMilliVolts(0, ithreshold);
	if (m_displayDetect.GetNHZtags() <1)
		m_displayDetect.AddHZtag(ithreshold, 0);
	else
		m_displayDetect.SetHZtagVal(0, ithreshold);
	
	// update spike channel displayed
	m_spkBarView.SetSpkList(m_pSpkListVSD);
	m_spkShapeView.SetSpkList(m_pSpkListVSD);
}

void CSpikeDetectionView::OnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	SerializeWindowsState (BSAVE, m_iDetectParms);
	int icursel = m_tabCtrl.GetCurSel();
	SerializeWindowsState (BRESTORE, icursel);
	UpdateDetectionSettings(icursel);
	*pResult = 0;
}

void CSpikeDetectionView::OnToolsEditstimulus()
{
	m_pspkDocVSD->SortStimArray();

	CEditStimArrayDlg dlg;
	dlg.m_pIntervalArrays.RemoveAll();
	dlg.m_pIntervalArrays.Add(&m_pspkDocVSD->m_stimIntervals);
	dlg.m_rate = m_samplingRate;
	dlg.m_pstimsaved = &GetDocument()->m_stimsaved;

	if (IDOK == dlg.DoModal())
	{
		//m_pspkDocVSD->m_stimIntervals.nitems=0;	// zero stimuli
		//m_pspkDocVSD->m_stimIntervals.intervalsArray.RemoveAll();

		//CIntervalsAndLevels* pSti = &(m_pspkDocVSD->m_stimIntervals);
		//CIntervalsAndLevels* pStiDlg = (CIntervalsAndLevels*) dlg.m_pIntervalArrays.GetAt(0);
		//for (int i=0; i< pStiDlg->intervalsArray.GetCount(); i++)
		//{
		//	pSti->intervalsArray.InsertAt(i, pStiDlg->intervalsArray[i]);
		//	m_pspkDocVSD->m_stimIntervals.nitems++;
		//}

		UpdateVTtags();
		m_spkBarView.Invalidate();
		m_displayDetect.Invalidate();
		m_displayData.Invalidate();
		m_pspkDocVSD->SetModifiedFlag(TRUE);
	}
}

void CSpikeDetectionView::OnEnChangeChanselected()
{
	if (!mm_ichanselected.m_bEntryDone)
		return;

	switch (mm_ichanselected.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ichanselected++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ichanselected--;
		break;
	}
	SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
}

void CSpikeDetectionView::OnEnChangeChanselected2()
{
	if (!mm_ichanselected2.m_bEntryDone)
		return;

	switch (mm_ichanselected2.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ichanselected2++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ichanselected2--;
		break;
	}
	SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
}

void CSpikeDetectionView::OnCbnSelchangeTransform2()
{
	int method = m_CBtransform2.GetCurSel();					// this is the extract method requested
	
	// get parameters from document
	CAcqDataDoc* pDatDoc = GetDocument()->m_pDat;

	int const docChan = m_pSpkListVSD->GetextractChan();		// source channel	
	short const nchans = pDatDoc->GetpWaveFormat()->scan_count;	// number of data chans / source buffer
	short const offset = 1; //(method>0) ? 1 : nchans;				// offset between points / detection
	short const nspan = pDatDoc->GetTransfDataSpan(method);		// nb pts to read before transf
	m_pSpkListVSD->SetextractTransform(method);					// save transform method

	// pre-load data
	int spikelen = m_pSpkListVSD->GetSpikeLength();
	int spkpretrig = m_pSpkListVSD->GetSpikePretrig();
	long iitime = m_pSpkListVSD->GetSpikeTime(0) -spkpretrig;
	long lRWFirst0 = iitime - spikelen;
	long lRWLast0 = iitime + spikelen;
	if (!pDatDoc->LoadRawData(&lRWFirst0, &lRWLast0, nspan))
		return;													// exit if error reported
	short* pData = pDatDoc->LoadTransfData(lRWFirst0, lRWLast0, method,	docChan);

	// loop over all spikes now
	int totalspikes = m_pSpkListVSD->GetTotalSpikes();
	for (int ispk = 0; ispk < totalspikes; ispk++)
	{
		// make sure that source data are loaded and get pointer to it (pData)
		iitime = m_pSpkListVSD->GetSpikeTime(ispk);
		long lRWFirst = iitime -spkpretrig;						// first point 
		long lRWLast = lRWFirst + spikelen;						// last pt needed
		if (!pDatDoc->LoadRawData(&lRWFirst, &lRWLast, nspan))
			break;												// exit loop if error reported

		pData = pDatDoc->LoadTransfData(lRWFirst, lRWLast, method, docChan);
		short* pDataSpike0 = pData +(iitime -spkpretrig -lRWFirst)*offset;
		m_pSpkListVSD->SetSpikeData(ispk, pDataSpike0, offset);  // nchans should be 1 if they come from the transform buffer as data are not interleaved...
		m_pSpkListVSD->CenterSpikeAmplitude(ispk, 0, spikelen, 1); // 1=center average
	}
	m_pspkDocVSD->SetModifiedFlag(TRUE);
	
	int max, min;
	m_pSpkListVSD->GetTotalMaxMin(TRUE, &max, &min);
	int middle = (max + min)/2;
	m_spkShapeView.SetYWExtOrg(m_spkShapeView.GetYWExtent(), middle);
	m_spkBarView.SetYWExtOrg(m_spkShapeView.GetYWExtent(), middle);
		UpdateSpkShapeWndScale(FALSE);


	HighlightSpikes(TRUE);								// highlight spikes	
	UpdateLegends();
	UpdateTabs();
}

void CSpikeDetectionView::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	UpdateDetectionSettings(icursel);
	*pResult = 0;
}

void CSpikeDetectionView::UpdateTabs()
{
	// load initial data
	BOOL bReplace = (m_tabCtrl.GetItemCount() == m_pspkDocVSD->GetSpkListSize());
	if (!bReplace)
		m_tabCtrl.DeleteAllItems();

	// load list of detection parameters 
	int currlist = m_pspkDocVSD->GetSpkListCurrentIndex();
	for (int i = 0; i< m_pspkDocVSD->GetSpkListSize(); i++)
	{
		CString cs;
		CSpikeList* pspklist = m_pspkDocVSD->SetSpkListCurrent(i);
		cs.Format(_T("#%i %s"), i, (LPCTSTR) pspklist->GetComment());
		if (!bReplace)
			m_tabCtrl.InsertItem(i, cs);
		else
		{
			TCITEM TabCtrlItem;
			TabCtrlItem.mask = TCIF_TEXT;
			LPTSTR lptszStr = cs.GetBuffer(0);
			TabCtrlItem.pszText = lptszStr;
			m_tabCtrl.SetItem(i, &TabCtrlItem);
			cs.ReleaseBuffer();
		}
	}
	m_pspkDocVSD->SetSpkListCurrent(currlist);

	////////////////////
	m_iDetectParms = GetDocument()->GetcurrentSpkListIndex();
	m_tabCtrl.SetCurSel(m_iDetectParms);
}