// vdordina.cpp : implementation file
//

#include "StdAfx.h"
//#include "Cscale.h"
//#include "chart.h"
#include "ChartData.h"

#include "resource.h"
#include "DlgDataViewOrdinates.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgDataViewOrdinates dialog
//
// this dialog displays interdependent values
// the values changes the curves display
// curves parameters are: channel (1 selected or all)
//						zero (internal representation) * volts_per_bin
//						extent (internal) * volts_per_bin
//
// the parameters displayed are not the internal values but
// m_x max		 amplitude of curve selected (top of the window) * volt_factor
// m_x min		 amplitude () * volt factor
// m_x center	amplitude of the zero * volt_factor
//
// these parameters are mirrored by parameters without attenuation
// m_max (volts) = m_x max / volt factor
// m_min (volts) = m_x min / volt factor
// m_center (volts) = m_x_center / volt_factor
//
// the formula to convert these params into internal parameters are
//    (if zero volt = 2048)
// zero = ((m_max+m_min)/2) / volts_per_bin + 2048
//				(2048 = 4096(full scale)/2: 12 bits encoding)
// extent = (max-min) / volts_per_bin
// volts_per_pixel = extent * volts_per_bin / line_view.GetRectHeight()
//
// therefore when one parameter is changed, others are modified accordingly:
// (0=unchanged, 1=changed)
//	change ---------- center---max-----min---
//	chan				1		1		1
//	center				1		1		1
//	max					1		1		0
//	min					1		0		1

DlgDataViewOrdinates::DlgDataViewOrdinates(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)	  
{
}

void DlgDataViewOrdinates::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_CHANSELECT, m_chan_select);
	DDX_CBIndex(p_dx, IDC_VERTUNITS, m_i_unit);
	DDX_Text(p_dx, IDC_VERTCENTER, m_x_center);
	DDX_Text(p_dx, IDC_VERTMAX, m_x_max);
	DDX_Text(p_dx, IDC_VERTMIN, m_x_min);
	if (m_b_changed)
	{
		save_chan_list_data(m_channel);
		m_b_changed = FALSE;
	}
}

BEGIN_MESSAGE_MAP(DlgDataViewOrdinates, CDialog)
	ON_CBN_SELCHANGE(IDC_VERTUNITS, on_sel_change_vert_units)
	ON_CBN_SELCHANGE(IDC_CHANSELECT, on_sel_change_chan_select)
	ON_EN_KILLFOCUS(IDC_VERTMAX, on_kill_focus_vert_mx_mi)
	ON_EN_KILLFOCUS(IDC_VERTCENTER, on_kill_focus_vert_center)
	ON_EN_KILLFOCUS(IDC_VERTMIN, on_kill_focus_vert_mx_mi)
END_MESSAGE_MAP()

BOOL DlgDataViewOrdinates::OnInitDialog()
{
	// save extent and zero before calling base class
	// this is an awful patch. But necessary. For some reason really unclear to
	// me, the parameters (zero and extent) of channel 0 / chan_list / line_view
	// are MODIFIED after calling the base class CDialog::OnInitDialog
	int i;
	m_n_chan_max = m_p_chart_data_wnd->get_channel_list_size(); 
	for (i = 0; i < m_n_chan_max; i++) 
	{
		CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(i);
		m_settings_.Add(static_cast<WORD>(chan->get_y_zero())); 
		m_settings_.Add(static_cast<WORD>(chan->get_y_extent())); 
	}
	CDialog::OnInitDialog();

	// load channel description CComboBox
	int j = 0; 
	for (i = 0; i < m_n_chan_max; i++) 
	{
		CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(i);
		chan->set_y_zero(m_settings_.GetAt(j));
		j++; 
		chan->set_y_extent(m_settings_.GetAt(j));
		j++;
		m_chan_select.AddString(chan->get_comment()); 
	}
	m_chan_select.SetCurSel(m_channel); 

	load_chan_list_data(m_channel);
	m_i_unit = 1; // select "mV" as unit within combo box
	static_cast<CComboBox*>(GetDlgItem(IDC_VERTUNITS))->SetCurSel(m_i_unit);
	change_units(m_i_unit, TRUE); 
	UpdateData(FALSE);
	return TRUE;
}

void DlgDataViewOrdinates::load_chan_list_data(int i)
{
	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(i);
	// compute max and min from zero and extent
	m_volts_per_bin_ = chan->get_volts_per_bin();
	const auto zero = static_cast<float>(chan->get_y_zero());
	const auto extent = static_cast<float>(chan->get_y_extent());

	m_x_center = (zero) * m_volts_per_bin_ / m_p10_;
	const auto x_extent = (extent * m_volts_per_bin_ / m_p10_) / 2.f;
	m_x_min = m_x_center - x_extent;
	m_x_max = m_x_center + x_extent;

	m_b_changed = FALSE;
}

void DlgDataViewOrdinates::save_chan_list_data(const int index_list) const
{
	const auto b_check = static_cast<CButton*>(GetDlgItem(IDC_CHECKALL))->GetCheck();
	auto index_first = index_list; 
	auto index_last = index_list + 1; 
	if (b_check) 
	{
		index_first = 0; 
		index_last = m_n_chan_max; 
	}

	for (auto j = index_first; j < index_last; j++)
	{
		CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(j);
		const auto v_per_bin = chan->get_volts_per_bin();
		const auto x_zero = ((m_x_max + m_x_min) / 2.f) * m_p10_;
		const auto x_extent = (m_x_max - m_x_min) * m_p10_;
		auto i = static_cast<int>(x_zero / v_per_bin) + chan->get_data_bin_zero();
		chan->set_y_zero(i);
		i = static_cast<int>(x_extent / v_per_bin);
		chan->set_y_extent(i);
	}
	m_p_chart_data_wnd->Invalidate();
}

void DlgDataViewOrdinates::change_units(const int new_scale, const BOOL b_new)
{
	auto newp10 = m_p10_;
	if (b_new)
	{
		switch (new_scale)
		{
		case 0: newp10 = 1.0f;
			break; // volts
		case 1: newp10 = 0.001f;
			break; // milli-volts
		case 2: newp10 = 0.000001f;
			break; // micro-volts
		default: newp10 = 1.0f;
			break;
		}
	}

	if (newp10 != m_p10_)
	{
		// convert content of control edit printing scaled values
		m_x_center = (m_x_center * m_p10_) / newp10;
		m_x_max = (m_x_max * m_p10_) / newp10;
		m_x_min = (m_x_min * m_p10_) / newp10;
		m_p10_ = newp10;
	}
}

void DlgDataViewOrdinates::on_sel_change_vert_units()
{
	m_i_unit = static_cast<CComboBox*>(GetDlgItem(IDC_VERTUNITS))->GetCurSel();
	change_units(m_i_unit, TRUE);
	UpdateData(FALSE);
}

void DlgDataViewOrdinates::on_sel_change_chan_select()
{
	if (m_b_changed)
		save_chan_list_data(m_channel); 
	const auto new_index = static_cast<CComboBox*>(GetDlgItem(IDC_CHANSELECT))->GetCurSel();
	m_channel = new_index; 
	load_chan_list_data(m_channel);
	change_units(m_i_unit, FALSE); 
	UpdateData(FALSE);
}

void DlgDataViewOrdinates::on_kill_focus_vert_center()
{
	auto diff = m_x_center;
	UpdateData(TRUE);
	if (diff == m_x_center)
		return;
	diff = m_x_center - diff;
	m_x_max += diff;
	m_x_min += diff;

	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(m_channel);
	const auto vper_bin = chan->get_volts_per_bin();
	const auto xzero = ((m_x_max + m_x_min) / 2.f) * m_p10_;
	const auto zero = static_cast<int>(xzero / vper_bin) + chan->get_data_bin_zero();
	chan->set_y_zero(zero);
	m_b_changed = TRUE;
	UpdateData(FALSE);
}

void DlgDataViewOrdinates::on_kill_focus_vert_mx_mi()
{
	UpdateData(TRUE);

	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(m_channel);
	const auto v_per_bin = chan->get_volts_per_bin();
	const auto x_zero = ((m_x_max + m_x_min) / 2.f) * m_p10_;
	const auto x_extent = (m_x_max - m_x_min) * m_p10_;
	const auto zero = static_cast<int>(x_zero / v_per_bin) + chan->get_data_bin_zero();
	const auto extent = static_cast<int>(x_extent / v_per_bin);

	chan->set_y_zero(zero);
	chan->set_y_extent(extent);
	load_chan_list_data(m_channel);
	change_units(m_i_unit, FALSE);

	m_b_changed = TRUE;
	UpdateData(FALSE);
}

void DlgDataViewOrdinates::OnCancel()
{
	// restore extent of the channels
	m_n_chan_max = m_p_chart_data_wnd->get_channel_list_size();
	auto j = 0;
	for (auto i = 0; i < m_n_chan_max; i++)
	{
		CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(i);
		chan->set_y_zero(m_settings_[j]);
		j++;
		chan->set_y_extent(m_settings_[j]);
		j++;
	}
	CDialog::OnCancel();
}

void DlgDataViewOrdinates::OnOK()
{
	// trap CR to validate current field
	switch (GetFocus()->GetDlgCtrlID())
	{
	// select was on OK
	case 1:
		{
			const auto b_check = static_cast<CButton*>(GetDlgItem(IDC_CHECKALL))->GetCheck();
			// exit, do not store changes of current channel -- nothing was changed
			if (m_b_changed || b_check)
				save_chan_list_data(m_channel);
			CDialog::OnOK();
		}
		break;
	// trap return
	case IDC_CHANSELECT:
		break;
	case IDC_VERTCENTER:
		on_kill_focus_vert_center();
		break;
	case IDC_VERTUNITS:
		break;
	case IDC_VERTMAX:
	case IDC_VERTMIN:
		on_kill_focus_vert_mx_mi();
		break;
	default:
		CDialog::OnOK();
		break;
	}
}
