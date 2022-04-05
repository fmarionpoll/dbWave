#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "SpikeClassListBox.h"

#include "ChartWnd.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "DlgListBClaSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct rowStruct
{
	CString* pcs;
	ChartSpikeShapeWnd* pspk_shapes;
	ChartSpikeBarWnd* pspk_bars;
};

BEGIN_MESSAGE_MAP(CSpikeClassListBox, CListBox)
	ON_WM_SIZE()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CSpikeClassListBox::CSpikeClassListBox()
{
	m_clrBkgnd = GetSysColor(COLOR_SCROLLBAR);
	m_brBkgnd.CreateSolidBrush(m_clrBkgnd);
}

CSpikeClassListBox::~CSpikeClassListBox()
= default;

LRESULT CSpikeClassListBox::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	auto icursel = static_cast<int>(HIWORD(lParam)) / 2; // why lParam/2???
	const int threshold = LOWORD(lParam); // value associated

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		SetMouseCursorType(threshold);
		break;

	case HINT_CHANGEHZLIMITS: // abscissa have changed
		{
			const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(icursel));
			m_lFirst = (pptr->pspk_bars)->GetTimeFirst();
			m_lLast = (pptr->pspk_bars)->GetTimeLast();
			SetTimeIntervals(m_lFirst, m_lLast);
		}
		break;

	case HINT_HITSPIKE: // spike is selected
		SelectSpike(threshold);
		m_selspikeLB = threshold; // selected spike
		m_oldsel = icursel; // current line / CListBox
		m_bHitspk = TRUE;
		break;

	case HINT_CHANGEZOOM:
		{
			auto* pptr2 = reinterpret_cast<rowStruct*>(GetItemData(icursel));
			const auto y_we = pptr2->pspk_bars->GetYWExtent();
			const auto y_wo = pptr2->pspk_bars->GetYWOrg();
			SetYzoom(y_we, y_wo);
		}
		break;

	case HINT_DROPPED: // HIWORD(lParam) = ID of control who have sent this message
		if (!m_bHitspk)
			return 0L;
	// change selection
		if (icursel != m_oldsel)
		{
			// patch: when we displace a spike to line 0, the line nb is not correct (shadow window intercepting mouse?)
			if (icursel < 0 || icursel > GetCount())
				icursel = 0;
			ChangeSpikeClass(m_selspikeLB, p_spikelist_->GetclassID(icursel));
		}
		m_bHitspk = FALSE;
		break;

	default:
		break;
	}
	// forward message to parent
	GetParent()->PostMessage(WM_MYMESSAGE, wParam, MAKELPARAM(threshold, GetDlgCtrlID()));
	return 0L;
}

void CSpikeClassListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	// all items are of fixed size
	// must use LBS_OWNERDRAWVARIABLE for this to work
	lpMIS->itemHeight = m_rowheight;
}

void CSpikeClassListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC dc;
	dc.Attach(lpDIS->hDC);

	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		// get data
		CRect rc_text = lpDIS->rcItem;
		rc_text.right = rc_text.left + m_widthText;
		const auto pptr = reinterpret_cast<rowStruct*>(lpDIS->itemData);

		// display text
		const auto textlen = (pptr->pcs)->GetLength();
		dc.DrawText(*(pptr->pcs), textlen, rc_text, DT_LEFT | DT_WORDBREAK);

		// display spikes
		const auto col1 = m_widthText + m_widthSeparator;
		const auto col2 = col1 + m_widthSpikes + m_widthSeparator;
		auto rect_spikes = CRect(col1 + 1, lpDIS->rcItem.top + 1, col1 + m_widthSpikes, lpDIS->rcItem.bottom - 1);
		(pptr->pspk_shapes)->MoveWindow(rect_spikes, FALSE);
		{
			CDC dc_mem;
			CBitmap bmp_mem;
			CBitmap* p_old_bmp = nullptr;
			dc_mem.CreateCompatibleDC(&dc);
			bmp_mem.CreateCompatibleBitmap(&dc, rect_spikes.Width(), rect_spikes.Height());
			p_old_bmp = dc_mem.SelectObject(&bmp_mem);
			//draw data
			(pptr->pspk_shapes)->SetDisplayAreaSize(rect_spikes.Width(), rect_spikes.Height());
			(pptr->pspk_shapes)->PlotDataToDC(&dc_mem);
			// transfer data to DC and clean the memory DC
			dc.BitBlt(rect_spikes.left, rect_spikes.top, rect_spikes.Width(), rect_spikes.Height(), &dc_mem, 0, 0,
			          SRCCOPY);
			dc_mem.SelectObject(p_old_bmp);
			dc_mem.DeleteDC();
		}

		// display bars
		auto rect_bars = CRect(col2 + 1, lpDIS->rcItem.top + 1, col2 + m_widthBars, lpDIS->rcItem.bottom - 1);
		(pptr->pspk_bars)->MoveWindow(rect_bars, FALSE);
		{
			CDC dc_mem;
			CBitmap bmp_mem;
			CBitmap* p_old_bmp = nullptr;
			dc_mem.CreateCompatibleDC(&dc);
			bmp_mem.CreateCompatibleBitmap(&dc, rect_bars.Width(), rect_bars.Height());
			p_old_bmp = dc_mem.SelectObject(&bmp_mem);
			// draw actual data
			(pptr->pspk_bars)->SetDisplayAreaSize(rect_bars.Width(), rect_bars.Height());
			(pptr->pspk_bars)->PlotDataToDC(&dc_mem);
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

void CSpikeClassListBox::DeleteItem(LPDELETEITEMSTRUCT lpDI)
{
	const auto pptr = reinterpret_cast<rowStruct*>(lpDI->itemData);
	const auto pcs = pptr->pcs;
	delete pcs;

	auto pspk_shapes = pptr->pspk_shapes;
	delete pspk_shapes;

	auto pspk_bars = pptr->pspk_bars;
	delete pspk_bars;

	delete pptr;
}

void CSpikeClassListBox::SetRowHeight(int rowheight)
{
	m_rowheight = rowheight;
	for (auto n_index = 0; n_index < GetCount(); n_index++)
		SetItemHeight(n_index, rowheight);
}

void CSpikeClassListBox::SetLeftColWidth(int leftwidth)
{
	m_leftcolwidth = leftwidth;
}

void CSpikeClassListBox::SetColsWidth(int coltext, int colspikes, int colseparator)
{
	m_widthSpikes = colspikes;
	m_widthSeparator = colseparator;
	m_widthText = m_leftcolwidth - colspikes - 2 * colseparator;
	CRect rect;
	GetClientRect(rect);
	m_widthBars = rect.Width() - m_leftcolwidth;
}

int CSpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	const auto pptr1 = reinterpret_cast<rowStruct*>(lpCIS->itemData1);
	const auto pptr2 = reinterpret_cast<rowStruct*>(lpCIS->itemData2);
	auto iresult = 1; // value to return (default:clas1>clas2)
	const auto clas1 = (pptr1->pspk_bars)->GetSelClass();
	const auto clas2 = (pptr2->pspk_bars)->GetSelClass();
	if (clas1 == clas2)
		iresult = 0; // exact match
	else if (clas1 < clas2)
		iresult = -1; // lower goes first

	return iresult;
}

void CSpikeClassListBox::SetSourceData(SpikeList* pSList, CdbWaveDoc* pdbDoc)
{
	// erase content of the list box
	SetRedraw(FALSE);
	ResetContent();

	p_dbwave_doc_ = pdbDoc;
	p_spikelist_ = pSList;
	if (pSList == nullptr || pdbDoc == nullptr)
		return;
	p_spike_doc_ = pdbDoc->m_pSpk;

	// add as many windows as necessary; store pointer into listbox
	const auto rect_spikes = CRect(0, 0, 0, 0); //CRect(0, 0, size, size);
	const auto rect_bars = CRect(0, 0, 0, 0);

	auto i_id = 0;
	int nbclasses = GetHowManyClassesInCurrentSpikeList();
	const auto nspikes = p_spikelist_->GetTotalSpikes();
	for (auto i = 0; i < nbclasses; i++)
	{
		const auto iclass = p_spikelist_->GetclassID(i);

		// 1) create spike form button
		ChartSpikeShapeWnd* pspkShapes = nullptr;
		if (p_spikelist_->GetSpikeLength() > 0)
		{
			pspkShapes = new (ChartSpikeShapeWnd);
			ASSERT(pspkShapes != NULL);
			pspkShapes->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i_id);

			pspkShapes->SetSourceData(p_spikelist_, pdbDoc);
			pspkShapes->SetPlotMode(PLOT_ONECLASSONLY, iclass);
			pspkShapes->SetRangeMode(RANGE_INDEX);
			pspkShapes->SetSpkIndexes(0, nspikes - 1);
			pspkShapes->SetbDrawframe(TRUE);
			pspkShapes->SetCursorMaxOnDblClick(m_cursorIndexMax);
			i_id++;
		}

		// 2) bars with spike height
		auto* pspk_bars = new (ChartSpikeBarWnd);
		ASSERT(pspk_bars != NULL);
		pspk_bars->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_bars, this, i_id);

		pspk_bars->SetSourceData(p_spikelist_, pdbDoc);
		pspk_bars->SetPlotMode(PLOT_ONECLASSONLY, iclass);
		pspk_bars->SetRangeMode(RANGE_INDEX);
		pspk_bars->SetSpkIndexes(0, nspikes - 1);
		pspk_bars->SetbDrawframe(TRUE);
		pspk_bars->SetCursorMaxOnDblClick(m_cursorIndexMax);
		i_id++;

		// 3) create text
		auto* pcs = new CString();
		pcs->Format(_T("class %i\nn=%i"), iclass, p_spikelist_->GetclassNbspk(i));
		ASSERT(pcs != NULL);

		// 4) create array of 3 pointers and pass it to the listbox
		const auto pptr = new(rowStruct);
		ASSERT(pptr != NULL);
		pptr->pcs = pcs;
		pptr->pspk_shapes = pspkShapes;
		pptr->pspk_bars = pspk_bars;
		AddString(LPTSTR(pptr));
	}
	// exit: allow data redrawing
	SetRedraw(TRUE);
}

void CSpikeClassListBox::SetTimeIntervals(long l_first, long l_last)
{
	m_lFirst = l_first;
	m_lLast = l_last;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		if (pptr->pspk_shapes != nullptr)
		{
			(pptr->pspk_shapes)->SetRangeMode(RANGE_TIMEINTERVALS);
			(pptr->pspk_shapes)->SetTimeIntervals(l_first, l_last);
		}
		(pptr->pspk_bars)->SetRangeMode(RANGE_TIMEINTERVALS);
		(pptr->pspk_bars)->SetTimeIntervals(l_first, l_last);
	}
}

int CSpikeClassListBox::GetHowManyClassesInCurrentSpikeList()
{
	if (!p_spikelist_->IsClassListValid())
		p_spikelist_->UpdateClassList();
	const auto nspikes = p_spikelist_->GetTotalSpikes();
	auto nbclasses = 1;
	if (nspikes > 0)
		nbclasses = p_spikelist_->GetNbclasses();
	return nbclasses;
}

void CSpikeClassListBox::SetSpkList(SpikeList* p_spike_list)
{
	p_spikelist_ = p_spike_list;
	int nbclasses = GetHowManyClassesInCurrentSpikeList();

	if (nbclasses == GetCount())
	{
		for (auto i = 0; i < GetCount(); i++)
		{
			const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
			(pptr->pspk_shapes)->SetSpkList(p_spike_list);
			(pptr->pspk_bars)->SetSpkList(p_spike_list);
		}
	}
	else
	{
		SetSourceData(p_spike_list, p_dbwave_doc_);
	}
}

int CSpikeClassListBox::SelectSpike(int spikeno)
{
	/*
	BOOL bAll = TRUE;
	int cla = 0;
	if (spikeno >= 0)
	{
		cla = m_pSList->get_class(spikeno);		// load spike class
		bAll = FALSE;
	}

	int oldspk=0;
	int spki=0;
;*/

	const auto b_multiple_selection = FALSE;
	auto cla = 0;

	// select spike
	if (spikeno >= 0)
	{
		// get address of spike parms
		const auto p_spike_element = p_spikelist_->GetSpike(spikeno);
		cla = p_spike_element->get_class();

		// multiple selection
		if (false)
		{
			auto nflaggedspikes = p_spikelist_->ToggleSpikeFlag(spikeno);
			if (p_spikelist_->GetSpikeFlagArrayCount() < 1)
				spikeno = -1;
		}
		// single selection
		p_spikelist_->SetSingleSpikeFlag(spikeno);
	}
	// un-select all spikes
	else
		p_spikelist_->RemoveAllSpikeFlags();

	// select corresponding row
	if (spikeno >= 0)
	{
		for (auto i = 0; i < GetCount(); i++) // search row where this class is stored
		{
			const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i)); // get pointer to row objects
			if ((pptr->pspk_bars)->GetSelClass() == cla)
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
	if (spikeno > 0)
		oldspk = spikeno;
	return oldspk;
}

int CSpikeClassListBox::SetMouseCursorType(int cursorm)
{
	auto oldcursor = 0;
	for (auto i = 0; i < GetCount(); i++)
	{
		auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		if (pptr->pspk_shapes != nullptr)
			(pptr->pspk_shapes)->SetMouseCursorType(cursorm);
		oldcursor = (pptr->pspk_bars)->SetMouseCursorType(cursorm);
	}
	return oldcursor;
}

void CSpikeClassListBox::OnSize(UINT nType, int cx, int cy)
{
	CListBox::OnSize(nType, cx, cy);
	m_widthBars = cx - m_leftcolwidth;
	// move all windows out of the way to prevent displaying old rows
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		CRect rect(0, 0, 0, 0);
		if (pptr->pspk_shapes != nullptr)
			(pptr->pspk_shapes)->MoveWindow(rect, FALSE);
		(pptr->pspk_bars)->MoveWindow(rect, FALSE);
	}
}

void CSpikeClassListBox::SetYzoom(int y_we, int y_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		if (pptr->pspk_shapes != nullptr)
			(pptr->pspk_shapes)->SetYWExtOrg(y_we, y_wo);
		(pptr->pspk_bars)->SetYWExtOrg(y_we, y_wo);
	}
}

void CSpikeClassListBox::SetXzoom(int x_we, int x_wo)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		if (pptr->pspk_shapes != nullptr)
			(pptr->pspk_shapes)->SetXWExtOrg(x_we, x_wo);
	}
}

int CSpikeClassListBox::GetYWExtent()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(0));
	return (pptr->pspk_bars)->GetYWExtent();
}

int CSpikeClassListBox::GetYWOrg()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(0));
	return (pptr->pspk_bars)->GetYWOrg();
}

int CSpikeClassListBox::GetXWExtent()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(0));
	auto i = 0;
	if ((pptr->pspk_shapes) != nullptr)
		i = (pptr->pspk_shapes)->GetXWExtent();
	return i;
}

int CSpikeClassListBox::GetXWOrg()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(0));
	auto i = 0;
	if ((pptr->pspk_shapes) != nullptr)
		i = (pptr->pspk_shapes)->GetXWOrg();
	return i;
}

float CSpikeClassListBox::GetExtent_mV()
{
	ASSERT(GetCount() > 0);
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(0));
	auto x = 0.f;
	if ((pptr->pspk_shapes) != nullptr)
		x = (pptr->pspk_shapes)->GetExtent_mV();
	return x;
}

void CSpikeClassListBox::OnRButtonUp(UINT nFlags, CPoint point)
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

HBRUSH CSpikeClassListBox::CtlColor(CDC* p_dc, UINT nCtlColor)
{
	p_dc->SetTextColor(m_clrText); // text
	p_dc->SetBkColor(m_clrBkgnd); // text bkgnd
	return m_brBkgnd;
	// Return a non-NULL brush if the parent's handler should not be called
}

void CSpikeClassListBox::ChangeSpikeClass(int spikeno, int newclass)
{
	if (0 == GetCount())
		return;

	// ---------------- 1) old spike : deselect spike
	// ----------------    and remove from corresp line (destroy?)

	const auto oldclass = p_spikelist_->GetSpike(spikeno)->get_class();
	p_spikelist_->GetSpike(spikeno)->set_class(newclass);

	// get row corresponding to oldclass
	rowStruct* pptr = nullptr;
	int irow;
	for (irow = 0; irow < GetCount(); irow++)
	{
		pptr = reinterpret_cast<rowStruct*>(GetItemData(irow)); // get pointer to row objects
		if ((pptr->pspk_bars)->GetSelClass() == oldclass)
			break;
	}
	//ASSERT((pptr->pspkBars)->GetSelClass() == oldclass);
	if (pptr != nullptr)
	{
		if (pptr->pspk_shapes != nullptr)
			(pptr->pspk_shapes)->SelectSpikeShape(-1); // de-select old spike / spike
		(pptr->pspk_bars)->SelectSpike(-1); // de-select old spike / bars
	}
	// decrease total nb of spikes within class
	CRect rect;
	auto nbspikes = p_spikelist_->GetclassNbspk(irow) - 1;
	// reset all if line ought to be suppressed
	if (nbspikes > 0)
	{
		p_spikelist_->SetclassNbspk(irow, nbspikes);
		UpdateString(pptr, oldclass, nbspikes);
	}
	else
	{
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		SetSourceData(p_spikelist_, p_dbwave_doc_);
		SetTimeIntervals(l_first, l_last);
		SelectSpike(spikeno);
		return;
	}

	// ---------------- 2) new class? add to other row and select

	// new row?
	pptr = nullptr;
	for (irow = 0; irow < GetCount(); irow++)
	{
		pptr = reinterpret_cast<rowStruct*>(GetItemData(irow)); // get pointer to row objects
		if ((pptr->pspk_bars)->GetSelClass() == newclass)
			break;
	}
	// reset all if line ought to be added
	if (irow >= GetCount())
	{
		const auto l_first = m_lFirst;
		const auto l_last = m_lLast;
		SetSourceData(p_spikelist_, p_dbwave_doc_);
		SetTimeIntervals(l_first, l_last);
	}
	else
	{
		nbspikes = p_spikelist_->GetclassNbspk(irow) + 1;
		p_spikelist_->SetclassNbspk(irow, nbspikes);
		UpdateString(pptr, newclass, nbspikes);
	}
	if (!GetItemRect(irow, &rect))
		return;
	SelectSpike(spikeno);
}

void CSpikeClassListBox::UpdateString(void* ptr, int iclass, int nbspikes)
{
	// create text
	const auto pptr = static_cast<rowStruct*>(ptr);
	delete pptr->pcs;
	auto pcs = new CString;
	ASSERT(pcs != NULL);
	pcs->Format(_T("class %i\nn=%i"), iclass, nbspikes);
	pptr->pcs = pcs;
}

void CSpikeClassListBox::PrintItem(CDC* p_dc, CRect* prect1, CRect* prect2, CRect* prect3, int i)
{
	if ((i < 0) || (i > GetCount() - 1))
		return;
	const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));

	// print text
	const auto textlen = (pptr->pcs)->GetLength();
	p_dc->DrawText(*(pptr->pcs), textlen, prect1, DT_LEFT | DT_WORDBREAK);

	// spike shape
	if (pptr->pspk_shapes != nullptr)
		pptr->pspk_shapes->Print(p_dc, prect2);

	// spike bars
	if (pptr->pspk_bars != nullptr)
		pptr->pspk_bars->Print(p_dc, prect3);
}

void CSpikeClassListBox::XorTempVTtag(int xpoint)
{
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		pptr->pspk_bars->XorTempVTtag(xpoint);
	}
}

void CSpikeClassListBox::ResetBarsXortag()
{
	for (int i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		pptr->pspk_bars->ResetXortag();
	}
}

void CSpikeClassListBox::ReflectBarsMouseMoveMessg(HWND hwnd)
{
	m_hwndBarsReflect = hwnd;
	for (auto i = 0; i < GetCount(); i++)
	{
		const auto pptr = reinterpret_cast<rowStruct*>(GetItemData(i));
		(pptr->pspk_bars)->ReflectMouseMoveMessg(hwnd);
		if (hwnd != nullptr)
			pptr->pspk_bars->SetMouseCursorType(CURSOR_CROSS);
	}
}

void CSpikeClassListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_hwndBarsReflect != nullptr && point.x >= (m_widthText + m_widthSpikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwndBarsReflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwndBarsReflect, WM_MOUSEMOVE, nFlags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnMouseMove(nFlags, point);
}

void CSpikeClassListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_hwndBarsReflect != nullptr && point.x >= (m_widthText + m_widthSpikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwndBarsReflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwndBarsReflect, WM_LBUTTONUP, nFlags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	else
		CListBox::OnLButtonUp(nFlags, point);
}
