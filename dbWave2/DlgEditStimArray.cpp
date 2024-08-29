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
= default;


void DlgEditStimArray::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTIM, list_control);
}

BEGIN_MESSAGE_MAP(DlgEditStimArray, CDialog)
	ON_WM_SIZE()
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
	m_stretch.attach_dialog_parent(this); // attach dialog pointer
	m_stretch.new_prop(IDC_LISTSTIM, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.new_prop(IDOK, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDCANCEL, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_INSERT, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_DELETE, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_DELETE3, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_COPY, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_PASTE, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_REORDER, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_EXPORT, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.new_prop(IDC_SIZEBOX, SZEQ_XREQ, SZEQ_YBEQ);
}

BOOL DlgEditStimArray::OnInitDialog()
{
	CDialog::OnInitDialog();

	make_dialog_stretchable();
	m_initialized = TRUE;
	list_control.init_listbox(_T("i"), 60, _T("time (s)"), 80);

	transfer_intervals_array_to_control_list();

	if (intervals_saved.get_size() < 1)
		GetDlgItem(IDC_PASTE)->EnableWindow(FALSE);

	if (tag_list == nullptr)
		GetDlgItem(IDC_IMPORTFROMDATA)->EnableWindow(false);

	list_control.select_item(0);
	return FALSE;
}

void DlgEditStimArray::transfer_intervals_array_to_control_list()
{
	list_control.DeleteAllItems();
	const auto n_items = intervals.get_size();
	intervals.channel_sampling_rate = m_sampling_rate;
	for (int i = 0; i < n_items; i++) 
	{
		const float time_interval = static_cast<float>(intervals.get_at(i)) / m_sampling_rate;
		list_control.add_new_item(i, time_interval);
	}
}

void DlgEditStimArray::transfer_control_list_to_intervals_array()
{
	const int n_items = list_control.GetItemCount();
	intervals.remove_all();

	for (auto i = 0; i < n_items; i++) 
	{
		const float value = list_control.get_item_value(i);
		const long ii = static_cast<long>(value * m_sampling_rate);
		intervals.add_item(ii);
	}
}

void DlgEditStimArray::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 1 || cy > 1)
		m_stretch.resize_controls(nType, cx, cy);
	CDialog::OnSize(nType, cx, cy);
}

void DlgEditStimArray::OnBnClickedDelete()
{
	m_item_index = list_control.get_index_item_selected();
	if (m_item_index < 0)
		return;

	list_control.SetItemState(m_item_index, 0, LVIS_SELECTED | LVIS_FOCUSED);
	list_control.DeleteItem(m_item_index);
	intervals.remove_at(m_item_index);
	const auto last_index = list_control.GetItemCount() - 1;
	if (m_item_index > last_index)
		m_item_index = last_index;

	reset_list_order();
	list_control.select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedInsert()
{
	m_item_index = list_control.get_index_item_selected() +1;
	float time_interval = 0.;
	if (list_control.GetItemCount() >= m_item_index)
		time_interval = list_control.get_item_value(m_item_index) / m_sampling_rate;
	list_control.add_new_item(m_item_index, time_interval);
	reset_list_order();
	list_control.select_item(m_item_index);
}

void DlgEditStimArray::OnBnClickedDelete3()
{
	list_control.DeleteAllItems();
	intervals.remove_all();
}

void DlgEditStimArray::OnBnClickedReOrder()
{
	transfer_control_list_to_intervals_array();

	auto n_items = intervals.get_size();
	for (auto i = 0; i < n_items - 1; i++)
	{
		auto i_min = intervals.get_at(i);
		for (auto j = i + 1; j < n_items; j++)
		{
			if (intervals.get_at(j) < i_min)
			{
				intervals.set_at(i, intervals.get_at(j));
				intervals.set_at(j, i_min);
				i_min = intervals.get_at(i);
			}
		}
	}

	transfer_intervals_array_to_control_list();
	list_control.select_item(0);
	UpdateData(FALSE);
}


void DlgEditStimArray::OnBnClickedCopy()
{
	intervals_saved = intervals;
	GetDlgItem(IDC_PASTE)->EnableWindow(TRUE);
}

void DlgEditStimArray::OnBnClickedPaste()
{
	const auto n_items = intervals_saved.n_items;
	for (auto j = n_items - 1; j >= 0; j--)
	{
		auto pos = list_control.GetFirstSelectedItemPosition();
		if (pos != nullptr)
			m_item_index = list_control.GetNextSelectedItem(pos) + 1;
		else
			m_item_index = 0;
		const float time_interval = static_cast<float>(intervals.get_at(m_item_index)) / m_sampling_rate;
		list_control.set_item(m_item_index, time_interval);

		intervals.insert_at(m_item_index, 0L);
		intervals.set_at(m_item_index, intervals_saved.get_at(j));
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
	CMultiDocTemplate* pTempl = p_app->m_note_view_template;
	const auto p_document = pTempl->OpenDocumentFile(nullptr);
	auto pos = p_document->GetFirstViewPosition();
	auto p_view = static_cast<CRichEditView*>(p_document->GetNextView(pos));
	CRichEditCtrl& p_edit = p_view->GetRichEditCtrl();
	p_edit.SetWindowText(cs_buffer);
}

void DlgEditStimArray::OnBnClickedImportfromdata()
{
	intervals.channel_sampling_rate = m_sampling_rate;

	const int n_tags = tag_list->get_tag_list_size();
	for (int i = 0; i < n_tags; i++)
	{
		const long l_interval = tag_list->get_tag_value_long(i);
		intervals.add_item(l_interval);
		const float time_interval = static_cast<float>(l_interval) / m_sampling_rate;
		list_control.add_new_item(i, time_interval);
	}
}

void DlgEditStimArray::reset_list_order()
{
	const auto n_items = list_control.GetItemCount();
	for (int i = 0; i < n_items; i++)
		list_control.set_item_index(i);
}

void DlgEditStimArray::OnBnClickedOk()
{
	transfer_control_list_to_intervals_array();
	CDialog::OnOK();
}

