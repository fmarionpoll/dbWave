#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "SpikeClassListBox.h"

#include "DlgListBClaSize.h"
#include "RowItem.h"


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
	context.m_brush_background.CreateSolidBrush(context.m_color_background);
}

SpikeClassListBox::~SpikeClassListBox()
= default;

LRESULT SpikeClassListBox::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	auto i_current_selected = static_cast<int>(HIWORD(lParam)) / 2; // why lParam/2???
	const int threshold = LOWORD(lParam);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		SetMouseCursorType(threshold);
		break;

	case HINT_CHANGEHZLIMITS: // abscissa have changed
	{
		const auto row_item = get_row_item(i_current_selected);
		row_item->get_time_intervals(m_lFirst, m_lLast);
		SetTimeIntervals(m_lFirst, m_lLast);
	}
	break;

	case HINT_HITSPIKE:
		TRACE("hist spike\n");
		if (i_current_selected != threshold) 
		{
			SelectSpike(threshold);
		}
		break;

	case HINT_CHANGEZOOM:
		{
			const auto* row_item = get_row_item(i_current_selected);
			int y_we, y_wo;
			row_item->get_zoom_y(y_we, y_wo);
			SetYzoom(y_we, y_wo);
		}
		break;

	case HINT_DROPPED:
		TRACE("drop spike \n");
		{
			const auto row_item = get_row_item(i_current_selected);
			if (row_item != nullptr) {
				const int new_class_id = row_item->get_class_id();
				m_spike_list->change_class_of_flagged_spikes(new_class_id);
			}
		}
		break;

	default:
		break;
	}
	// forward message to parent
	GetParent()->PostMessage(WM_MYMESSAGE, wParam, MAKELPARAM(threshold, GetDlgCtrlID()));
	return 0L;
}

void SpikeClassListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	lpMIS->itemHeight = context.m_row_height;
}

void SpikeClassListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	const auto row_item = reinterpret_cast<RowItem*>(lpDIS->itemData);
	row_item->DrawItem(lpDIS);
}


void SpikeClassListBox::DeleteItem(LPDELETEITEMSTRUCT lpDI)
{
	const auto item = reinterpret_cast<RowItem*>(lpDI->itemData);
	delete item;
}

void SpikeClassListBox::SetRowHeight(int row_height)
{
	context.m_row_height = row_height;
	for (auto n_index = 0; n_index < GetCount(); n_index++)
		SetItemHeight(n_index, row_height);
}

void SpikeClassListBox::SetLeftColumnWidth(int left_width)
{
	context.m_left_column_width = left_width;
}

void SpikeClassListBox::SetColumnsWidth(int width_spikes, int width_separator)
{
	context.m_widthSpikes = width_spikes;
	context.m_widthSeparator = width_separator;
	context.m_widthText = context.m_left_column_width - width_spikes - 2 * width_separator;
	CRect rect;
	GetClientRect(rect);
	context.m_widthBars = rect.Width() - context.m_left_column_width;
}

int SpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	const auto row_item1 = reinterpret_cast<RowItem*>(lpCIS->itemData1);
	const auto row_item2 = reinterpret_cast<RowItem*>(lpCIS->itemData2);

	auto result = 1; 
	if (row_item1->get_class_id() == row_item2->get_class_id())
		result = 0;
	else if (row_item1->get_class_id() < row_item2->get_class_id())
		result = -1;

	return result;
}

void SpikeClassListBox::set_source_data(SpikeList* pSList, CdbWaveDoc* pdbDoc)
{
	// erase content of the list box
	SetRedraw(FALSE);
	ResetContent();

	if (pdbDoc != nullptr)
	{
		m_dbwave_doc = pdbDoc;
		m_spike_list = pSList;
		if (pSList == nullptr || pdbDoc == nullptr)
			return;
		m_spike_doc = pdbDoc->m_pSpk;
	}

	// add as many windows as necessary; store pointer into listbox
	auto i_id = 0;
	const int n_classes = count_classes_in_current_spike_list();
	for (auto i_row = 0; i_row < n_classes; i_row++)
	{
		const int class_id = m_spike_list->get_class_id(i_row);
		if (class_id < 0)
			continue;
		add_row_item(class_id, i_id);
		i_id += 2;
	}
	SetRedraw(TRUE);
}

RowItem* SpikeClassListBox::add_row_item(int class_id, int i_id)
{
	const auto row_item = new(RowItem);
	ASSERT(row_item != NULL);
	row_item->CreateItem(this, m_dbwave_doc, m_spike_list, class_id, i_id, &context);
	
	AddString(reinterpret_cast<LPTSTR>(row_item));
	return row_item;
}

void SpikeClassListBox::SetTimeIntervals(long l_first, long l_last)
{
	m_lFirst = l_first;
	m_lLast = l_last;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_time_intervals(l_first, l_last);
	}
}

int SpikeClassListBox::count_classes_in_current_spike_list() const
{
	m_spike_list->UpdateClassList();
	const auto n_spikes = m_spike_list->get_spikes_count();
	auto n_classes = 1;
	if (n_spikes > 0)
		n_classes = m_spike_list->get_classes_count();
	return n_classes;
}

void SpikeClassListBox::SetSpkList(SpikeList* p_spike_list)
{
	m_spike_list = p_spike_list;

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

int SpikeClassListBox::SelectSpike(int spike_no)
{
	/*
	BOOL bAll = TRUE;
	int cla = 0;
	if (spike_no >= 0)
	{
		cla = m_pSList->get_class_id(spike_no);		// load spike class
		bAll = FALSE;
	}

	int oldspk=0;
	int spki=0;
;*/

	const auto b_multiple_selection = FALSE;
	auto cla = 0;

	// select spike
	if (spike_no >= 0)
	{
		// get address of spike parameters
		const auto p_spike_element = m_spike_list->get_spike(spike_no);
		cla = p_spike_element->get_class_id();

		// multiple selection
		if (false)
		{
			auto nflaggedspikes = m_spike_list->ToggleSpikeFlag(spike_no);
			if (m_spike_list->GetSpikeFlagArrayCount() < 1)
				spike_no = -1;
		}
		// single selection
		m_spike_list->set_single_spike_flag(spike_no);
	}
	// un-select all spikes
	else
		m_spike_list->RemoveAllSpikeFlags();

	// select corresponding row
	if (spike_no >= 0)
	{
		for (auto i = 0; i < GetCount(); i++) // search row where this class is stored
		{
			const auto row_item = get_row_item(i); 
			if (row_item->get_class_id() == cla)
			{
				SetCurSel(i); // select corresponding row
				break;
			}
		}
	}
	Invalidate();

	// return spike selected
	auto spike_selected = 0;
	if (spike_no > 0)
		spike_selected = spike_no;
	return spike_selected;
}

int SpikeClassListBox::SetMouseCursorType(int cursor_m) const
{
	auto old_cursor = 0;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		old_cursor = row_item->set_mouse_cursor_type(cursor_m);
	}
	return old_cursor;
}

void SpikeClassListBox::OnSize(UINT nType, int cx, int cy)
{
	CListBox::OnSize(nType, cx, cy);
	context.m_widthBars = cx - context.m_left_column_width;
	// move all windows out of the way to prevent displaying old rows
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->move_row_out_of_the_way();
	}
}

void SpikeClassListBox::SetYzoom(int y_we, int y_wo) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_y_zoom(y_we, y_wo);
	}
}

void SpikeClassListBox::SetXzoom(int x_we, int x_wo) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->set_x_zoom(x_we, x_wo);
	}
}

int SpikeClassListBox::GetYWExtent() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo;
	row_item->get_zoom_y(we, wo);
	return we;
}

int SpikeClassListBox::GetYWOrg() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo;
	row_item->get_zoom_y(we, wo);
	return wo;
}

int SpikeClassListBox::GetXWExtent() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo = 0;
	row_item->get_zoom_x_shapes(we, wo);
	return we;
}

int SpikeClassListBox::GetXWOrg() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	int we, wo = 0;
	row_item->get_zoom_x_shapes(we, wo);
	return wo;
}

float SpikeClassListBox::GetExtent_mV() const
{
	ASSERT(GetCount() > 0);
	const auto row_item = get_row_item(0);
	const auto x = row_item->get_zoom_y_shapes_mv();
	return x;
}

void SpikeClassListBox::OnRButtonUp(UINT nFlags, CPoint point)
{
	CListBox::OnRButtonUp(nFlags, point);

	DlgListBClaSize dlg;
	dlg.m_rowheight = GetRowHeight();
	dlg.m_textcol = GetColumnsTextWidth();
	dlg.m_superpcol = GetColumnsSpikesWidth();
	dlg.m_intercolspace = GetColumnsSeparatorWidth();
	if (IDOK == dlg.DoModal())
	{
		SetRowHeight(dlg.m_rowheight);
		SetColumnsWidth(dlg.m_superpcol, dlg.m_intercolspace);
		Invalidate();
	}
}

HBRUSH SpikeClassListBox::CtlColor(CDC* p_dc, UINT nCtlColor)
{
	p_dc->SetTextColor(context.m_color_text);
	p_dc->SetBkColor(context.m_color_background);
	return context.m_brush_background;
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

void SpikeClassListBox::remove_spike_from_row(int spike_no) 
{
	const auto current_class = m_spike_list->get_spike(spike_no)->get_class_id();
	const int row_index = get_row_index_of_spike_class(current_class);
	if (row_index < 0)
		return;

	const auto row_item = get_row_item(row_index);
	row_item->select_individual_spike(-1);
	const auto n_spikes = m_spike_list->decrement_class_n_items(current_class);
	if (n_spikes > 0)
		row_item->update_string(current_class, n_spikes);
	else
	{
		DeleteString(row_index);
	}
}

void SpikeClassListBox::add_spike_to_row(int spike_no)
{
	const auto current_class = m_spike_list->get_spike(spike_no)->get_class_id();
	if (current_class < 0)
		return;
	int row_index = get_row_index_of_spike_class(current_class);
	int n_spikes = 1;
	if (row_index < 0)
	{
		m_spike_list->add_class_id(current_class);
		
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		set_source_data(m_spike_list, m_dbwave_doc);
		SetTimeIntervals(l_first, l_last);
		row_index = get_row_index_of_spike_class(current_class);
		if (row_index < 0)
			return;
	}
	else
		n_spikes = m_spike_list->increment_class_n_items(current_class);

	const auto row_item = get_row_item(row_index);
	row_item->update_string(current_class, n_spikes);
}

void SpikeClassListBox::ChangeSpikeClass(int spike_no, int new_class_id)
{
	m_spike_list->change_spike_class_id(spike_no, new_class_id);
	update_rows_from_spike_list();
}

void SpikeClassListBox::update_rows_from_spike_list()
{
	const int n_classes = count_classes_in_current_spike_list();
	const int n_row_items = GetCount();
	int i_id = 0;
	int i_row = 0;
	if (n_row_items > 0)
	{
		const auto lastItem = get_row_item(n_row_items - 1);
		i_id = (lastItem->get_row_id() / 2 + 1) * 2;
	}

	for (auto i_class = 0; i_class < n_classes; i_class++)
	{
		const int class_id = m_spike_list->get_class_id(i_class);
		if (class_id < 0)
			continue;

		const int descriptor_index = m_spike_list->get_class_id_descriptor_index(class_id);
		const auto row_item = get_row_item(i_row);
		if (i_row < n_row_items)
		{
			row_item->set_class_id(class_id);
			row_item->update_string(class_id, m_spike_list->get_class_n_items(descriptor_index));
		}
		else
		{
			add_row_item(class_id, i_id);
		}
		i_id += 2;
		i_row++;
	}

	if (i_row <= n_classes)
	{
		for (int i = n_classes - 1; i >= i_row; i--)
			DeleteString(i);
	}
	SetRedraw(TRUE);
}

void SpikeClassListBox::PrintItem(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3, int i) const
{
	if ((i < 0) || (i > GetCount() - 1))
		return;
	const auto row_item = get_row_item(i);
	row_item->print(p_dc, rect1, rect2, rect3);
}

void SpikeClassListBox::XorTempVTtag(int x_point) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->get_chart_bars()->XorTempVTtag(x_point);
	}
}

void SpikeClassListBox::ResetBarsXortag() const
{
	for (int i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->get_chart_bars()->ResetXortag();
	}
}

void SpikeClassListBox::ReflectBarsMouseMoveMessg(const HWND hwnd)
{
	m_hwnd_bars_reflect = hwnd;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = get_row_item(i);
		row_item->get_chart_bars()->ReflectMouseMoveMessg(hwnd);
		if (hwnd != nullptr)
			row_item->get_chart_bars()->SetMouseCursorType(CURSOR_CROSS);
	}
}

void SpikeClassListBox::OnMouseMove(const UINT nFlags, CPoint point)
{
	if (m_hwnd_bars_reflect != nullptr && point.x >= (context.m_widthText + context.m_widthSpikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwnd_bars_reflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwnd_bars_reflect, WM_MOUSEMOVE, nFlags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnMouseMove(nFlags, point);
}

void SpikeClassListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_hwnd_bars_reflect != nullptr && point.x >= (context.m_widthText + context.m_widthSpikes))
	{
		// convert coordinates and reflect move message
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwnd_bars_reflect, &rect0);
		::SendMessage(m_hwnd_bars_reflect, WM_LBUTTONUP, nFlags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnLButtonUp(nFlags, point);
}
