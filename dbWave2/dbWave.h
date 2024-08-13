#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif
#pragma warning (disable : 4995)
#include <afxdao.h>

#include "dbMultiDocTemplate.h"
#include "options_input.h"
#include "OPTIONS_MEASURE.h"
#include "options_import.h"
#include "options_output.h"
#include "options_view_data.h"
#include "options_view_spikes.h"
#include "options_spk_classification.h"
#include "spike_detection_array.h"
#include "options_detect_stimulus.h"


class CdbWaveApp final : public CWinAppEx
{
public:
	CdbWaveApp() noexcept;
	BOOL InitInstance() override;
	int ExitInstance() override;
	void PreLoadState() override;
	void LoadCustomState() override;
	void SaveCustomState() override;

	UINT app_look{};
	BOOL hi_color_icons = TRUE;

	BOOL PreTranslateMessage(MSG* pMsg) override;

	CdbMultiDocTemplate* m_dbWaveView_Template = nullptr;
	CdbMultiDocTemplate* m_dbBrowseView_Template = nullptr;
	CdbMultiDocTemplate* m_NoteView_Template = nullptr;
	CdbMultiDocTemplate* m_dataView_Template = nullptr;
	CdbMultiDocTemplate* m_spikeView_Template = nullptr;
	CdbMultiDocTemplate* m_ADView_Template = nullptr;

	HMENU m_hDBView{};
	HMENU m_hDataView{};
	HMENU m_hSpikeView{};
	HMENU m_hBrowseView{};
	HMENU m_hAcqView{};

	// Implementation
	CStringArray m_cs_parameter_files;
	CString m_comment;
	options_detect_stimulus stimulus_detect;
	spike_detection_array spk_detect_array;
	options_spk_classification spk_classification;

	options_view_data options_view_data;
	options_view_spikes options_view_spikes;
	options_measure options_view_data_measure;
	options_import options_import;
	options_input options_acq_data;
	options_output options_output_data;

	CMemFile* m_p_view_data_memory_file = nullptr;
	CArray<CMemFile*, CMemFile*> view_spikes_memory_file_ptr_array;
	CMemFile* m_p_view_spikes_memory_file = nullptr;
	CMemFile* m_p_sort1_spikes_memory_file = nullptr;
	CStringArray m_temporary_mdb_files;
	CSharedFile* m_psf = nullptr;
	BOOL m_ad_card_found = TRUE;

	void default_parameters(BOOL b_read);
	BOOL archive_parameter_files(const CString& filename, BOOL b_read);
	void serialize_parameters(int n, CArchive& ar);
	void SetPrinterOrientation();
	BOOL get_file_names_dlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames);
	void FilePrintSetup();
	CString get_my_documents_my_dbwave_path();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();

	DECLARE_MESSAGE_MAP()
};

extern CdbWaveApp the_app;

void DisplayDaoException(CDaoException* e, int i_id);
