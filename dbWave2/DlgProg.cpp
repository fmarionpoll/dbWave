// CG: This file was added by the Progress Dialog component

#include "StdAfx.h"
#include "resource.h"
#include "DlgProg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDlgProgress::CDlgProgress(UINT nCaptionID)
{
	m_nCaptionID = IDP_PROGRESS_CAPTION;
	if (nCaptionID != 0)
		m_nCaptionID = nCaptionID;
	m_bCancel = FALSE;
	m_nLower = 0;
	m_nUpper = 100;
	m_nStep = 10;
	m_bParentDisabled = FALSE;
}

CDlgProgress::~CDlgProgress()
{
	if (m_hWnd != nullptr)
		DestroyWindow();
}

BOOL CDlgProgress::DestroyWindow()
{
	ReEnableParent();
	return CDialog::DestroyWindow();
}

void CDlgProgress::ReEnableParent()
{
	if (m_bParentDisabled && (m_pParentWnd != nullptr))
		m_pParentWnd->EnableWindow(TRUE);
	m_bParentDisabled = FALSE;
}

BOOL CDlgProgress::Create(CWnd* pParent)
{
	// Get the true parent of the dialog
	m_pParentWnd = CWnd::GetSafeOwner(pParent);

	// m_bParentDisabled is used to re-enable the parent window
	// when the dialog is destroyed. So we don't want to set
	// it to TRUE unless the parent was already enabled.

	if ((m_pParentWnd != nullptr) && m_pParentWnd->IsWindowEnabled())
	{
		m_pParentWnd->EnableWindow(FALSE);
		m_bParentDisabled = TRUE;
	}

	if (!CDialog::Create(CDlgProgress::IDD, pParent))
	{
		ReEnableParent();
		return FALSE;
	}

	return TRUE;
}

void CDlgProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, CG_IDC_PROGDLG_PROGRESS, m_Progress);
}

BEGIN_MESSAGE_MAP(CDlgProgress, CDialog)

END_MESSAGE_MAP()

void CDlgProgress::SetStatus(LPCTSTR lpszMessage)
{
	ASSERT(m_hWnd); // Don't call this _before_ the dialog has
					// been created. Can be called from OnInitDialog
	auto p_wnd_status = GetDlgItem(CG_IDC_PROGDLG_STATUS);

	// Verify that the static text control exists
	ASSERT(p_wnd_status != NULL);
	p_wnd_status->SetWindowText(lpszMessage);
}

void CDlgProgress::OnCancel()
{
	m_bCancel = TRUE;
}

void CDlgProgress::SetRange(int nLower, int nUpper)
{
	m_nLower = nLower;
	m_nUpper = nUpper;
	m_Progress.SetRange(nLower, nUpper);
}

int CDlgProgress::SetPos(int nPos)
{
	PumpMessages();
	const auto i_result = m_Progress.SetPos(nPos);
	UpdatePercent(nPos);
	return i_result;
}

int CDlgProgress::SetStep(int nStep)
{
	m_nStep = nStep; // Store for later use in calculating percentage
	return m_Progress.SetStep(nStep);
}

int CDlgProgress::OffsetPos(int nPos)
{
	PumpMessages();
	const auto i_result = m_Progress.OffsetPos(nPos);
	UpdatePercent(i_result + nPos);
	return i_result;
}

int CDlgProgress::StepIt()
{
	PumpMessages();
	const auto i_result = m_Progress.StepIt();
	UpdatePercent(i_result + m_nStep);
	return i_result;
}

void CDlgProgress::PumpMessages()
{
	// Must call Create() before using the dialog
	ASSERT(m_hWnd != NULL);

	MSG msg;
	// Handle dialog messages
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

BOOL CDlgProgress::CheckCancelButton()
{
	// Process all pending messages
	PumpMessages();

	// Reset m_bCancel to FALSE so that
	// CheckCancelButton returns FALSE until the user
	// clicks Cancel again. This will allow you to call
	// CheckCancelButton and still continue the operation.
	// If m_bCancel stayed TRUE, then the next call to
	// CheckCancelButton would always return TRUE

	const auto b_result = m_bCancel;
	m_bCancel = FALSE;

	return b_result;
}

void CDlgProgress::UpdatePercent(int nNewPos)
{
	auto p_wnd_percent = GetDlgItem(CG_IDC_PROGDLG_PERCENT);

	const auto n_divisor = m_nUpper - m_nLower;
	ASSERT(n_divisor > 0);  // m_nLower should be smaller than m_nUpper

	const auto n_dividend = (nNewPos - m_nLower);
	ASSERT(n_dividend >= 0);   // Current position should be greater than m_nLower

	auto n_percent = n_dividend * 100 / n_divisor;

	// Since the Progress Control wraps, we will wrap the percentage
	// along with it. However, don't reset 100% back to 0%
	if (n_percent != 100)
		n_percent %= 100;

	// Display the percentage
	CString str_buf;
	str_buf.Format(_T("%d%c"), n_percent, _T('%'));

	CString str_cur; // get current percentage
	p_wnd_percent->GetWindowText(str_cur);

	if (str_cur != str_buf)
		p_wnd_percent->SetWindowText(str_buf);
}

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CDlgProgress::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_Progress.SetRange(m_nLower, m_nUpper);
	m_Progress.SetStep(m_nStep);
	m_Progress.SetPos(m_nLower);

	CString str_caption;
	VERIFY(str_caption.LoadString(m_nCaptionID));
	SetWindowText(str_caption);

	return TRUE;
}