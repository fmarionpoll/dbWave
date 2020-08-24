#include "StdAfx.h"
//#include "dbWave.h"
#include "EditDAMseqDlg.h"
#include "afxdialogex.h"

// CEditDAMseqDlg dialog

IMPLEMENT_DYNAMIC(CEditDAMseqDlg, CDialogEx)

CEditDAMseqDlg::CEditDAMseqDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditDAMseqDlg::IDD, pParent)

	, m_mSeqRatio(0)
	, m_mSeqDelay(0)
	, m_mSeqSeed(0)
{
	m_pParent = pParent;
}

CEditDAMseqDlg::~CEditDAMseqDlg()
{
}

void CEditDAMseqDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MSEQ_RATIO, m_mSeqRatio);
	DDX_Text(pDX, IDC_MSEQ_DELAY, m_mSeqDelay);
	DDX_Text(pDX, IDC_MSEQ_SEED, m_mSeqSeed);
	DDX_Control(pDX, IDC_MSEQ_RANDOM_SEED, m_mSeqRandomSeed);
}

BEGIN_MESSAGE_MAP(CEditDAMseqDlg, CDialogEx)

	ON_BN_CLICKED(IDC_MSEQ_RANDOM_SEED, &CEditDAMseqDlg::OnBnClickedMseqRandomSeed)

END_MESSAGE_MAP()

// CEditDAMseqDlg message handlers

BOOL CEditDAMseqDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_mSeqRatio = m_outDParms.mseq_iRatio;
	m_mSeqDelay = m_outDParms.mseq_iDelay;
	m_mSeqSeed = m_outDParms.mseq_iSeed;

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDAMseqDlg::OnOK()
{
	UpdateData(TRUE);

	m_outDParms.mseq_iRatio = m_mSeqRatio;
	m_outDParms.mseq_iDelay = m_mSeqDelay;
	m_outDParms.mseq_iSeed = m_mSeqSeed;

	CDialogEx::OnOK();
}

void CEditDAMseqDlg::OnBnClickedMseqRandomSeed()
{
	UpdateData(TRUE);

	auto the_time = CTime::GetCurrentTime();
	const UINT seed = ((the_time.GetSecond() << 8) + (the_time.GetMinute() << 4) + (the_time.GetHour())) * 10000000 & 0x7fffffff;
	m_mSeqSeed = seed;
	UpdateData(FALSE);
}