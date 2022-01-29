#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "dbMultiDocTemplate.h"
#include "OPTIONS_ACQDATA.h"
#include "OPTIONS_VIEWDATAMEASURE.h"
#include "OPTIONS_IMPORT.h"
#include "OPTIONS_OUTPUTDATA.h"
#include "OPTIONS_VIEWDATA.h"
#include "OPTIONS_VIEWSPIKES.h"
#include "spikepar.h"

class CdbWaveApp : public CWinAppEx
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CdbWaveApp();

	CdbMultiDocTemplate* m_pdbWaveViewTemplate;
	CdbMultiDocTemplate* m_pdbBrowseViewTemplate{};
	CdbMultiDocTemplate* m_pNoteViewTemplate{};
	CdbMultiDocTemplate* m_pdataViewTemplate{};
	CdbMultiDocTemplate* m_pspikeViewTemplate{};
	CdbMultiDocTemplate* m_pADViewTemplate{};

	HMENU	m_hDBView{};
	HMENU	m_hDataView{};
	HMENU	m_hSpikeView{};
	HMENU	m_hBrowseView{};
	HMENU	m_hAcqView{};

	// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// Implementation
	UINT  m_nAppLook{};
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	CStringArray			m_csParmFiles;
	CString					m_comment;
	STIMDETECT 				stiD;
	SPKDETECTARRAY			spkDA;
	SPKCLASSIF				spkC;

	OPTIONS_VIEWDATA 		options_viewdata;
	OPTIONS_VIEWSPIKES 		options_viewspikes;
	OPTIONS_VIEWDATAMEASURE	options_viewdata_measure;
	OPTIONS_IMPORT			options_import;
	OPTIONS_ACQDATA			options_acqdata;
	OPTIONS_OUTPUTDATA		options_outputdata;

	CMemFile* m_pviewdataMemFile;
	CArray <CMemFile*, CMemFile*> viewspikesmemfile_ptr_array;
	CMemFile* m_pviewspikesMemFile;
	CMemFile* m_psort1spikesMemFile;
	CStringArray			m_tempMDBfiles;
	CSharedFile* m_psf{};
	BOOL					m_bADcardFound;

	void	Defaultparameters(BOOL b_read);
	BOOL	ParmFile(CString& filename, BOOL b_read);
	void	SetPrinterOrientation();
	BOOL	GetFilenamesDlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames);
	void	FilePrintSetup();
	CString Get_MyDocuments_MydbWavePath();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();

	DECLARE_MESSAGE_MAP()
};

extern CdbWaveApp theApp;

void DisplayDaoException(CDaoException* e, int iID);
