#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "SpikeClassListBox.h"

#include "DlgListBClaSize.h"


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

SpikeClassListBox::SpikeClassListBox() : m_clrBkgnd(GetSysColor(COLOR_SCROLLBAR))
{
	m_brBkgnd.CreateSolidBrush(m_clrBkgnd);
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
			const RowStruct* row = reinterpret_cast<RowStruct*>(GetItemData(i_current_selected));
			m_lFirst = (row->row_chart_bars)->GetTimeFirst();
			m_lLast = (row->row_chart_bars)->GetTimeLast();
			SetTimeIntervals(m_lFirst, m_lLast);
		}
		break;

	case HINT_HITSPIKE: // spike is selected
		SelectSpike(threshold);
		m_spike_hit = threshold; // selected spike
		m_spike_hit_row = i_current_selected; // current line / CListBox
		m_is_spike_hit = TRUE;
		break;

	case HINT_CHANGEZOOM:
		{
			const auto* pptr2 = reinterpret_cast<RowStruct*>(GetItemData(i_current_selected));
			const auto y_we = pptr2->row_chart_bars->GetYWExtent();
			const auto y_wo = pptr2->row_chart_bars->GetYWOrg();
			SetYzoom(y_we, y_wo);
		}
		break;

	case HINT_DROPPED: // HIWORD(lParam) = ID of control who have sent this message
		if (!m_is_spike_hit)
			return 0L;
	// change selection
		if (i_current_selected != m_spike_hit_row)
		{
			// patch: when we displace a spike to line 0, the line nb is not correct (shadow window intercepting mouse?)
			if (i_current_selected < 0 || i_current_selected > GetCount())
				i_current_selected = 0;
			ChangeSpikeClass(m_spike_hit, m_spike_list->GetclassID(i_current_selected));
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
	// all items are of fixed size
	// must use LBS_OWNERDRAWVARIABLE for this to work
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
		const auto pptr = reinterpret_cast<RowStruct*>(lpDIS->itemData);

		// display text
		const auto textlen = (pptr->row_comment)->GetLength();
		dc.DrawText(*(pptr->row_comment), textlen, rc_text, DT_LEFT | DT_WORDBREAK);

		// display spikes
		const auto col1 = m_widthText + m_widthSeparator;
		const auto col2 = col1 + m_widthSpikes + m_widthSeparator;
		auto rect_spikes = CRect(col1 + 1, lpDIS->rcItem.top + 1, col1 + m_widthSpikes, lpDIS->rcItem.bottom - 1);
		(pptr->row_chart_shapes)->MoveWindow(rect_spikes, FALSE);
		{
			CDC dc_mem;
			CBitmap bmp_mem;
			CBitmap* p_old_bmp = nullptr;
			dc_mem.CreateCompatibleDC(&dc);
			bmp_mem.CreateCompatibleBitmap(&dc, rect_spikes.Width(), rect_spikes.Height());
			p_old_bmp = dc_mem.SelectObject(&bmp_mem);
			//draw data
			(pptr->row_chart_shapes)->SetDisplayAreaSize(rect_spikes.Width(), rect_spikes.Height());
			(pptr->row_chart_shapes)->PlotDataToDC(&dc_mem);
			// transfer data to DC and clean the memory DC
			dc.BitBlt(rect_spikes.left, rect_spikes.top, rect_spikes.Width(), rect_spikes.Height(), &dc_mem, 0, 0,
			          SRCCOPY);
			dc_mem.SelectObject(p_old_bmp);
			dc_mem.DeleteDC();
		}

		// display bars
		auto rect_bars = CRect(col2 + 1, lpDIS->rcItem.top + 1, col2 + m_widthBars, lpDIS->rcItem.bottom - 1);
		(pptr->row_chart_bars)->MoveWindow(rect_bars, FALSE);
		{
			CDC dc_mem;
			CBitmap bmp_mem;
			CBitmap* p_old_bmp = nullptr;
			dc_mem.CreateCompatibleDC(&dc);
			bmp_mem.CreateCompatibleBitmap(&dc, rect_bars.Width(), rect_bars.Height());
			p_old_bmp = dc_mem.SelectObject(&bmp_mem);
			// draw actual data
			(pptr->row_chart_bars)->SetDisplayAreaSize(rect_bars.Width(), rect_bars.Height());
			(pptr->row_chart_bars)->PlotDataToDC(&dc_mem);
			// transfer data to DC and clean the memory DC
			dc.BitBlt(rect_bars.left, rect_bars.top, rect_bars.Width(), rect_bars.Height(), &dc_mem, 0, 0, SRCCOPY);
			dc_mem.SelectObject(p_old_bmp);
			dc_mem.DeleteDC();
		}
	}

	// item is selected -- add frame
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		const auto cr = RGB(0, 0, 255); // 2 Blue RGB(0,0,0);
		CBrush br(cr);
		dc.FrameRect(&(lpDIS->rcItem), &br);
	}

	// Item has been de-selected -- remove frame
	if (!(lpDIS->itemState & ODS_SELECTED) && // item not selected
		(lpDIS->itemAction & ODA_SELECT))
	{
		dc.FrameRect(&(lpDIS->rcItem), &m_brBkgnd);
	}
	dc.Detach();
}

void SpikeClassListBox::DeleteItem(LPDELETEITEMSTRUCT lpDI)
{
	const auto item = reinterpret_cast<RowStruct*>(lpDI->itemData);
	const auto item_comment = item->row_comment;
	delete item_comment;

	const auto item_spike_shapes = item->row_chart_shapes;
	delete item_spike_shapes;

	const auto item_spike_bars = item->row_chart_bars;
	delete item_spike_bars;

	delete item;
}

void SpikeClassListBox::SetRowHeight(int rowheight)
{
	m_row_height = rowheight;
	for (auto n_index = 0; n_index < GetCount(); n_index++)
		SetItemHeight(n_index, rowheight);
}

void SpikeClassListBox::SetLeftColWidth(int leftwidth)
{
	m_left_column_width = leftwidth;
}

void SpikeClassListBox::SetColsWidth(int coltext, int colspikes, int colseparator)
{
	m_widthSpikes = colspikes;
	m_widthSeparator = colseparator;
	m_widthText = m_left_column_width - colspikes - 2 * colseparator;
	CRect rect;
	GetClientRect(rect);
	m_widthBars = rect.Width() - m_left_column_width;
}

int SpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	const auto pptr1 = reinterpret_cast<RowStruct*>(lpCIS->itemData1);
	const auto pptr2 = reinterpret_cast<RowStruct*>(lpCIS->itemData2);
	auto iresult = 1; // value to return (default:clas1>clas2)
	const auto clas1 = (pptr1->row_chart_bars)->GetSelClass();
	const auto clas2 = (pptr2->row_chart_bars)->GetSelClass();
	if (clas1 == clas2)
		iresult = 0; // exact match
	else if (clas1 < clas2)
		iresult = -1; // lower goes first

	return iresult;
}

void SpikeClassListBox::set_source_data(SpikeList* pSList, CdbWaveDoc* pdbDoc)
{
	// erase content of the list box
	SetRedraw(FALSE);
	ResetContent();

	m_dbwave_doc = pdbDoc;
	m_spike_list = pSList;
	if (pSList == nullptr || pdbDoc == nullptr)
		return;
	m_spike_doc = pdbDoc->m_pSpk;

	// add as many windows as necessary; store pointer into listbox
	const auto rect_spikes = CRect(0, 0, 0, 0); //CRect(0, 0, size, size);
	const auto rect_bars = CRect(0, 0, 0, 0);

	auto i_id = 0;
	int nbclasses = GetHowManyClassesInCurrentSpikeList();
	const auto nspikes = m_spike_list->GetTotalSpikes();
	for (auto i = 0; i < nbclasses; i++)
	{
		const auto iclass = m_spike_list->GetclassID(i);

		// 1) create spike form button
		ChartSpikeShape* pspkShapes = nullptr;
		if (m_spike_list->GetSpikeLength() > 0)
		{
			pspkShapes = new (ChartSpikeShape);
			ASSERT(pspkShapes != NULL);
			pspkShapes->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i_id);

			pspkShapes->set_source_data(m_spike_list, pdbDoc);
			pspkShapes->set_plot_mode(PLOT_ONECLASSONLY, iclass);
			pspkShapes->SetRangeMode(RANGE_INDEX);
			pspkShapes->SetSpkIndexes(0, nspikes - 1);
			pspkShapes->SetbDrawframe(TRUE);
			pspkShapes->SetCursorMaxOnDblClick(m_cursorIndexMax);
			i_id++;
		}

		// 2) bars with spike height
		auto* pspk_bars = new (ChartSpikeBar);
		ASSERT(pspk_bars != NULL);
		pspk_bars->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_bars, this, i_id);

		pspk_bars->set_source_data(m_spike_list, pdbDoc);
		pspk_bars->set_plot_mode(PLOT_ONECLASSONLY, iclass);
		pspk_bars->SetRangeMode(RANGE_INDEX);
		pspk_bars->SetSpkIndexes(0, nspikes - 1);
		pspk_bars->SetbDrawframe(TRUE);
		pspk_bars->SetCursorMaxOnDblClick(m_cursorIndexMax);
		i_id++;

		// 3) create text
		auto* pcs = new CString();
		pcs->Format(_T("class %i\nn=%i"), iclass, m_spike_list->GetclassNbspk(i));
		ASSERT(pcs != NULL);

		// 4) create array of 3 pointers and pass it to the listbox
		const auto pptr = new(RowStruct);
		ASSERT(pptr != NULL);
		pptr->row_comment = pcs;
		pptr->row_chart_shapes = pspkShapes;
		pptr->row_chart_bars = pspk_bars;
		AddString(LPTSTR(pptr));
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
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		if (pptr->row_chart_shapes != nullptr)
		{
			(pptr->row_chart_shapes)->SetRangeMode(RANGE_TIMEINTERVALS);
			(pptr->row_chart_shapes)->SetTimeIntervals(l_first, l_last);
		}
		(pptr->row_chart_bars)->SetRangeMode(RANGE_TIMEINTERVALS);
		(pptr->row_chart_bars)->SetTimeIntervals(l_first, l_last);
	}
}

int SpikeClassListBox::GetHowManyClassesInCurrentSpikeList()
{
	if (!m_spike_list->IsClassListValid())
		m_spike_list->UpdateClassList();
	const auto nspikes = m_spike_list->GetTotalSpikes();
	auto nbclasses = 1;
	if (nspikes > 0)
		nbclasses = m_spike_list->GetNbclasses();
	return nbclasses;
}

void SpikeClassListBox::SetSpkList(SpikeList* p_spike_list)
{
	m_spike_list = p_spike_list;
	int nbclasses = GetHowManyClassesInCurrentSpikeList();

	if (nbclasses == GetCount())
	{
		for (auto i = 0; i < GetCount(); i++)
		{
			const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
			(pptr->row_chart_shapes)->SetSpkList(p_spike_list);
			(pptr->row_chart_bars)->SetSpkList(p_spike_list);
		}
	}
	else
	{
		set_source_data(p_spike_list, m_dbwave_doc);
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
		const auto p_spike_element = m_spike_list->GetSpike(spike_no);
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
			const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i)); // get pointer to row objects
			if ((pptr->row_chart_bars)->GetSelClass() == cla)
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
	auto oldcursor = 0;
	for (auto i = 0; i < GetCount(); i++)
	{
		auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		if (pptr->row_chart_shapes != nullptr)
			(pptr->row_chart_shapes)->SetMouseCursorType(cursor_m);
		oldcursor = (pptr->row_chart_bars)->SetMouseCursorType(cursor_m);
	}
	return oldcursor;
}

void SpikeClassListBox::OnSize(UINT nType, int cx, int cy)
{
	CListBox::OnSize(nType, cx, cy);
	m_widthBars = cx - m_left_column_width;
	// move all windows out of the way to prevent displaying old rows
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		CRect rect(0, 0, 0, 0);
		if (pptr->row_chart_shapes != nullptr)
			(pptr->row_chart_shapes)->MoveWindow(rect, FALSE);
		(pptr->row_chart_bars)->MoveWindow(rect, FALSE);
	}
}

void SpikeClassListBox::SetYzoom(int y_we, int y_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		if (pptr->row_chart_shapes != nullptr)
			(pptr->row_chart_shapes)->SetYWExtOrg(y_we, y_wo);
		(pptr->row_chart_bars)->SetYWExtOrg(y_we, y_wo);
	}
}

void SpikeClassListBox::SetXzoom(int x_we, int x_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		if (pptr->row_chart_shapes != nullptr)
			(pptr->row_chart_shapes)->SetXWExtOrg(x_we, x_wo);
	}
}

int SpikeClassListBox::GetYWExtent()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(0));
	return (pptr->row_chart_bars)->GetYWExtent();
}

int SpikeClassListBox::GetYWOrg()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(0));
	return (pptr->row_chart_bars)->GetYWOrg();
}

int SpikeClassListBox::GetXWExtent()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(0));
	auto i = 0;
	if ((pptr->row_chart_shapes) != nullptr)
		i = (pptr->row_chart_shapes)->GetXWExtent();
	return i;
}

int SpikeClassListBox::GetXWOrg()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(0));
	auto i = 0;
	if ((pptr->row_chart_shapes) != nullptr)
		i = (pptr->row_chart_shapes)->GetXWOrg();
	return i;
}

float SpikeClassListBox::GetExtent_mV()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(0));
	auto x = 0.f;
	if ((pptr->row_chart_shapes) != nullptr)
		x = (pptr->row_chart_shapes)->GetExtent_mV();
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
		SetColsWidth(dlg.m_textcol, dlg.m_superpcol, dlg.m_intercolspace);
		Invalidate();
	}
}

HBRUSH SpikeClassListBox::CtlColor(CDC* p_dc, UINT nCtlColor)
{
	p_dc->SetTextColor(m_clrText); // text
	p_dc->SetBkColor(m_clrBkgnd); // text bkgnd
	return m_brBkgnd;
	// Return a non-NULL brush if the parent's handler should not be called
}

int SpikeClassListBox::get_row_index_of_spike_class(int spike_class)
{
	RowStruct* row_item = nullptr;
	int row_index;
	for (row_index = 0; row_index < GetCount(); row_index++)
	{
		row_item = reinterpret_cast<RowStruct*>(GetItemData(row_index));
		if ((row_item->row_chart_bars)->GetSelClass() == spike_class)
			break;
	}
	return row_index;
}

void SpikeClassListBox::remove_spike_from_row(int spike_no)
{
	const auto old_class = m_spike_list->GetSpike(spike_no)->get_class();

	// get row corresponding to old_class
	RowStruct* row_item = nullptr;
	int row_old;
	for (row_old = 0; row_old < GetCount(); row_old++)
	{
		row_item = reinterpret_cast<RowStruct*>(GetItemData(row_old)); 
		if ((row_item->row_chart_bars)->GetSelClass() == old_class)
			break;
	}

	// unselect spike
	if (row_item != nullptr)
	{
		if (row_item->row_chart_shapes != nullptr)
			(row_item->row_chart_shapes)->SelectSpikeShape(-1);
		(row_item->row_chart_bars)->SelectSpike(-1);
	}

	// decrease total nb of spikes within class
	const auto n_spikes = m_spike_list->GetclassNbspk(row_old) - 1;

	// reset all if row should be removed
	if (n_spikes > 0)
	{
		m_spike_list->SetclassNbspk(row_old, n_spikes);
		update_string(row_item, old_class, n_spikes);
	}
	else
	{
		// row suppressed here?
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		set_source_data(m_spike_list, m_dbwave_doc);
		SetTimeIntervals(l_first, l_last);
		SelectSpike(spike_no);
	}
}

void SpikeClassListBox::ChangeSpikeClass(int spike_no, int new_class)
{
	if (0 == GetCount() || new_class == m_spike_list->GetSpike(spike_no)->get_class())
		return;

	// ---------------- 1) old spike : deselect spike and remove from corresp line (destroy?)
	remove_spike_from_row(spike_no);
	

	// ---------------- 2) new class? add to other row and select
	m_spike_list->GetSpike(spike_no)->set_class(new_class);

	// new row?
	RowStruct* pptr = nullptr;
	int i_row;
	for (i_row = 0; i_row < GetCount(); i_row++)
	{
		pptr = reinterpret_cast<RowStruct*>(GetItemData(i_row)); 
		if ((pptr->row_chart_bars)->GetSelClass() == new_class)
			break;
	}
	// reset all if line ought to be added
	if (i_row >= GetCount())
	{
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		set_source_data(m_spike_list, m_dbwave_doc);
		SetTimeIntervals(l_first, l_last);
	}
	else
	{
		int n_spikes = m_spike_list->GetclassNbspk(i_row) + 1;
		m_spike_list->SetclassNbspk(i_row, n_spikes);
		update_string(pptr, new_class, n_spikes);
	}

	CRect rect;
	if (!GetItemRect(i_row, &rect))
		return;
	SelectSpike(spike_no);
}

void SpikeClassListBox::update_string(void* ptr, int i_class, int nb_spikes)
{
	// create text
	const auto pptr = static_cast<RowStruct*>(ptr);
	delete pptr->row_comment;
	auto pcs = new CString;
	ASSERT(pcs != NULL);
	pcs->Format(_T("class %i\nn=%i"), i_class, nb_spikes);
	pptr->row_comment = pcs;
}

void SpikeClassListBox::PrintItem(CDC* p_dc, CRect* prect1, CRect* prect2, CRect* prect3, int i)
{
	if ((i < 0) || (i > GetCount() - 1))
		return;
	const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));

	// print text
	const auto textlen = (pptr->row_comment)->GetLength();
	p_dc->DrawText(*(pptr->row_comment), textlen, prect1, DT_LEFT | DT_WORDBREAK);

	// spike shape
	if (pptr->row_chart_shapes != nullptr)
		pptr->row_chart_shapes->Print(p_dc, prect2);

	// spike bars
	if (pptr->row_chart_bars != nullptr)
		pptr->row_chart_bars->Print(p_dc, prect3);
}

void SpikeClassListBox::XorTempVTtag(int x_point)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		pptr->row_chart_bars->XorTempVTtag(x_point);
	}
}

void SpikeClassListBox::ResetBarsXortag()
{
	for (int i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		pptr->row_chart_bars->ResetXortag();
	}
}

void SpikeClassListBox::ReflectBarsMouseMoveMessg(HWND hwnd)
{
	m_hwnd_bars_reflect = hwnd;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<RowStruct*>(GetItemData(i));
		(pptr->row_chart_bars)->ReflectMouseMoveMessg(hwnd);
		if (hwnd != nullptr)
			pptr->row_chart_bars->SetMouseCursorType(CURSOR_CROSS);
	}
}

void SpikeClassListBox::OnMouseMove(UINT nFlags, CPoint point)
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
