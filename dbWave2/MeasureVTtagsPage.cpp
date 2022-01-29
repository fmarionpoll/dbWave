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
	const auto lk = m_pChartDataWnd->m_VTtags.GetTagLVal(m_index);
	m_timesec = static_cast<float>(lk) / m_samplingrate;

	return TRUE;
}

void CMeasureVTtagsPage::SetspacedTagsOptions()
{
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(m_pMO->bSetTagsforCompleteFile);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(m_pMO->bSetTagsforCompleteFile);
	// validate dependent edit box accordingly
	GetDlgItem(IDC_NPERIODSSTATIC)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
	GetDlgItem(IDC_NPERIODSEDIT)->EnableWindow(!m_pMO->bSetTagsforCompleteFile);
}

void CMeasureVTtagsPage::OnSetDuplicateMode()
{
	m_pMO->bSetTagsforCompleteFile = static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck();
	SetspacedTagsOptions();
}

void CMeasureVTtagsPage::OnCancel()
{
	OnDeleteSeries();
	if (m_pMO->wOption != 0)
	{
		m_pChartDataWnd->m_VTtags.RemoveAllTags();
		if (m_pMO->wOption == 1)
			m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->GetpHZtags());
	}
	CPropertyPage::OnCancel();
}

void CMeasureVTtagsPage::OnOK()
{
	auto p_tag_list = m_pdatDoc->GetpVTtags();
	p_tag_list->CopyTagList(&m_pChartDataWnd->m_VTtags);
	m_pMO->b_changed = TRUE;
	if (m_pMO->wOption != 0)
	{
		m_pChartDataWnd->m_VTtags.RemoveAllTags();
		if (m_pMO->wOption == 1)
			m_pChartDataWnd->m_HZtags.CopyTagList(m_pdatDoc->GetpHZtags());
	}
	CPropertyPage::OnOK();
}

BOOL CMeasureVTtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// set check button
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pMO->bSaveTags);

	// save initial state of VTtags
	//TODO bug here
	m_pChartDataWnd->m_VTtags.CopyTagList(m_pdatDoc->GetpVTtags());
	m_pChartDataWnd->m_HZtags.RemoveAllTags();
	m_pChartDataWnd->Invalidate();
	m_nbtags = m_pChartDataWnd->m_VTtags.GetNTags();
	GetVTtagVal(0);

	// subclassed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_timesec.SubclassDlgItem(IDC_TIMESEC, this));
	VERIFY(mm_duration.SubclassDlgItem(IDC_DURATION, this));
	VERIFY(mm_period.SubclassDlgItem(IDC_PERIOD, this));
	VERIFY(mm_nperiods.SubclassDlgItem(IDC_NPERIODSEDIT, this));
	VERIFY(mm_timeshift.SubclassDlgItem(IDC_TIMESHIFT, this));
	m_verylast = static_cast<float>(m_pChartDataWnd->GetDocumentLast()) / m_samplingrate;
	SetspacedTagsOptions();
	m_duration = m_pMO->duration; // on/OFF duration (sec)
	m_period = m_pMO->period; // period (sec)
	m_nperiods = m_pMO->nperiods; // nb of duplicates
	m_timeshift = m_pMO->timeshift; // shift tags
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureVTtagsPage::OnRemove()
{
	if (m_index >= 0 && m_index < m_nbtags)
	{
		m_pChartDataWnd->m_VTtags.RemoveTag(m_index);
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
	m_pMO->bSaveTags = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
}

void CMeasureVTtagsPage::OnEnChangeItem()
{
	if (mm_index.m_bEntryDone)
	{
		switch (mm_index.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_index++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_index--;
			break;
		default: ;
		}
		mm_index.m_bEntryDone = FALSE; // clear flag
		mm_index.m_nChar = 0; // empty buffer
		mm_index.SetSel(0, -1); // select all text
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
		switch (mm_timesec.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_timesec++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_timesec--;
			break;
		default: ;
		}
		mm_timesec.m_bEntryDone = FALSE; // clear flag
		mm_timesec.m_nChar = 0; // empty buffer
		mm_timesec.SetSel(0, -1); // select all text
		// update dependent parameters
		if (m_timesec < 0)
			m_timesec = 0.0f;
		if (m_timesec >= m_verylast)
			m_timesec = m_verylast;
		UpdateData(FALSE);
		const auto lk = static_cast<long>(m_timesec * m_samplingrate);
		if (m_index >= 0 && m_index < m_nbtags)
		{
			m_pChartDataWnd->m_VTtags.SetTagLVal(m_index, lk);
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
		switch (mm_duration.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_duration++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_duration--;
			break;
		default: ;
		}
		mm_duration.m_bEntryDone = FALSE; // clear flag
		mm_duration.m_nChar = 0; // empty buffer
		mm_duration.SetSel(0, -1); // select all text
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
		switch (mm_period.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_period++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_period--;
			break;
		default: ;
		}
		mm_period.m_bEntryDone = FALSE; // clear flag
		mm_period.m_nChar = 0; // empty buffer
		mm_period.SetSel(0, -1); // select all text
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
		switch (mm_nperiods.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_nperiods++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_nperiods--;
			break;
		default: ;
		}
		mm_nperiods.m_bEntryDone = FALSE; // clear flag
		mm_nperiods.m_nChar = 0; // empty buffer
		mm_nperiods.SetSel(0, -1); // select all text
		// update dependent parameters
		if (m_nperiods < 1)
			m_nperiods = 1;
		m_pMO->nperiods = m_nperiods;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::OnEnChangeTimeshift()
{
	if (mm_timeshift.m_bEntryDone)
	{
		switch (mm_timeshift.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_timeshift++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_timeshift--;
			break;
		default: ;
		}
		mm_timeshift.m_bEntryDone = FALSE; // clear flag
		mm_timeshift.m_nChar = 0; // empty buffer
		mm_timeshift.SetSel(0, -1); // select all text
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
	for (auto i = 0; i < m_nbtags; i++)
		m_pChartDataWnd->m_VTtags.SetTagLVal(i, m_pChartDataWnd->m_VTtags.GetTagLVal(i) + offset);
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
	if (!m_pMO->bSetTagsforCompleteFile)
	{
		m_nbtags = m_pChartDataWnd->m_VTtags.GetNTags();
		time = m_pChartDataWnd->m_VTtags.GetTagLVal(m_nbtags - 1) / m_samplingrate;
		time_end = m_period * static_cast<float>(m_nperiods) + time;
		// delete this one which will be re-created within the loop
		m_pChartDataWnd->m_VTtags.RemoveTag(m_nbtags - 1);
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
		m_pChartDataWnd->m_VTtags.AddLTag(static_cast<long>(time * m_samplingrate), 0);
		m_pChartDataWnd->m_VTtags.AddLTag(static_cast<long>((time + m_duration) * m_samplingrate), 0);
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
	auto p_tags_list = m_pdatDoc->GetpVTtags();
	m_pChartDataWnd->m_VTtags.CopyTagList(p_tags_list);
	m_nbtags = p_tags_list->GetNTags();
	m_pChartDataWnd->Invalidate();
}

void CMeasureVTtagsPage::OnDeleteAll()
{
	m_pChartDataWnd->m_VTtags.RemoveAllTags();
	m_pChartDataWnd->Invalidate();
	m_nbtags = 0;
	GetVTtagVal(0);
	UpdateData(FALSE);
}
