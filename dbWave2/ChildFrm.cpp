// ChildFrm.cpp : implementation of the CChildFrame class
//	accelerator

#include "StdAfx.h"
#include "resource.h"
#include "afxpriv.h"
#include "dbWave.h"
#include "dbWaveDoc.h"

#include "MainFrm.h"
#include "ChartData.h"
#include "ViewdbWave.h"
#include "ViewData.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"

#include "DlgFindFiles.h"
#include "DlgProgress.h"
#include "DlgRejectRecord.h"
#include "DlgGotoRecord.h"
#include "DlgImportFiles.h"
#include "DlgDeleteRecordOptions.h"

#include "CApplication.h"
#include "CPivotCell.h"
#include "CPivotField.h"
#include "CPivotTable.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "DlgTransferFiles.h"
#include "DlgExportData.h"
#include "ViewADcontinuous.h"

#include "ViewSpikeSort.h"
#include "ViewSpikeDetect.h"
#include "ViewSpikes.h"
#include "ViewSpikeTemplate.h"
#include "ViewSpikeHist.h"
#include "ChildFrm.h"

#include "DlgBrowseFile.h"
#include "DlgDataComments.h"
#include "DlgExportSpikeInfos.h"
#include "DlgLoadSaveOptions.h"
#include "DlgPrintMargins.h"
#include "DlgdbEditRecord.h"
#include "FilenameCleanupUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)

	ON_COMMAND(ID_VIEW_CURSOR_MODE_NORMAL, &CChildFrame::on_view_cursor_mode_normal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSOR_MODE_NORMAL, &CChildFrame::on_update_view_cursor_mode_normal)
	ON_COMMAND(ID_VIEW_CURSOR_MODE_MEASURE, &CChildFrame::on_view_cursor_mode_measure)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSOR_MODE_MEASURE, &CChildFrame::on_update_view_cursor_mode_measure)
	ON_COMMAND(ID_VIEW_CURSOR_MODE_ZOOMIN, &CChildFrame::on_view_cursor_mode_zoom_in)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSOR_MODE_ZOOMIN, &CChildFrame::on_update_view_cursor_mode_zoom_in)
	ON_COMMAND(ID_OPTIONS_BROWSE_MODE, &CChildFrame::on_options_browse_mode)
	ON_COMMAND(ID_OPTIONS_PRINTMARGINS, &CChildFrame::on_options_print_margins)
	ON_COMMAND(ID_OPTIONS_LOAD_SAVE_OPTIONS, &CChildFrame::on_options_load_save_options)
	ON_MESSAGE(WM_MYMESSAGE, &CChildFrame::on_my_message)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_DATABASE, ID_VIEW_ACQUIRE_DATA, &CChildFrame::replace_view_index)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_DATABASE, ID_VIEW_ACQUIRE_DATA, &CChildFrame::on_update_view_menu)

	ON_COMMAND(ID_TOOLS_EXPORT_DATA_COMMENTS, &CChildFrame::on_tools_export_data_comments)
	ON_COMMAND(ID_TOOLS_EXPORT_DATA_AS_TEXT, &CChildFrame::on_tools_export_data_as_text)
	ON_COMMAND(ID_TOOLS_EXPORT_NUMBER_OF_SPIKES, &CChildFrame::on_tools_export_number_of_spikes)
	ON_COMMAND(ID_TOOLS_EXPORT_DATA_FILE, &CChildFrame::on_tools_export_data_file)
	ON_COMMAND(ID_TOOLS_REMOVE_MISSING_FILE_NAMES, &CChildFrame::on_tools_remove_missing_files)
	ON_COMMAND(ID_TOOLS_REMOVE_DUPLICATE_FILES, &CChildFrame::on_tools_remove_duplicate_files)
	ON_COMMAND(ID_TOOLS_REMOVE_ARTEFACT_FILES, &CChildFrame::on_tools_remove_artefact_files)
	ON_COMMAND(ID_TOOLS_RESTORE_DELETED_FILES, &CChildFrame::on_tools_restore_deleted_files)
	ON_COMMAND(ID_TOOLS_CHECK_CONSISTENCY, &CChildFrame::on_tools_check_file_lists_consistency)
	ON_COMMAND(ID_TOOLS_SYNCHRO_CURRENT_FILE, &CChildFrame::on_tools_synchronize_source_information_current_file)
	ON_COMMAND(ID_TOOLS_SYNCHRO_ALL_FILES, &CChildFrame::on_tools_synchronize_source_information_all_files)
	ON_COMMAND(ID_TOOLS_TRANSFER_FILES, &CChildFrame::on_tools_copy_all_project_files)
	ON_COMMAND(ID_TOOLS_IMPORT_DATA_FILES, &CChildFrame::on_tools_import_data_files)
	ON_COMMAND(ID_TOOLS_IMPORT_SPIKE_FILES, &CChildFrame::on_tools_import_spike_files)
	ON_COMMAND(ID_TOOLS_IMPORT_DATABASE, &CChildFrame::on_tools_import_database)
	ON_COMMAND(ID_TOOLS_IMPORT_ATF_FILES, &CChildFrame::on_tools_import_atl_files)

	ON_COMMAND(ID_RECORD_DELETE_CURRENT, &CChildFrame::on_record_delete)
	ON_COMMAND(ID_RECORD_GOTO_RECORD, &CChildFrame::on_record_goto)
	ON_COMMAND(ID_RECORD_ADD, &CChildFrame::on_record_add)

	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_TOOLS_PATHS_RELATIVE, &CChildFrame::on_tools_paths_relative)
	ON_COMMAND(ID_TOOLS_PATHS_ABSOLUTE, &CChildFrame::on_tools_paths_absolute)
	ON_COMMAND(ID_TOOLS_PATH, &CChildFrame::on_tools_path)
	ON_COMMAND(ID_TOOLS_IMPORT, &CChildFrame::on_tools_import)
	ON_COMMAND(ID_TOOLS_SYNCHRO, &CChildFrame::on_tools_synchro)
	ON_COMMAND(ID_TOOLS_REMOVE_UNUSED, &CChildFrame::on_tools_remove_unused)
	ON_COMMAND(ID_TOOLS_GARBAGE, &CChildFrame::on_tools_garbage)
	ON_COMMAND(ID_TOOLS_COMPACT_DATABASE, &CChildFrame::on_tools_compact_database)
	ON_COMMAND(ID_TOOLS_CLEANUP_FILENAMES, &CChildFrame::on_tools_cleanup_filenames)

END_MESSAGE_MAP()

CChildFrame::CChildFrame()
= default;

CChildFrame::~CChildFrame()
= default;

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT & cs)
{
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext & dc) const
{
	CMDIChildWndEx::Dump(dc);
}

#endif //_DEBUG

void CChildFrame::on_view_cursor_mode_normal()
{
	m_cursor_state = CURSOR_ARROW;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::on_update_view_cursor_mode_normal(CCmdUI * p_cmd_ui)
{
	p_cmd_ui->SetCheck(m_cursor_state == CURSOR_ARROW);
}

void CChildFrame::on_view_cursor_mode_measure()
{
	m_cursor_state = CURSOR_CROSS;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::on_update_view_cursor_mode_measure(CCmdUI * p_cmd_ui)
{
	p_cmd_ui->SetCheck(m_cursor_state == CURSOR_CROSS);
}

void CChildFrame::on_view_cursor_mode_zoom_in()
{
	m_cursor_state = CURSOR_ZOOM;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::on_update_view_cursor_mode_zoom_in(CCmdUI * p_cmd_ui)
{
	p_cmd_ui->SetCheck(m_cursor_state == CURSOR_ZOOM);
}

void CChildFrame::on_options_browse_mode()
{
	DlgBrowseFile dlg;
	dlg.p_options_view_data = &(static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data);
	dlg.DoModal();
}

void CChildFrame::on_options_print_margins()
{
	DlgPrintMargins dlg;
	const auto p_source = &(static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data);
	dlg.options_view_data = p_source;
	dlg.DoModal();
}

void CChildFrame::on_options_load_save_options()
{
	DlgLoadSaveOptions dlg;
	if (IDOK == dlg.DoModal())
	{
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		const auto p_param_files = &(p_app->m_cs_parameter_files);
		p_param_files->RemoveAll();
		for (auto i = 0; i < dlg.p_files.GetSize(); i++)
			p_param_files->Add(dlg.p_files.GetAt(i));
	}
}

void CChildFrame::on_tools_export_data_comments()
{
	DlgDataComments dlg;
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	dlg.p_options_view_data = &(p_app->options_view_data);
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		export_ascii(0);
	}
}

void CChildFrame::on_tools_export_number_of_spikes()
{
	DlgExportSpikeInfos dlg;
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	dlg.options_view_spikes = &p_app->options_view_spikes;
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		export_ascii(1);
	}
}

void CChildFrame::on_tools_export_data_as_text()
{
	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc != nullptr)
	{
		p_dbWave_doc->export_datafiles_as_text_files();
		p_dbWave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
		PostMessage(WM_MYMESSAGE, HINT_SHARED_MEM_FILLED, NULL);
	}
}

void CChildFrame::export_ascii(const int option)
{
	CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	const auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	switch (option)
	{
	case 0:
		p_db_wave_doc->export_data_ascii_comments(&sf);
		break;
	case 1:
		p_db_wave_doc->export_number_of_spikes(&sf);
		break;
	default:
		break;
	}

	const auto dw_len = static_cast<SIZE_T>(sf.GetLength());
	auto h_mem = sf.Detach();
	if (!h_mem)
		return;
	h_mem = GlobalReAlloc(h_mem, dw_len, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	if (!h_mem)
		return;
	auto* p_source = new COleDataSource();
	p_source->CacheGlobalData(CF_UNICODETEXT, h_mem);
	p_source->SetClipboard();

	//pass data to excel - if it does not work, paste to a new text document
	switch (option)
	{
	case 0: // export CAcqData
	{
		auto flag = FALSE;
		if (p_app->options_view_data.b_to_excel)
			flag = export_to_excel();
		if (!p_app->options_view_data.b_to_excel || !flag)
		{
			CMultiDocTemplate* note_view_template = p_app->m_note_view_template;
			const auto p_dbWave_doc_export = note_view_template->OpenDocumentFile(nullptr);
			auto pos = p_dbWave_doc_export->GetFirstViewPosition();
			const auto p_view = static_cast<ViewNoteDoc*>(p_dbWave_doc_export->GetNextView(pos));
			auto& p_edit = p_view->GetRichEditCtrl();
			p_edit.Paste();
		}
	}
	break;
	case 1:
	{
		auto flag = FALSE;
		if (p_app->options_view_spikes.b_export_to_excel)
			flag = export_to_excel_and_build_pivot(option);

		if (!p_app->options_view_spikes.b_export_to_excel || !flag)
		{
			CMultiDocTemplate* note_view_template = p_app->m_note_view_template;
			const auto p_dbWave_doc_export = note_view_template->OpenDocumentFile(nullptr);
			auto pos = p_dbWave_doc_export->GetFirstViewPosition();
			const auto p_view = static_cast<ViewNoteDoc*>(p_dbWave_doc_export->GetNextView(pos));
			auto& p_edit = p_view->GetRichEditCtrl();
			p_edit.Paste();
		}
	}
	break;
	default:
		break;
	}
}

void CChildFrame::ActivateFrame(int n_cmd_show)
{
	if (n_cmd_show == -1)
		n_cmd_show = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(n_cmd_show);
}

LRESULT CChildFrame::on_my_message(WPARAM w_param, LPARAM l_param)
{
	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
	{
		const short low_param = LOWORD(l_param);
		m_cursor_state = low_param;
	}
	break;

	case HINT_SHARED_MEM_FILLED:
	{
		const auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (p_app->m_psf != nullptr)
		{
			CMultiDocTemplate* note_view_template = p_app->m_note_view_template;
			const auto p_doc_export = note_view_template->OpenDocumentFile(nullptr);
			auto pos = p_doc_export->GetFirstViewPosition();
			const auto* p_view = static_cast<ViewNoteDoc*>(p_doc_export->GetNextView(pos));
			auto& p_edit = p_view->GetRichEditCtrl();
			OpenClipboard();
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, (p_app->m_psf)->Detach());
			CloseClipboard();
			p_edit.Paste();
		}
	}
	break;

	default:
		break;
	}
	return 0L;
}

void CChildFrame::replace_view_index(UINT n_id)
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	const auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
	auto b_active_panes = TRUE;
	switch (n_id)
	{
	case ID_VIEW_DATABASE:
		replace_view(RUNTIME_CLASS(ViewdbWave), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_db_view);
		break;
	case ID_VIEW_DATA_FILE:
		if (!p_db_wave_doc->db_get_current_dat_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewData), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_data_view);
		break;
	case ID_VIEW_SPIKE_DETECTION:
		if (!p_db_wave_doc->db_get_current_dat_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewSpikeDetection), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_data_view);
		break;
	case ID_VIEW_SPIKE_DISPLAY:
		if (!p_db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewSpikes), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_spike_view);
		break;
	case ID_VIEW_SPIKE_SORTING_AMPLITUDE:
		if (!p_db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewSpikeSort), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_spike_view);
		break;
	case ID_VIEW_SPIKE_SORTING_TEMPLATES:
		if (!p_db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewSpikeTemplates), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_spike_view);
		break;
	case ID_VIEW_SPIKE_TIME_SERIES:
		if (!p_db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty())
			replace_view(RUNTIME_CLASS(ViewSpikeHist), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_spike_view);
		break;
	case ID_VIEW_ACQUIRE_DATA:
		replace_view(RUNTIME_CLASS(ViewADcontinuous), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_data_view);
		b_active_panes = FALSE;
		break;

	default:
		n_id = 0;
		replace_view(RUNTIME_CLASS(ViewdbWave), static_cast<CdbWaveApp*>(AfxGetApp())->h_menu_data_view);
		break;
	}
	p_mainframe->ActivatePropertyPane(b_active_panes);
	p_mainframe->ActivateFilterPane(b_active_panes);
	m_previous_view_on = m_view_on;
	m_view_on = n_id;

	// update all views
	auto doc_type = 1;
	if (n_id < ID_VIEW_SPIKE_DISPLAY || n_id == ID_VIEW_ACQUIRE_DATA)
		doc_type = 0;
	p_db_wave_doc->UpdateAllViews(nullptr, MAKELPARAM(HINT_REPLACE_VIEW, doc_type), nullptr);
}

void CChildFrame::on_update_view_menu(CCmdUI * p_cmd_ui)
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	BOOL flag = (p_db_wave_doc != nullptr);

	switch (p_cmd_ui->m_nID)
	{
	case ID_VIEW_SPIKE_DISPLAY:
	case ID_VIEW_SPIKE_SORTING_AMPLITUDE:
	case ID_VIEW_SPIKE_SORTING_TEMPLATES:
	case ID_VIEW_SPIKE_TIME_SERIES:
		flag = (flag
			&& !p_db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty()
			&& m_view_on != ID_VIEW_ACQUIRE_DATA);
		break;

	case ID_VIEW_ACQUIRE_DATA:
		flag = p_app->m_ad_card_found;
		break;

	default:
		flag = (flag && !p_db_wave_doc->db_get_current_dat_file_name().IsEmpty());
		break;
	}

	p_cmd_ui->Enable(flag);
	p_cmd_ui->SetCheck(m_view_on == p_cmd_ui->m_nID);
}

void CChildFrame::replace_view(CRuntimeClass * p_view_class, const HMENU h_menu)
{
	// assume that the views replaced are of dbTableView type
	const auto p_current_view = GetActiveView();
	if ((p_current_view->IsKindOf(p_view_class)) == TRUE)
		return;

	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc == nullptr)
		return;

	p_dbWave_doc->h_my_menu = h_menu;

	CSize size;
	CRect rect;
	p_current_view->GetClientRect(rect);
	size.cx = rect.right;
	size.cy = rect.bottom;

	// delete old view without deleting document
	const auto b_auto_del = p_dbWave_doc->m_bAutoDelete;
	p_dbWave_doc->m_bAutoDelete = FALSE;
	p_dbWave_doc->close_current_data_file();
	p_current_view->DestroyWindow();
	p_dbWave_doc->m_bAutoDelete = b_auto_del;

	// create new view
	CCreateContext context;
	context.m_pNewViewClass = p_view_class;
	context.m_pCurrentDoc = p_dbWave_doc;
	context.m_pNewDocTemplate = nullptr;
	context.m_pLastView = nullptr;
	context.m_pCurrentFrame = nullptr;

	// create view inside a splitter
	const auto p_new_view = static_cast<CView*>(CreateView(&context));
	p_new_view->SendMessage(WM_INITIALUPDATE, 0, 0);
	GetMDIFrame()->MDISetMenu(CMenu::FromHandle(h_menu), nullptr);
	GetMDIFrame()->DrawMenuBar();
	RecalcLayout();

	// adjust size of the view replacing the previous one
	p_new_view->SendMessage(WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(size.cx, size.cy));
	SetActiveView(p_new_view);
}

void CChildFrame::on_tools_remove_missing_files()
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	p_db_wave_doc->remove_missing_files();
	p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_remove_duplicate_files()
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	p_db_wave_doc->remove_duplicate_files();
	p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_check_file_lists_consistency()
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	p_db_wave_doc->remove_false_spk_files();
	p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_restore_deleted_files()
{
	// scan directories and rename files *.datdel into *.dat and *.spkdel into *.spk
	CStringArray file_names;
	DlgFindFiles dlg;
	dlg.m_p_file_names = &file_names;
	dlg.m_sel_init = 0;
	dlg.m_i_option = 1;
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;
	dlg.m_pdbDoc = p_db_wave_doc;


	const auto i_result = dlg.DoModal();

	if (IDOK == i_result)
	{
		DlgProgress dlg_progress;
		dlg_progress.Create();
		dlg_progress.set_step(1);
		auto i_step = 0;
		CString cs_comment;

		const auto n_files = file_names.GetSize();
		for (auto i = 0; i < n_files; i++)
		{
			auto cs_old_name = file_names[i];
			auto cs_new_name = cs_old_name.Left(cs_old_name.GetLength() - 3);
			cs_comment.Format(_T("Rename file [%i / %i] "), i + 1, n_files);
			cs_comment += cs_old_name;
			dlg_progress.set_status(cs_comment);
			if (MulDiv(i, 100, n_files) > i_step)
			{
				dlg_progress.step_it();
				i_step = MulDiv(i, 100, n_files);
			}
			CFile::Rename(cs_old_name, cs_new_name);
		}
	}

	p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_synchronize_source_information_current_file()
{
	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc == nullptr)
		return;
	p_dbWave_doc->synchronize_source_infos(FALSE);
	p_dbWave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_synchronize_source_information_all_files()
{
	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc == nullptr)
		return;
	p_dbWave_doc->synchronize_source_infos(TRUE);
	p_dbWave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_tools_remove_artefact_files()
{
	DlgRejectRecord dlg1;
	long n_consecutive_points = 1000;
	short jitter = 4;
	BOOL flag_rejected_file_as;

	dlg1.m_b_consecutive_points = TRUE;
	dlg1.m_n_consecutive_points = n_consecutive_points;
	dlg1.m_jitter = jitter;
	if (IDOK == dlg1.DoModal())
	{
		n_consecutive_points = dlg1.m_n_consecutive_points;
		jitter = static_cast<short>(dlg1.m_jitter);
		flag_rejected_file_as = dlg1.m_flag;
	}
	else
		return;

	// search
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	auto i_step = 0;
	CString cs_comment;
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;
	const int n_files = p_db_wave_doc->db_get_records_count();

	for (int i_file = 0; i_file < n_files; i_file++)
	{
		// check if user wants to stop
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;

		cs_comment.Format(_T("Processing file [%i / %i] "), i_file + 1, n_files);
		cs_comment += p_db_wave_doc->db_get_current_dat_file_name();
		dlg.set_status(cs_comment);

		// load file

		if (p_db_wave_doc->db_set_current_record_position(i_file)) {
			const auto b_ok = p_db_wave_doc->open_current_data_file();
			if (!b_ok)
				continue;
		}
		else
			continue;

		auto p_dat = p_db_wave_doc->m_p_data_doc;
		if (p_dat == nullptr)
			continue;

		auto consecutive_points = 0;
		long l_data_first = 0;
		const auto l_data_last = p_dat->get_doc_channel_length() - 1;

		while (l_data_first < l_data_last)
		{
			// read a chunk of data
			auto l_read_write_first = l_data_first;
			auto l_read_write_last = l_data_last;
			if (!p_dat->load_raw_data(&l_read_write_first, &l_read_write_last, 0))
				break; // exit if error reported
			if (l_read_write_last > l_data_last)
				l_read_write_last = l_data_last;
			const auto p_data0 = p_dat->load_transformed_data(l_data_first, l_read_write_last, 0, 0);

			// compute initial offset (address of first point)
			// assume that detection is on channel 1
			auto p_data = p_data0;
			short last_value = 0;
			for (auto cx = l_data_first; cx <= l_read_write_last; cx++)
			{
				if (abs(last_value - *p_data) <= jitter)
				{
					consecutive_points++;
					if (consecutive_points > consecutive_points)
						break;
				}
				else
					consecutive_points = 0;
				last_value = *p_data;
				p_data++;
			}
			l_data_first = l_read_write_last + 1;
		}
		// change flag if condition met
		if (consecutive_points >= consecutive_points)
			p_db_wave_doc->db_set_current_record_flag(flag_rejected_file_as);

		// update interface
		if (MulDiv(i_file, 100, n_files) > i_step)
		{
			dlg.step_it();
			i_step = MulDiv(i_file, 100, n_files);
		}
	}
	// exit: update all views
	p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::on_record_goto()
{
	DlgGotoRecord dlg;
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;
	dlg.m_record_pos = p_db_wave_doc->db_get_current_record_position();
	dlg.m_record_id = p_db_wave_doc->db_get_current_record_id();
	dlg.m_b_goto_record_id = static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data.b_goto_record_id;

	if (IDOK == dlg.DoModal())
	{
		static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data.b_goto_record_id = dlg.m_b_goto_record_id;
		BOOL bSuccess = FALSE;
		if (!dlg.m_b_goto_record_id)
			bSuccess = p_db_wave_doc->db_set_current_record_position(dlg.m_record_pos);
		else
			bSuccess = p_db_wave_doc->db_move_to_id(dlg.m_record_id);
		if (bSuccess)
			p_db_wave_doc->UpdateAllViews(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
	}
}

void CChildFrame::on_tools_import_files(int i_filter)
{
	DlgFindFiles dlg;
	CStringArray file_names;
	dlg.m_p_file_names = &file_names;
	dlg.m_sel_init = i_filter;
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;
	dlg.m_pdbDoc = p_db_wave_doc;
	if (IDOK == dlg.DoModal())
	{
		CdbWaveDoc* p_db_wave_doc1 = CdbWaveDoc::get_active_mdi_document();
		if (p_db_wave_doc1 == nullptr)
			return;
		p_db_wave_doc1->import_file_list(file_names);
		p_db_wave_doc1->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
		// display files which were discarded in a separate document
		PostMessage(WM_MYMESSAGE, HINT_SHARED_MEM_FILLED, NULL);
	}
}

void CChildFrame::on_tools_import_atl_files()
{
	DlgFindFiles dlg;
	CStringArray file_names;
	dlg.m_p_file_names = &file_names;
	dlg.m_sel_init = 6;
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
		return;

	dlg.m_pdbDoc = p_db_wave_doc;
	if (IDOK == dlg.DoModal())
	{
		DlgImportFiles dlg2;
		CStringArray converted_files;
		dlg2.m_p_converted_files = &converted_files;
		dlg2.m_p_file_name_array = &file_names;
		dlg2.m_option = ATFFILE;
		CdbWaveDoc* p_db_wave_doc1 = CdbWaveDoc::get_active_mdi_document();
		if (p_db_wave_doc1 == nullptr)
			return;

		dlg2.m_pdb_doc = p_db_wave_doc1;
		if (IDOK == dlg2.DoModal())
		{
			CdbWaveDoc* p_db_wave_doc2 = CdbWaveDoc::get_active_mdi_document();
			if (p_db_wave_doc2 == nullptr)
				return;
			p_db_wave_doc2->import_file_list(converted_files);
			if (p_db_wave_doc2->db_move_last())
				p_db_wave_doc2->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
			// display files which were discarded in a separate document
			PostMessage(WM_MYMESSAGE, HINT_SHARED_MEM_FILLED, NULL);
		}
	}
}

void CChildFrame::on_record_delete()
{
	const auto p_view = GetActiveView();
	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc == nullptr)
		return;

	// save index current file
	auto current_index = p_dbWave_doc->db_get_current_record_position();
	if (current_index < 0)
		current_index = 0;

	auto b_delete = TRUE;
	if (!m_b_keep_choice_)
	{
		b_delete = FALSE;
		DlgDeleteRecordOptions dlg;
		dlg.m_b_delete_file = m_b_delete_file_;
		dlg.m_b_keep_choice = m_b_keep_choice_;

		if (IDOK == dlg.DoModal())
		{
			m_b_delete_file_ = dlg.m_b_delete_file;
			m_b_keep_choice_ = dlg.m_b_keep_choice;
			b_delete = TRUE;
		}
	}

	if (b_delete)
	{
		// delete records from the database and collect names of files to change
		// save list of data files to delete into a temporary array
		if (p_view->IsKindOf(RUNTIME_CLASS(ViewdbWave)))
			static_cast<ViewdbWave*>(p_view)->delete_records();
		else
			p_dbWave_doc->db_delete_current_record();

		// update views and rename "erased" files
		p_dbWave_doc->UpdateAllViews(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
		if (p_dbWave_doc->db_set_current_record_position(current_index))
			p_dbWave_doc->UpdateAllViews(nullptr, HINT_DOC_MOVE_RECORD, nullptr);

		// delete erased files
		if (m_b_delete_file_)
			p_dbWave_doc->delete_erased_files();
	}
}

BOOL CChildFrame::export_to_excel()
{
	//define a few constants
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant cov_zero(static_cast<short>(0));

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication o_app;
	o_app.CreateDispatch(_T("Excel.Application"));
	if (!o_app)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks o_books = o_app.get_Workbooks();
	CWorkbook o_book = o_books.Add(v_opt);
	CWorksheets o_sheets = o_book.get_Worksheets();
	auto index = COleVariant(static_cast<short>(1));
	CWorksheet odata_sheet = o_sheets.get_Item(index); // select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt); // select first cell

	odata_sheet.Paste(v_opt, v_opt); // paste data
	//odataSheet._PasteSpecial(COleVariant(_T("Text")), vOpt, vOpt, vOpt, vOpt, vOpt);

	odata_sheet.put_Name(_T("data")); // change name of sheet

	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}

BOOL CChildFrame::export_to_excel_and_build_pivot(int option)
{
	//define a few constants
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant cov_zero(static_cast<short>(0));

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication o_app;
	o_app.CreateDispatch(_T("Excel.Application"));
	if (!o_app)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks o_books = o_app.get_Workbooks();
	CWorkbook o_book = o_books.Add(v_opt);
	CWorksheets o_sheets = o_book.get_Worksheets();
	auto item = COleVariant(static_cast<short>(1));
	CWorksheet odata_sheet = o_sheets.get_Item(item); // select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt); // select first cell
	odata_sheet.Paste(v_opt, v_opt); // paste data
	odata_sheet.put_Name(_T("data")); // change name of sheet
	if (option == 1)
	{
		auto l_cell_1 = COleVariant(_T("A5"));
		CRange o_range1 = odata_sheet.get_Range(l_cell_1, v_opt); // select first cell of the table
		const auto col1 = o_range1.get_Column();
		const auto row1 = o_range1.get_Row();
		o_range1.Select();

		CRange o_select = o_range1.get_End(-4121); //xlDown); XlDirection.xlDown
		const auto row2 = o_select.get_Row();
		o_select = o_range1.get_End(-4161); //xlToRight); XlDirection.xlToRight
		const auto col2 = o_select.get_Column();

		auto row_size = COleVariant(row2 - row1 + 1);
		auto column_size = COleVariant(col2 - col1 + 1);
		o_range1 = o_range1.get_Resize(row_size, column_size);
		o_range1.Select(); // select data area

		// build range address as text (I was unable to pass the range into PivotTableWizard)
		CString cs2;
		constexpr auto i_alphabet = 26;
		if (col2 > i_alphabet)
		{
			const int decimal_col = col2 / i_alphabet;
			const int uni_col = col2 - (decimal_col * i_alphabet);
			cs2.Format(_T("%c%c%d"), _T('A') + (decimal_col - 1) % i_alphabet, _T('A') + (uni_col - 1) % i_alphabet, row2);
		}
		else
			cs2.Format(_T("%c%d"), 'A' + (col2 - 1) % i_alphabet, row2);
		CString cs1;
		cs1.Format(_T("%c%d"), 'A' + (col1 - 1) % i_alphabet, row1);
		cs1 = cs1 + _T(":") + cs2;
		cs2 = odata_sheet.get_Name();
		cs1 = cs2 + _T("!") + cs1;

		auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (p_app->options_view_spikes.b_export_pivot)
		{
			CString cs_bin;
			build_excel_pivot(&o_app, &odata_sheet, cs1, _T("pivot_cnt"), static_cast<short>(-4112), col2);
			build_excel_pivot(&o_app, &odata_sheet, cs1, _T("pivot_std"), static_cast<short>(-4156), col2);
			build_excel_pivot(&o_app, &odata_sheet, cs1, _T("pivot_avg"), static_cast<short>(-4106), col2);
		}
	}

	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}

void CChildFrame::build_excel_pivot(void* po_app, void* p_odata_sheet, CString cs_source_data_address, CString cs_name_sheet,
	short xl_consolidation_function, int col2)
{
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	auto* o_app = static_cast<CApplication*>(po_app);
	auto* odata_sheet = static_cast<CWorksheet*>(p_odata_sheet);

	odata_sheet->Activate();
	auto source_data = COleVariant(cs_source_data_address);
	CPivotTable o_pivot1 = odata_sheet->PivotTableWizard(
		cov_xl_database, //const VARIANT& SourceType,
		source_data, //const VARIANT& SourceData,
		v_opt, //const VARIANT& TableDestination,
		v_opt, //const VARIANT& TableName,
		cov_false, //const VARIANT& RowGrand,
		cov_false, //const VARIANT& ColumnGrand,
		v_opt, //const VARIANT& SaveData,
		v_opt, //const VARIANT& HasAutoFormat,
		v_opt, //const VARIANT& AutoPage,
		v_opt, //const VARIANT& Reserved,
		v_opt, //const VARIANT& BackgroundQuery,
		v_opt, //const VARIANT& OptimizeCache,
		v_opt, //const VARIANT& PageFieldOrder,
		v_opt, //const VARIANT& PageFieldWrapCount,
		v_opt, //const VARIANT& ReadData,
		v_opt); //const VARIANT& Connection)

	CWorksheet pivot_sheet1 = o_app->get_ActiveSheet();
	pivot_sheet1.put_Name(cs_name_sheet);

	// get options
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto option_view_spikes = &(p_app->options_view_spikes);

	// add fields to pivot table
	if (option_view_spikes->b_acq_comments)
	{
		auto row_field = COleVariant(_T("type"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_false);
		row_field = COleVariant(_T("stim1"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_true);
		row_field = COleVariant(_T("conc1"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_true);
		for (auto i = 1; i <= 3; i++)
		{
			auto index = COleVariant(static_cast<short>(i));
			CPivotField o_field = o_pivot1.get_RowFields(index);
			index = COleVariant(static_cast<short>(1));
			o_field.put_Subtotals(index, cov_false);
		}
	}

	// loop over the bins
	CString cs_bin;
	auto bin = 0;
	const auto col1 = option_view_spikes->n_comment_columns + 1;
	for (auto i = col1; i <= col2; i++, bin++)
	{
		cs_bin.Format(_T("bin_%i"), bin);
		auto index = COleVariant(static_cast<short>(i));
		const auto field = o_pivot1.PivotFields(index);
		auto caption = COleVariant(cs_bin);
		auto function = COleVariant(xl_consolidation_function);

		o_pivot1.AddDataField(field, caption, function);
	}

	if (col1 < col2) // error fired if only 1 bin is measured
	{
		CPivotField o_field = o_pivot1.get_DataPivotField();
		o_field.put_Orientation(2);
	}
}

void CChildFrame::on_record_add()
{
	replace_view_index(ID_VIEW_ACQUIRE_DATA);
}

void CChildFrame::on_tools_import_data_files()
{
	on_tools_import_files(0);
}

void CChildFrame::on_tools_import_spike_files()
{
	on_tools_import_files(1);
}

void CChildFrame::on_tools_import_database()
{
	CFileDialog dlg_file(TRUE);
	CString file_name;
	constexpr int max_files = 100;
	constexpr int buff_size = (max_files * (MAX_PATH + 1)) + 1;
	dlg_file.GetOFN().lpstrFile = file_name.GetBuffer(buff_size);
	dlg_file.GetOFN().nMaxFile = buff_size;
	dlg_file.GetOFN().lpstrFilter = _T("Database Files\0*.mdb");
	dlg_file.GetOFN().lpstrTitle = _T("Select a database to be merged with current database...");
	constexpr int check_id = 1001;
	HRESULT b_result = dlg_file.AddCheckButton(check_id, L"Copy data files to a new sub-directory", FALSE);
	if (IDOK == dlg_file.DoModal())
	{
		BOOL copy_data_to_new_sub_directory;
		b_result = dlg_file.GetCheckButtonState(check_id, copy_data_to_new_sub_directory);
		TRACE(L"reading the value for check_id %d\n", copy_data_to_new_sub_directory);

		CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
		if (p_db_wave_doc == nullptr)
			return;

		if (p_db_wave_doc->import_data_files_from_another_data_base(file_name, copy_data_to_new_sub_directory))
			boolean result = p_db_wave_doc->db_move_last();
		p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}

void CChildFrame::on_tools_copy_all_project_files()
{
	CString destination_path = _T("c:\\Temp");
	DlgTransferFiles dlg;
	dlg.m_csPathname = destination_path;
	if (IDOK == dlg.DoModal())
	{
		destination_path = dlg.m_csPathname;
		CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
		if (p_db_wave_doc == nullptr)
			return;
		p_db_wave_doc->copy_files_to_directory(destination_path);
	}
}

void CChildFrame::on_tools_export_data_file()
{
	DlgExportData dlg;
	CdbWaveDoc* p_dbWave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_dbWave_doc == nullptr)
		return;
	dlg.m_db_doc = p_dbWave_doc;
	dlg.DoModal();
}

void CChildFrame::on_mdi_activate(BOOL b_activate, CWnd * p_activate_wnd, CWnd * p_deactivate_wnd)
{
	CMDIChildWndEx::OnMDIActivate(b_activate, p_activate_wnd, p_deactivate_wnd);
	const auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (b_activate)
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_MDI_ACTIVATE, NULL);
}

void CChildFrame::on_tools_paths_relative()
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc != nullptr)
	{
		p_db_wave_doc->db_set_paths_relative();
		p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}

void CChildFrame::on_tools_paths_absolute()
{
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc != nullptr)
	{
		p_db_wave_doc->db_set_paths_absolute();
		p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}

void CChildFrame::on_tools_path()
{
	// TODO: Add your command handler code here
}

void CChildFrame::on_tools_remove_unused()
{
	const CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc != nullptr)
	{
		p_db_wave_doc->db_delete_unused_entries();
		AfxMessageBox(_T("Accessory tables cleaned of all un-used entries"));
	}
}

void CChildFrame::on_tools_import()
{
	// TODO: Add your command handler code here
}

void CChildFrame::on_tools_synchro()
{
	// TODO: Add your command handler code here
}

void CChildFrame::on_tools_garbage()
{
	// TODO: Add your command handler code here
}

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

void CChildFrame::on_tools_compact_database()
{
	CString file_name;
	auto p = file_name.GetBuffer(FILE_LIST_BUFFER_SIZE);
	CFileDialog dlg(TRUE);

	auto& ofn = dlg.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;
	ofn.lpstrFilter = _T("MDB Files (*.mdb)\0*.mdb\0\0");
	ofn.lpstrTitle = _T("Select MDB File");
	const auto result = dlg.DoModal();
	file_name.ReleaseBuffer();

	if (result == IDOK)
	{
		file_name = dlg.GetPathName();
		const auto i_pos = file_name.ReverseFind('.');
		const auto file_name_new = file_name.Left(i_pos) + _T("_new.mdb");

		// compact database and save new file
		CdbTable::compact_data_base(file_name, file_name_new);

		const auto cs = file_name + _T(" database compacted and saved as ") + file_name_new;
		AfxMessageBox(cs);
	}
}

void CChildFrame::on_tools_cleanup_filenames()
{
	// Get the active document
	CdbWaveDoc* p_db_wave_doc = CdbWaveDoc::get_active_mdi_document();
	if (p_db_wave_doc == nullptr)
	{
		AfxMessageBox(_T("No database document is currently open."), MB_OK | MB_ICONERROR);
		return;
	}

	BOOL success = FALSE;
	success = CleanupDataFileFilenames(p_db_wave_doc);
	if (success)
	{
		success = CleanupSpikeFileFilenames(p_db_wave_doc);
	}

	if (success)
	{
		// Update all views to reflect changes
		p_db_wave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}
