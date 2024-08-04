// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "ChartWnd.h"

#include "dbWaveDoc.h"
#include "resource.h"

#include "MeasureOptionsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureOptionsPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CMeasureOptionsPage property page

CMeasureOptionsPage::CMeasureOptionsPage() : CPropertyPage(IDD), m_wSourcechan(0), m_pMO(nullptr),
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
	m_pChartDataWnd->horizontal_tags.remove_all_tags();
	m_pChartDataWnd->vertical_tags.copy_tag_list(m_pdatDoc->get_vt_tags_list());
	m_pChartDataWnd->Invalidate();
	ShowLimitsParms(FALSE);
}

void CMeasureOptionsPage::OnHorizontaltags()
{
	m_pChartDataWnd->vertical_tags.remove_all_tags();
	m_pChartDataWnd->horizontal_tags.copy_tag_list(m_pdatDoc->get_hz_tags_list());
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
	m_pMO->b_extrema = m_bExtrema;
	m_pMO->b_diff_extrema = m_bDiffExtrema;
	m_pMO->b_diff_data_limits = m_bDiffDatalimits;
	m_pMO->b_half_rise_time = m_bHalfrisetime;
	m_pMO->b_half_recovery = m_bHalfrecovery;
	m_pMO->b_data_limits = m_bDatalimits;
	m_pMO->w_source_channel = m_uiSourceChan;
	m_pMO->w_stimulus_threshold = m_uiStimulusThreshold;
	m_pMO->w_stimulus_channel = m_uiStimuluschan;
	m_pMO->f_stimulus_offset = m_fStimulusoffset;
	m_pMO->b_all_files = m_bAllFiles;
	m_pMO->b_time = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();

	auto i_id = GetCheckedRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG);
	switch (i_id)
	{
	case IDC_VERTICALTAGS:
		i_id = 0;
		m_pChartDataWnd->horizontal_tags.remove_all_tags();
		m_pChartDataWnd->vertical_tags.copy_tag_list(m_pdatDoc->get_vt_tags_list());
		break;
	case IDC_HORIZONTALTAGS:
		i_id = 1;
		m_pChartDataWnd->vertical_tags.remove_all_tags();
		m_pChartDataWnd->horizontal_tags.copy_tag_list(m_pdatDoc->get_hz_tags_list());
		break;
	case IDC_RECTANGLETAG: i_id = 2;
		break;
	case IDC_STIMULUSTAG: i_id = 3;
		break;
	default: i_id = 2;
		break;
	}
	m_pMO->w_option = i_id;
	m_pMO->b_all_channels = static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
	m_pMO->b_changed = TRUE;
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

	m_bExtrema = m_pMO->b_extrema;
	m_bDiffExtrema = m_pMO->b_diff_extrema;
	m_bDiffDatalimits = m_pMO->b_diff_data_limits;
	m_bHalfrisetime = m_pMO->b_half_rise_time;
	m_bHalfrecovery = m_pMO->b_half_recovery;
	m_bDatalimits = m_pMO->b_data_limits;
	m_uiSourceChan = m_pMO->w_source_channel;
	m_uiStimulusThreshold = m_pMO->w_stimulus_threshold;
	m_uiStimuluschan = m_pMO->w_stimulus_channel;
	m_fStimulusoffset = m_pMO->f_stimulus_offset;
	m_bAllFiles = m_pMO->b_all_files;
	int i_id;
	auto flag = FALSE;
	switch (m_pMO->w_option)
	{
	case 0: i_id = IDC_VERTICALTAGS;
		break;
	case 1: i_id = IDC_HORIZONTALTAGS;
		break;
	case 2: i_id = IDC_RECTANGLETAG;
		break;
	case 3: i_id = IDC_STIMULUSTAG;
		flag = TRUE;
		break;
	default: i_id = IDC_VERTICALTAGS;
		break;
	}
	ShowLimitsParms(flag);

	auto i_id1 = IDC_ALLCHANNELS;
	flag = FALSE;
	if (!m_pMO->b_all_channels)
	{
		i_id1 = IDC_SINGLECHANNEL;
		flag = TRUE;
	}
	ShowChanParm(flag);

	CheckRadioButton(IDC_ALLCHANNELS, IDC_SINGLECHANNEL, i_id1);
	CheckRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG, i_id);

	GetDlgItem(IDC_CHECKRISETIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECKRECOVERYTIME)->EnableWindow(FALSE);
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->b_time);

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
