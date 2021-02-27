// CMeasureOptionsPage.cpp : implementation file
//
// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "chart.h"
//#include "ChartData.h"
//#include "Editctrl.h"
//#include "NoteDoc.h"
#include "dbWaveDoc.h"
#include "resource.h"

#include "MeasureOptionsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureOptionsPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CMeasureOptionsPage property page

CMeasureOptionsPage::CMeasureOptionsPage() : CPropertyPage(CMeasureOptionsPage::IDD), m_wSourcechan(0), m_pMO(nullptr),
m_pdbDoc(nullptr), m_pdatDoc(nullptr), m_pChartDataWnd(nullptr)
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
	m_pChartDataWnd-> m_HZtags.RemoveAllTags();
	m_pChartDataWnd->m_VTtags.CopyTagList(m_pdatDoc->GetpVTtags());
	m_pChartDataWnd->Invalidate();
	ShowLimitsParms(FALSE);
}

void CMeasureOptionsPage::OnHorizontaltags()
{
	m_pChartDataWnd->m_VTtags.RemoveAllTags();
	m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->GetpHZtags());
	m_pChartDataWnd->Invalidate();
	ShowLimitsParms(FALSE);
}

void CMeasureOptionsPage::OnStimulustag()
{
	ShowLimitsParms(TRUE);
}

// save options
void CMeasureOptionsPage::SaveOptions()
{
	m_pMO->bExtrema = m_bExtrema;
	m_pMO->bDiffExtrema = m_bDiffExtrema;
	m_pMO->bDiffDatalimits = m_bDiffDatalimits;
	m_pMO->bHalfrisetime = m_bHalfrisetime;
	m_pMO->bHalfrecovery = m_bHalfrecovery;
	m_pMO->bDatalimits = m_bDatalimits;
	m_pMO->wSourceChan = m_uiSourceChan;
	m_pMO->wStimulusthresh = m_uiStimulusThreshold;
	m_pMO->wStimuluschan = m_uiStimuluschan;
	m_pMO->fStimulusoffset = m_fStimulusoffset;
	m_pMO->bAllFiles = m_bAllFiles;
	m_pMO->btime = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();

	auto i_id = GetCheckedRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG);
	switch (i_id)
	{
	case IDC_VERTICALTAGS:
		i_id = 0;
		m_pChartDataWnd-> m_HZtags.RemoveAllTags();
		m_pChartDataWnd->m_VTtags.CopyTagList(m_pdatDoc->GetpVTtags());
		break;
	case IDC_HORIZONTALTAGS:
		i_id = 1;
		m_pChartDataWnd->m_VTtags.RemoveAllTags();
		m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->GetpHZtags());
		break;
	case IDC_RECTANGLETAG:		i_id = 2; break;
	case IDC_STIMULUSTAG:		i_id = 3; break;
	default:					i_id = 2; break;
	}
	m_pMO->wOption = i_id;
	m_pMO->bAllChannels = ((CButton*)GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
	m_pMO->bChanged = TRUE;
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

	m_bExtrema = m_pMO->bExtrema;
	m_bDiffExtrema = m_pMO->bDiffExtrema;
	m_bDiffDatalimits = m_pMO->bDiffDatalimits;
	m_bHalfrisetime = m_pMO->bHalfrisetime;
	m_bHalfrecovery = m_pMO->bHalfrecovery;
	m_bDatalimits = m_pMO->bDatalimits;
	m_uiSourceChan = m_pMO->wSourceChan;
	m_uiStimulusThreshold = m_pMO->wStimulusthresh;
	m_uiStimuluschan = m_pMO->wStimuluschan;
	m_fStimulusoffset = m_pMO->fStimulusoffset;
	m_bAllFiles = m_pMO->bAllFiles;
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
		i_id1 = IDC_SINGLECHANNEL;
		flag = TRUE;
	}
	ShowChanParm(flag);

	CheckRadioButton(IDC_ALLCHANNELS, IDC_SINGLECHANNEL, i_id1);
	CheckRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG, i_id);

	GetDlgItem(IDC_CHECKRISETIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECKRECOVERYTIME)->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->btime);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}