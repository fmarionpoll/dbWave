// browsefi.cpp : implementation file
//

#include "StdAfx.h"
#include "Browsefi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrowseFileDlg dialog


CBrowseFileDlg::CBrowseFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBrowseFileDlg::IDD, pParent)
{
	m_allchannels = FALSE;
	m_centercurves = FALSE;
	m_completefile = FALSE;
	m_maximizegain = FALSE;
	m_splitcurves = FALSE;
	m_multirowdisplay = FALSE;
	m_keepforeachfile = FALSE;
	mfBR = nullptr;
}

void CBrowseFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ALLCHANNELS, m_allchannels);
	DDX_Check(pDX, IDC_CENTERCURVES, m_centercurves);
	DDX_Check(pDX, IDC_COMPLETEFILE, m_completefile);
	DDX_Check(pDX, IDC_MAXIMIZEGAIN, m_maximizegain);
	DDX_Check(pDX, IDC_SPLITCURVES, m_splitcurves);
	DDX_Check(pDX, IDC_MULTILINEDISPLAY, m_multirowdisplay);
	DDX_Check(pDX, IDC_KEEPFOREACHFILE, m_keepforeachfile);
}

BEGIN_MESSAGE_MAP(CBrowseFileDlg, CDialog)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBrowseFileDlg message handlers

BOOL CBrowseFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (mfBR != nullptr) {
		m_allchannels = mfBR->bAllChannels;
		m_centercurves = mfBR->bCenterCurves;
		m_completefile = mfBR->bEntireRecord;
		m_splitcurves = mfBR->bSplitCurves;
		m_maximizegain = mfBR->bMaximizeGain;
		m_multirowdisplay = mfBR->bMultirowDisplay;
		m_keepforeachfile = mfBR->bKeepForEachFile;
	}
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBrowseFileDlg::OnOK()
{   
	UpdateData(TRUE);
	mfBR->bAllChannels = m_allchannels;
	mfBR->bCenterCurves = m_centercurves;
	mfBR->bEntireRecord = m_completefile;
	mfBR->bSplitCurves = m_splitcurves;
	mfBR->bMaximizeGain = m_maximizegain;	
	mfBR->bMultirowDisplay = m_multirowdisplay;
	mfBR->bKeepForEachFile = m_keepforeachfile;
	mfBR->bChanged = TRUE;
	CDialog::OnOK();
}
