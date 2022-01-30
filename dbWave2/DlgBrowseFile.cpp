
#include "StdAfx.h"
#include "DlgBrowseFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgBrowseFile::DlgBrowseFile(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgBrowseFile::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(DlgBrowseFile, CDialog)

END_MESSAGE_MAP()

BOOL DlgBrowseFile::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (mfBR != nullptr)
	{
		m_allchannels = mfBR->bAllChannels;
		m_centercurves = mfBR->bCenterCurves;
		m_completefile = mfBR->bEntireRecord;
		m_splitcurves = mfBR->bSplitCurves;
		m_maximizegain = mfBR->bMaximizeGain;
		m_multirowdisplay = mfBR->bMultirowDisplay;
		m_keepforeachfile = mfBR->bKeepForEachFile;
	}
	UpdateData(FALSE);
	return TRUE; 
}

void DlgBrowseFile::OnOK()
{
	UpdateData(TRUE);
	mfBR->bAllChannels = m_allchannels;
	mfBR->bCenterCurves = m_centercurves;
	mfBR->bEntireRecord = m_completefile;
	mfBR->bSplitCurves = m_splitcurves;
	mfBR->bMaximizeGain = m_maximizegain;
	mfBR->bMultirowDisplay = m_multirowdisplay;
	mfBR->bKeepForEachFile = m_keepforeachfile;
	mfBR->b_changed = TRUE;
	CDialog::OnOK();
}
