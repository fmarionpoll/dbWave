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
	SAFE_DELETE(m_image_list)
}

void DlgEditStimArray::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTIM, m_stimulus_array_control);
	DDX_Control(pDX, IDC_EDIT_TEXT, m_edit_control);
	DDX_Text(pDX, IDC_EDIT_TEXT, m_item_value);
}

BEGIN_MESSAGE_MAP(DlgEditStimArray, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_EDIT_BUTTON, &DlgEditStimArray::OnBnClickedEditButton)
	ON_EN_KILLFOCUS(IDC_REORDER, &DlgEditStimArray::OnEnKillfocusReOrder)
	ON_BN_CLICKED(IDC_DELETE, &DlgEditStimArray::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_INSERT, &DlgEditStimArray::OnBnClickedInsert)
	ON_BN_CLICKED(IDC_DELETE3, &DlgEditStimArray::OnBnClickedDelete3)
	ON_BN_CLICKED(IDC_REORDER, &DlgEditStimArray::OnBnClickedReOrder)
	ON_BN_CLICKED(IDC_COPY, &DlgEditStimArray::OnBnClickedCopy)
	ON_BN_CLICKED(IDC_PASTE, &DlgEditStimArray::OnBnClickedPaste)
	ON_BN_CLICKED(IDC_EXPORT, &DlgEditStimArray::OnBnClickedExport)
	ON_BN_CLICKED(IDC_IMPORTFROMDATA, &DlgEditStimArray::OnBnClickedImportfromdata)
	ON_BN_CLICKED(IDOK, &DlgEditStimArray::OnBnClickedOk)
END_MESSAGE_MAP()

void DlgEditStimArray::make_dialog_stretchable()
{
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
	m_stretch.newProp(IDC_EDIT_BUTTON, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT_TEXT, SZEQ_XLEQ, SZEQ_YBEQ);
}

void DlgEditStimArray::init_listbox()
{
	const DWORD dw_style = m_stimulus_array_control.GetExtendedStyle();
	m_stimulus_array_control.SetExtendedStyle(dw_style | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Add bitmap to display stim on/off
	m_image_list = new CImageList;
	m_image_list->Create(16, 16, ILC_COLOR, 2, 2);
	CBitmap bm1, bm2;
	bm1.LoadBitmap(IDB_STIMON);
	m_image_list->Add(&bm1, nullptr);
	bm2.LoadBitmap(IDB_STIMOFF);
	m_image_list->Add(&bm2, nullptr);

	// add 2 columns (icon & time)
	LVCOLUMN lv_column;
	lv_column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lv_column.iOrder = 1;
	lv_column.cx = 80;
	lv_column.pszText = _T("time (s)");
	lv_column.fmt = LVCFMT_LEFT;
	m_stimulus_array_control.InsertColumn(0, &lv_column);

	lv_column.iOrder = 0;
	lv_column.cx = 60;
	lv_column.pszText = _T("i");
	lv_column.fmt = LVCFMT_LEFT;
	m_stimulus_array_control.InsertColumn(0, &lv_column);

	m_stimulus_array_control.SetImageList(m_image_list, LVSIL_SMALL);
}

BOOL DlgEditStimArray::OnInitDialog()
{
	CDialog::OnInitDialog();

	make_dialog_stretchable();
	m_initialized = TRUE;
	init_listbox();

	// hide/display combo and load data into listbox
	ASSERT(1 == intervals_and_levels_array.GetSize());
	intervals_and_levels = intervals_and_levels_array.GetAt(0);
	transfer_intervals_array_to_dialog_list();

	// update paste button (disabled if stimsaved is empty
	if (intervals_and_levels_saved->intervalsArray.GetSize() < 1)
		GetDlgItem(IDC_PASTE)->EnableWindow(FALSE);

	if (tag_list == nullptr)
		GetDlgItem(IDC_IMPORTFROMDATA)->EnableWindow(false);

	// select first item in the list
	select_item(0);
	return FALSE;
}

void DlgEditStimArray::select_item(const int item)
{
	m_item_index = item;
	if (m_item_index < 0)
		GetDlgItem(IDOK)->SetFocus();
	else
	{
		m_stimulus_array_control.SetItemState(m_item_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_stimulus_array_control.SetFocus();
	}
}

void DlgEditStimArray::transfer_intervals_array_to_dialog_list()
{
	m_stimulus_array_control.DeleteAllItems();
	const auto n_items = intervals_and_levels->intervalsArray.GetSize();
	intervals_and_levels->channel_sampling_rate = m_sampling_rate;
	for (int i = 0; i < n_items; i++)
		addNewItem(i, intervals_and_levels->intervalsArray[i]);
}

void DlgEditStimArray::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 1 || cy > 1)
		m_stretch.ResizeControls(nType, cx, cy);
	CDialog::OnSize(nType, cx, cy);
}

void DlgEditStimArray::OnBnClickedEditButton()
{
	if (!mode_edit)
	{
		if( get_row_selected() < 0) return;
		set_edit_value();
		set_active_edit_overlay();
	}
	else
	{
		get_edit_value();
		set_inactive_edit_overlay();
	}
}

void DlgEditStimArray::set_edit_value ()
{
	m_item_value = static_cast<float>(intervals_and_levels->intervalsArray[m_item_index]) / m_sampling_rate;

	UpdateData(FALSE);
	m_edit_control.SetFocus();
	m_edit_control.SetSel(0, -1, FALSE);
}

void DlgEditStimArray::get_edit_value()
{
	UpdateData(TRUE);
	CString cs;
	m_edit_control.GetWindowText(cs);

	intervals_and_levels->intervalsArray[m_item_index] = static_cast<long>(m_item_value * m_sampling_rate);
	LVITEM lvi{};
	setSubItem1(lvi, m_item_index, intervals_and_levels->intervalsArray[m_item_index], cs);
	m_stimulus_array_control.SetItem(&lvi);
	m_stimulus_array_control.SetFocus();
}

void DlgEditStimArray::set_active_edit_overlay()
{
	m_edit_control.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_BUTTON)->SetWindowText(_T("&Validate"));
	mode_edit = true;

	CRect rect;
	m_stimulus_array_control.GetSubItemRect(m_item_index, 1, LVIR_LABEL, rect);
	int column_width = m_stimulus_array_control.GetColumnWidth(1);
	rect.right = rect.left + column_width;
	m_stimulus_array_control.MapWindowPoints(this, rect);
	m_edit_control.MoveWindow(&rect);
}

void DlgEditStimArray::set_inactive_edit_overlay()
{
	m_edit_control.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_BUTTON)->SetWindowText(_T("&Edit"));
	mode_edit = false;
}

void DlgEditStimArray::OnEnKillfocusReOrder()
{
	get_edit_value();
	set_inactive_edit_overlay();
}

int DlgEditStimArray::get_row_selected()
{
	m_item_index = -1;
	auto pos = m_stimulus_array_control.GetFirstSelectedItemPosition();
	if (pos != nullptr)
		m_item_index = m_stimulus_array_control.GetNextSelectedItem(pos);
	return m_item_index;
}

void DlgEditStimArray::OnBnClickedDelete()
{
	if (get_row_selected() < 0)
		return;

	m_stimulus_array_control.SetItemState(m_item_index, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_stimulus_array_control.DeleteItem(m_item_index);
	intervals_and_levels->intervalsArray.RemoveAt(m_item_index);
	const auto last_index = m_stimulus_array_control.GetItemCount() - 1;
	if (m_item_index > last_index)
		m_item_index = last_index;

	resetListOrder();
	select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedInsert()
{
	if (get_row_selected() >= 0)
		m_item_index += 1;
	else
		m_item_index = 0;

	LVITEM lvi;
	CString cs0;
	setSubItem0(lvi, m_item_index, cs0);
	intervals_and_levels->intervalsArray.InsertAt(m_item_index, 0L);
	m_stimulus_array_control.InsertItem(&lvi);

	CString cs1;
	setSubItem1(lvi, m_item_index, intervals_and_levels->intervalsArray[m_item_index], cs1);
	m_stimulus_array_control.SetItem(&lvi);

	resetListOrder();
	select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedDelete3()
{
	m_stimulus_array_control.DeleteAllItems();
	intervals_and_levels->intervalsArray.RemoveAll();
}

void DlgEditStimArray::OnBnClickedReOrder()
{
	// sort sti
	const auto n_items = intervals_and_levels->intervalsArray.GetSize();
	for (auto i = 0; i < n_items - 1; i++)
	{
		auto i_min = intervals_and_levels->intervalsArray[i];
		for (auto j = i + 1; j < n_items; j++)
		{
			if (intervals_and_levels->intervalsArray[j] < i_min)
			{
				intervals_and_levels->intervalsArray[i] = intervals_and_levels->intervalsArray[j];
				intervals_and_levels->intervalsArray[j] = i_min;
				i_min = intervals_and_levels->intervalsArray[i];
			}
		}
	}

	// Use the LV_ITEM structure to insert the items

	CString cs;
	ASSERT(n_items == m_stimulus_array_control.GetItemCount());
	for (auto i = 0; i < n_items; i++)
	{
		LVITEM lvi{};
		CString cs0, cs1;
		setSubItem0(lvi, i, cs0);
		setSubItem1(lvi, i, intervals_and_levels->intervalsArray[i], cs1);
		m_stimulus_array_control.SetItem(&lvi);
	}
	//ResetListOrder();
	select_item(0);
	UpdateData(FALSE);
}

void DlgEditStimArray::OnBnClickedCopy()
{
	*intervals_and_levels_saved = *intervals_and_levels;
	GetDlgItem(IDC_PASTE)->EnableWindow(TRUE);
}

void DlgEditStimArray::OnBnClickedPaste()
{
	const auto n_items = intervals_and_levels_saved->n_items;
	for (auto j = n_items - 1; j >= 0; j--)
	{
		auto pos = m_stimulus_array_control.GetFirstSelectedItemPosition();
		if (pos != nullptr)
			m_item_index = m_stimulus_array_control.GetNextSelectedItem(pos) + 1;
		else
			m_item_index = 0;

		LVITEM lvi{};
		CString cs0, cs1;
		setSubItem0(lvi, m_item_index, cs0);
		// add item in the list
		intervals_and_levels->intervalsArray.InsertAt(m_item_index, 0L);
		intervals_and_levels->intervalsArray[m_item_index] = intervals_and_levels_saved->intervalsArray[j];
		setSubItem1(lvi, m_item_index, intervals_and_levels->intervalsArray[m_item_index], cs1);
		m_stimulus_array_control.SetItem(&lvi);
	}
	resetListOrder();
}

void DlgEditStimArray::OnBnClickedExport()
{
	CString cs_buffer;
	const auto nitems = m_stimulus_array_control.GetItemCount();
	// copy results from CListCtrl into text buffer
	for (auto i = 0; i < nitems; i++)
	{
		auto cs = m_stimulus_array_control.GetItemText(i, 1);
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
	intervals_and_levels->channel_sampling_rate = m_sampling_rate;

	const int n_tags = tag_list->GetNTags();
	for (int i = 0; i < n_tags; i++)
	{
		const long l_interval = tag_list->GetTagLVal(i);
		intervals_and_levels->AddTimeInterval(l_interval);
		addNewItem(i, l_interval);
	}
}

void DlgEditStimArray::setSubItem0(LVITEM& lvi, const int item, CString& cs)
{
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	cs.Format(_T("%i"), item);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
	lvi.iImage = item % 2;
}

void DlgEditStimArray::setSubItem1(LVITEM& lvi, int iItem, long lInterval, CString& cs) const
{
	lvi.iItem = iItem;
	lvi.iSubItem = 1;
	lvi.mask = LVIF_TEXT;
	cs.Format(_T("%10.3f"), (static_cast<float>(lInterval) / m_sampling_rate));
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
}

void DlgEditStimArray::addNewItem(int i, long lInterval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	setSubItem0(lvi, i, cs0);
	m_stimulus_array_control.InsertItem(&lvi);
	setSubItem1(lvi, i, lInterval, cs1);
	m_stimulus_array_control.SetItem(&lvi);
}

void DlgEditStimArray::resetListOrder()
{
	const auto n_items = m_stimulus_array_control.GetItemCount();
	for (int i = 0; i < n_items; i++)
	{
		LVITEM lvi;
		CString cs;
		setSubItem0(lvi, i, cs);
		m_stimulus_array_control.SetItem(&lvi);
	}
}


void DlgEditStimArray::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
