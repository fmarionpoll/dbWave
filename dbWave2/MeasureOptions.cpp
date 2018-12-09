// CMeasureOptionsPage.cpp : implementation file
//
// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "resource.h"
#include "measureoptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureOptionsPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CMeasureVTtagsPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CMeasureHZtagsPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CMeasureResultsPage, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CMeasureVTtagsPage property page

CMeasureVTtagsPage::CMeasureVTtagsPage() : CPropertyPage(CMeasureVTtagsPage::IDD)
{
	m_nbtags = 0;
	m_nperiods = 0;
	m_period = 0.0f;
	m_timesec = 0.0f;
	m_timeshift = 0.0f;
	m_duration = 0.0f;
	m_plineview = nullptr;
}

CMeasureVTtagsPage::~CMeasureVTtagsPage()
{
}

void CMeasureVTtagsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ITEM,	m_index);
	DDX_Text(pDX, IDC_NBTAGS, m_nbtags);
	DDX_Text(pDX, IDC_NPERIODSEDIT, m_nperiods);
	DDX_Text(pDX, IDC_PERIOD, m_period);
	DDX_Text(pDX, IDC_TIMESEC, m_timesec);
	DDX_Text(pDX, IDC_TIMESHIFT, m_timeshift);
	DDX_Text(pDX, IDC_DURATION, m_duration);
}


BEGIN_MESSAGE_MAP(CMeasureVTtagsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_CHANGE(IDC_ITEM, OnEnChangeItem)
	ON_EN_CHANGE(IDC_TIMESEC, OnEnChangeTimesec)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_RADIO1, OnSetDuplicateMode)
	ON_EN_CHANGE(IDC_DURATION, OnEnChangeDuration)
	ON_EN_CHANGE(IDC_PERIOD, OnEnChangePeriod)
	ON_EN_CHANGE(IDC_NPERIODSEDIT, OnEnChangeNperiodsedit)
	ON_EN_CHANGE(IDC_TIMESHIFT, OnEnChangeTimeshift)
	ON_BN_CLICKED(IDC_BUTTON2, OnShiftTags)
	ON_BN_CLICKED(IDC_BUTTON1, OnAddTags)
	ON_BN_CLICKED(IDC_BUTTON3, OnDeleteSeries)
	ON_BN_CLICKED(IDC_RADIO2, OnSetDuplicateMode)
	ON_BN_CLICKED(IDC_BUTTON4, OnDeleteAll)
END_MESSAGE_MAP()

// get VT tag val and feed value into control edit
BOOL CMeasureVTtagsPage::GetVTtagVal(int index)
{
	BOOL flag = (m_nbtags > 0);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);		

	if (index < 0 || index >= m_nbtags)
		return FALSE;
	m_index=index;
	long lk = m_plineview->GetVTtagLval(m_index);
	m_timesec = ((float)lk)/m_samplingrate;	

	return TRUE;
}

// set buttons according to choice concerning how VT tags are generated
void CMeasureVTtagsPage::SetspacedTagsOptions()
{
	((CButton*) GetDlgItem(IDC_RADIO1))->SetCheck(m_pMO->bSetTagsforCompleteFile);
	((CButton*) GetDlgItem(IDC_RADIO2))->SetCheck(!m_pMO->bSetTagsforCompleteFile);
	// validate dependent edit box accordingly
	GetDlgItem(IDC_NPERIODSSTATIC)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
	GetDlgItem(IDC_NPERIODSEDIT)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
}
// get options from one button, set flag and modify related buttons accordingly
void CMeasureVTtagsPage::OnSetDuplicateMode() 
{
	m_pMO->bSetTagsforCompleteFile= ((CButton*) GetDlgItem(IDC_RADIO1))->GetCheck();	
	SetspacedTagsOptions();
}

// Cancel: delete tags series
void CMeasureVTtagsPage::OnCancel() 
{
	OnDeleteSeries();
	if (m_pMO->wOption != 0)
	{
		m_plineview->DelAllVTtags();
		if (m_pMO->wOption == 1)
			m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
	}
	CPropertyPage::OnCancel();
}

// OK - tell awave that parameters have changed
void CMeasureVTtagsPage::OnOK() 
{
	CTagList* ptaglist = m_pdatDoc->GetpVTtags();
	ptaglist->CopyTagList(m_plineview->GetVTtagList());
	m_pMO->bChanged=TRUE;
	if (m_pMO->wOption != 0)
	{
		m_plineview->DelAllVTtags();
		if (m_pMO->wOption == 1)
			m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
	}
	CPropertyPage::OnOK();
}

// init dialog
BOOL CMeasureVTtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	// set check button
	((CButton*) GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->bSaveTags);

	// save initial state of VTtags
	m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
	m_plineview->DelAllHZtags();	
	m_plineview->Invalidate();
	m_nbtags = m_plineview->GetNVTtags();
	GetVTtagVal(0);

	// subclassed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));	
	VERIFY(mm_timesec.SubclassDlgItem(IDC_TIMESEC, this));
	VERIFY(mm_duration.SubclassDlgItem(IDC_DURATION, this));
	VERIFY(mm_period.SubclassDlgItem(IDC_PERIOD, this));
	VERIFY(mm_nperiods.SubclassDlgItem(IDC_NPERIODSEDIT, this));
	VERIFY(mm_timeshift.SubclassDlgItem(IDC_TIMESHIFT, this));
	m_verylast = ((float)m_plineview->GetDocumentLast())/m_samplingrate;
	SetspacedTagsOptions();
	m_duration = m_pMO->duration;		// on/OFF duration (sec)
	m_period=m_pMO->period;				// period (sec)
	m_nperiods=m_pMO->nperiods;			// nb of duplicates
	m_timeshift=m_pMO->timeshift;		// shift tags
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// remove current VT tag pointed to by m_index
void CMeasureVTtagsPage::OnRemove() 
{
	if (m_index >= 0 && m_index < m_nbtags)
	{	
		m_plineview->DelVTtag(m_index);
		m_nbtags--;
	}
	if (m_index > m_nbtags-1)
		m_index = m_nbtags-1;
	m_plineview->Invalidate();

	GetVTtagVal(m_index);
	UpdateData(FALSE);
}

// save tags within the file on exit
void CMeasureVTtagsPage::OnCheck1() 
{	
	m_pMO->bSaveTags = ((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();
}

// change no of tag selected for edition
void CMeasureVTtagsPage::OnEnChangeItem() 
{
	if (!mm_index.m_bEntryDone)
		return;

	switch (mm_index.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_index++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_index--;	break;
	}
	mm_index.m_bEntryDone=FALSE;	// clear flag
	mm_index.m_nChar=0;			// empty buffer
	mm_index.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_index >= m_nbtags)
		m_index = m_nbtags-1;
	if (m_index < 0)
		m_index = 0;
	GetVTtagVal(m_index);
	UpdateData(FALSE);	
	return;
}

// change time index of tag currently edited
void CMeasureVTtagsPage::OnEnChangeTimesec() 
{
	if (!mm_timesec.m_bEntryDone)
		return;

	switch (mm_timesec.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_timesec++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_timesec--;	break;
	}
	mm_timesec.m_bEntryDone=FALSE;	// clear flag
	mm_timesec.m_nChar=0;			// empty buffer
	mm_timesec.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_timesec < 0)
		m_timesec = 0.0f;
	if (m_timesec >= m_verylast)
		m_timesec = m_verylast;
	UpdateData(FALSE);	
	long lk = (long)(m_timesec*m_samplingrate);
	if (m_index >= 0 && m_index < m_nbtags)
	{
		m_plineview->SetVTtagLval(m_index, lk);
		m_plineview->Invalidate();
	}
	return;
}

// change duration parameter - duration of the stimulation or interval
// interval between 2 consecutive tags defining a stimulation pulse
// tag(n) and tag(n+1)
void CMeasureVTtagsPage::OnEnChangeDuration() 
{
	if (!mm_duration.m_bEntryDone)
		return;

	switch (mm_duration.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_duration++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_duration--;	break;
	}
	mm_duration.m_bEntryDone=FALSE;	// clear flag
	mm_duration.m_nChar=0;			// empty buffer
	mm_duration.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_duration < 0.)
		m_duration = 0.0f;
	if (m_duration >= m_period)
		m_duration = m_period;
	m_pMO->duration = m_duration;
	UpdateData(FALSE);
	return;
}

// period - time between tag(n) and tag(n+2)
void CMeasureVTtagsPage::OnEnChangePeriod() 
{
	if (!mm_period.m_bEntryDone)
		return;

	switch (mm_period.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_period++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_period--;	break;
	}
	mm_period.m_bEntryDone=FALSE;	// clear flag
	mm_period.m_nChar=0;			// empty buffer
	mm_period.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_period < m_duration)
		m_period = m_duration;
	m_pMO->period= m_period;
	UpdateData(FALSE);
	return;
}

// number of periods to add
void CMeasureVTtagsPage::OnEnChangeNperiodsedit() 
{
	if (!mm_nperiods.m_bEntryDone)
		return;

	switch (mm_nperiods.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_nperiods++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_nperiods--;	break;
	}
	mm_nperiods.m_bEntryDone=FALSE;	// clear flag
	mm_nperiods.m_nChar=0;			// empty buffer
	mm_nperiods.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_nperiods < 1)
		m_nperiods = 1;
	m_pMO->nperiods=m_nperiods;
	UpdateData(FALSE);
	return;
}


// time interval to add to each tag
void CMeasureVTtagsPage::OnEnChangeTimeshift() 
{
	if (!mm_timeshift.m_bEntryDone)
		return;

	switch (mm_timeshift.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_timeshift++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_timeshift--;	break;
	}
	mm_timeshift.m_bEntryDone=FALSE;	// clear flag
	mm_timeshift.m_nChar=0;			// empty buffer
	mm_timeshift.SetSel(0, -1);		// select all text    
	// update dependent parameters
	m_pMO->timeshift = m_timeshift;
	UpdateData(FALSE);
	return;
}


/////////////////////////////////////////////////////////////////////////////
// execute

void CMeasureVTtagsPage::OnShiftTags() 
{
	long loffset = (long)(m_timeshift*m_samplingrate);
	for (int i=0; i< m_nbtags; i++)
		m_plineview->SetVTtagLval(i, m_plineview->GetVTtagLval(i) + loffset);
	// update data
	m_plineview->Invalidate();
	GetVTtagVal(m_index);
	UpdateData(FALSE);
}

// add n tags according to options (duration, period, n intervals)
void CMeasureVTtagsPage::OnAddTags() 
{
	float time;
	float time_end;

	// compute limits
	if (!m_pMO->bSetTagsforCompleteFile)
	{
		m_nbtags = m_plineview->GetNVTtags();
		time=m_plineview->GetVTtagLval(m_nbtags-1)/m_samplingrate;
		time_end = m_period * ((float) m_nperiods) + time;
		// delete this one which will be re-created within the loop
		m_plineview->DelVTtag(m_nbtags-1);
		m_nbtags--;
		
	}
	// total file, start at zero
	else
	{
		time = 0.0f;
		time_end = m_verylast;
	}

	// loop until requested interval is completed
	float time0= time;
	float nintervals=0.0f;
	while (time <= time_end)
	{		
		m_plineview->AddVTLtag((long) (time*m_samplingrate));
		m_plineview->AddVTLtag((long) ((time+m_duration)*m_samplingrate));		
		nintervals++;
		time= time0 + m_period*nintervals;
	}
	m_nbtags += (int) nintervals*2;
	m_plineview->Invalidate();
	UpdateData(FALSE);
}

// delete series of points added during edit session
void CMeasureVTtagsPage::OnDeleteSeries()
{
	// delete present tags	
	CTagList* ptags = m_pdatDoc->GetpVTtags();
	m_plineview->SetVTtagList(ptags);
	m_nbtags = ptags->GetNTags();	
	m_plineview->Invalidate();
}

// delete all VT tags
void CMeasureVTtagsPage::OnDeleteAll() 
{
	m_plineview->DelAllVTtags();
	m_plineview->Invalidate();
	m_nbtags=0;
	GetVTtagVal(0);
	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureOptionsPage property page

CMeasureOptionsPage::CMeasureOptionsPage() : CPropertyPage(CMeasureOptionsPage::IDD)
{
	m_bExtrema = FALSE;
	m_bDiffExtrema = FALSE;
	m_bDiffDatalimits = FALSE;
	m_bHalfrisetime = FALSE;
	m_bHalfrecovery = FALSE;
	m_bDatalimits = FALSE;
	m_uiSourceChan = 0;
	m_wStimulusthresh = 0;
	m_wStimuluschan = 0;
	m_bAllFiles = FALSE;
	m_uiSourceChan = 0;
	m_fStimulusoffset = 0.0f;
	m_uiStimulusThreshold = 0;
	m_uiStimuluschan = 0;
	m_bLimitsval = false;
}

CMeasureOptionsPage::~CMeasureOptionsPage()
{
}

void CMeasureOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECKEXTREMA, m_bExtrema);
	DDX_Check(pDX, IDC_CHECKDIFFERENCE, m_bDiffExtrema);
	DDX_Check(pDX, IDC_CHECK2, m_bDiffDatalimits);
	DDX_Check(pDX, IDC_CHECKRISETIME, m_bHalfrisetime);
	DDX_Check(pDX, IDC_CHECKRECOVERYTIME, m_bHalfrecovery);
	DDX_Check(pDX, IDC_CHECKATLIMITS, m_bDatalimits);
	DDX_Check(pDX, IDC_APPLYTOALLFILES, m_bAllFiles);
	DDX_Text(pDX, IDC_SOURCECHANNEL, m_uiSourceChan);
	DDX_Text(pDX, IDC_TIMEOFFSET, m_fStimulusoffset);
	DDX_Text(pDX, IDC_STIMULUSTHRESHOLD, m_uiStimulusThreshold);
	DDX_Text(pDX, IDC_STIMULUSCHANNEL, m_uiStimuluschan);
}


BEGIN_MESSAGE_MAP(CMeasureOptionsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ALLCHANNELS, OnAllchannels)
	ON_BN_CLICKED(IDC_SINGLECHANNEL, OnSinglechannel)
	ON_BN_CLICKED(IDC_VERTICALTAGS, OnVerticaltags)
	ON_BN_CLICKED(IDC_HORIZONTALTAGS, OnHorizontaltags)
	ON_BN_CLICKED(IDC_STIMULUSTAG, OnStimulustag)
END_MESSAGE_MAP()

// display groups of controls as visible or disabled

void CMeasureOptionsPage::ShowLimitsParms(BOOL bShow)
{
	GetDlgItem(IDC_STIMULUSCHANNEL)->EnableWindow(bShow);
	GetDlgItem(IDC_STIMULUSTHRESHOLD)->EnableWindow(bShow);
	GetDlgItem(IDC_TIMEOFFSET)->EnableWindow(bShow);
}

void CMeasureOptionsPage::ShowChanParm(BOOL bShow)
{
	GetDlgItem(IDC_SOURCECHANNEL)->EnableWindow(bShow);
}

void CMeasureOptionsPage::OnAllchannels() 
{
	ShowChanParm(FALSE);
}

void CMeasureOptionsPage::OnSinglechannel() 
{
	ShowChanParm(TRUE);
}

void CMeasureOptionsPage::OnVerticaltags() 
{
	m_plineview->DelAllHZtags();
	m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
	m_plineview->Invalidate();
	ShowLimitsParms(FALSE);
}

void CMeasureOptionsPage::OnHorizontaltags() 
{
	m_plineview->DelAllVTtags();
	m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
	m_plineview->Invalidate();
	ShowLimitsParms(FALSE);
}

void CMeasureOptionsPage::OnStimulustag() 
{
	ShowLimitsParms(TRUE);
}

// save options
void CMeasureOptionsPage::SaveOptions()
{
	m_pMO->bExtrema= m_bExtrema;
	m_pMO->bDiffExtrema= m_bDiffExtrema;
	m_pMO->bDiffDatalimits= m_bDiffDatalimits;
	m_pMO->bHalfrisetime= m_bHalfrisetime;
	m_pMO->bHalfrecovery= m_bHalfrecovery;
	m_pMO->bDatalimits= m_bDatalimits;
	m_pMO->wSourceChan= m_uiSourceChan;
	m_pMO->wStimulusthresh= m_uiStimulusThreshold;
	m_pMO->wStimuluschan= m_uiStimuluschan;
	m_pMO->fStimulusoffset =m_fStimulusoffset;
	m_pMO->bAllFiles=m_bAllFiles;
	m_pMO->btime=((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();

	int iID = GetCheckedRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG);
	switch (iID)
	{
	case IDC_VERTICALTAGS:
		iID = 0;
		m_plineview->DelAllHZtags();
		m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
		break;
	case IDC_HORIZONTALTAGS:
		iID = 1;
		m_plineview->DelAllVTtags();
		m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
		break;
	case IDC_RECTANGLETAG:		iID = 2; break;
	case IDC_STIMULUSTAG:		iID = 3; break;
	default:					iID = 2; break;
	}
	m_pMO->wOption = iID;	
	m_pMO->bAllChannels = ((CButton*) GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
	m_pMO->bChanged=TRUE;
}

BOOL CMeasureOptionsPage::OnKillActive() 
{
	UpdateData(TRUE);
	SaveOptions();
	return CPropertyPage::OnKillActive();
}

void CMeasureOptionsPage::OnOK() 
{
	UpdateData(TRUE);
	SaveOptions();
	CPropertyPage::OnOK();
}

BOOL CMeasureOptionsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_bExtrema =m_pMO->bExtrema;
	m_bDiffExtrema =m_pMO->bDiffExtrema;
	m_bDiffDatalimits = m_pMO->bDiffDatalimits;
	m_bHalfrisetime =m_pMO->bHalfrisetime;
	m_bHalfrecovery =m_pMO->bHalfrecovery;
	m_bDatalimits =m_pMO->bDatalimits;
	m_uiSourceChan =m_pMO->wSourceChan;
	m_uiStimulusThreshold =m_pMO->wStimulusthresh;
	m_uiStimuluschan =m_pMO->wStimuluschan;
	m_fStimulusoffset =m_pMO->fStimulusoffset;
	m_bAllFiles =m_pMO->bAllFiles;
	int iID;
	BOOL flag = FALSE;	
	switch (m_pMO->wOption)
	{
	case 0:		iID = IDC_VERTICALTAGS; break;
	case 1:		iID = IDC_HORIZONTALTAGS; break;
	case 2: 	iID = IDC_RECTANGLETAG; break;
	case 3:		iID = IDC_STIMULUSTAG; flag = TRUE; break;
	default:	iID = IDC_VERTICALTAGS; break;
	}
	ShowLimitsParms(flag);

	int iID1 = IDC_ALLCHANNELS;
	flag = FALSE;
	if (!m_pMO->bAllChannels)
	{
		iID1=IDC_SINGLECHANNEL;
		flag =TRUE;
	}
	ShowChanParm(flag);

	CheckRadioButton(IDC_ALLCHANNELS, IDC_SINGLECHANNEL, iID1);
	CheckRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG, iID);
	((CButton*) GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->btime);

	GetDlgItem(IDC_CHECKRISETIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECKRECOVERYTIME)->EnableWindow(FALSE);
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureHZtagsPage property page

CMeasureHZtagsPage::CMeasureHZtagsPage() : CPropertyPage(CMeasureHZtagsPage::IDD)
{
	m_datachannel = 0;
	m_index = 0;
	m_mvlevel = 0.0f;
	m_nbcursors = 0;
	m_plineview = nullptr;
}

CMeasureHZtagsPage::~CMeasureHZtagsPage()
{
}

void CMeasureHZtagsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATACHANNEL, m_datachannel);
	DDX_Text(pDX, IDC_INDEX, m_index);
	DDX_Text(pDX, IDC_MVLEVEL, m_mvlevel);
	DDX_Text(pDX, IDC_NBCURSORS, m_nbcursors);
}


BEGIN_MESSAGE_MAP(CMeasureHZtagsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CENTER, OnCenter)
	ON_BN_CLICKED(IDC_ADJUST, OnAdjust)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_CHANGE(IDC_DATACHANNEL, OnEnChangeDatachannel)
	ON_EN_CHANGE(IDC_INDEX, OnEnChangeIndex)
	ON_EN_CHANGE(IDC_MVLEVEL, OnEnChangeMvlevel)
	ON_BN_CLICKED(IDC_BUTTON4, OnDeleteAll)
END_MESSAGE_MAP()

BOOL CMeasureHZtagsPage::GetHZcursorVal(int index)
{
	BOOL flag = (m_nbcursors > 0 && index < m_nbcursors && index >= 0);
	GetDlgItem(IDC_ADJUST)->EnableWindow(flag);
	GetDlgItem(IDC_CENTER)->EnableWindow(flag);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);
	
	if (index < 0 || index >= m_nbcursors)
		return FALSE;
	m_index=index;
	m_datachannel=m_plineview->GetHZtagChan(index);
	int k = m_plineview->GetHZtagVal(m_index);
	m_mvlevel = m_plineview->ConvertChanlistDataBinsToMilliVolts(m_datachannel, k);
	
	return TRUE;
}

void CMeasureHZtagsPage::OnCenter() 
{
	int max, min;
	m_plineview->GetChanlistMaxMin(m_datachannel, &max, &min);
	int val = (max+min)/2;
	m_plineview->SetHZtagVal(m_index, val);
	m_plineview->Invalidate();
	m_mvlevel = m_plineview->ConvertChanlistDataBinsToMilliVolts(m_datachannel, val);
	UpdateData(FALSE);
}


void CMeasureHZtagsPage::OnRemove() 
{
	if (m_index >= 0 && m_index < m_nbcursors)
	{
		m_plineview->DelHZtag(m_index);    
		m_nbcursors--;
	}
	if (m_index > m_nbcursors-1)
		m_index = m_nbcursors;
	GetHZcursorVal(m_index);
	m_plineview->Invalidate();
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::OnEnChangeDatachannel() 
{
	if (!mm_datachannel.m_bEntryDone)
		return;

	switch (mm_datachannel.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_datachannel++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_datachannel--;	break;
		}
	mm_datachannel.m_bEntryDone=FALSE;	// clear flag
	mm_datachannel.m_nChar=0;			// empty buffer
	mm_datachannel.SetSel(0, -1);		// select all text    

	// update dependent parameters
	if (m_datachannel < 0)
		m_datachannel = 0;		
	if (m_datachannel >= m_plineview->GetChanlistSize())
		m_datachannel = m_plineview->GetChanlistSize()-1;
	if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
		{		
		m_plineview->SetHZtagChan(m_index, m_datachannel);
		m_plineview->Invalidate();
		}
	UpdateData(FALSE);	
	return;
	
}

void CMeasureHZtagsPage::OnEnChangeIndex() 
{
	if (!mm_index.m_bEntryDone)
		return;

	switch (mm_index.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_index++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_index--;	break;
	}
	mm_index.m_bEntryDone=FALSE;	// clear flag
	mm_index.m_nChar=0;			// empty buffer
	mm_index.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_index >= m_nbcursors)
		m_index = m_nbcursors-1;
	if (m_index < 0)
		m_index = 0;
	GetHZcursorVal(m_index);
	UpdateData(FALSE);	
	return;
}

void CMeasureHZtagsPage::OnEnChangeMvlevel() 
{
	if (!mm_mvlevel.m_bEntryDone)
		return;

	switch (mm_mvlevel.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_mvlevel++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_mvlevel--;	break;
	}
	mm_mvlevel.m_bEntryDone=FALSE;	// clear flag
	mm_mvlevel.m_nChar=0;			// empty buffer
	mm_mvlevel.SetSel(0, -1);		// select all text    

	// update dependent parameters	
	UpdateData(FALSE);
	if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
	{
		int val = m_plineview->ConvertChanlistVoltstoDataBins(m_datachannel, m_mvlevel/1000.0f);
		m_plineview->SetHZtagVal(m_index, val);
		m_plineview->Invalidate();
	}
	return;
}

void CMeasureHZtagsPage::OnAdjust() 
{
	int max, min;
	m_plineview->GetChanlistMaxMin(m_datachannel, &max, &min);
	// get nb cursors / m_datachannel
	int ncur=0;
	for (int i= m_nbcursors-1; i>= 0; i--)
		if (m_plineview->GetHZtagChan(i) == m_datachannel)
			ncur++;

	// then split cursors across m_datachannel span
	if (ncur == 1)
	{
		OnCenter();
		return;
	}
	
	int dv = (max-min)/(ncur-1);
	int val = min;
	for (int i= 0; i< m_nbcursors; i++)
	{
		if (m_plineview->GetHZtagChan(i) == m_datachannel)
		{
			m_plineview->SetHZtagVal(i, val);
			val+= dv;
		}
	}
	m_plineview->Invalidate();
	val = m_plineview->GetHZtagVal(m_index);
	m_mvlevel = m_plineview->ConvertChanlistDataBinsToMilliVolts(m_datachannel, val);
	UpdateData(FALSE);
}


void CMeasureHZtagsPage::OnOK() 
{
	CTagList* ptaglist = m_pdatDoc->GetpHZtags();
	ptaglist->CopyTagList(m_plineview->GetHZtagList());
	m_pMO->bChanged=TRUE;
	if (m_pMO->wOption != 1)
	{
		m_plineview->DelAllHZtags();
		if (m_pMO->wOption == 0)
			m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
	}
	CPropertyPage::OnOK();
}

void CMeasureHZtagsPage::OnCancel() 
{
	// restore initial state of HZcursors
	if (m_pMO->wOption != 1)
	{
		m_plineview->DelAllHZtags();
		if (m_pMO->wOption == 0)
			m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
	}
	else
		m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
	m_plineview->Invalidate();

	CPropertyPage::OnCancel();
}

BOOL CMeasureHZtagsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
	m_plineview->DelAllVTtags();
	m_plineview->Invalidate();
	m_nbcursors=m_plineview->GetNHZtags();
	GetHZcursorVal(0);

	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_datachannel.SubclassDlgItem(IDC_DATACHANNEL, this));
	VERIFY(mm_mvlevel.SubclassDlgItem(IDC_MVLEVEL, this));
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureHZtagsPage::OnDeleteAll() 
{
	m_plineview->DelAllHZtags();
	m_plineview->Invalidate();
	m_nbcursors=0;
	GetHZcursorVal(0);
	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage property page

CMeasureResultsPage::CMeasureResultsPage() : CPropertyPage(CMeasureResultsPage::IDD)
{

	m_plineview = nullptr;
}

CMeasureResultsPage::~CMeasureResultsPage()
{
}

void CMeasureResultsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESULTS, m_CEditResults);
	DDX_Control(pDX, IDC_LIST1, m_listResults);
}


BEGIN_MESSAGE_MAP(CMeasureResultsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
END_MESSAGE_MAP()


void CMeasureResultsPage::OnExport() 
{
	CString csBuffer;
	// copy results from CListCtrl into text buffer
	m_CEditResults.GetWindowText(csBuffer);
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();

	CMultiDocTemplate* pTempl = pApp->m_pNoteViewTemplate;
	CDocument* pDoc = pTempl->OpenDocumentFile(nullptr);
	POSITION pos = pDoc->GetFirstViewPosition();
	CRichEditView* pView = (CRichEditView*)pDoc->GetNextView(pos);
	CRichEditCtrl& pEdit = pView->GetRichEditCtrl();	
	pEdit.SetWindowText(csBuffer);		// copy content of window into CString
}

void CMeasureResultsPage::OutputTitle()
{
	// prepare clistctrl column headers
	int icol = m_listResults.InsertColumn(0, _T("#"),  LVCFMT_LEFT, 20, 0)+1;
	
	// column headers for vertical tags	
	m_csTitle = _T("#");
	CString csCols= _T("");	
	m_nbdatacols=0;

	switch (m_pMO->wOption)
	{
	// ......................  vertical tags
	case 0:
		if (m_pMO->bDatalimits)
		{			
			if (!m_pMO->btime)	csCols += _T("\tt1(mV)\tt2(mV)");
			else				csCols += _T("\tt1(mV)\tt1(s)\tt2(mV)\tt2(s)");
		}
		if (m_pMO->bDiffDatalimits)
		{
			csCols += _T("\tt2-t1(mV)");
			if (m_pMO->btime)	csCols += _T("\tt2-t1(s)");
		}
		if (m_pMO->bExtrema)
		{
			if (!m_pMO->btime)	csCols += _T("\tMax(mV)\tmin(mV)");
			else				csCols += _T("\tMax(mV)\tMax(s)\tmin(mV)\tmin(s)");
		}
		if (m_pMO->bDiffExtrema)
		{
			csCols += _T("\tDiff(mV)");
			if (m_pMO->btime)	csCols += _T("\tdiff(s)");
		}
		if (m_pMO->bHalfrisetime)	csCols += _T("\t1/2rise(s)");
		if (m_pMO->bHalfrecovery)	csCols += _T("\t1/2reco(s)");
		break;

	// ......................  horizontal cursors
	case 1:
		if (m_pMO->bDatalimits)		csCols += _T("\tv1(mV)\tv2(mV)");
		if (m_pMO->bDiffDatalimits)	csCols += _T("\tv2-v1(mV)");
		break;

	// ......................  rectangle area
	case 2:	
		break;
	// ......................  detect stimulus and then measure
	case 3:
		break;
	default:
		break;
	}

	// now set columns - get nb of data channels CString
	if (!csCols.IsEmpty())
	{
		int ichan0=0;									// assume all data channels
		int ichan1=m_plineview->GetChanlistSize()-1;	// requested
		int npix_mV = (m_listResults.GetStringWidth(_T("0.000000"))*3)/2;

		if (!m_pMO->bAllChannels)					// else if flag set
		{											// restrict to a single chan
			ichan0=m_pMO->wSourceChan;				// single channel
			ichan1=ichan0;
		}
		m_nbdatacols=0;
		TCHAR* pszT = m_szT;		
		TCHAR seps[]  = _T("\t");		// separator = tab
		TCHAR* ptoken;				// pointer within pstring
		TCHAR* next_token= nullptr;
		
		for (int ichan=ichan0; ichan <= ichan1; ichan++)
		{
			CString cs = csCols;
			TCHAR* pstring = cs.GetBuffer(cs.GetLength()+1);
			pstring++;					// skip first tab
			ptoken = _tcstok_s( pstring, seps, &next_token );
			while (ptoken != nullptr)
			{
				if (m_pMO->bAllChannels)
					wsprintf(m_szT, _T("ch%i-%s"), ichan, ptoken);
				else
					pszT = ptoken;
				icol = 1+ m_listResults.InsertColumn(icol, pszT, LVCFMT_LEFT, npix_mV, icol);
				m_csTitle += seps;
				m_csTitle += pszT;
				ptoken = _tcstok_s(nullptr, seps, &next_token );	// get next token
			}
			
			if (m_nbdatacols == 0)		// number of data columns
				m_nbdatacols=icol-1;
			cs.ReleaseBuffer();			// release character buffer
		}
	}
	m_csTitle += _T("\r\n");
	return;
}


void CMeasureResultsPage::MeasureFromVTtags(int ichan)
{	
	int ntags = m_plineview->GetNVTtags();
	int itag1=-1;
	int itag2=-1;	

	// assume that VT is ordered; measure only between pairs
	int line = 0;
	for (int i = 0; i< ntags; i++)
	{
		// search first tag
		if (itag1 < 0)
		{
			itag1 = i;
			continue;
		}
		// second tag found: measure
		if (itag2 < 0)
		{
			itag2 = i;
			long l1 = m_plineview->GetVTtagLval(itag1);
			long l2 = m_plineview->GetVTtagLval(itag2);
			MeasureWithinInterval(ichan, line, l1, l2);
			line++;			
			itag1 = -1;
			itag2 = -1;			
		}		
	}

	// cope with isolated tags
	if (itag1 > 0 && itag2 <0)
	{
		long l1 = m_plineview->GetVTtagLval(itag1);
		MeasureWithinInterval(ichan, line, l1, l1);
		line++;		
	}	
	return;
}

void CMeasureResultsPage::GetMaxMin(int ichan, long lFirst, long lLast)
{
	short* pData;
	int nchans;							// n raw channels
	short* pBuf= m_pdatDoc->LoadRawDataParams(&nchans);
	int sourceChan = m_plineview->GetChanlistSourceChan(ichan);
	int itransf = m_plineview->GetChanlistTransformMode(ichan);
	int nspan = m_pdatDoc->GetTransfDataSpan(itransf);

	// get first data point (init max and min)
	long lBUFchanFirst = lFirst;
	long lBUFchanLast  = lLast;	
	int offset;
	m_pdatDoc->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan);
	if (itransf>0)
	{
		pData = m_pdatDoc->LoadTransfData(lFirst, lBUFchanLast, itransf, sourceChan);
		offset = 1;
	}
	else
	{
		pData = pBuf + (lFirst-lBUFchanFirst)*nchans + sourceChan;
		offset = nchans;
	}
	m_max= *pData;
	m_min= m_max;	
	m_imax = lFirst;
	m_imin = m_imax;
	m_first = m_max;

	// loop through data points
	while (lFirst < lLast)
	{
		// load file data and get a pointer to these data
		lBUFchanFirst = lFirst;
		lBUFchanLast  = lLast;	
		m_pdatDoc->LoadRawData(&lBUFchanFirst, &lBUFchanLast, nspan);
		if (lLast < lBUFchanLast)
			lBUFchanLast = lLast;
		if (itransf>0)
			pData = m_pdatDoc->LoadTransfData(lFirst, lBUFchanLast, itransf, sourceChan);		
		else
			pData = pBuf + (lFirst-lBUFchanFirst)*nchans + sourceChan;
		// now search for max and min		
		for (int i= lFirst; i<= lBUFchanLast; i++)
		{
			m_last = *pData;
			pData += offset;
			if (m_last > m_max)
			{
				m_max = m_last;
				m_imax = i;
			}
			else if (m_last < m_min)
			{
				m_min = m_last;
				m_imin = i;
			}

		}
		// search ended, update variable
		lFirst = lBUFchanLast+1;
	}
}


// output results into CListCtrl
void CMeasureResultsPage::MeasureWithinInterval(int ichan, int line, long l1, long l2)
{
	// get scale factor for ichan and sampling rate
	m_mVperBin = m_plineview->GetChanlistVoltsperDataBin(ichan)*1000.0f;
	float rate = m_pdatDoc->GetpWaveFormat()->chrate;

	int icol = (m_col-1)*m_nbdatacols+1;		// output data into column icol
	int item = m_listResults.GetItemCount();	// compute which line will receive data	
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);		// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table	
	}
	else
		item=line;								// else set variable value

	// measure parameters / selected interval; save results within common vars
	GetMaxMin(ichan, l1, l2);
	
	// output data according to options : data value at limits
	//m_szT[0] = '\t';								// prepare string for edit output
	CString csdummy;
	float	xdummy;
	CString csFmt(_T("\t%f"));
	if (m_pMO->bDatalimits)
	{
		// tag1(mV)__tag1(s)__tag2(mV)__tag2(s)		
		//_gcvt_s(&m_szT[1], 63, ((float)m_first)*m_mVperBin, FLT_DIG );		
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)m_first)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, (((float)l1)/rate), FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)l1) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
		//_gcvt_s(&m_szT[1], 63, ((float)m_last)*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)m_last)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, ((float)l2)/rate, FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)l2) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
	}

	if (m_pMO->bDiffDatalimits)
	{
		//diff(mV)__diff(s)";
		//_gcvt_s(&m_szT[1], 63, ((float)(m_last-m_first))*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)(m_last - m_first))*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, ((float)l2-l1)/rate, FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)l2 - l1) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
	}

	// measure max and min (val, time)
	if (m_pMO->bExtrema)
	{
		// max(mV)__max(s)__min(mV)__min(s)";	
		//_gcvt_s(&m_szT[1], 63, ((float)m_max)*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)m_max)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, ((float)m_imax)/rate, FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)m_imax) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
		//_gcvt_s(&m_szT[1], 63, ((float)m_min)*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)m_min)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, ((float)m_imin)/rate, FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)m_imin) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
	}

	// difference between extrema (val, time)
	if (m_pMO->bDiffExtrema)
	{
		//diff(mV)__diff(s)";
		//_gcvt_s(&m_szT[1], 63, ((float)(m_max-m_min))*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)(m_max - m_min))*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
		if (m_pMO->btime)
		{
			//_gcvt_s(&m_szT[1], 63, ((float)(m_imax-m_imin))/rate, FLT_DIG);
			//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
			xdummy = ((float)(m_imax - m_imin)) / rate;
			csdummy.Format(csFmt, xdummy);
			m_listResults.SetItemText(item, icol, csdummy); icol++;
		}
	}

	// time necessary to reach half of the amplitude
	if (m_pMO->bHalfrisetime)
	{
	}

	// time necessary to regain half of the initial amplitude
	if (m_pMO->bHalfrecovery)
	{
	}
}

void CMeasureResultsPage::MeasureFromHZcur(int ichan)
{	
	int ntags = m_plineview->GetNHZtags();
	int itag1=-1;
	int itag2=-1;

	// assume that HZ is ordered; measure only between pairs
	int line = 0;
	for (int i = 0; i< ntags; i++)
	{
		// search first tag
		if (itag1 < 0)
		{
			itag1 = i;
			continue;
		}
		// second tag found: measure
		if (itag2 < 0)
		{
			itag2 = i;
			int v1 = m_plineview->GetHZtagVal(itag1);
			int v2 = m_plineview->GetHZtagVal(itag2);
			MeasureBetweenHZ(ichan, line, v1, v2);
			line++;
			itag1 = -1;
			itag2 = -1;			
		}		
	}

	// cope with isolated tags
	if (itag1 > 0 && itag2 <0)
	{
		int v1 = m_plineview->GetHZtagVal(itag1);
		MeasureBetweenHZ(ichan, line, v1, v1);
		line++;
	}	
}

// output results both into CEdit control (via pCopy) and within CListCtrl
void CMeasureResultsPage::MeasureBetweenHZ(int ichan, int line, int v1, int v2)
{
	// get scale factor for ichan and sampling rate
	m_mVperBin = m_plineview->GetChanlistVoltsperDataBin(ichan)*1000.0f;

	int icol = (m_col-1)*m_nbdatacols+1;		// output data into column icol
	int item = m_listResults.GetItemCount();	// compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);	// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table
	}
	else
		item=line;								// else set variable value

	// measure parameters / selected interval; save results within common vars	
	//m_szT[0]='\t';								// prepare string for edit output	
	CString csdummy;
	float	xdummy;
	CString csFmt(_T("\t%f"));
	
	// output data according to options : data value at limits
	if (m_pMO->bDatalimits)
	{
		// tag1(mV)__tag1(s)__tag2(mV)__tag2(s)
		//_gcvt_s(&m_szT[1], 63, ((float)v1)*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)v1)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;

/*		_gcvt_s(&m_szT[1], 63, ((float)v2)*m_mVperBin, FLT_DIG);
		m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;*/	
		xdummy = ((float)v2)*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
	}

	if (m_pMO->bDiffDatalimits)
	{
		//diff(mV)__diff(s)";
		//_gcvt_s(&m_szT[1], 63, ((float)(v2-v1))*m_mVperBin, FLT_DIG);
		//m_listResults.SetItemText(item, icol, &m_szT[1]); icol++;
		xdummy = ((float)(v2 - v1))*m_mVperBin;
		csdummy.Format(csFmt, xdummy);
		m_listResults.SetItemText(item, icol, csdummy); icol++;
	}

	return;
}

void CMeasureResultsPage::MeasureFromRect(int ichan)
{
}

void CMeasureResultsPage::MeasureFromStim(int ichan)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage message handlers

BOOL CMeasureResultsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	MeasureParameters();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// measure parameters either from current file or from the entire series
BOOL CMeasureResultsPage::MeasureParameters()
{
	// compute file indexes
	int currentfileindex = m_pdbDoc->DBGetCurrentRecordPosition();
	ASSERT(currentfileindex >= 0);
	int ifirst = currentfileindex;
	int ilast = ifirst;	
	if (m_pMO->bAllFiles)
	{
		ifirst = 0;
		ilast = m_pdbDoc->DBGetNRecords()-1;
	}

	// prepare listcontrol
	while (m_listResults.DeleteColumn(0) != 0);
		m_listResults.DeleteAllItems();

	// prepare clipboard and Edit control (CEdit)
	HANDLE hCopy= nullptr;
	if (OpenClipboard())
	{
		EmptyClipboard();		// prepare clipboard and copy text to buffer
		DWORD dwLen = 32768;	// 32 Kb
		HANDLE hCopy = (HANDLE) ::GlobalAlloc (GHND, dwLen);
		if (hCopy == nullptr)
		{
			AfxMessageBox(_T("Memory low: unable to allocate memory"));
			return TRUE;
		}
		TCHAR* pCopy = (TCHAR*) ::GlobalLock((HGLOBAL) hCopy);
		TCHAR* pCopy0= pCopy;
		
		// export Ascii: begin //////////////////////////////////////////////
		BeginWaitCursor();
		OutputTitle();
		m_pdbDoc->DBSetCurrentRecordPosition(ifirst);
		m_pdbDoc->DBGetCurrentDatFileName();
		OPTIONS_VIEWDATA* pVD = new OPTIONS_VIEWDATA;
		ASSERT(pVD != NULL);
		CString csOut=_T("**filename\tdate\ttime\tcomment\tchannel\r\n");

		for (int i=ifirst; i<=ilast; i++, m_pdbDoc->DBMoveNext())
		{
			// open data file			
			m_pdbDoc->OpenCurrentDataFile();
			
			pVD->bacqcomments=TRUE;		// global comment
			pVD->bacqdate=TRUE;			// acquisition date
			pVD->bacqtime=TRUE;			// acquisition time
			pVD->bfilesize=FALSE;		// file size
			pVD->bacqchcomment=FALSE;	// acq channel indiv comment
			pVD->bacqchsetting=FALSE;	// acq chan indiv settings (gain, filter, etc)
			CString cs = csOut;
			cs += m_pdbDoc->m_pDat->GetDataFileInfos(pVD);
			pCopy += wsprintf(pCopy, _T("%s\r\n"), (LPCTSTR) cs);
	
			// output title for this data set
			int ichan0=0;
			int ichan1=m_plineview->GetChanlistSize()-1;
			if (!m_pMO->bAllChannels)
			{
				ichan0=m_pMO->wSourceChan;
				ichan1=ichan0;
			}
			if (ichan0 >= m_plineview->GetChanlistSize())
				ichan0 = 0;
			if (ichan1 >= m_plineview->GetChanlistSize())
				ichan1 = m_plineview->GetChanlistSize()-1;

			m_col=1;
			for (int ichan=ichan0; ichan <= ichan1; ichan++)
			{
				// measure according to option
				switch (m_pMO->wOption)
				{
				// ......................  vertical tags
				case 0:	MeasureFromVTtags(ichan); break;
				// ......................  horizontal cursors
				case 1:	MeasureFromHZcur(ichan); break;
				// ......................  rectangle area
				case 2:	MeasureFromRect(ichan); break;
				// ......................  detect stimulus and then measure
				case 3: MeasureFromStim(ichan); break;
				default:
					break;
				}
				m_col++;
			}

			// transfer content of clistctrl to clipboard
			pCopy += wsprintf(pCopy, _T("%s"), (LPCTSTR) m_csTitle);
			int nblines = m_listResults.GetItemCount();			
			int nbcols = m_nbdatacols*(m_col-1)+1;
			for (int item=0; item < nblines; item++)
			{
				for (int icol=0; icol< nbcols; icol++)
				{
					CString csContent = m_listResults.GetItemText(item, icol);
					pCopy += wsprintf(pCopy, _T("%s\t"), (LPCTSTR) csContent);
				}
				pCopy--; *pCopy = '\r'; pCopy++;
				*pCopy = '\n'; pCopy++;
			}
			// if not, then next file will override the results computed before			
		}

		*pCopy = 0;
		m_pdbDoc->DBSetCurrentRecordPosition(currentfileindex );
		m_pdbDoc->DBGetCurrentDatFileName();
		m_pdbDoc->OpenCurrentDataFile();
		EndWaitCursor();     // it's done

		// export Ascii: end //////////////////////////////////////////////
		m_CEditResults.SetWindowText(pCopy0);		
		::GlobalUnlock((HGLOBAL) hCopy);
		SetClipboardData (CF_TEXT, hCopy);				
		CloseClipboard();		// close connect w.clipboard
		delete pVD;				// delete temporary object

	}
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


// remove tags on exit
void CMeasureResultsPage::OnOK() 
{

	m_plineview->Invalidate();
	CPropertyPage::OnOK();
}


// measure parameters each time this page is selected
BOOL CMeasureResultsPage::OnSetActive() 
{
	MeasureParameters();
	return CPropertyPage::OnSetActive();
}
