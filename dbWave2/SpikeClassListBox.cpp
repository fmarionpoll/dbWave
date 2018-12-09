// lbspkcla.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"

#include "Spikedoc.h"
#include "dbWaveDoc.h"
#include "Listbcla.h"
#include "SpikeClassListBox.h"

#include "scopescr.h"
#include "Spikebar.h"
#include "spikeshape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct MYPTR
{	
	CString*		pcs;
	CSpikeShapeWnd* pspkShapes;
	CSpikeBarWnd*	pspkBars;
};

/////////////////////////////////////////////////////////////////////////////
// CSpikeClassListBox

//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpikeClassListBox, CListBox)
	ON_WM_SIZE()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)	
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////////

CSpikeClassListBox::CSpikeClassListBox()
{
	m_rowheight=20;
	m_leftcolwidth=20;
	m_pSList = nullptr;
	m_pSDoc = nullptr;
	m_bText=TRUE;
	m_bSpikes=TRUE;
	m_bBars=TRUE;	
	m_widthText=-1;
	m_widthSpikes=-1;
	m_widthBars=-1;	
	m_widthSeparator=5;
	m_clrText = RGB( 0, 0, 0 );
	m_clrBkgnd = GetSysColor(COLOR_SCROLLBAR);
	m_brBkgnd.CreateSolidBrush( m_clrBkgnd );
	m_topIndex=-1;
	m_bHitspk=FALSE;
	m_hwndBarsReflect = nullptr;
}


// --------------------------------------------------------------------------------

CSpikeClassListBox::~CSpikeClassListBox()
{
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeClassListBox message handlers

LRESULT CSpikeClassListBox::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int icursel = (int) HIWORD(lParam)/2; // why lParam/2???
	int threshold = LOWORD(lParam);	// value associated	
	//// select row that received a hit
	//if (GetSel(icursel) ==0)
	//	SetCurSel(icursel);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		SetMouseCursorType(threshold);
		break;

	case HINT_CHANGEHZLIMITS:		// abcissa have changed
		{
		MYPTR* pptr = (MYPTR*) GetItemData(icursel);
		m_lFirst = (pptr->pspkBars)->GetTimeFirst();
		m_lLast = (pptr->pspkBars)->GetTimeLast();
		SetTimeIntervals(m_lFirst, m_lLast);
		}
		break;

	case HINT_HITSPIKE:				// spike is selected
		SelectSpike(threshold);
		m_selspikeLB = threshold;		// selected spike
		m_oldsel = icursel;			// current line / CListBox
		m_bHitspk=TRUE;
		break;

	case HINT_CHANGEZOOM:
		{
		MYPTR* pptr = (MYPTR*) GetItemData(icursel);
		int yWE = (pptr->pspkBars)->GetYWExtent();
		int yWO = (pptr->pspkBars)->GetYWOrg();
		SetYzoom(yWE, yWO);
		}
		break;

	case HINT_DROPPED:	// HIWORD(lParam) = ID of control who have sent this message
		if (!m_bHitspk)
			return 0L;
		// change selection
		if (icursel != m_oldsel) {
			// patch: when we displace a spike to line 0, the line nb is not correct (shadow window intercepting mouse?)
			if (icursel <0 || icursel > GetCount())
				icursel = 0;
			ChangeSpikeClass(m_selspikeLB, m_pSList->GetclassID(icursel));
		}
		m_bHitspk=FALSE;
		break;

	default:
		//GetParent()->PostMessage(WM_MYMESSAGE, wParam, lParam);
		break;		
	}
	// forward message to parent
	GetParent()->PostMessage(WM_MYMESSAGE, wParam, MAKELPARAM(threshold, GetDlgCtrlID()));
	return 0L;	
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	// all items are of fixed size
	// must use LBS_OWNERDRAWVARIABLE for this to work
	lpMIS->itemHeight = m_rowheight;	
}


// --------------------------------------------------------------------------------

void CSpikeClassListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC dc;
	dc.Attach(lpDIS->hDC);

	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		// get data
		CRect rcText = lpDIS->rcItem;
		rcText.right = rcText.left + m_widthText;
		MYPTR* pptr = (MYPTR*) lpDIS->itemData;
		
		// display text
		int textlen = (pptr->pcs)->GetLength();
		dc.DrawText(*(pptr->pcs), textlen, rcText, DT_LEFT|DT_WORDBREAK);
		
		// display spikes
		int col1 = m_widthText+m_widthSeparator;
		int col2 = col1+m_widthSpikes+m_widthSeparator;
		CRect rectSpikes = CRect(col1+1, lpDIS->rcItem.top+1, col1+m_widthSpikes, lpDIS->rcItem.bottom -1);
		(pptr->pspkShapes)->MoveWindow(rectSpikes, FALSE);
		{
			CDC dcMem;
			CBitmap bmpMem;
			CBitmap* pOldBmp= nullptr;
			dcMem.CreateCompatibleDC(&dc);
			bmpMem.CreateCompatibleBitmap(&dc, rectSpikes.Width(),rectSpikes.Height());
			pOldBmp = dcMem.SelectObject(&bmpMem);
			//draw data
			(pptr->pspkShapes)->SetDisplayAreaSize(rectSpikes.Width(), rectSpikes.Height());
			(pptr->pspkShapes)->PlotDatatoDC(&dcMem);
			// transfer data to DC and clean the memory DC
			dc.BitBlt(rectSpikes.left, rectSpikes.top, rectSpikes.Width(), rectSpikes.Height(), &dcMem, 0, 0, SRCCOPY);
			dcMem.SelectObject(pOldBmp);
			dcMem.DeleteDC();
		}
		
		// display bars
		CRect rectBars   = CRect(col2+1, lpDIS->rcItem.top+1, col2+m_widthBars, lpDIS->rcItem.bottom -1);
		(pptr->pspkBars)->MoveWindow(rectBars, FALSE);
		{
			CDC dcMem;
			CBitmap bmpMem;
			CBitmap* pOldBmp= nullptr;
			dcMem.CreateCompatibleDC(&dc);
			bmpMem.CreateCompatibleBitmap(&dc, rectBars.Width(),rectBars.Height());
			pOldBmp = dcMem.SelectObject(&bmpMem);
			// draw actual data
			(pptr->pspkBars)->SetDisplayAreaSize(rectBars.Width(), rectBars.Height());
			(pptr->pspkBars)->PlotDatatoDC(&dcMem);
			// transfer data to DC and clean the memory DC
			dc.BitBlt(rectBars.left, rectBars.top, rectBars.Width(), rectBars.Height(), &dcMem, 0, 0, SRCCOPY);
			dcMem.SelectObject(pOldBmp);
			dcMem.DeleteDC();
		}
	}

	// item is selected -- add frame
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		COLORREF cr = RGB(  0,   0, 255);	// 2 Blue RGB(0,0,0);
		CBrush br(cr);
		dc.FrameRect(&(lpDIS->rcItem), &br);

	}

	// Item has been de-selected -- remove frame
	if (!(lpDIS->itemState & ODS_SELECTED) &&	// item not selected
		(lpDIS->itemAction & ODA_SELECT))
	{
		dc.FrameRect(&(lpDIS->rcItem), &m_brBkgnd);
	}
	dc.Detach();
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::DeleteItem(LPDELETEITEMSTRUCT lpDI)
{
	MYPTR* pptr = (MYPTR*) lpDI->itemData;
	CString* pcs =	pptr->pcs;
	delete pcs;

	CSpikeShapeWnd* pspkShapes = pptr->pspkShapes;
	if (pspkShapes != nullptr)
	{
		pspkShapes->DestroyWindow();
		delete pspkShapes;
	}
	CSpikeBarWnd* pspkBars = pptr->pspkBars;
	pspkBars->DestroyWindow();
	delete pspkBars;

	delete pptr;
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetRowHeight(int rowheight)
{
	m_rowheight = rowheight;
	for (int nIndex=0; nIndex<GetCount(); nIndex++)
		SetItemHeight(nIndex, rowheight);
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetLeftColWidth(int leftwidth)
{
	m_leftcolwidth = leftwidth;	
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetColsWidth(int coltext, int colspikes, int colseparator)
{	
	m_widthSpikes = colspikes;
	m_widthSeparator = colseparator;
	m_widthText = m_leftcolwidth - colspikes - 2*colseparator;
	CRect rect;
	GetClientRect (rect);
	m_widthBars = rect.Width() - m_leftcolwidth;
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	MYPTR* pptr1 = (MYPTR*) lpCIS->itemData1;
	MYPTR* pptr2 = (MYPTR*) lpCIS->itemData2;
	int iresult = 1;		// value to return (default:clas1>clas2)
	int clas1 = (pptr1->pspkBars)->GetSelClass();
	int clas2 = (pptr2->pspkBars)->GetSelClass();
	if (clas1 == clas2)
		iresult=0;			// exact match
	else if (clas1 < clas2)
		iresult=-1;			// lower goes first

	return iresult;
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetSourceData(CSpikeList* pSList, CSpikeDoc* pSDoc)
{
	// erase content of the list box
	SetRedraw(FALSE);	// prevent redrawing
	ResetContent();		// clear content
	m_pSList = pSList;
	m_pSDoc  = pSDoc;
	if (pSList == nullptr || pSDoc == nullptr)
		return;
	
	// get list of classes
	if (!pSList->IsClassListValid())
		pSList->UpdateClassList();

	// add as many window as necessary; store pointer into listbox	
	int size = m_rowheight;
	CRect rectSpikes = CRect(0,0,0,0); //CRect(0, 0, size, size);
	CRect rectBars   = CRect(0,0,0,0); ; //CRect(size, 0, 3*size, size);
	
	int iID = 0;
	int nspikes = pSList->GetTotalSpikes();	
	int nbclasses =1;
	if (nspikes > 0)
		nbclasses = pSList->GetNbclasses();

	for (int i=0; i<nbclasses; i++)
	{
		int iclass = pSList->GetclassID(i);	//CSpikeList

		// 1) create spike form button
		CSpikeShapeWnd* pspkShapes = nullptr;
		if (pSList->GetSpikeLength () > 0)
		{
			pspkShapes = new (CSpikeShapeWnd);
			ASSERT(pspkShapes != NULL);
			pspkShapes->Create(_T(""), WS_CHILD|WS_VISIBLE, rectSpikes, this, iID);

			pspkShapes->SetSourceData(pSList);
			pspkShapes->SetPlotMode(PLOT_ONECLASSONLY, iclass);
			pspkShapes->SetRangeMode(RANGE_INDEX);
			pspkShapes->SetSpkIndexes(0, nspikes-1);
			pspkShapes->SetbDrawframe(TRUE); 
			iID++;
		}
		
		// 2) bars with spike height
		CSpikeBarWnd* pspkBars = new (CSpikeBarWnd);
		ASSERT(pspkBars != NULL);
		pspkBars->Create(_T(""), WS_CHILD| WS_VISIBLE, rectBars, this, iID);

		pspkBars->SetSourceData(pSList, pSDoc);
		pspkBars->SetPlotMode(PLOT_ONECLASSONLY, iclass);
		pspkBars->SetRangeMode(RANGE_INDEX);
		pspkBars->SetSpkIndexes(0, nspikes-1);
		pspkBars->SetbDrawframe(TRUE); 
		iID++;
		
		// 3) create text
		CString* pcs = new CString();		
		pcs->Format(_T("class %i\nn=%i"), iclass, (int) pSList->GetclassNbspk(i));
		ASSERT(pcs != NULL);

		// 4) create array of 3 pointers and pass it to the listbox
		MYPTR* pptr = new(MYPTR);
		ASSERT(pptr != NULL);
		pptr->pcs= pcs;
		pptr->pspkShapes=pspkShapes;
		pptr->pspkBars=pspkBars;
		AddString(LPTSTR(pptr));
	}
	// exit: allow data redrawing
	SetRedraw(TRUE);
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetTimeIntervals(long lFirst, long lLast)
{
	m_lFirst=lFirst;
	m_lLast=lLast;
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		if (pptr->pspkShapes != nullptr)
		{
			(pptr->pspkShapes)->SetRangeMode(RANGE_TIMEINTERVALS);
			(pptr->pspkShapes)->SetTimeIntervals(lFirst, lLast);
		}
		(pptr->pspkBars)->SetRangeMode(RANGE_TIMEINTERVALS);
		(pptr->pspkBars)->SetTimeIntervals(lFirst, lLast);
	}
}

void CSpikeClassListBox::SetSpkList(CSpikeList* pSL)
{
	m_pSList = pSL;
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		(pptr->pspkShapes)->SetSpkList(pSL);
		(pptr->pspkBars)->SetSpkList(pSL);
	}
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::SelectSpike(int spikeno)
{
	/*
	BOOL bAll = TRUE;
	int cla = 0;
	if (spikeno >= 0)
	{
		cla = m_pSList->GetSpikeClass(spikeno);		// load spike class
		bAll = FALSE;
	}

	int oldspk=0;
	int spki=0;
;*/

	BOOL bMultipleSelection = FALSE;
	int cla = 0;

	// select spike
	if (spikeno >= 0)
	{	// get address of spike parms
		CSpikeElemt* pS = m_pSList->GetSpikeElemt(spikeno);	
		cla = pS->GetSpikeClass();

		// multiple selection
		if (bMultipleSelection)
		{
			int nflaggedspikes = m_pSList->ToggleSpikeFlag(spikeno);
			if (m_pSList->GetSpikeFlagArrayCount() < 1)
				spikeno=-1;
		}
		// single selection
		else
			m_pSList->SetSingleSpikeFlag(spikeno);
	}
	// un-select all spikes
	else
		m_pSList->RemoveAllSpikeFlags();

	// select corresponding row
	if (spikeno >= 0) 
	{
		for (int i=0; i<GetCount(); i++)				// search row where this class is stored
		{
			MYPTR* pptr = (MYPTR*) GetItemData(i);		// get pointer to row objects
			if ((pptr->pspkBars)->GetSelClass() == cla)
			{
				SetCurSel(i);							// select corresponding row
				break;
			}
		}
	}

	// update display
	Invalidate();

	// return spike selected
	int oldspk = 0;
	if (spikeno > 0)
		oldspk = spikeno;
	return oldspk;
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::SetMouseCursorType(int cursorm)
{
	int oldcursor=0;
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		if (pptr->pspkShapes != nullptr)
			(pptr->pspkShapes)->SetMouseCursorType(cursorm);
		oldcursor = (pptr->pspkBars)->SetMouseCursorType(cursorm);
	}
	return oldcursor;
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::OnSize(UINT nType, int cx, int cy) 
{
	CListBox::OnSize(nType, cx, cy);
	m_widthBars = cx - m_leftcolwidth;
	// move all windows out of the way to prevent displaying old rows
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		CRect rect(0,0,0,0);
		if (pptr->pspkShapes != nullptr)
			(pptr->pspkShapes)->MoveWindow(rect, FALSE);
		(pptr->pspkBars)->	MoveWindow(rect, FALSE);
	}
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetYzoom(int yWE, int yWO)
{
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		if (pptr->pspkShapes != nullptr)
			(pptr->pspkShapes)->SetYWExtOrg(yWE, yWO);
		(pptr->pspkBars)->SetYWExtOrg(yWE, yWO);
	}
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::SetXzoom(int xWE, int xWO)
{
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		if (pptr->pspkShapes != nullptr)
			(pptr->pspkShapes)->SetXWExtOrg(xWE, xWO);
	}
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::GetYWExtent()
{
	ASSERT(GetCount() >0);
	MYPTR* pptr = (MYPTR*) GetItemData(0);
	return (pptr->pspkBars)->GetYWExtent();
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::GetYWOrg()
{
	ASSERT(GetCount() >0);
	MYPTR* pptr = (MYPTR*) GetItemData(0);
	return (pptr->pspkBars)->GetYWOrg();
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::GetXWExtent()
{
	ASSERT(GetCount() >0);
	MYPTR* pptr = (MYPTR*) GetItemData(0);
	int i = 0;
	if ((pptr->pspkShapes) != nullptr)
		i = (pptr->pspkShapes)->GetXWExtent();
	return i;
}

// --------------------------------------------------------------------------------

int CSpikeClassListBox::GetXWOrg()
{
	ASSERT(GetCount() >0);
	MYPTR* pptr = (MYPTR*) GetItemData(0);
	int i = 0;
	if ((pptr->pspkShapes) != nullptr)
		i = (pptr->pspkShapes)->GetXWOrg();
	return i;
}

// --------------------------------------------------------------------------------

float CSpikeClassListBox::GetExtent_mV()
{
	ASSERT(GetCount() >0);
	MYPTR* pptr = (MYPTR*) GetItemData(0);
	float x = 0.f;
	if ((pptr->pspkShapes) != nullptr)
		x = (pptr->pspkShapes)->GetExtent_mV();
	return x;
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::OnRButtonUp(UINT nFlags, CPoint point) 
{	
	CListBox::OnRButtonUp(nFlags, point);
	// change row and col sizes
	CListBClaSizeDlg dlg;
	dlg.m_rowheight = GetRowHeight();
	dlg.m_textcol = GetColsTextWidth();
	dlg.m_superpcol = GetColsSpikesWidth();	
	dlg.m_intercolspace=GetColsSeparatorWidth();
	if (IDOK == dlg.DoModal())
	{
		SetRowHeight(dlg.m_rowheight);
		SetColsWidth(dlg.m_textcol, dlg.m_superpcol, dlg.m_intercolspace);
		Invalidate();
	}
}

// --------------------------------------------------------------------------------

HBRUSH CSpikeClassListBox::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	pDC->SetTextColor(m_clrText );	// text
	pDC->SetBkColor(m_clrBkgnd );	// text bkgnd
	return m_brBkgnd;
	// Return a non-NULL brush if the parent's handler should not be called
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::ChangeSpikeClass(int spikeno, int newclass)
{
	if (0 == GetCount())
		return;

	// ---------------- 1) old spike : deselect spike 
	// ----------------    and remove from corresp line (destroy?)

	int oldclass = m_pSList->GetSpikeClass(spikeno);
	m_pSList->SetSpikeClass(spikeno, newclass);

	// get row corresponding to oldclass
	MYPTR* pptr = nullptr;
	int irow = 0;
	for (irow = 0; irow < GetCount(); irow++)
	{
		pptr = (MYPTR*)GetItemData(irow);		// get pointer to row objects
		if ((pptr->pspkBars)->GetSelClass() == oldclass)
			break;
	}
	//ASSERT((pptr->pspkBars)->GetSelClass() == oldclass);
	if (pptr != nullptr)
	{
		if (pptr->pspkShapes != nullptr)
			(pptr->pspkShapes)->SelectSpikeShape(-1);	// de-select old spike / spike
		(pptr->pspkBars)->SelectSpike(-1);				// de-select old spike / bars
	}
	// decrease total nb of spikes within class
	CRect rect;
	int nbspikes = m_pSList->GetclassNbspk(irow)-1;
	// reset all if line ought to be suppressed
	if (nbspikes > 0)
	{
		m_pSList->SetclassNbspk(irow, nbspikes);
		UpdateString(pptr, oldclass, nbspikes);
	}
	else
	{
		long lFirst = m_lFirst;
		long lLast = m_lLast;
		SetSourceData(m_pSList, m_pSDoc);
		SetTimeIntervals(lFirst, lLast);
		SelectSpike(spikeno);
		return;
	}

	// ---------------- 2) new class? add to other row and select

	// new row?
	pptr = nullptr;
	for (irow =0; irow <GetCount(); irow++)
	{
		pptr = (MYPTR*) GetItemData(irow);		// get pointer to row objects
		if ((pptr->pspkBars)->GetSelClass() == newclass)
			break;
	}
	// reset all if line ought to be added
	if (irow >= GetCount())
	{
		long lFirst = m_lFirst;
		long lLast = m_lLast;
		SetSourceData(m_pSList, m_pSDoc);
		SetTimeIntervals(lFirst, lLast);
	}
	else
	{
		nbspikes = m_pSList->GetclassNbspk(irow)+1;
		m_pSList->SetclassNbspk(irow, nbspikes);
		UpdateString(pptr, newclass, nbspikes);
	}
	GetItemRect( irow, &rect);
	SelectSpike(spikeno);
}

// --------------------------------------------------------------------------------

void CSpikeClassListBox::UpdateString(void* ptr, int iclass, int nbspikes)
{
	// create text
	MYPTR* pptr = (MYPTR*) ptr;
	delete ((CString*) pptr->pcs);
	CString* pcs = new CString;
	ASSERT(pcs != NULL);
	pcs->Format(_T("class %i\nn=%i"), iclass, nbspikes);
	pptr->pcs= pcs;
}

void CSpikeClassListBox::PrintItem(CDC* pDC, CRect* prect1, CRect* prect2, CRect* prect3,int i )
{
	if ((i< 0) || (i> GetCount()-1))
		return;
	MYPTR* pptr = (MYPTR*) GetItemData(i);

	// print text
	int textlen = (pptr->pcs)->GetLength();
	pDC->DrawText(*(pptr->pcs), textlen, prect1, DT_LEFT|DT_WORDBREAK);

	// spike shape
	if (pptr->pspkShapes != nullptr)	
		((CSpikeShapeWnd*) (pptr->pspkShapes))->Print(pDC, prect2);

	// spike bars
	if (pptr->pspkBars != nullptr) 
		((CSpikeBarWnd*) (pptr->pspkBars))->Print(pDC, prect3);
}

void CSpikeClassListBox::XorTempVTtag(int xpoint)
{
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		(pptr->pspkBars)->XorTempVTtag(xpoint);
	}
}

void CSpikeClassListBox::ResetBarsXortag()
{
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		(pptr->pspkBars)->ResetXortag ();
	}
}

void CSpikeClassListBox::ReflectBarsMouseMoveMessg(HWND hwnd)
{
	m_hwndBarsReflect = hwnd; 
	for (int i=0; i<GetCount(); i++)
	{
		MYPTR* pptr = (MYPTR*) GetItemData(i);
		(pptr->pspkBars)->ReflectMouseMoveMessg(hwnd);
		if (hwnd != nullptr)
			(pptr->pspkBars)->SetMouseCursorType(CURSOR_MEASURE);
	}
}

void CSpikeClassListBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_hwndBarsReflect != nullptr && point.x >= (m_widthText + m_widthSpikes))
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);		
		::GetWindowRect(m_hwndBarsReflect, & rect0);

		// reflect mouse move message
		::SendMessage(m_hwndBarsReflect, WM_MOUSEMOVE, nFlags, 
			MAKELPARAM(point.x + (rect1.left-rect0.left), 
			point.y + (rect1.top-rect0.top)));
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
		::GetWindowRect(m_hwndBarsReflect, & rect0);

		// reflect mouse move message
		::SendMessage(m_hwndBarsReflect, WM_LBUTTONUP, nFlags, 
			MAKELPARAM(point.x + (rect1.left-rect0.left), 
			point.y + (rect1.top-rect0.top)));
	}
	else	
		CListBox::OnLButtonUp(nFlags, point);
}

