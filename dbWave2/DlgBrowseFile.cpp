
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
		m_allchannels = mfBR->b_all_channels;
		m_centercurves = mfBR->b_center_curves;
		m_completefile = mfBR->b_complete_record;
		m_splitcurves = mfBR->b_split_curves;
		m_maximizegain = mfBR->b_maximize_gain;
		m_multirowdisplay = mfBR->b_multiple_rows;
		m_keepforeachfile = mfBR->b_keep_for_each_file;
	}
	UpdateData(FALSE);
	return TRUE; 
}

void DlgBrowseFile::OnOK()
{
	UpdateData(TRUE);
	mfBR->b_all_channels = m_allchannels;
	mfBR->b_center_curves = m_centercurves;
	mfBR->b_complete_record = m_completefile;
	mfBR->b_split_curves = m_splitcurves;
	mfBR->b_maximize_gain = m_maximizegain;
	mfBR->b_multiple_rows = m_multirowdisplay;
	mfBR->b_keep_for_each_file = m_keepforeachfile;
	mfBR->b_changed = TRUE;
	CDialog::OnOK();
}
