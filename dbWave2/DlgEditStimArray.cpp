#include "StdAfx.h"
#include "dbWave.h"
#include "DlgEditStimArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgEditStimArray, CDialog)

DlgEditStimArray::DlgEditStimArray(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

DlgEditStimArray::~DlgEditStimArray()
{
	SAFE_DELETE(m_pimagelist)
}

void DlgEditStimArray::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTIM, m_stimarrayCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_csEdit);
	DDX_Text(pDX, IDC_EDIT1, m_value);
}

BEGIN_MESSAGE_MAP(DlgEditStimArray, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_EDIT, &DlgEditStimArray::OnBnClickedEdit)
	ON_EN_KILLFOCUS(IDC_REORDER, &DlgEditStimArray::OnEnKillfocusReOrder)
	ON_BN_CLICKED(IDC_DELETE, &DlgEditStimArray::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_INSERT, &DlgEditStimArray::OnBnClickedInsert)
	ON_BN_CLICKED(IDC_DELETE3, &DlgEditStimArray::OnBnClickedDelete3)
	ON_BN_CLICKED(IDC_REORDER, &DlgEditStimArray::OnBnClickedReOrder)
	ON_BN_CLICKED(IDC_COPY, &DlgEditStimArray::OnBnClickedCopy)
	ON_BN_CLICKED(IDC_PASTE, &DlgEditStimArray::OnBnClickedPaste)
	ON_BN_CLICKED(IDC_EXPORT, &DlgEditStimArray::OnBnClickedExport)
	ON_BN_CLICKED(IDC_IMPORTFROMDATA, &DlgEditStimArray::OnBnClickedImportfromdata)
END_MESSAGE_MAP()

// CEditStimArrayDlg message handlers

BOOL DlgEditStimArray::OnInitDialog()
{
	CDialog::OnInitDialog();

	// init dialog size
	m_stretch.AttachDialogParent(this); // attach dialog pointer
	m_stretch.newProp(IDC_LISTSTIM, XLEQ_XREQ, YTEQ_YBEQ);

	m_stretch.newProp(IDOK, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDCANCEL, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_INSERT, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_DELETE, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_DELETE3, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_COPY, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_PASTE, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_REORDER, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_EXPORT, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SIZEBOX, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT1, SZEQ_XLEQ, SZEQ_YBEQ);

	m_binit = TRUE;

	// change style of listbox
	DWORD dw_style = m_stimarrayCtrl.GetExtendedStyle();
	m_stimarrayCtrl.SetExtendedStyle(dw_style | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Add my bitmap to display stim on/off
	m_pimagelist = new CImageList;
	m_pimagelist->Create(16, 16, ILC_COLOR, 2, 2);
	CBitmap bm1, bm2;
	bm1.LoadBitmap(IDB_STIMON);
	m_pimagelist->Add(&bm1, nullptr);
	bm2.LoadBitmap(IDB_STIMOFF);
	m_pimagelist->Add(&bm2, nullptr);

	// add 2 columns (icon & time)
	LVCOLUMN lvcol;
	lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lvcol.iOrder = 1;
	lvcol.cx = 150;
	lvcol.pszText = _T("time (s)");
	lvcol.fmt = LVCFMT_LEFT;
	m_stimarrayCtrl.InsertColumn(0, &lvcol);

	lvcol.iOrder = 0;
	lvcol.cx = 60;
	lvcol.pszText = _T("i");
	lvcol.fmt = LVCFMT_LEFT;
	m_stimarrayCtrl.InsertColumn(0, &lvcol);

	m_stimarrayCtrl.SetImageList(m_pimagelist, LVSIL_SMALL);

	// hide/display combo and load data into listbox
	const auto n_arrays = intervalsandlevels_ptr_array.GetSize();
	ASSERT(n_arrays == 1);
	m_pstim = intervalsandlevels_ptr_array.GetAt(0);
	transferIntervalsArrayToDialogList(m_pstim);

	// update paste button (disabled if stimsaved is empty
	if (m_pstimsaved->intervalsArray.GetSize() < 1)
		GetDlgItem(IDC_PASTE)->EnableWindow(FALSE);

	if (m_pTagList == nullptr)
		GetDlgItem(IDC_IMPORTFROMDATA)->EnableWindow(false);

	// select first item in the list
	selectItem(0);
	return FALSE;
}

void DlgEditStimArray::selectItem(int item)
{
	m_iItem = item;
	if (m_iItem < 0)
		GetDlgItem(IDOK)->SetFocus();
	else
	{
		m_stimarrayCtrl.SetItemState(m_iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_stimarrayCtrl.SetFocus();
	}
}

void DlgEditStimArray::transferIntervalsArrayToDialogList(CIntervalsAndLevels* pstim)
{
	m_stimarrayCtrl.DeleteAllItems();
	const auto nitems = m_pstim->intervalsArray.GetSize();
	m_pstim->chrate = m_rate;
	for (int i = 0; i < nitems; i++)
		addNewItem(i, m_pstim->intervalsArray[i]);
}

void DlgEditStimArray::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 1 || cy > 1)
		m_stretch.ResizeControls(nType, cx, cy);
	CDialog::OnSize(nType, cx, cy);
}

void DlgEditStimArray::OnBnClickedEdit()
{
	auto pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos == nullptr)
		return;
	m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos);

	CRect rect;
	m_stimarrayCtrl.GetSubItemRect(m_iItem, 1, LVIR_LABEL, rect);
	m_stimarrayCtrl.MapWindowPoints(this, rect);
	m_csEdit.MoveWindow(&rect);
	CString cs;
	m_value = static_cast<float>(m_pstim->intervalsArray[m_iItem]) / m_rate;
	m_csEdit.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT)->SetWindowText(_T("&Validate"));

	UpdateData(FALSE);
	m_csEdit.SetFocus();
	m_csEdit.SetSel(0, -1, FALSE);
}

void DlgEditStimArray::OnEnKillfocusReOrder()
{
	UpdateData(TRUE);
	CString cs;
	m_csEdit.GetWindowText(cs);
	m_csEdit.ShowWindow(SW_HIDE);

	m_pstim->intervalsArray[m_iItem] = static_cast<long>(m_value * m_rate);
	LVITEM lvi{};
	setSubItem1(lvi, m_iItem, m_pstim->intervalsArray[m_iItem], cs);
	m_stimarrayCtrl.SetItem(&lvi);
	m_stimarrayCtrl.SetFocus();

	GetDlgItem(IDC_EDIT)->SetWindowText(_T("&Edit"));
}

void DlgEditStimArray::OnBnClickedDelete()
{
	auto pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos == nullptr)
		return;
	m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos);

	m_stimarrayCtrl.SetItemState(m_iItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_stimarrayCtrl.DeleteItem(m_iItem);
	m_pstim->intervalsArray.RemoveAt(m_iItem);
	const auto ilast = m_stimarrayCtrl.GetItemCount() - 1;
	if (m_iItem > ilast)
		m_iItem = ilast;

	resetListOrder();
	selectItem(m_iItem);
}

void DlgEditStimArray::OnBnClickedInsert()
{
	auto pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
	if (pos != nullptr)
		m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos) + 1;
	else
		m_iItem = 0;

	LVITEM lvi;
	CString cs0, cs1;
	setSubItem0(lvi, m_iItem, cs0);
	m_pstim->intervalsArray.InsertAt(m_iItem, 0L);
	m_stimarrayCtrl.InsertItem(&lvi);
	setSubItem1(lvi, m_iItem, m_pstim->intervalsArray[m_iItem], cs1);
	m_stimarrayCtrl.SetItem(&lvi);

	resetListOrder();
	selectItem(m_iItem);
}

void DlgEditStimArray::OnBnClickedDelete3()
{
	m_stimarrayCtrl.DeleteAllItems();
	m_pstim->intervalsArray.RemoveAll();
}

void DlgEditStimArray::OnBnClickedReOrder()
{
	// sort sti
	const auto nitems = m_pstim->intervalsArray.GetSize();
	for (auto i = 0; i < nitems - 1; i++)
	{
		auto imin = m_pstim->intervalsArray[i];
		for (auto j = i + 1; j < nitems; j++)
		{
			if (m_pstim->intervalsArray[j] < imin)
			{
				m_pstim->intervalsArray[i] = m_pstim->intervalsArray[j];
				m_pstim->intervalsArray[j] = imin;
				imin = m_pstim->intervalsArray[i];
			}
		}
	}

	// Use the LV_ITEM structure to insert the items

	CString cs;
	ASSERT(nitems == m_stimarrayCtrl.GetItemCount());
	for (auto i = 0; i < nitems; i++)
	{
		LVITEM lvi{};
		CString cs0, cs1;
		setSubItem0(lvi, i, cs0);
		setSubItem1(lvi, i, m_pstim->intervalsArray[i], cs1);
		m_stimarrayCtrl.SetItem(&lvi);
	}
	//ResetListOrder();
	selectItem(0);
	UpdateData(FALSE);
}

void DlgEditStimArray::OnBnClickedCopy()
{
	*m_pstimsaved = *m_pstim;
	GetDlgItem(IDC_PASTE)->EnableWindow(TRUE);
}

void DlgEditStimArray::OnBnClickedPaste()
{
	const auto nitems = m_pstimsaved->nitems;
	for (auto j = nitems - 1; j >= 0; j--)
	{
		auto pos = m_stimarrayCtrl.GetFirstSelectedItemPosition();
		if (pos != nullptr)
			m_iItem = m_stimarrayCtrl.GetNextSelectedItem(pos) + 1;
		else
			m_iItem = 0;

		LVITEM lvi{};
		CString cs0, cs1;
		setSubItem0(lvi, m_iItem, cs0);
		// add item in the list
		m_pstim->intervalsArray.InsertAt(m_iItem, 0L);
		m_pstim->intervalsArray[m_iItem] = m_pstimsaved->intervalsArray[j];
		setSubItem1(lvi, m_iItem, m_pstim->intervalsArray[m_iItem], cs1);
		m_stimarrayCtrl.SetItem(&lvi);
	}
	resetListOrder();
}

void DlgEditStimArray::OnBnClickedExport()
{
	CString cs_buffer;
	const auto nitems = m_stimarrayCtrl.GetItemCount();
	// copy results from CListCtrl into text buffer
	for (auto i = 0; i < nitems; i++)
	{
		auto cs = m_stimarrayCtrl.GetItemText(i, 1);
		cs_buffer += cs + _T("\n");
	}

	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	CMultiDocTemplate* pTempl = p_app->m_pNoteViewTemplate;
	const auto p_document = pTempl->OpenDocumentFile(nullptr);
	auto pos = p_document->GetFirstViewPosition();
	auto p_view = static_cast<CRichEditView*>(p_document->GetNextView(pos));
	CRichEditCtrl& p_edit = p_view->GetRichEditCtrl();
	p_edit.SetWindowText(cs_buffer);
}

void DlgEditStimArray::OnBnClickedImportfromdata()
{
	const auto nitems = m_pstim->intervalsArray.GetSize();
	m_pstim->chrate = m_rate;

	int nTags = m_pTagList->GetNTags();
	for (int i = 0; i < nTags; i++)
	{
		long lInterval = m_pTagList->GetTagLVal(i);
		m_pstim->AddInterval(lInterval);
		addNewItem(i, lInterval);
	}
}

void DlgEditStimArray::setSubItem0(LVITEM& lvi, int i, CString& cs)
{
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	cs.Format(_T("%i"), i);
	lvi.pszText = (LPTSTR)static_cast<LPCTSTR>(cs);
	lvi.iImage = i % 2;
}

void DlgEditStimArray::setSubItem1(LVITEM& lvi, int iItem, long lInterval, CString& cs)
{
	lvi.iItem = iItem;
	lvi.iSubItem = 1;
	lvi.mask = LVIF_TEXT;
	cs.Format(_T("%10.3f"), (static_cast<float>(lInterval) / m_rate));
	lvi.pszText = (LPTSTR)static_cast<LPCTSTR>(cs);
}

void DlgEditStimArray::addNewItem(int i, long lInterval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	setSubItem0(lvi, i, cs0);
	m_stimarrayCtrl.InsertItem(&lvi);
	setSubItem1(lvi, i, lInterval, cs1);
	m_stimarrayCtrl.SetItem(&lvi);
}

void DlgEditStimArray::resetListOrder()
{
	const auto nitems = m_stimarrayCtrl.GetItemCount();
	for (int i = 0; i < nitems; i++)
	{
		LVITEM lvi;
		CString cs;
		setSubItem0(lvi, i, cs);
		m_stimarrayCtrl.SetItem(&lvi);
	}
}
