#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif
#pragma warning (disable : 4995)
#include <afxdao.h>

#include "dbMultiDocTemplate.h"
#include "OPTIONS_ACQDATA.h"
#include "OPTIONS_VIEWDATAMEASURE.h"
#include "OPTIONS_IMPORT.h"
#include "OPTIONS_OUTPUTDATA.h"
#include "OPTIONS_VIEWDATA.h"
#include "OPTIONS_VIEWSPIKES.h"
#include "SPKCLASSIF.h"
#include "SPKDETECTARRAY.h"
#include "STIMDETECT.h"


class CdbWaveApp final : public CWinAppEx
{
public:
	BOOL PreTranslateMessage(MSG* pMsg) override;
	CdbWaveApp();

	CdbMultiDocTemplate* m_pdbWaveViewTemplate = nullptr;
	CdbMultiDocTemplate* m_pdbBrowseViewTemplate = nullptr;
	CdbMultiDocTemplate* m_pNoteViewTemplate = nullptr;
	CdbMultiDocTemplate* m_pdataViewTemplate = nullptr;
	CdbMultiDocTemplate* m_pspikeViewTemplate = nullptr;
	CdbMultiDocTemplate* m_pADViewTemplate = nullptr;

	HMENU m_hDBView{};
	HMENU m_hDataView{};
	HMENU m_hSpikeView{};
	HMENU m_hBrowseView{};
	HMENU m_hAcqView{};

	// Overrides
public:
	BOOL InitInstance() override;
	int ExitInstance() override;

	// Implementation
	UINT m_nAppLook{};
	BOOL m_bHiColorIcons = TRUE;

	void PreLoadState() override;
	void LoadCustomState() override;
	void SaveCustomState() override;

	CStringArray m_csParmFiles;
	CString m_comment;
	STIMDETECT stim_detect;
	SPKDETECTARRAY spk_detect_array;
	SPKCLASSIF spk_classif;

	OPTIONS_VIEWDATA options_viewdata;
	OPTIONS_VIEWSPIKES options_viewspikes;
	OPTIONS_VIEWDATAMEASURE options_viewdata_measure;
	OPTIONS_IMPORT options_import;
	OPTIONS_ACQDATA options_acqdata;
	OPTIONS_OUTPUTDATA options_outputdata;

	CMemFile* m_pviewdataMemFile = nullptr;
	CArray<CMemFile*, CMemFile*> viewspikesmemfile_ptr_array;
	CMemFile* m_pviewspikesMemFile = nullptr;
	CMemFile* m_psort1spikesMemFile = nullptr;
	CStringArray m_tempMDBfiles;
	CSharedFile* m_psf = nullptr;
	BOOL m_bADcardFound = TRUE;

	void Defaultparameters(BOOL b_read);
	BOOL archive_parameter_files(const CString& filename, BOOL b_read);
	void serialize_parameters(int n, CArchive& ar);
	void SetPrinterOrientation();
	BOOL GetFilenamesDlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames);
	void FilePrintSetup();
	CString Get_MyDocuments_MydbWavePath();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();

	DECLARE_MESSAGE_MAP()
};

extern CdbWaveApp theApp;

void DisplayDaoException(CDaoException* e, int iID);
