#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "SpikeClassListBox.h"

#include "DlgListBClaSize.h"
#include "SpikeClassRowItem.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BEGIN_MESSAGE_MAP(SpikeClassListBox, CListBox)
	ON_WM_SIZE()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

SpikeClassListBox::SpikeClassListBox()
{
	context_.m_brush_background.CreateSolidBrush(context_.m_color_background);
}

SpikeClassListBox::~SpikeClassListBox()
= default;

void SpikeClassListBox::MeasureItem(const LPMEASUREITEMSTRUCT lp_mis)
{
	lp_mis->itemHeight = context_.m_row_height;
}

void SpikeClassListBox::DrawItem(const LPDRAWITEMSTRUCT lp_dis)
{
	const auto row_item = reinterpret_cast<SpikeClassRowItem*>(lp_dis->itemData);
	row_item->draw_item(lp_dis);
}


void SpikeClassListBox::DeleteItem(const LPDELETEITEMSTRUCT lp_di)
{
	const auto item = reinterpret_cast<SpikeClassRowItem*>(lp_di->itemData);
	delete item;
}

void SpikeClassListBox::set_row_height(const int row_height)
{
	context_.m_row_height = row_height;
	for (auto n_index = 0; n_index < GetCount(); n_index++)
		SetItemHeight(n_index, row_height);
}

void SpikeClassListBox::set_left_column_width(const int row_width)
{
	context_.m_left_column_width = row_width;
}

void SpikeClassListBox::set_columns_width(const int width_spikes, const int width_separator)
{
	context_.m_width_spikes = width_spikes;
	context_.m_width_separator = width_separator;
	context_.m_width_text = context_.m_left_column_width - width_spikes - 2 * width_separator;
	CRect rect;
	GetClientRect(rect);
	context_.m_width_bars = rect.Width() - context_.m_left_column_width;
}

int SpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lp_cis)
{
	const auto row_item1 = reinterpret_cast<SpikeClassRowItem*>(lp_cis->itemData1);
	const auto row_item2 = reinterpret_cast<SpikeClassRowItem*>(lp_cis->itemData2);

	auto result = 1; 
	if (row_item1->get_class_id() == row_item2->get_class_id())
		result = 0;
	else if (row_item1->get_class_id() < row_item2->get_class_id())
		result = -1;

	return result;
}

void SpikeClassListBox::set_source_data(SpikeList* p_s_list, CdbWaveDoc* pdb_doc) // TODO
{
	// erase content of the list box
	SetRedraw(FALSE);
	ResetContent();

	m_dbwave_doc_ = pdb_doc;
	m_spike_list_ = p_s_list;
	if (p_s_list == nullptr || pdb_doc == nullptr)
		return;
	m_spike_doc_ = pdb_doc->m_p_spk_doc;
	
	// add as many windows as necessary; store pointer into listbox
	auto i_id = 0;
	const int n_classes = count_classes_in_current_spike_list();
	for (auto i_row = 0; i_row < n_classes; i_row++)
	{
		const int class_id = m_spike_list_->get_class_id(i_row);
		add_row_item(class_id, i_id); 
		i_id += 2;
	}
	SetRedraw(TRUE);
}

SpikeClassRowItem* SpikeClassListBox::add_row_item(const int class_id, const int i_id)
{
	const auto row_item = new(SpikeClassRowItem);
	ASSERT(row_item != NULL);
	row_item->create_item(this, m_dbwave_doc_, m_spike_list_, class_id, i_id, &context_);
	AddString(reinterpret_cast<LPTSTR>(row_item));
	return row_item;
}

void SpikeClassListBox::set_time_intervals(const long l_first, const long l_last)
{
	m_l_first_ = l_first;
	m_l_last_ = l_last;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_time_intervals(l_first, l_last);
	}
}

int SpikeClassListBox::count_classes_in_current_spike_list() const
{
	m_spike_list_->update_class_list();
	return m_spike_list_->get_classes_count();
}

void SpikeClassListBox::set_spk_list(SpikeList* p_spike_list)
{
	m_spike_list_ = p_spike_list;

	if (count_classes_in_current_spike_list() == GetCount())
	{
		for (auto i = 0; i < GetCount(); i++)
		{
			const auto row_item = get_row_item(i);
			row_item->set_spk_list(p_spike_list);
		}
	}
	else
	{
		set_source_data(p_spike_list, nullptr);
	}
}

int SpikeClassListBox::select_spike(db_spike& spike_selected)
{
	/*
	BOOL bAll = TRUE;
	int cla = 0;
	if (spike_no >= 0)
	{
		cla = m_pSList->get_class_id(spike_no);		// load spike class
		bAll = FALSE;
	}

	int old_spk=0;
	int spki=0;
;*/

	constexpr auto b_multiple_selection = false;
	auto cla = 0;

	// select spike
	if (spike_selected.spike_index >= 0)
	{
		// get address of spike parameters
		const auto p_spike_element = m_spike_list_->get_spike(spike_selected.spike_index);
		cla = p_spike_element->get_class_id();

		// multiple selection
		//if (false)
		//{
		//	auto n_flagged_spikes = m_spike_list_->toggle_spike_flag(spike_selected.spike_index);
		//	if (m_spike_list_->get_spike_flag_array_count() < 1)
		//		spike_selected.spike_index = -1;
		//}
		// single selection
		m_spike_list_->set_spike_flag(spike_selected.spike_index, true);
	}
	// un-select all spikes
	else
		m_spike_list_->clear_flagged_spikes();

	// select corresponding row
	if (spike_selected.spike_index >= 0)
	{
		for (auto i = 0; i < GetCount(); i++) // search row where this class is stored
		{
			const auto row_item = get_row_item(i); 
			if (row_item->get_class_id() == cla)
			{
				SetCurSel(i);
				break;
			}
		}
	}
	Invalidate();

	// return spike selected
	auto spike_sel = 0;
	if (spike_selected.spike_index > 0)
		spike_sel = spike_selected.spike_index;
	return spike_sel;
}

int SpikeClassListBox::set_mouse_cursor_type(const int cursor_m) const
{
	auto old_cursor = 0;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		old_cursor = row_item->set_mouse_cursor_type(cursor_m);
	}
	return old_cursor;
}

void SpikeClassListBox::OnSize(UINT n_type, int cx, int cy)
{
	CListBox::OnSize(n_type, cx, cy);
	context_.m_width_bars = cx - context_.m_left_column_width;
	// move all windows out of the way to prevent displaying old rows
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->move_row_out_of_the_way();
	}
}

void SpikeClassListBox::set_y_zoom(int y_we, int y_wo) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_y_zoom(y_we, y_wo);
	}
}

void SpikeClassListBox::set_x_zoom(int x_we, int x_wo) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_x_zoom(x_we, x_wo);
	}
}

int SpikeClassListBox::get_yw_extent() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo;
	row_item->get_zoom_y(we, wo);
	return we;
}

int SpikeClassListBox::get_yw_org() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo;
	row_item->get_zoom_y(we, wo);
	return wo;
}

int SpikeClassListBox::get_xw_extent() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo = 0;
	row_item->get_zoom_x_shapes(we, wo);
	return we;
}

int SpikeClassListBox::get_xw_org() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo = 0;
	row_item->get_zoom_x_shapes(we, wo);
	return wo;
}

float SpikeClassListBox::get_extent_mv() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	const auto x = row_item->get_zoom_y_shapes_mv();
	return x;
}


HBRUSH SpikeClassListBox::CtlColor(CDC* p_dc, UINT n_ctl_color)
{
	p_dc->SetTextColor(context_.m_color_text);
	p_dc->SetBkColor(context_.m_color_background);
	return context_.m_brush_background;
}

int SpikeClassListBox::get_row_index_of_spike_class(int spike_class) const
{
	int row_index = -1;
	for (int index = 0; index < GetCount(); index++)
	{
		const auto row_item = get_row_item(index);
		if (row_item->get_class_id() == spike_class)
		{
			row_index = index;
			break;
		}
	}
	return row_index;
}

void SpikeClassListBox::remove_spike_from_row(const int spike_no) 
{
	const auto current_class = m_spike_list_->get_spike(spike_no)->get_class_id();
	const int row_index = get_row_index_of_spike_class(current_class);
	if (row_index < 0)
		return;

	const auto row_item = get_row_item(row_index);
	row_item->select_individual_spike(-1);
	const auto n_spikes = m_spike_list_->decrement_class_n_items(current_class);
	if (n_spikes > 0)
		row_item->update_string(current_class, n_spikes);
	else
	{
		DeleteString(row_index);
	}
}

void SpikeClassListBox::add_spike_to_row(const int spike_no)
{
	const auto class_id = m_spike_list_->get_spike(spike_no)->get_class_id();

	int row_index = get_row_index_of_spike_class(class_id);
	int n_spikes = 1;
	if (row_index < 0)
	{
		m_spike_list_->add_class_id(class_id);
		
		const auto l_first = m_l_first_;
		const auto l_last = m_l_last_;
		set_source_data(m_spike_list_, m_dbwave_doc_);
		set_time_intervals(l_first, l_last);
		row_index = get_row_index_of_spike_class(class_id);
		if (row_index < 0)
			return;
	}
	else
		n_spikes = m_spike_list_->increment_class_n_items(class_id);

	const auto row_item = get_row_item(row_index);
	row_item->update_string(class_id, n_spikes);
}

void SpikeClassListBox::change_spike_class(const int spike_no, const int new_class_id)
{
	m_spike_list_->change_spike_class_id(spike_no, new_class_id);
	update_rows_from_spike_list();
}

void SpikeClassListBox::update_rows_from_spike_list() 
{
	const int n_classes = count_classes_in_current_spike_list();
	int n_row_items = GetCount();

	for (auto i_class = 0; i_class < n_classes; i_class++)
	{
		const int spike_list_class_id = m_spike_list_->get_class_id(i_class);
		if (i_class < n_row_items) 
			get_row_item(i_class)->set_class_id(spike_list_class_id);
		else
		{
			const SpikeClassRowItem* row_item = add_row_item(spike_list_class_id, i_class * 2);
			row_item->set_time_intervals(m_l_first_, m_l_last_);
		}
	}

	if (GetCount() > n_classes)
	{
		n_row_items = GetCount() -1;
		for (int i = n_row_items; i >= n_classes; i--)
			DeleteString(i);
	}
	SetRedraw(TRUE);
}

void SpikeClassListBox::print_item(CDC* p_dc, CRect* rect1, const CRect* rect2, const CRect* rect3, const int i) const
{
	if ((i < 0) || (i > GetCount() - 1))
		return;
	const auto row_item = get_row_item(i);
	row_item->print(p_dc, rect1, rect2, rect3);
}

void SpikeClassListBox::xor_temp_vt_tag(const int x_point) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->get_chart_bars()->xor_temp_vt_tag(x_point);
	}
}

void SpikeClassListBox::reflect_bar_mouse_move_message(const HWND h_wnd)
{
	h_wnd_bars_reflect_ = h_wnd;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->get_chart_bars()->reflect_mouse_move_message(h_wnd);
		if (h_wnd != nullptr)
			row_item->get_chart_bars()->set_mouse_cursor_type(CURSOR_CROSS);
	}
}

LRESULT SpikeClassListBox::OnMyMessage(WPARAM w_param, LPARAM l_param)
{
	const auto row_selected = static_cast<int>(HIWORD(l_param)) / 2; // row index
	const int threshold = LOWORD(l_param);
	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
		set_mouse_cursor_type(threshold);
		break;

	case HINT_CHANGE_HZ_LIMITS:
		set_horizontal_limits(row_selected);
		break;

	case HINT_HIT_SPIKE:
		{
			db_spike spike_hit = m_dbwave_doc_->get_spike_hit();
			select_spike(spike_hit);
		}
		break;

	case HINT_CHANGE_ZOOM:
		set_y_zoom(row_selected);
		break;

	case HINT_DROPPED:
		set_class_of_dropped_spike(row_selected);
		break;

	default:
		break;
	}
	// forward message to parent
	GetParent()->PostMessage(WM_MYMESSAGE, w_param, MAKELPARAM(threshold, GetDlgCtrlID()));
	return 0L;
}

void SpikeClassListBox::set_horizontal_limits(const int row_selected)
{
	const auto row_item = get_row_item(row_selected);
	row_item->get_time_intervals(m_l_first_, m_l_last_);
	set_time_intervals(m_l_first_, m_l_last_);
}

void SpikeClassListBox::set_y_zoom(const int row_selected) const
{
	const auto* row_item = get_row_item(row_selected);
	int y_we, y_wo;
	row_item->get_zoom_y(y_we, y_wo);
	set_y_zoom(y_we, y_wo);
}

void SpikeClassListBox::set_class_of_dropped_spike(const int row_selected) const
{
	const auto row_item = get_row_item(row_selected);
	if (row_item != nullptr)
	{
		const int new_class_id = row_item->get_class_id();
		m_spike_list_->change_class_of_flagged_spikes(new_class_id);
	}
}

void SpikeClassListBox::OnMouseMove(const UINT n_flags, CPoint point)
{
	if (h_wnd_bars_reflect_ != nullptr && point.x >= (context_.m_width_text + context_.m_width_spikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(h_wnd_bars_reflect_, &rect0);

		// reflect mouse move message
		::SendMessage(h_wnd_bars_reflect_, WM_MOUSEMOVE, n_flags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnMouseMove(n_flags, point);
}

void SpikeClassListBox::OnLButtonUp(UINT n_flags, CPoint point)
{
	if (h_wnd_bars_reflect_ != nullptr && point.x >= (context_.m_width_text + context_.m_width_spikes))
	{
		// convert coordinates and reflect move message
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(h_wnd_bars_reflect_, &rect0);
		::SendMessage(h_wnd_bars_reflect_, WM_LBUTTONUP, n_flags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnLButtonUp(n_flags, point);
}

void SpikeClassListBox::OnRButtonUp(UINT n_flags, CPoint point)
{
	CListBox::OnRButtonUp(n_flags, point);

	DlgListBClaSize dlg;
	dlg.m_row_height = get_row_height();
	dlg.m_text_col = get_columns_text_width();
	dlg.m_super_p_col = get_columns_spikes_width();
	dlg.m_inter_col_space = get_columns_separator_width();
	if (IDOK == dlg.DoModal())
	{
		set_row_height(dlg.m_row_height);
		set_columns_width(dlg.m_super_p_col, dlg.m_inter_col_space);
		Invalidate();
	}
}