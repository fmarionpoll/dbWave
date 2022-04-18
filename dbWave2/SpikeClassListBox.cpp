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

SpikeClassListBox::SpikeClassListBox() : m_color_background(GetSysColor(COLOR_SCROLLBAR))
{
	m_brush_background.CreateSolidBrush(m_color_background);
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
			const RowItem* row_item = reinterpret_cast<RowItem*>(GetItemData(i_current_selected));
			m_lFirst = (row_item->chart_bars)->GetTimeFirst();
			m_lLast = (row_item->chart_bars)->GetTimeLast();
			SetTimeIntervals(m_lFirst, m_lLast);
		}
		break;

	case HINT_HITSPIKE: // spike is selected
		SelectSpike(threshold);
		m_spike_hit = threshold; 
		m_spike_hit_row = i_current_selected; 
		m_is_spike_hit = TRUE;
		break;

	case HINT_CHANGEZOOM:
		{
			const auto* row_item = reinterpret_cast<RowItem*>(GetItemData(i_current_selected));
			const auto y_we = row_item->chart_bars->GetYWExtent();
			const auto y_wo = row_item->chart_bars->GetYWOrg();
			SetYzoom(y_we, y_wo);
		}
		break;

	case HINT_DROPPED: 
		if (!m_is_spike_hit)
			return 0L;
	// change selection
		if (i_current_selected != m_spike_hit_row)
		{
			// patch: when we displace a spike to line 0, the line nb is not correct (shadow window intercepting mouse?)
			if (i_current_selected < 0 || i_current_selected > GetCount())
				i_current_selected = 0;
			ChangeSpikeClass(m_spike_hit, m_spike_list->get_class_id(i_current_selected));
		}
		m_is_spike_hit = FALSE;
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
	lpMIS->itemHeight = m_row_height;
}

void SpikeClassListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC dc;
	dc.Attach(lpDIS->hDC);

	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		// get data
		CRect rc_text = lpDIS->rcItem;
		rc_text.right = rc_text.left + m_widthText;
		const auto row_item = reinterpret_cast<RowItem*>(lpDIS->itemData);

		// display text
		const auto text_length = (row_item->comment)->GetLength();
		dc.DrawText(*(row_item->comment), text_length, rc_text, DT_LEFT | DT_WORDBREAK);

		// display spikes
		const auto col1 = m_widthText + m_widthSeparator;
		const auto col2 = col1 + m_widthSpikes + m_widthSeparator;
		auto rect_spikes = CRect(col1 + 1, lpDIS->rcItem.top + 1, col1 + m_widthSpikes, lpDIS->rcItem.bottom - 1);
		row_item->chart_shapes->sub_item_draw(dc, rect_spikes);

		// display bars
		auto rect_bars = CRect(col2 + 1, lpDIS->rcItem.top + 1, col2 + m_widthBars, lpDIS->rcItem.bottom - 1);
		row_item->chart_bars->sub_item_draw(dc, rect_bars);
	}

	// item is selected -- add frame
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		constexpr auto color_blue = RGB(0, 0, 255); 
		CBrush br(color_blue);
		dc.FrameRect(&(lpDIS->rcItem), &br);
	}

	// Item has been de-selected -- remove frame
	if (!(lpDIS->itemState & ODS_SELECTED) && // item not selected
		(lpDIS->itemAction & ODA_SELECT))
	{
		dc.FrameRect(&(lpDIS->rcItem), &m_brush_background);
	}
	dc.Detach();
}


void SpikeClassListBox::DeleteItem(LPDELETEITEMSTRUCT lpDI)
{
	const auto item = reinterpret_cast<RowItem*>(lpDI->itemData);
	delete item;
}

void SpikeClassListBox::SetRowHeight(int row_height)
{
	m_row_height = row_height;
	for (auto n_index = 0; n_index < GetCount(); n_index++)
		SetItemHeight(n_index, row_height);
}

void SpikeClassListBox::SetLeftColWidth(int left_width)
{
	m_left_column_width = left_width;
}

void SpikeClassListBox::SetColsWidth(int width_spikes, int width_separator)
{
	m_widthSpikes = width_spikes;
	m_widthSeparator = width_separator;
	m_widthText = m_left_column_width - width_spikes - 2 * width_separator;
	CRect rect;
	GetClientRect(rect);
	m_widthBars = rect.Width() - m_left_column_width;
}

int SpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	const auto row_item1 = reinterpret_cast<RowItem*>(lpCIS->itemData1);
	const auto row_item2 = reinterpret_cast<RowItem*>(lpCIS->itemData2);

	auto result = 1; 
	if (row_item1->class_id == row_item2->class_id)
		result = 0;
	else if (row_item1->class_id < row_item2->class_id)
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
	const auto rect_spikes = CRect(0, 0, 0, 0); 
	const auto rect_bars = CRect(0, 0, 0, 0);

	auto i_id = 0;
	const int n_classes = count_classes_in_current_spike_list();
	
	for (auto i_row = 0; i_row < n_classes; i_row++)
	{
		const auto i_class = m_spike_list->get_class_id(i_row);
		const auto row_item = new(RowItem);
		ASSERT(row_item != NULL);
		row_item->CreateItem(m_dbwave_doc, m_spike_list, i_class, i_id);
		i_id += 2;
		AddString(reinterpret_cast<LPTSTR>(row_item));
	}
	// exit: allow data redrawing
	SetRedraw(TRUE);
}


void SpikeClassListBox::SetTimeIntervals(long l_first, long l_last)
{
	m_lFirst = l_first;
	m_lLast = l_last;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		if (row_item->chart_shapes != nullptr)
		{
			(row_item->chart_shapes)->SetRangeMode(RANGE_TIMEINTERVALS);
			(row_item->chart_shapes)->SetTimeIntervals(l_first, l_last);
		}
		(row_item->chart_bars)->SetRangeMode(RANGE_TIMEINTERVALS);
		(row_item->chart_bars)->SetTimeIntervals(l_first, l_last);
	}
}

int SpikeClassListBox::count_classes_in_current_spike_list()
{
	if (!m_spike_list->IsClassListValid())
		m_spike_list->UpdateClassList();
	const auto nspikes = m_spike_list->get_spikes_count();
	auto nbclasses = 1;
	if (nspikes > 0)
		nbclasses = m_spike_list->get_classes_count();
	return nbclasses;
}

void SpikeClassListBox::SetSpkList(SpikeList* p_spike_list)
{
	m_spike_list = p_spike_list;

	if (count_classes_in_current_spike_list() == GetCount())
	{
		for (auto i = 0; i < GetCount(); i++)
		{
			const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
			(row_item->chart_shapes)->SetSpkList(p_spike_list);
			(row_item->chart_bars)->SetSpkList(p_spike_list);
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
		cla = m_pSList->get_class(spike_no);		// load spike class
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
		// get address of spike parms
		const auto p_spike_element = m_spike_list->get_spike(spike_no);
		cla = p_spike_element->get_class();

		// multiple selection
		if (false)
		{
			auto nflaggedspikes = m_spike_list->ToggleSpikeFlag(spike_no);
			if (m_spike_list->GetSpikeFlagArrayCount() < 1)
				spike_no = -1;
		}
		// single selection
		m_spike_list->SetSingleSpikeFlag(spike_no);
	}
	// un-select all spikes
	else
		m_spike_list->RemoveAllSpikeFlags();

	// select corresponding row
	if (spike_no >= 0)
	{
		for (auto i = 0; i < GetCount(); i++) // search row where this class is stored
		{
			const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i)); 
			if (row_item->class_id == cla)
			{
				SetCurSel(i); // select corresponding row
				break;
			}
		}
	}

	// update display
	Invalidate();

	// return spike selected
	auto oldspk = 0;
	if (spike_no > 0)
		oldspk = spike_no;
	return oldspk;
}

int SpikeClassListBox::SetMouseCursorType(int cursor_m)
{
	auto old_cursor = 0;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		if (row_item->chart_shapes != nullptr)
			(row_item->chart_shapes)->SetMouseCursorType(cursor_m);
		old_cursor = (row_item->chart_bars)->SetMouseCursorType(cursor_m);
	}
	return old_cursor;
}

void SpikeClassListBox::OnSize(UINT nType, int cx, int cy)
{
	CListBox::OnSize(nType, cx, cy);
	m_widthBars = cx - m_left_column_width;
	// move all windows out of the way to prevent displaying old rows
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		CRect rect(0, 0, 0, 0);
		if (row_item->chart_shapes != nullptr)
			(row_item->chart_shapes)->MoveWindow(rect, FALSE);
		(row_item->chart_bars)->MoveWindow(rect, FALSE);
	}
}

void SpikeClassListBox::SetYzoom(int y_we, int y_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		if (row_item->chart_shapes != nullptr)
			(row_item->chart_shapes)->SetYWExtOrg(y_we, y_wo);
		(row_item->chart_bars)->SetYWExtOrg(y_we, y_wo);
	}
}

void SpikeClassListBox::SetXzoom(int x_we, int x_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		if (row_item->chart_shapes != nullptr)
			(row_item->chart_shapes)->SetXWExtOrg(x_we, x_wo);
	}
}

int SpikeClassListBox::GetYWExtent()
{
	ASSERT(GetCount() > 0);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(0));
	return (row_item->chart_bars)->GetYWExtent();
}

int SpikeClassListBox::GetYWOrg()
{
	ASSERT(GetCount() > 0);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(0));
	return (row_item->chart_bars)->GetYWOrg();
}

int SpikeClassListBox::GetXWExtent()
{
	ASSERT(GetCount() > 0);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(0));
	auto i = 0;
	if ((row_item->chart_shapes) != nullptr)
		i = (row_item->chart_shapes)->GetXWExtent();
	return i;
}

int SpikeClassListBox::GetXWOrg()
{
	ASSERT(GetCount() > 0);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(0));
	auto i = 0;
	if ((row_item->chart_shapes) != nullptr)
		i = (row_item->chart_shapes)->GetXWOrg();
	return i;
}

float SpikeClassListBox::GetExtent_mV()
{
	ASSERT(GetCount() > 0);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(0));
	auto x = 0.f;
	if ((row_item->chart_shapes) != nullptr)
		x = (row_item->chart_shapes)->GetExtent_mV();
	return x;
}

void SpikeClassListBox::OnRButtonUp(UINT nFlags, CPoint point)
{
	CListBox::OnRButtonUp(nFlags, point);
	// change row and col sizes
	DlgListBClaSize dlg;
	dlg.m_rowheight = GetRowHeight();
	dlg.m_textcol = GetColsTextWidth();
	dlg.m_superpcol = GetColsSpikesWidth();
	dlg.m_intercolspace = GetColsSeparatorWidth();
	if (IDOK == dlg.DoModal())
	{
		SetRowHeight(dlg.m_rowheight);
		SetColsWidth(dlg.m_superpcol, dlg.m_intercolspace);
		Invalidate();
	}
}

HBRUSH SpikeClassListBox::CtlColor(CDC* p_dc, UINT nCtlColor)
{
	p_dc->SetTextColor(m_color_text);
	p_dc->SetBkColor(m_color_background); 
	return m_brush_background;
}

int SpikeClassListBox::get_row_index_of_spike_class(int spike_class) const
{
	int row_index = -1;
	for (int index = 0; index < GetCount(); index++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(index));
		if ((row_item->chart_bars)->GetSelClass() == spike_class)
		{
			row_index = index;
			break;
		}
	}
	return row_index;
}

void SpikeClassListBox::remove_spike_from_row(int spike_no)
{
	const auto old_class = m_spike_list->get_spike(spike_no)->get_class();
	const int row_old_index = get_row_index_of_spike_class(old_class);
	if (row_old_index < 0)
		return;
	const auto row_old_item = reinterpret_cast<RowItem*>(GetItemData(row_old_index));

	if (row_old_item->chart_shapes != nullptr)
		(row_old_item->chart_shapes)->SelectSpikeShape(-1);
	row_old_item->chart_bars->SelectSpike(-1);
	const auto n_spikes = m_spike_list->decrement_class_n_items(old_class);

	// reset all if row should be removed
	if (n_spikes > 0)
	{
		update_string(row_old_item, old_class, n_spikes);
	}
}

void SpikeClassListBox::add_spike_to_row(int spike_no)
{
	const auto current_class = m_spike_list->get_spike(spike_no)->get_class();

	int row_index = get_row_index_of_spike_class(current_class);
	if (row_index < 0)
	{
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		set_source_data(m_spike_list, m_dbwave_doc);
		SetTimeIntervals(l_first, l_last);
		// TODO here: add row
		row_index = get_row_index_of_spike_class(current_class);
	}


	const int n_spikes = m_spike_list->increment_class_n_items(current_class);
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(row_index));
	update_string(row_item, current_class, n_spikes);
}

void SpikeClassListBox::ChangeSpikeClass(int spike_no, int new_class)
{
	if (0 == GetCount() || new_class == m_spike_list->get_spike(spike_no)->get_class())
		return;

	// ---------------- 1) old spike : deselect spike and remove from corresp line (destroy?)
	remove_spike_from_row(spike_no);

	// ---------------- 2) new class? add to other row and select
	m_spike_list->get_spike(spike_no)->set_class(new_class);
	add_spike_to_row(spike_no);


	//// new row?
	//RowItem* row_item = nullptr;
	//int i_row;
	//for (i_row = 0; i_row < GetCount(); i_row++)
	//{
	//	row_item = reinterpret_cast<RowItem*>(GetItemData(i_row)); 
	//	if ((row_item->chart_bars)->GetSelClass() == new_class)
	//		break;
	//}
	//// reset all if line ought to be added
	//if (i_row >= GetCount())
	//{
	//	const auto l_first = m_lFirst;
	//	const auto l_last = m_lLast;
	//	set_source_data(m_spike_list, m_dbwave_doc);
	//	SetTimeIntervals(l_first, l_last);
	//}
	//else
	//{
	//	int n_spikes = m_spike_list->get_class_n_items(i_row) + 1;
	//	m_spike_list->set_class_n_items(i_row, n_spikes);
	//	update_string(row_item, new_class, n_spikes);
	//}

	//CRect rect;
	//if (!GetItemRect(i_row, &rect))
	//	return;
	SelectSpike(spike_no);
}

void SpikeClassListBox::update_string(void* ptr, int i_class, int n_spikes)
{
	const auto row_item = static_cast<RowItem*>(ptr);
	delete row_item->comment;
	const auto c_string = new CString;
	ASSERT(c_string != NULL);
	c_string->Format(_T("class %i\nn=%i"), i_class, n_spikes);
	row_item->comment = c_string;
}

void SpikeClassListBox::PrintItem(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3, int i) const
{
	if ((i < 0) || (i > GetCount() - 1))
		return;
	const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));

	// print text
	const auto text_length = (row_item->comment)->GetLength();
	p_dc->DrawText(*(row_item->comment), text_length, rect1, DT_LEFT | DT_WORDBREAK);

	// spike shape
	if (row_item->chart_shapes != nullptr)
		row_item->chart_shapes->Print(p_dc, rect2);

	// spike bars
	if (row_item->chart_bars != nullptr)
		row_item->chart_bars->Print(p_dc, rect3);
}

void SpikeClassListBox::XorTempVTtag(int x_point) const
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		row_item->chart_bars->XorTempVTtag(x_point);
	}
}

void SpikeClassListBox::ResetBarsXortag() const
{
	for (int i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		row_item->chart_bars->ResetXortag();
	}
}

void SpikeClassListBox::ReflectBarsMouseMoveMessg(const HWND hwnd)
{
	m_hwnd_bars_reflect = hwnd;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto row_item = reinterpret_cast<RowItem*>(GetItemData(i));
		(row_item->chart_bars)->ReflectMouseMoveMessg(hwnd);
		if (hwnd != nullptr)
			row_item->chart_bars->SetMouseCursorType(CURSOR_CROSS);
	}
}

void SpikeClassListBox::OnMouseMove(const UINT nFlags, CPoint point)
{
	if (m_hwnd_bars_reflect != nullptr && point.x >= (m_widthText + m_widthSpikes))
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
	if (m_hwnd_bars_reflect != nullptr && point.x >= (m_widthText + m_widthSpikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwnd_bars_reflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwnd_bars_reflect, WM_LBUTTONUP, nFlags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnLButtonUp(nFlags, point);
}
