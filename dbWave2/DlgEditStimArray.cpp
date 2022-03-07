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
}

void DlgEditStimArray::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTIM, list_control);
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

BOOL DlgEditStimArray::OnInitDialog()
{
	CDialog::OnInitDialog();

	make_dialog_stretchable();
	m_initialized = TRUE;
	list_control.init_listbox(_T("i"), 60, _T("time (s)"), 80);

	ASSERT(1 == intervals_array.GetSize());
	intervals = intervals_array.GetAt(0);
	transfer_intervals_array_to_dialog_list();

	if (intervals_saved->intervalsArray.GetSize() < 1)
		GetDlgItem(IDC_PASTE)->EnableWindow(FALSE);

	if (tag_list == nullptr)
		GetDlgItem(IDC_IMPORTFROMDATA)->EnableWindow(false);

	list_control.select_item(0);
	return FALSE;
}



void DlgEditStimArray::transfer_intervals_array_to_dialog_list()
{
	list_control.DeleteAllItems();
	const auto n_items = intervals->intervalsArray.GetSize();
	intervals->channel_sampling_rate = m_sampling_rate;
	for (int i = 0; i < n_items; i++) {
		const float time_interval = static_cast<float>(intervals->intervalsArray[i]) / m_sampling_rate;
		list_control.add_new_item(i, time_interval);
	}
}

void DlgEditStimArray::transfer_dialog_list_to_intervals_array()
{
	//CString cs;
	//const int n_items = list_control.GetItemCount();
	//CArray<long, long>* p_intervals = &(intervals->intervalsArray);
	//p_intervals->RemoveAll();
	//p_intervals->SetSize(n_items);

	//LVITEM lvi{};
	//for (auto i = 0; i < n_items; i++) 
	//{
	//	lvi.iItem = i;
	//	list_control.GetItem(&lvi);

	//	lvi.iSubItem = 1;
	//	lvi.mask = LVIF_TEXT;
	//	cs.Format(_T("%10.3f"), (static_cast<float>(lInterval) / m_sampling_rate));
	//	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
	//	

	//	intervals->AddTimeInterval(ii);
	//}
	
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
		m_item_index = list_control.get_index_item_selected();
		if (m_item_index < 0) 
			return;
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
	UpdateData(FALSE);
	list_control.set_edit_value();
}

void DlgEditStimArray::get_edit_value()
{
	UpdateData(TRUE);
	m_item_value = list_control.get_edit_value();
	intervals->intervalsArray[m_item_index] = static_cast<long>(m_item_value * m_sampling_rate);
	list_control.SetFocus();
}

void DlgEditStimArray::set_active_edit_overlay()
{
	GetDlgItem(IDC_EDIT_BUTTON)->SetWindowText(_T("&Validate"));
	mode_edit = true;
	list_control.set_active_edit_overlay();
}

void DlgEditStimArray::set_inactive_edit_overlay()
{
	GetDlgItem(IDC_EDIT_BUTTON)->SetWindowText(_T("&Edit"));
	list_control.set_inactive_edit_overlay();
	mode_edit = false;
}

void DlgEditStimArray::OnEnKillfocusReOrder()
{
	get_edit_value();
	set_inactive_edit_overlay();
}


void DlgEditStimArray::OnBnClickedDelete()
{
	m_item_index = list_control.get_index_item_selected();
	if (m_item_index < 0)
		return;

	list_control.SetItemState(m_item_index, 0, LVIS_SELECTED | LVIS_FOCUSED);
	list_control.DeleteItem(m_item_index);
	intervals->intervalsArray.RemoveAt(m_item_index);
	const auto last_index = list_control.GetItemCount() - 1;
	if (m_item_index > last_index)
		m_item_index = last_index;

	reset_list_order();
	list_control.select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedInsert()
{
	m_item_index = list_control.get_index_item_selected() +1;

	const float time_interval = static_cast<float>(intervals->intervalsArray[m_item_index]) / m_sampling_rate;
	list_control.set_list_control_item(m_item_index, time_interval);
	reset_list_order();
	list_control.select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedDelete3()
{
	list_control.DeleteAllItems();
	intervals->intervalsArray.RemoveAll();
}

void DlgEditStimArray::OnBnClickedReOrder()
{
	// TODO: algorithm wrong: it assumes intervals contains updated data

	// sort sti
	const auto n_items = intervals->intervalsArray.GetSize();
	for (auto i = 0; i < n_items - 1; i++)
	{
		auto i_min = intervals->intervalsArray[i];
		for (auto j = i + 1; j < n_items; j++)
		{
			if (intervals->intervalsArray[j] < i_min)
			{
				intervals->intervalsArray[i] = intervals->intervalsArray[j];
				intervals->intervalsArray[j] = i_min;
				i_min = intervals->intervalsArray[i];
			}
		}
	}

	// Use the LV_ITEM structure to insert the items

	CString cs;
	ASSERT(n_items == list_control.GetItemCount());
	for (auto i = 0; i < n_items; i++) {
		const float time_interval = static_cast<float>(intervals->intervalsArray[i]) / m_sampling_rate;
		list_control.set_list_control_item(i, time_interval);
	}
	
	//ResetListOrder();
	list_control.select_item(0);
	UpdateData(FALSE);
}


void DlgEditStimArray::OnBnClickedCopy()
{
	// TODO: algorithm wrong - data are not up-to-date in intervals
	*intervals_saved = *intervals;
	GetDlgItem(IDC_PASTE)->EnableWindow(TRUE);
}

void DlgEditStimArray::OnBnClickedPaste()
{
	const auto n_items = intervals_saved->n_items;
	for (auto j = n_items - 1; j >= 0; j--)
	{
		auto pos = list_control.GetFirstSelectedItemPosition();
		if (pos != nullptr)
			m_item_index = list_control.GetNextSelectedItem(pos) + 1;
		else
			m_item_index = 0;
		const float time_interval = static_cast<float>(intervals->intervalsArray[m_item_index]) / m_sampling_rate;
		list_control.set_list_control_item(m_item_index, time_interval);

		// add item in the list
		intervals->intervalsArray.InsertAt(m_item_index, 0L);
		intervals->intervalsArray[m_item_index] = intervals_saved->intervalsArray[j];

	}
	reset_list_order();
}

void DlgEditStimArray::OnBnClickedExport()
{
	CString cs_buffer;
	const auto nitems = list_control.GetItemCount();
	// copy results from CListCtrl into text buffer
	for (auto i = 0; i < nitems; i++)
	{
		auto cs = list_control.GetItemText(i, 1);
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
	intervals->channel_sampling_rate = m_sampling_rate;

	const int n_tags = tag_list->GetNTags();
	for (int i = 0; i < n_tags; i++)
	{
		const long l_interval = tag_list->GetTagLVal(i);
		intervals->AddTimeInterval(l_interval);
		const float time_interval = static_cast<float>(l_interval) / m_sampling_rate;
		list_control.add_new_item(i, time_interval);
	}
}

void DlgEditStimArray::reset_list_order()
{
	const auto n_items = list_control.GetItemCount();
	for (int i = 0; i < n_items; i++)
		list_control.set_sub_item_0_value(i);
}

void DlgEditStimArray::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
