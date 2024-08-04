// CMeasureOptionsPage.cpp : implementation file
//
// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
//#include "NoteDoc.h"
#include "dbWaveDoc.h"
#include "resource.h"
#include "MeasureVTtagsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureVTtagsPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CMeasureVTtagsPage property page

CMeasureVTtagsPage::CMeasureVTtagsPage() : CPropertyPage(IDD), m_index(0), m_pMO(nullptr),
                                           m_pdbDoc(nullptr), m_pdatDoc(nullptr), m_samplingrate(0), m_verylast(0)
{
	m_nbtags = 0;
	m_nperiods = 0;
	m_period = 0.0f;
	m_timesec = 0.0f;
	m_timeshift = 0.0f;
	m_duration = 0.0f;
	m_pChartDataWnd = nullptr;
}

CMeasureVTtagsPage::~CMeasureVTtagsPage()
{
}

void CMeasureVTtagsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ITEM, m_index);
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

BOOL CMeasureVTtagsPage::GetVTtagVal(int index)
{
	const BOOL flag = (m_nbtags > 0);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);

	if (index < 0 || index >= m_nbtags)
		return FALSE;
	m_index = index;
	const auto lk = m_pChartDataWnd->vertical_tags.get_value_long(m_index);
	m_timesec = static_cast<float>(lk) / m_samplingrate;

	return TRUE;
}

void CMeasureVTtagsPage::SetspacedTagsOptions()
{
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(m_pMO->b_set_tags_for_complete_file);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(m_pMO->b_set_tags_for_complete_file);
	// validate dependent edit box accordingly
	GetDlgItem(IDC_NPERIODSSTATIC)->EnableWindow(!m_pMO->b_set_tags_for_complete_file);
	GetDlgItem(IDC_NPERIODSEDIT)->EnableWindow(!m_pMO->b_set_tags_for_complete_file);
}

void CMeasureVTtagsPage::OnSetDuplicateMode()
{
	m_pMO->b_set_tags_for_complete_file = static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck();
	SetspacedTagsOptions();
}

void CMeasureVTtagsPage::OnCancel()
{
	OnDeleteSeries();
	if (m_pMO->w_option != 0)
	{
		m_pChartDataWnd->vertical_tags.remove_all_tags();
		if (m_pMO->w_option == 1)
			m_pChartDataWnd->horizontal_tags.copy_tag_list(m_pdatDoc->get_hz_tags_list());
	}
	CPropertyPage::OnCancel();
}

void CMeasureVTtagsPage::OnOK()
{
	auto p_tag_list = m_pdatDoc->get_vt_tags_list();
	p_tag_list->copy_tag_list(&m_pChartDataWnd->vertical_tags);
	m_pMO->b_changed = TRUE;
	if (m_pMO->w_option != 0)
	{
		m_pChartDataWnd->vertical_tags.remove_all_tags();
		if (m_pMO->w_option == 1)
			m_pChartDataWnd->horizontal_tags.copy_tag_list(m_pdatDoc->get_hz_tags_list());
	}
	CPropertyPage::OnOK();
}

BOOL CMeasureVTtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// set check button
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->b_save_tags);

	// save initial state of VTtags
	//TODO bug here
	m_pChartDataWnd->vertical_tags.copy_tag_list(m_pdatDoc->get_vt_tags_list());
	m_pChartDataWnd->horizontal_tags.remove_all_tags();
	m_pChartDataWnd->Invalidate();
	m_nbtags = m_pChartDataWnd->vertical_tags.get_tag_list_size();
	GetVTtagVal(0);

	// subclassed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_timesec.SubclassDlgItem(IDC_TIMESEC, this));
	VERIFY(mm_duration.SubclassDlgItem(IDC_DURATION, this));
	VERIFY(mm_period.SubclassDlgItem(IDC_PERIOD, this));
	VERIFY(mm_nperiods.SubclassDlgItem(IDC_NPERIODSEDIT, this));
	VERIFY(mm_timeshift.SubclassDlgItem(IDC_TIMESHIFT, this));
	m_verylast = static_cast<float>(m_pChartDataWnd->get_document_last()) / m_samplingrate;
	SetspacedTagsOptions();
	m_duration = m_pMO->duration; // on/OFF duration (sec)
	m_period = m_pMO->period; // period (sec)
	m_nperiods = m_pMO->n_periods; // nb of duplicates
	m_timeshift = m_pMO->time_shift; // shift tags
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureVTtagsPage::OnRemove()
{
	if (m_index >= 0 && m_index < m_nbtags)
	{
		m_pChartDataWnd->vertical_tags.remove_tag(m_index);
		m_nbtags--;
	}
	if (m_index > m_nbtags - 1)
		m_index = m_nbtags - 1;
	m_pChartDataWnd->Invalidate();

	GetVTtagVal(m_index);
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::OnCheck1()
{
	m_pMO->b_save_tags = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
}

void CMeasureVTtagsPage::OnEnChangeItem()
{
	if (mm_index.m_bEntryDone)
	{
		mm_index.OnEnChange(this, m_index, 1, -1);
		// update dependent parameters
		if (m_index >= m_nbtags)
			m_index = m_nbtags - 1;
		if (m_index < 0)
			m_index = 0;
		GetVTtagVal(m_index);
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::OnEnChangeTimesec()
{
	if (mm_timesec.m_bEntryDone)
	{
		mm_timesec.OnEnChange(this, m_timesec, 1.f, -1.f);
		// update dependent parameters
		if (m_timesec < 0)
			m_timesec = 0.0f;
		if (m_timesec >= m_verylast)
			m_timesec = m_verylast;
		UpdateData(FALSE);
		const auto lk = static_cast<long>(m_timesec * m_samplingrate);
		if (m_index >= 0 && m_index < m_nbtags)
		{
			m_pChartDataWnd->vertical_tags.set_value_long(m_index, lk);
			m_pChartDataWnd->Invalidate();
		}
	}
}

// change duration parameter - duration of the stimulation or interval
// interval between 2 consecutive tags defining a stimulation pulse
// tag(n) and tag(n+1)
void CMeasureVTtagsPage::OnEnChangeDuration()
{
	if (mm_duration.m_bEntryDone)
	{
		mm_duration.OnEnChange(this, m_duration, 1.f, -1.f);
		// update dependent parameters
		if (m_duration < 0.)
			m_duration = 0.0f;
		if (m_duration >= m_period)
			m_duration = m_period;
		m_pMO->duration = m_duration;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::OnEnChangePeriod()
{
	if (mm_period.m_bEntryDone)
	{
		mm_period.OnEnChange(this, m_period, 1.f, -1.f);
		// update dependent parameters
		if (m_period < m_duration)
			m_period = m_duration;
		m_pMO->period = m_period;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::OnEnChangeNperiodsedit()
{
	if (mm_nperiods.m_bEntryDone)
	{
		mm_nperiods.OnEnChange(this, m_nperiods, 1, -1);
		// update dependent parameters
		if (m_nperiods < 1)
			m_nperiods = 1;
		m_pMO->n_periods = m_nperiods;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::OnEnChangeTimeshift()
{
	if (mm_timeshift.m_bEntryDone)
	{
		mm_timeshift.OnEnChange(this, m_timeshift, 1.f, -1.f);
		// update dependent parameters
		m_pMO->time_shift = m_timeshift;
		UpdateData(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// execute

void CMeasureVTtagsPage::OnShiftTags()
{
	const auto offset = static_cast<long>(m_timeshift * m_samplingrate);
	for (auto i = 0; i < m_nbtags; i++)
		m_pChartDataWnd->vertical_tags.set_value_long(i, m_pChartDataWnd->vertical_tags.get_value_long(i) + offset);
	// update data
	m_pChartDataWnd->Invalidate();
	GetVTtagVal(m_index);
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::OnAddTags()
{
	float time;
	float time_end;

	// compute limits
	if (!m_pMO->b_set_tags_for_complete_file)
	{
		m_nbtags = m_pChartDataWnd->vertical_tags.get_tag_list_size();
		time = m_pChartDataWnd->vertical_tags.get_value_long(m_nbtags - 1) / m_samplingrate;
		time_end = m_period * static_cast<float>(m_nperiods) + time;
		// delete this one which will be re-created within the loop
		m_pChartDataWnd->vertical_tags.remove_tag(m_nbtags - 1);
		m_nbtags--;
	}
	// total file, start at zero
	else
	{
		time = 0.0f;
		time_end = m_verylast;
	}

	// loop until requested interval is completed
	const auto time0 = time;
	auto n_intervals = 0.0f;
	while (time <= time_end)
	{
		m_pChartDataWnd->vertical_tags.add_l_tag(static_cast<long>(time * m_samplingrate), 0);
		m_pChartDataWnd->vertical_tags.add_l_tag(static_cast<long>((time + m_duration) * m_samplingrate), 0);
		n_intervals++;
		time = time0 + m_period * n_intervals;
	}
	m_nbtags += static_cast<int>(n_intervals) * 2;
	m_pChartDataWnd->Invalidate();
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::OnDeleteSeries()
{
	// delete present tags
	auto p_tags_list = m_pdatDoc->get_vt_tags_list();
	m_pChartDataWnd->vertical_tags.copy_tag_list(p_tags_list);
	m_nbtags = p_tags_list->get_tag_list_size();
	m_pChartDataWnd->Invalidate();
}

void CMeasureVTtagsPage::OnDeleteAll()
{
	m_pChartDataWnd->vertical_tags.remove_all_tags();
	m_pChartDataWnd->Invalidate();
	m_nbtags = 0;
	GetVTtagVal(0);
	UpdateData(FALSE);
}
