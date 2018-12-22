#pragma once
// dbWave.h : main header file for the DBWAVE application

/////////////////////////////////////////////////////////////////////////////
// CdbWaveApp:
// See dbWave.cpp for the implementation of this class
//

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

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
	OPTIONS_VIEWDATA 		vdP;
	OPTIONS_VIEWSPIKES 		vdS;
	OPTIONS_VIEWDATAMEASURE	vdM;
	OPTIONS_IMPORT			ivO;
	OPTIONS_ACQDATA			acqD;
	OPTIONS_OUTPUTDATA		outD;

	CMemFile*				m_pviewdataMemFile;
	CArray <CMemFile*, CMemFile*> viewspikesmemfile_ptr_array;
	CMemFile*				m_pviewspikesMemFile;
	CMemFile*				m_psort1spikesMemFile;
	CStringArray			m_tempMDBfiles;
	CSharedFile*			m_psf{};
	BOOL					m_bADcardFound;

	void	Defaultparameters(BOOL b_read);
	BOOL	ParmFile(CString& filename, BOOL b_read);
	void	SetPrinterOrientation();
	BOOL	GetFilenamesDlg(int iIDS, LPCSTR szTitle, int* iFilterIndex, CStringArray* filenames);
	void	FilePrintSetup();
	CString Get_MyDocuments_MydbWavePath();
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
};

extern CdbWaveApp theApp;
void DisplayDaoException(CDaoException* e, int iID);
