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



CMeasureVTtagsPage::CMeasureVTtagsPage() : CPropertyPage(IDD)
{
}

CMeasureVTtagsPage::~CMeasureVTtagsPage()
= default;

void CMeasureVTtagsPage::DoDataExchange(CDataExchange* p_dx)
{
	CPropertyPage::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_ITEM, m_index);
	DDX_Text(p_dx, IDC_NBTAGS, m_nb_tags);
	DDX_Text(p_dx, IDC_NPERIODSEDIT, m_n_periods);
	DDX_Text(p_dx, IDC_PERIOD, m_period);
	DDX_Text(p_dx, IDC_TIMESEC, m_time_sec);
	DDX_Text(p_dx, IDC_TIMESHIFT, m_time_shift);
	DDX_Text(p_dx, IDC_DURATION, m_duration);
}

BEGIN_MESSAGE_MAP(CMeasureVTtagsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_REMOVE, on_remove)
	ON_EN_CHANGE(IDC_ITEM, on_en_change_item)
	ON_EN_CHANGE(IDC_TIMESEC, on_en_change_time_sec)
	ON_BN_CLICKED(IDC_CHECK1, on_check1)
	ON_BN_CLICKED(IDC_RADIO1, on_set_duplicate_mode)
	ON_EN_CHANGE(IDC_DURATION, on_en_change_duration)
	ON_EN_CHANGE(IDC_PERIOD, on_en_change_period)
	ON_EN_CHANGE(IDC_NPERIODSEDIT, on_en_change_n_periods_edit)
	ON_EN_CHANGE(IDC_TIMESHIFT, on_en_change_time_shift)
	ON_BN_CLICKED(IDC_BUTTON2, on_shift_tags)
	ON_BN_CLICKED(IDC_BUTTON1, on_add_tags)
	ON_BN_CLICKED(IDC_BUTTON3, on_delete_series)
	ON_BN_CLICKED(IDC_RADIO2, on_set_duplicate_mode)
	ON_BN_CLICKED(IDC_BUTTON4, on_delete_all)
END_MESSAGE_MAP()

BOOL CMeasureVTtagsPage::get_vt_tag_value(const int index)
{
	const BOOL flag = (m_nb_tags > 0);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);

	if (index < 0 || index >= m_nb_tags)
		return FALSE;
	m_index = index;
	const auto lk = m_p_chart_data_wnd->vt_tags.get_tag_value_long(m_index);
	m_time_sec = static_cast<float>(lk) / m_sampling_rate;

	return TRUE;
}

void CMeasureVTtagsPage::set_spaced_tags_options() const
{
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(m_p_options_measure->b_set_tags_for_complete_file);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(m_p_options_measure->b_set_tags_for_complete_file);
	// validate dependent edit box accordingly
	GetDlgItem(IDC_NPERIODSSTATIC)->EnableWindow(!m_p_options_measure->b_set_tags_for_complete_file);
	GetDlgItem(IDC_NPERIODSEDIT)->EnableWindow(!m_p_options_measure->b_set_tags_for_complete_file);
}

void CMeasureVTtagsPage::on_set_duplicate_mode()
{
	m_p_options_measure->b_set_tags_for_complete_file = static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck();
	set_spaced_tags_options();
}

void CMeasureVTtagsPage::OnCancel()
{
	on_delete_series();
	if (m_p_options_measure->w_option != MEASURE_VERTICAL)
	{
		m_p_chart_data_wnd->vt_tags.remove_all_tags();
		if (m_p_options_measure->w_option == MEASURE_HORIZONTAL)
			m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
	}
	CPropertyPage::OnCancel();
}

void CMeasureVTtagsPage::OnOK()
{
	auto p_tag_list = m_p_dat_doc->get_vt_tags_list();
	p_tag_list->copy_tag_list(&m_p_chart_data_wnd->vt_tags);
	m_p_options_measure->b_changed = TRUE;
	if (m_p_options_measure->w_option != MEASURE_VERTICAL)
	{
		m_p_chart_data_wnd->vt_tags.remove_all_tags();
		if (m_p_options_measure->w_option == MEASURE_HORIZONTAL)
			m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
	}
	CPropertyPage::OnOK();
}

BOOL CMeasureVTtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// set check button
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_p_options_measure->b_save_tags);

	// save initial state of VT tags
	//TODO bug here
	m_p_chart_data_wnd->vt_tags.copy_tag_list(m_p_dat_doc->get_vt_tags_list());
	m_p_chart_data_wnd->hz_tags.remove_all_tags();
	m_p_chart_data_wnd->Invalidate();
	m_nb_tags = m_p_chart_data_wnd->vt_tags.get_tag_list_size();
	get_vt_tag_value(0);

	// subclassed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_time_sec.SubclassDlgItem(IDC_TIMESEC, this));
	VERIFY(mm_duration.SubclassDlgItem(IDC_DURATION, this));
	VERIFY(mm_period.SubclassDlgItem(IDC_PERIOD, this));
	VERIFY(mm_n_periods.SubclassDlgItem(IDC_NPERIODSEDIT, this));
	VERIFY(mm_time_shift.SubclassDlgItem(IDC_TIMESHIFT, this));
	m_very_last = static_cast<float>(m_p_chart_data_wnd->get_document_last()) / m_sampling_rate;
	set_spaced_tags_options();
	m_duration = m_p_options_measure->duration; // on/OFF duration (sec)
	m_period = m_p_options_measure->period; // period (sec)
	m_n_periods = m_p_options_measure->n_periods; // nb of duplicates
	m_time_shift = m_p_options_measure->time_shift; // shift tags
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureVTtagsPage::on_remove()
{
	if (m_index >= 0 && m_index < m_nb_tags)
	{
		m_p_chart_data_wnd->vt_tags.remove_tag(m_index);
		m_nb_tags--;
	}
	if (m_index > m_nb_tags - 1)
		m_index = m_nb_tags - 1;
	m_p_chart_data_wnd->Invalidate();

	get_vt_tag_value(m_index);
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::on_check1()
{
	m_p_options_measure->b_save_tags = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
}

void CMeasureVTtagsPage::on_en_change_item()
{
	if (mm_index.m_b_entry_done)
	{
		mm_index.on_en_change(this, m_index, 1, -1);
		// update dependent parameters
		if (m_index >= m_nb_tags)
			m_index = m_nb_tags - 1;
		if (m_index < 0)
			m_index = 0;
		get_vt_tag_value(m_index);
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::on_en_change_time_sec()
{
	if (mm_time_sec.m_b_entry_done)
	{
		mm_time_sec.on_en_change(this, m_time_sec, 1.f, -1.f);
		// update dependent parameters
		if (m_time_sec < 0)
			m_time_sec = 0.0f;
		if (m_time_sec >= m_very_last)
			m_time_sec = m_very_last;
		UpdateData(FALSE);
		const auto lk = static_cast<long>(m_time_sec * m_sampling_rate);
		if (m_index >= 0 && m_index < m_nb_tags)
		{
			m_p_chart_data_wnd->vt_tags.set_value_long(m_index, lk);
			m_p_chart_data_wnd->Invalidate();
		}
	}
}

// change duration parameter - duration of the stimulation or 
// interval between 2 consecutive tags defining a stimulation pulse
// tag(n) and tag(n+1)
void CMeasureVTtagsPage::on_en_change_duration()
{
	if (mm_duration.m_b_entry_done)
	{
		mm_duration.on_en_change(this, m_duration, 1.f, -1.f);
		// update dependent parameters
		if (m_duration < 0.)
			m_duration = 0.0f;
		if (m_duration >= m_period)
			m_duration = m_period;
		m_p_options_measure->duration = m_duration;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::on_en_change_period()
{
	if (mm_period.m_b_entry_done)
	{
		mm_period.on_en_change(this, m_period, 1.f, -1.f);
		// update dependent parameters
		if (m_period < m_duration)
			m_period = m_duration;
		m_p_options_measure->period = m_period;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::on_en_change_n_periods_edit()
{
	if (mm_n_periods.m_b_entry_done)
	{
		mm_n_periods.on_en_change(this, m_n_periods, 1, -1);
		// update dependent parameters
		if (m_n_periods < 1)
			m_n_periods = 1;
		m_p_options_measure->n_periods = m_n_periods;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::on_en_change_time_shift()
{
	if (mm_time_shift.m_b_entry_done)
	{
		mm_time_shift.on_en_change(this, m_time_shift, 1.f, -1.f);
		// update dependent parameters
		m_p_options_measure->time_shift = m_time_shift;
		UpdateData(FALSE);
	}
}

void CMeasureVTtagsPage::on_shift_tags()
{
	const auto offset = static_cast<long>(m_time_shift * m_sampling_rate);
	for (auto i = 0; i < m_nb_tags; i++)
		m_p_chart_data_wnd->vt_tags.set_value_long(i, m_p_chart_data_wnd->vt_tags.get_tag_value_long(i) + offset);
	// update data
	m_p_chart_data_wnd->Invalidate();
	get_vt_tag_value(m_index);
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::on_add_tags()
{
	float time;
	float time_end;

	// compute limits
	if (!m_p_options_measure->b_set_tags_for_complete_file)
	{
		m_nb_tags = m_p_chart_data_wnd->vt_tags.get_tag_list_size();
		time = static_cast<float>(m_p_chart_data_wnd->vt_tags.get_tag_value_long(m_nb_tags - 1)) / m_sampling_rate;
		time_end = m_period * static_cast<float>(m_n_periods) + time;
		// delete this one which will be re-created within the loop
		m_p_chart_data_wnd->vt_tags.remove_tag(m_nb_tags - 1);
		m_nb_tags--;
	}
	// total file, start at zero
	else
	{
		time = 0.0f;
		time_end = m_very_last;
	}

	// loop until requested interval is completed
	const auto time0 = time;
	auto n_intervals = 0.0f;
	while (time <= time_end)
	{
		m_p_chart_data_wnd->vt_tags.add_l_tag(static_cast<long>(time * m_sampling_rate), 0);
		m_p_chart_data_wnd->vt_tags.add_l_tag(static_cast<long>((time + m_duration) * m_sampling_rate), 0);
		n_intervals++;
		time = time0 + m_period * n_intervals;
	}
	m_nb_tags += static_cast<int>(n_intervals) * 2;
	m_p_chart_data_wnd->Invalidate();
	UpdateData(FALSE);
}

void CMeasureVTtagsPage::on_delete_series()
{
	// delete present tags
	auto p_tags_list = m_p_dat_doc->get_vt_tags_list();
	m_p_chart_data_wnd->vt_tags.copy_tag_list(p_tags_list);
	m_nb_tags = p_tags_list->get_tag_list_size();
	m_p_chart_data_wnd->Invalidate();
}

void CMeasureVTtagsPage::on_delete_all()
{
	m_p_chart_data_wnd->vt_tags.remove_all_tags();
	m_p_chart_data_wnd->Invalidate();
	m_nb_tags = 0;
	get_vt_tag_value(0);
	UpdateData(FALSE);
}
