// EditStimArrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "EditStimArrayDlg.h"
#include ".\editstimarraydlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditStimArrayDlg dialog

IMPLEMENT_DYNAMIC(CEditStimArrayDlg, CDialog)

CEditStimArrayDlg::CEditStimArrayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditStimArrayDlg::IDD, pParent)
	, m_value(0)
{
	m_pimagelist = NULL;
}

CEditStimArrayDlg::~CEditStimArrayDlg()
{
	SAFE_DELETE(m_pimagelist);
}

void CEditStimArrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTIM, m_stimarrayCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_csEdit);
	DDX_Text(pDX, IDC_EDIT1, m_value);
}

BEGIN_MESSAGE_MAP(CEditStimArrayDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_EDIT, &CEditStimArrayDlg::OnBnClickedEdit)
	ON_EN_KILLFOCUS(IDC_EDIT1, &CEditStimArrayDlg::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_DELETE, &CEditStimArrayDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_INSERT, &CEditStimArrayDlg::OnBnClickedInsert)
	ON_BN_CLICKED(IDC_DELETE3, &CEditStimArrayDlg::OnBnClickedDelete3)
	ON_BN_CLICKED(IDC_BUTTON1, &CEditStimArrayDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_COPY, &CEditStimArrayDlg::OnBnClickedCopy)
	ON_BN_CLICKED(IDC_PASTE, &CEditStimArrayDlg::OnBnClickedPaste)
	ON_CBN_SELCHANGE(IDC_CHANCOMBO, &CEditStimArrayDlg::OnCbnSelchangeChancombo)
END_MESSAGE_MAP()


// CEditStimArrayDlg message handlers

BOOL CEditStimArrayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// init dialog size
	m_stretch.AttachDialogParent(this);	// attach dialog pointer
	m_stretch.newProp(IDC_LISTSTIM,	    XLEQ_XREQ,YTEQ_YBEQ);

	m_stretch.newProp(IDOK,				SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDCANCEL,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_INSERT,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_DELETE,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_DELETE3,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_COPY,			SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_PASTE,		SZEQ_XREQ,SZEQ_YTEQ);
	m_stretch.newProp(IDC_BUTTON1,		SZEQ_XREQ,SZEQ_YTEQ);

	m_stretch.newProp(IDC_SIZEBOX,		SZEQ_XREQ,SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT,			SZEQ_XLEQ,SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT1,		SZEQ_XLEQ,SZEQ_YBEQ);

	m_binit = TRUE;

	// change style of listbox
	DWORD dwStyle = m_stimarrayCtrl.GetExtendedStyle( );
	m_stimarrayCtrl.SetExtendedStyle(dwStyle|LVS_EX_GRIDLINES |LVS_EX_FULLROWSELECT);

	// Add my bitmap to display stim on/off
	m_pimagelist = new CImageList;
	m_pimagelist->Create(16, 16, ILC_COLOR, 2, 2);
	CBitmap bm1, bm2;
	bm1.LoadBitmap(IDB_STIMON);
	m_pimagelist->Add(&bm1, (CBitmap*) NULL);
	bm2.LoadBitmap(IDB_STIMOFF);
	m_pimagelist->Add(&bm2, (CBitmap*) NULL);

	// add 2 columns (icon & time)
	LVCOLUMN lvcol;
	lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lvcol.iOrder = 1;
	lvcol.cx = 150;
	lvcol.pszText = _T("time (s)");
	lvcol.fmt = LVCFMT_LEFT ;
	m_stimarrayCtrl.InsertColumn(0, &lvcol);

	lvcol.iOrder = 0;
	lvcol.cx = 60;
	lvcol.pszText = _T("i");
	lvcol.fmt = LVCFMT_LEFT ;
	m_stimarrayCtrl.InsertColumn(0, &lvcol);

	m_stimarrayCtrl.SetImageList(m_pimagelist, LVSIL_SMALL );

	// hide/display combo and load data into listbox
	int nArrays = m_pIntervalArrays.GetSize();
	if (nArrays > 1)
	{
		CString cs;
		CComboBox* pCombo = (CComboBox *)GetDlgItem(IDC_CHANCOMBO);
		for (int i = 0; i < nArrays; i++)
		{
			CIntervalsArray* ptr = (CIntervalsArray*)m_pIntervalArrays.GetAt(i);
			int chan = ptr->GetChan();
			cs.Format(_T("%i"), chan);
			pCombo->AddString(cs);
		}
		GetDlgItem(IDC_CHANSTATIC)->ShowWindow(SW_SHOW);
		pCombo->ShowWindow(SW_SHOW);
		pCombo->SetCurSel(0);
	}
	m_pstim = (CIntervalsArray*)m_pIntervalArrays.GetAt(0);
	TransferStimlevelToList(m_pstim);

	// update paste button (disabled if stimsaved is empty
	if(m_pstimsaved->iistimulus.GetSize() <1)
		GetDlgItem(IDC_PASTE)->EnableWindow(FALSE);

	// select first item in the list
	SelectItem(0);
	return FALSE;
}

void CEditStimArrayDlg::SelectItem(int item)
{
	m_iItem = item;
	if (m_iItem < 0)
	{
		GetDlgItem(IDOK)->SetFocus();
		return;
	}
	m_stimarrayCtrl.SetItemState(m_iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_stimarrayCtrl.SetFocus();
}

void CEditStimArrayDlg::ResetListOrder()
{
	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString cs;
	int nitems = m_stimarrayCtrl.GetItemCount();
	for (int i = 0; i< nitems; i++)
	{
		lvi.iItem = i;
		lvi.iSubItem =0;
		lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
		cs.Format(_T("%i"), i);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
		lvi.iImage = i%2;
		
		m_stimarrayCtrl.SetItem(&lvi);
	}
}

void CEditStimArrayDlg::TransferStimlevelToList(CIntervalsArray* pstim)
{
	m_stimarrayCtrl.DeleteAllItems();

	int nitems = m_pstim->iistimulus.GetSize();
	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString cs;
	for (int i = 0; i< nitems; i++)
	{
		lvi.iItem = i;

		// Insert the first item
		lvi.iSubItem = 0;
		lvi.mask = LVIF_IMAGE | LVIF_TEXT;
		cs.Format(_T("%i"), i);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
		lvi.iImage = i % 2;		// There are 8 images in the image list

		m_stimarrayCtrl.InsertItem(&lvi);

		// Set subitem 
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 1;
		cs.Format(_T("%10.3f"), ((float)m_pstim->iistimulus[i]) / m_rate);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);

		m_stimarrayCtrl.SetItem(&lvi);
	}
}

void CEditStimArrayDlg::TransferListToStimlevel()
{
}

void CEditStimArrayDlg::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 1 || cy > 1 ) 
		m_stretch.ResizeControls(nType, cx, cy);
	CDialog::OnSize(nType, cx, cy);
}

void CEditStimArrayDlg::OnBnClickedEdit()
{
	POSITION pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos);
	CRect rect;
	m_stimarrayCtrl.GetSubItemRect(m_iItem, 1, LVIR_LABEL, rect);
	m_stimarrayCtrl.MapWindowPoints(this, rect);

	m_csEdit.MoveWindow(&rect);
	CString cs;
	m_value = ((float) m_pstim->iistimulus[m_iItem])/m_rate;
	m_csEdit.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT)->SetWindowText(_T("&Validate"));

	UpdateData(FALSE);
	m_csEdit.SetFocus();
	m_csEdit.SetSel(0, -1, FALSE);
}

void CEditStimArrayDlg::OnEnKillfocusEdit1()
{
	UpdateData(TRUE);
	CString cs;
	m_csEdit.GetWindowText(cs);
	m_csEdit.ShowWindow(SW_HIDE);

	m_pstim->iistimulus[m_iItem] = (long)( m_value * m_rate);
	LVITEM lvi;	
	lvi.iItem = m_iItem;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	cs.Format(_T("%10.3f"), ((float) m_pstim->iistimulus[m_iItem])/m_rate);
	lvi.pszText = (LPTSTR)(LPCTSTR)(cs);

	m_stimarrayCtrl.SetItem(&lvi);
	m_stimarrayCtrl.SetFocus();

	GetDlgItem(IDC_EDIT)->SetWindowText(_T("&Edit"));
}

void CEditStimArrayDlg::OnBnClickedDelete()
{
	POSITION pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos);
	
	m_stimarrayCtrl.SetItemState(m_iItem, 0, LVIS_SELECTED|LVIS_FOCUSED);
	m_stimarrayCtrl.DeleteItem(m_iItem);
	m_pstim->iistimulus.RemoveAt(m_iItem);
	int ilast = m_stimarrayCtrl.GetItemCount() -1;
	if (m_iItem > ilast)
		m_iItem = ilast;
	
	ResetListOrder();
	SelectItem(m_iItem);
}

void CEditStimArrayDlg::OnBnClickedInsert()
{
	POSITION pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL)
		m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos) +1;
	else
		m_iItem = 0;
	
	LVITEM lvi;
	CString cs;
	lvi.iItem = m_iItem;
	
	// Insert the first item
	lvi.iSubItem =0;		// index value
	lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
	cs.Format(_T("%i"), m_iItem);
	lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
	lvi.iImage = m_iItem%2;
	m_stimarrayCtrl.InsertItem(&lvi);

	// add item in the list
	m_pstim->iistimulus.InsertAt(m_iItem, 0L);
	
	// Set subitem 
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;		// time value
	cs.Format(_T("%10.3f"), ((float) m_pstim->iistimulus[m_iItem])/m_rate);
	lvi.pszText = (LPTSTR)(LPCTSTR)(cs);	
	m_stimarrayCtrl.SetItem(&lvi);

	ResetListOrder();
	SelectItem(m_iItem);
}

void CEditStimArrayDlg::OnBnClickedDelete3()
{
	m_stimarrayCtrl.DeleteAllItems();
	m_pstim->iistimulus.RemoveAll();
}

void CEditStimArrayDlg::OnBnClickedButton1()
{
	// sort sti
	int nitems = m_pstim->iistimulus.GetSize();
	for (int i = 0; i< nitems-1; i++)
	{
		long imin = m_pstim->iistimulus[i];
		for (int j = i+1; j<nitems; j++)
		{
			if (m_pstim->iistimulus[j] < imin)
			{
				m_pstim->iistimulus[i] = m_pstim->iistimulus[j];
				m_pstim->iistimulus[j] = imin;
				imin = m_pstim->iistimulus[i];
			}
		}
	}

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString cs;
	ASSERT(nitems == m_stimarrayCtrl.GetItemCount());
	for (int i = 0; i< nitems; i++)
	{
		lvi.iItem = i;
		lvi.iSubItem = 1;		// time value
		lvi.mask = LVIF_TEXT;
		cs.Format(_T("%10.3f"), ((float) m_pstim->iistimulus[i])/m_rate);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
		lvi.iImage = i%2;
		
		m_stimarrayCtrl.SetItem(&lvi);
	}
	//ResetListOrder();
	SelectItem(0);
	UpdateData(FALSE);
}

void CEditStimArrayDlg::OnBnClickedCopy()
{
	*m_pstimsaved = *m_pstim;
	GetDlgItem(IDC_PASTE)->EnableWindow(TRUE);
}

void CEditStimArrayDlg::OnBnClickedPaste()
{
	int nitems = m_pstimsaved->nitems;

	for (int j = nitems-1; j >= 0; j--)
	{
		POSITION pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
		if (pos != NULL)
			m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos) +1;
		else
			m_iItem = 0;
	
		LVITEM lvi;
		CString cs;
		lvi.iItem = m_iItem;
	
		// Insert the first item
		lvi.iSubItem =0;		// index value
		lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
		cs.Format(_T("%i"), m_iItem);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
		lvi.iImage = m_iItem%2;
		m_stimarrayCtrl.InsertItem(&lvi);

		// add item in the list
		m_pstim->iistimulus.InsertAt(m_iItem, 0L);
		m_pstim->iistimulus[m_iItem] = m_pstimsaved->iistimulus[j];
	
		// Set subitem 
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 1;		// time value
		cs.Format(_T("%10.3f"), ((float) m_pstim->iistimulus[m_iItem])/m_rate);
		lvi.pszText = (LPTSTR)(LPCTSTR)(cs);	
		m_stimarrayCtrl.SetItem(&lvi);
	}
	ResetListOrder();
}


void CEditStimArrayDlg::OnCbnSelchangeChancombo()
{
	CComboBox* pCombo = (CComboBox *)GetDlgItem(IDC_CHANCOMBO);
	int isel = pCombo->GetCurSel();
	m_pstim = (CIntervalsArray*)m_pIntervalArrays.GetAt(isel);
	TransferStimlevelToList(m_pstim);
}
