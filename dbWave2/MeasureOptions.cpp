// CMeasureOptionsPage.cpp : implementation file
//
// TODO : measure data and output to notedocview

#include "StdAfx.h"
//#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "NoteDoc.h"
//#include "ViewNotedoc.h"
//#include "dbMainTable.h"
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

CMeasureVTtagsPage::CMeasureVTtagsPage() : CPropertyPage(CMeasureVTtagsPage::IDD), m_index(0), m_pMO(nullptr),
                                           m_pdbDoc(nullptr), m_pdatDoc(nullptr), m_samplingrate(0), m_verylast(0)
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
	const BOOL flag = (m_nbtags > 0);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);		

	if (index < 0 || index >= m_nbtags)
		return FALSE;
	m_index=index;
	const auto lk = m_plineview->GetVTtagLval(m_index);
	m_timesec = static_cast<float>(lk)/m_samplingrate;	

	return TRUE;
}

// set buttons according to choice concerning how VT tags are generated
void CMeasureVTtagsPage::SetspacedTagsOptions()
{
	dynamic_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(m_pMO->bSetTagsforCompleteFile);
	dynamic_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(!m_pMO->bSetTagsforCompleteFile);
	// validate dependent edit box accordingly
	GetDlgItem(IDC_NPERIODSSTATIC)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
	GetDlgItem(IDC_NPERIODSEDIT)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
}
// get options from one button, set flag and modify related buttons accordingly
void CMeasureVTtagsPage::OnSetDuplicateMode() 
{
	m_pMO->bSetTagsforCompleteFile= dynamic_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck();	
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
	auto p_tag_list = m_pdatDoc->GetpVTtags();
	p_tag_list->CopyTagList(m_plineview->GetVTtagList());
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
	dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->bSaveTags);

	// save initial state of VTtags
	//TODO bug here
	const auto p_vertical_tags_list = m_pdatDoc->GetpVTtags();
	if (p_vertical_tags_list != nullptr)
		m_plineview->SetVTtagList(p_vertical_tags_list);
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
	m_verylast = static_cast<float>(m_plineview->GetDocumentLast())/m_samplingrate;
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
	m_pMO->bSaveTags = dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
}

// change no of tag selected for edition
void CMeasureVTtagsPage::OnEnChangeItem() 
{
	if (mm_index.m_bEntryDone) {

		switch (mm_index.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_index++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_index--;	break;
		default:;
		}
		mm_index.m_bEntryDone = FALSE;	// clear flag
		mm_index.m_nChar = 0;			// empty buffer
		mm_index.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_index >= m_nbtags)
			m_index = m_nbtags - 1;
		if (m_index < 0)
			m_index = 0;
		GetVTtagVal(m_index);
		UpdateData(FALSE);
	}
}

// change time index of tag currently edited
void CMeasureVTtagsPage::OnEnChangeTimesec() 
{
	if (mm_timesec.m_bEntryDone) {

		switch (mm_timesec.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_timesec++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_timesec--;	break;
		default:;
		}
		mm_timesec.m_bEntryDone = FALSE;	// clear flag
		mm_timesec.m_nChar = 0;			// empty buffer
		mm_timesec.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_timesec < 0)
			m_timesec = 0.0f;
		if (m_timesec >= m_verylast)
			m_timesec = m_verylast;
		UpdateData(FALSE);
		auto const lk = static_cast<long>(m_timesec * m_samplingrate);
		if (m_index >= 0 && m_index < m_nbtags)
		{
			m_plineview->SetVTtagLval(m_index, lk);
			m_plineview->Invalidate();
		}
	}
}

// change duration parameter - duration of the stimulation or interval
// interval between 2 consecutive tags defining a stimulation pulse
// tag(n) and tag(n+1)
void CMeasureVTtagsPage::OnEnChangeDuration() 
{
	if (mm_duration.m_bEntryDone) {

		switch (mm_duration.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_duration++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_duration--;	break;
		default:;
		}
		mm_duration.m_bEntryDone = FALSE;	// clear flag
		mm_duration.m_nChar = 0;			// empty buffer
		mm_duration.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_duration < 0.)
			m_duration = 0.0f;
		if (m_duration >= m_period)
			m_duration = m_period;
		m_pMO->duration = m_duration;
		UpdateData(FALSE);
	}
}

// period - time between tag(n) and tag(n+2)
void CMeasureVTtagsPage::OnEnChangePeriod() 
{
	if (mm_period.m_bEntryDone) {

		switch (mm_period.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_period++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_period--;	break;
		default:;
		}
		mm_period.m_bEntryDone = FALSE;	// clear flag
		mm_period.m_nChar = 0;			// empty buffer
		mm_period.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_period < m_duration)
			m_period = m_duration;
		m_pMO->period = m_period;
		UpdateData(FALSE);
	}
}

// number of periods to add
void CMeasureVTtagsPage::OnEnChangeNperiodsedit() 
{
	if (mm_nperiods.m_bEntryDone) {

		switch (mm_nperiods.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_nperiods++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_nperiods--;	break;
		default:;
		}
		mm_nperiods.m_bEntryDone = FALSE;	// clear flag
		mm_nperiods.m_nChar = 0;			// empty buffer
		mm_nperiods.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_nperiods < 1)
			m_nperiods = 1;
		m_pMO->nperiods = m_nperiods;
		UpdateData(FALSE);
	}
}


// time interval to add to each tag
void CMeasureVTtagsPage::OnEnChangeTimeshift() 
{
	if (mm_timeshift.m_bEntryDone) {

		switch (mm_timeshift.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_timeshift++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_timeshift--;	break;
		default:;
		}
		mm_timeshift.m_bEntryDone = FALSE;	// clear flag
		mm_timeshift.m_nChar = 0;			// empty buffer
		mm_timeshift.SetSel(0, -1);		// select all text    
		// update dependent parameters
		m_pMO->timeshift = m_timeshift;
		UpdateData(FALSE);
	}
}


/////////////////////////////////////////////////////////////////////////////
// execute

void CMeasureVTtagsPage::OnShiftTags() 
{
	const auto offset = static_cast<long>(m_timeshift * m_samplingrate);
	for (auto i=0; i< m_nbtags; i++)
		m_plineview->SetVTtagLval(i, m_plineview->GetVTtagLval(i) + offset);
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
		time_end = m_period * static_cast<float>(m_nperiods) + time;
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
	const auto time0= time;
	auto n_intervals=0.0f;
	while (time <= time_end)
	{		
		m_plineview->AddVTLtag(static_cast<long>(time * m_samplingrate));
		m_plineview->AddVTLtag(static_cast<long>((time + m_duration) * m_samplingrate));		
		n_intervals++;
		time= time0 + m_period*n_intervals;
	}
	m_nbtags += static_cast<int>(n_intervals)*2;
	m_plineview->Invalidate();
	UpdateData(FALSE);
}

// delete series of points added during edit session
void CMeasureVTtagsPage::OnDeleteSeries()
{
	// delete present tags	
	auto p_tags_list = m_pdatDoc->GetpVTtags();
	m_plineview->SetVTtagList(p_tags_list);
	m_nbtags = p_tags_list->GetNTags();	
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

CMeasureOptionsPage::CMeasureOptionsPage() : CPropertyPage(CMeasureOptionsPage::IDD), m_wSourcechan(0), m_pMO(nullptr),
                                             m_pdbDoc(nullptr), m_pdatDoc(nullptr), m_plineview(nullptr)
{
	m_bExtrema = FALSE;
	m_bDiffExtrema = FALSE;
	m_bDiffDatalimits = FALSE;
	m_bHalfrisetime = FALSE;
	m_bHalfrecovery = FALSE;
	m_bDatalimits = FALSE;
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

void CMeasureOptionsPage::ShowLimitsParms(const BOOL b_show)
{
	GetDlgItem(IDC_STIMULUSCHANNEL)->EnableWindow(b_show);
	GetDlgItem(IDC_STIMULUSTHRESHOLD)->EnableWindow(b_show);
	GetDlgItem(IDC_TIMEOFFSET)->EnableWindow(b_show);
}

void CMeasureOptionsPage::ShowChanParm(const BOOL b_show)
{
	GetDlgItem(IDC_SOURCECHANNEL)->EnableWindow(b_show);
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
	m_pMO->btime=dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();

	auto i_id = GetCheckedRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG);
	switch (i_id)
	{
	case IDC_VERTICALTAGS:
		i_id = 0;
		m_plineview->DelAllHZtags();
		m_plineview->SetVTtagList(m_pdatDoc->GetpVTtags());
		break;
	case IDC_HORIZONTALTAGS:
		i_id = 1;
		m_plineview->DelAllVTtags();
		m_plineview->SetHZtagList(m_pdatDoc->GetpHZtags());
		break;
	case IDC_RECTANGLETAG:		i_id = 2; break;
	case IDC_STIMULUSTAG:		i_id = 3; break;
	default:					i_id = 2; break;
	}
	m_pMO->wOption = i_id;	
	m_pMO->bAllChannels = dynamic_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
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
	int i_id;
	auto flag = FALSE;	
	switch (m_pMO->wOption)
	{
	case 0:		i_id = IDC_VERTICALTAGS; break;
	case 1:		i_id = IDC_HORIZONTALTAGS; break;
	case 2: 	i_id = IDC_RECTANGLETAG; break;
	case 3:		i_id = IDC_STIMULUSTAG; flag = TRUE; break;
	default:	i_id = IDC_VERTICALTAGS; break;
	}
	ShowLimitsParms(flag);

	auto i_id1 = IDC_ALLCHANNELS;
	flag = FALSE;
	if (!m_pMO->bAllChannels)
	{
		i_id1=IDC_SINGLECHANNEL;
		flag =TRUE;
	}
	ShowChanParm(flag);

	CheckRadioButton(IDC_ALLCHANNELS, IDC_SINGLECHANNEL, i_id1);
	CheckRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG, i_id);
	dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->btime);

	//GetDlgItem(IDC_CHECKRISETIME)->EnableWindow(FALSE);
	//GetDlgItem(IDC_CHECKRECOVERYTIME)->EnableWindow(FALSE);
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureHZtagsPage property page

CMeasureHZtagsPage::CMeasureHZtagsPage() : CPropertyPage(CMeasureHZtagsPage::IDD), m_pMO(nullptr), m_pdbDoc(nullptr),
                                           m_pdatDoc(nullptr)
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

BOOL CMeasureHZtagsPage::GetHZcursorVal(const int index)
{
	const BOOL flag = (m_nbcursors > 0 && index < m_nbcursors && index >= 0);
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
	const auto val = (max+min)/2;
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
	if (mm_datachannel.m_bEntryDone) {

		switch (mm_datachannel.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_datachannel++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_datachannel--;	break;
		default: ;
		}
		mm_datachannel.m_bEntryDone = FALSE;	// clear flag
		mm_datachannel.m_nChar = 0;			// empty buffer
		mm_datachannel.SetSel(0, -1);		// select all text    

		// update dependent parameters
		if (m_datachannel < 0)
			m_datachannel = 0;
		if (m_datachannel >= m_plineview->GetChanlistSize())
			m_datachannel = m_plineview->GetChanlistSize() - 1;
		if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
		{
			m_plineview->SetHZtagChan(m_index, m_datachannel);
			m_plineview->Invalidate();
		}
		UpdateData(FALSE);
	}	
}

void CMeasureHZtagsPage::OnEnChangeIndex() 
{
	if (mm_index.m_bEntryDone) {

		switch (mm_index.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_index++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_index--;	break;
		default: ;
		}
		mm_index.m_bEntryDone = FALSE;	// clear flag
		mm_index.m_nChar = 0;			// empty buffer
		mm_index.SetSel(0, -1);		// select all text    
		// update dependent parameters
		if (m_index >= m_nbcursors)
			m_index = m_nbcursors - 1;
		if (m_index < 0)
			m_index = 0;
		GetHZcursorVal(m_index);
		UpdateData(FALSE);
	}
}

void CMeasureHZtagsPage::OnEnChangeMvlevel() 
{
	if (mm_mvlevel.m_bEntryDone) {

		switch (mm_mvlevel.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_mvlevel++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_mvlevel--;	break;
		default: ;
		}
		mm_mvlevel.m_bEntryDone = FALSE;	// clear flag
		mm_mvlevel.m_nChar = 0;			// empty buffer
		mm_mvlevel.SetSel(0, -1);		// select all text    

		// update dependent parameters	
		UpdateData(FALSE);
		if (m_nbcursors > 0 && m_index >= 0 && m_index < m_nbcursors)
		{
			const auto val = m_plineview->ConvertChanlistVoltstoDataBins(m_datachannel, m_mvlevel / 1000.0f);
			m_plineview->SetHZtagVal(m_index, val);
			m_plineview->Invalidate();
		}
	}
}

void CMeasureHZtagsPage::OnAdjust() 
{
	int max, min;
	m_plineview->GetChanlistMaxMin(m_datachannel, &max, &min);
	// get nb cursors / m_datachannel
	auto n_cursors=0;
	for (auto i= m_nbcursors-1; i>= 0; i--)
		if (m_plineview->GetHZtagChan(i) == m_datachannel)
			n_cursors++;

	// then split cursors across m_datachannel span
	if (n_cursors == 1)
	{
		OnCenter();
		return;
	}

	const auto dv = (max-min)/(n_cursors-1);
	auto val = min;
	for (auto i= 0; i< m_nbcursors; i++)
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
	auto p_tags_list = m_pdatDoc->GetpHZtags();
	p_tags_list->CopyTagList(m_plineview->GetHZtagList());
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

CMeasureResultsPage::CMeasureResultsPage() : CPropertyPage(CMeasureResultsPage::IDD), m_pdbDoc(nullptr)
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
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());

	CMultiDocTemplate* p_template = p_app->m_pNoteViewTemplate;
	const auto p_doc = p_template->OpenDocumentFile(nullptr);
	auto pos = p_doc->GetFirstViewPosition();
	const auto p_view = dynamic_cast<CRichEditView*>(p_doc->GetNextView(pos));
	CRichEditCtrl& pEdit = p_view->GetRichEditCtrl();	
	pEdit.SetWindowText(csBuffer);		// copy content of window into CString
}

void CMeasureResultsPage::OutputTitle()
{
	// prepare clistctrl column headers
	auto columns = m_listResults.InsertColumn(0, _T("#"),  LVCFMT_LEFT, 20, 0)+1;
	
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
		auto channel_first =0;									// assume all data channels
		auto channel_last =m_plineview->GetChanlistSize()-1;	// requested
		const auto n_pixels_mv = (m_listResults.GetStringWidth(_T("0.000000"))*3)/2;

		if (!m_pMO->bAllChannels)					// else if flag set
		{											// restrict to a single chan
			channel_first =m_pMO->wSourceChan;				// single channel
			channel_last =channel_first ;
		}
		m_nbdatacols=0;
		auto psz_t = m_szT;		
		TCHAR separators[]  = _T("\t");		// separator = tab
		TCHAR* next_token= nullptr;
		
		for (auto channel=channel_first ; channel <= channel_last ; channel++)
		{
			auto cs = csCols;
			auto p_string = cs.GetBuffer(cs.GetLength()+1);
			p_string++;					// skip first tab
			auto p_token = _tcstok_s(p_string, separators, &next_token);
			while (p_token != nullptr)
			{
				if (m_pMO->bAllChannels)
					wsprintf(m_szT, _T("ch%i-%s"), channel, p_token);
				else
					psz_t = p_token;
				columns = 1+ m_listResults.InsertColumn(columns, psz_t, LVCFMT_LEFT, n_pixels_mv, columns);
				m_csTitle += separators;
				m_csTitle += psz_t;
				p_token = _tcstok_s(nullptr, separators, &next_token );	// get next token
			}
			
			if (m_nbdatacols == 0)		// number of data columns
				m_nbdatacols=columns-1;
			cs.ReleaseBuffer();			// release character buffer
		}
	}
	m_csTitle += _T("\r\n");
	return;
}


void CMeasureResultsPage::MeasureFromVTtags(const int channel)
{
	const auto n_tags = m_plineview->GetNVTtags();
	auto tag_first=-1;
	auto tag_last=-1;	

	// assume that VT is ordered; measure only between pairs
	auto line = 0;
	for (auto i = 0; i< n_tags; i++)
	{
		// search first tag
		if (tag_first < 0)
		{
			tag_first = i;
			continue;
		}
		// second tag found: measure
		if (tag_last < 0)
		{
			tag_last = i;
			MeasureWithinInterval(channel, line, m_plineview->GetVTtagLval(tag_first), m_plineview->GetVTtagLval(tag_last));
			line++;			
			tag_first = -1;
			tag_last = -1;			
		}		
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last <0)
	{
		const auto l1 = m_plineview->GetVTtagLval(tag_first);
		MeasureWithinInterval(channel, line, l1, l1);	
	}	
	return;
}

void CMeasureResultsPage::GetMaxMin(const int channel, long l_first, const long l_last)
{
	short* p_data;
	int n_channels;							// n raw channels
	const auto p_buf= m_pdatDoc->LoadRawDataParams(&n_channels);
	const auto source_chan = m_plineview->GetChanlistSourceChan(channel);
	const auto transform_mode = m_plineview->GetChanlistTransformMode(channel);
	const auto span = m_pdatDoc->GetTransfDataSpan(transform_mode);

	// get first data point (init max and min)
	auto buf_chan_first = l_first;
	auto buf_chan_last  = l_last;	
	int offset;
	m_pdatDoc->LoadRawData(&buf_chan_first, &buf_chan_last, span);
	if (transform_mode>0)
	{
		p_data = m_pdatDoc->LoadTransfData(l_first, buf_chan_last, transform_mode, source_chan);
		offset = 1;
	}
	else
	{
		p_data = p_buf + (l_first-buf_chan_first)*n_channels + source_chan;
		offset = n_channels;
	}
	m_max= *p_data;
	m_min= m_max;	
	m_imax = l_first;
	m_imin = m_imax;
	m_first = m_max;

	// loop through data points
	while (l_first < l_last)
	{
		// load file data and get a pointer to these data
		buf_chan_first = l_first;
		buf_chan_last  = l_last;	
		m_pdatDoc->LoadRawData(&buf_chan_first, &buf_chan_last, span);
		if (l_last < buf_chan_last)
			buf_chan_last = l_last;
		if (transform_mode>0)
			p_data = m_pdatDoc->LoadTransfData(l_first, buf_chan_last, transform_mode, source_chan);		
		else
			p_data = p_buf + (l_first-buf_chan_first)*n_channels + source_chan;
		// now search for max and min		
		for (int i= l_first; i<= buf_chan_last; i++)
		{
			m_last = *p_data;
			p_data += offset;
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
		l_first = buf_chan_last+1;
	}
}


// output results into CListCtrl
void CMeasureResultsPage::MeasureWithinInterval(const int channel, const int line, const long l1, const long l2)
{
	// get scale factor for channel and sampling rate
	m_mVperBin = m_plineview->GetChanlistVoltsperDataBin(channel)*1000.0f;
	const auto rate = m_pdatDoc->GetpWaveFormat()->chrate;

	auto output_column = (m_col-1)*m_nbdatacols+1;		// output data into column icol
	auto item = m_listResults.GetItemCount();	// compute which line will receive data	
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);		// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table	
	}
	else
		item=line;								// else set variable value

	// measure parameters / selected interval; save results within common vars
	GetMaxMin(channel, l1, l2);
	
	// output data according to options : data value at limits
	//m_szT[0] = '\t';								// prepare string for edit output
	CString cs_dummy;
	float	x_dummy;
	const CString cs_fmt(_T("\t%f"));
	if (m_pMO->bDatalimits)
	{
		x_dummy = static_cast<float>(m_first)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(l1) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}

		x_dummy = static_cast<float>(m_last)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(l2) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	if (m_pMO->bDiffDatalimits)
	{
		x_dummy = static_cast<float>(m_last - m_first)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = (static_cast<float>(l2) - l1) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	// measure max and min (val, time)
	if (m_pMO->bExtrema)
	{
		x_dummy = static_cast<float>(m_max)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imax) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
		x_dummy = static_cast<float>(m_min)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imin) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	// difference between extrema (val, time)
	if (m_pMO->bDiffExtrema)
	{
		x_dummy = static_cast<float>(m_max - m_min)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imax - m_imin) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); 
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
	const auto number_of_tags = m_plineview->GetNHZtags();
	auto tag_first=-1;
	auto tag_last=-1;

	// assume that HZ is ordered; measure only between pairs
	int line = 0;
	for (int i = 0; i< number_of_tags; i++)
	{
		// search first tag
		if (tag_first < 0)
		{
			tag_first = i;
			continue;
		}
		// second tag found: measure
		if (tag_last < 0)
		{
			tag_last = i;
			MeasureBetweenHZ(ichan, line, m_plineview->GetHZtagVal(tag_first), m_plineview->GetHZtagVal(tag_last));
			line++;
			tag_first = -1;
			tag_last = -1;			
		}		
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last <0)
	{
		const auto v1 = m_plineview->GetHZtagVal(tag_first);
		MeasureBetweenHZ(ichan, line, v1, v1);
	}	
}

// output results both into CEdit control (via pCopy) and within CListCtrl
void CMeasureResultsPage::MeasureBetweenHZ(const int channel, const int line, const int v1, const int v2)
{
	// get scale factor for channel and sampling rate
	m_mVperBin = m_plineview->GetChanlistVoltsperDataBin(channel)*1000.0f;

	auto column_1 = (m_col-1)*m_nbdatacols+1;		// output data into column icol
	auto item = m_listResults.GetItemCount();	// compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);	// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table
	}
	else
		item=line;								// else set variable value

	// measure parameters / selected interval; save results within common vars	
	//m_szT[0]='\t';								// prepare string for edit output	
	CString cs_dummy;
	float	x_dummy;
	const CString cs_fmt(_T("\t%f"));
	
	// output data according to options : data value at limits
	if (m_pMO->bDatalimits)
	{
		x_dummy = static_cast<float>(v1)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy); column_1++;

		x_dummy = static_cast<float>(v2)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy); column_1++;
	}

	if (m_pMO->bDiffDatalimits)
	{
		x_dummy = static_cast<float>(v2 - v1)*m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy);
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
	const int current_file_index = m_pdbDoc->DBGetCurrentRecordPosition();
	ASSERT(current_file_index >= 0);
	auto i_first = current_file_index;
	auto i_last = i_first;	
	if (m_pMO->bAllFiles)
	{
		i_first = 0;
		i_last = m_pdbDoc->DBGetNRecords()-1;
	}

	// prepare listcontrol
//	while (m_listResults.DeleteColumn(0) != 0);
	m_listResults.DeleteAllItems();

	// prepare clipboard and Edit control (CEdit)
	if (OpenClipboard())
	{
		EmptyClipboard();		// prepare clipboard and copy text to buffer
		const DWORD dw_len = 32768;	// 32 Kb
		const auto h_copy = static_cast<HANDLE>(::GlobalAlloc(GHND, dw_len));
		if (h_copy == nullptr)
		{
			AfxMessageBox(_T("Memory low: unable to allocate memory"));
			return TRUE;
		}
		auto p_copy = static_cast<TCHAR*>(::GlobalLock(static_cast<HGLOBAL>(h_copy)));
		auto p_copy0= p_copy;
		
		// export Ascii: begin //////////////////////////////////////////////
		BeginWaitCursor();
		OutputTitle();
		m_pdbDoc->DBSetCurrentRecordPosition(i_first);
		m_pdbDoc->DBGetCurrentDatFileName();
		auto* p_vd = new OPTIONS_VIEWDATA;
		ASSERT(p_vd != NULL);
		const CString cs_out=_T("**filename\tdate\ttime\tcomment\tchannel\r\n");

		for (auto i=i_first; i<=i_last; i++, m_pdbDoc->DBMoveNext())
		{
			// open data file			
			m_pdbDoc->OpenCurrentDataFile();
			
			p_vd->bacqcomments=TRUE;		// global comment
			p_vd->bacqdate=TRUE;			// acquisition date
			p_vd->bacqtime=TRUE;			// acquisition time
			p_vd->bfilesize=FALSE;		// file size
			p_vd->bacqchcomment=FALSE;	// acq channel indiv comment
			p_vd->bacqchsetting=FALSE;	// acq chan indiv settings (gain, filter, etc)
			auto cs = cs_out;
			cs += m_pdbDoc->m_pDat->GetDataFileInfos(p_vd);
			p_copy += wsprintf(p_copy, _T("%s\r\n"), static_cast<LPCTSTR>(cs));
	
			// output title for this data set
			auto i_chan_0=0;
			auto i_chan_1=m_plineview->GetChanlistSize()-1;
			if (!m_pMO->bAllChannels)
			{
				i_chan_0=m_pMO->wSourceChan;
				i_chan_1=i_chan_0;
			}
			if (i_chan_0 >= m_plineview->GetChanlistSize())
				i_chan_0 = 0;
			if (i_chan_1 >= m_plineview->GetChanlistSize())
				i_chan_1 = m_plineview->GetChanlistSize()-1;

			m_col=1;
			for (auto i_chan=i_chan_0; i_chan <= i_chan_1; i_chan++)
			{
				// measure according to option
				switch (m_pMO->wOption)
				{
				// ......................  vertical tags
				case 0:	MeasureFromVTtags(i_chan); break;
				// ......................  horizontal cursors
				case 1:	MeasureFromHZcur(i_chan); break;
				// ......................  rectangle area
				case 2:	MeasureFromRect(i_chan); break;
				// ......................  detect stimulus and then measure
				case 3: MeasureFromStim(i_chan); break;
				default:
					break;
				}
				m_col++;
			}

			// transfer content of clistctrl to clipboard
			p_copy += wsprintf(p_copy, _T("%s"), (LPCTSTR) m_csTitle);
			const auto n_lines = m_listResults.GetItemCount();
			const auto n_columns = m_nbdatacols*(m_col-1)+1;
			for (auto item=0; item < n_lines; item++)
			{
				for (auto column=0; column< n_columns; column++)
				{
					auto cs_content = m_listResults.GetItemText(item, column);
					p_copy += wsprintf(p_copy, _T("%s\t"), static_cast<LPCTSTR>(cs_content));
				}
				p_copy--; *p_copy = '\r'; p_copy++;
				*p_copy = '\n'; p_copy++;
			}
			// if not, then next file will override the results computed before			
		}

		*p_copy = 0;
		m_pdbDoc->DBSetCurrentRecordPosition(current_file_index );
		m_pdbDoc->DBGetCurrentDatFileName();
		m_pdbDoc->OpenCurrentDataFile();
		EndWaitCursor();     // it's done

		// export Ascii: end //////////////////////////////////////////////
		m_CEditResults.SetWindowText(p_copy0);		
		::GlobalUnlock((HGLOBAL) h_copy);
		SetClipboardData (CF_TEXT, h_copy);				
		CloseClipboard();		// close connect w.clipboard
		delete p_vd;				// delete temporary object

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
