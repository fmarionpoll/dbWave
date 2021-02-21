// PivotDlg.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "resource.h"
//#include "GridCtrl\GridCtrl.h"
//#include "GridCtrl\GridCell.h"
//#include "GridCtrl\GridCellCombo_FMP.h"
//#include "GridCtrl\GridCellNumeric.h"
//#include "GridCtrl\GridCellCheck.h"
#include "afxdialogex.h"
#include "DlgPivot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPivotDlg dialog

IMPLEMENT_DYNAMIC(CPivotDlg, CDialogEx)

CPivotDlg::CPivotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPivotDlg::IDD, pParent)
{
	m_nFixCols = 1;
	m_nFixRows = 1;
	m_nCols = 3;
	m_nRows = 26;
}

CPivotDlg::~CPivotDlg()
{
}

void CPivotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_Grid);
}

BEGIN_MESSAGE_MAP(CPivotDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPivotDlg::OnBnClickedOk)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CPivotDlg message handlers

BOOL CPivotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPivotDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

bool CPivotDlg::VirtualCompare(int c1, int c2)
{
	//CGridCtrl *pThis = CGridCtrl::m_This; // Mmm, in big virtual mode you must well optimize this function
	//int col = pThis->m_CurCol;            // a first version with CStrings was catastrophic....
	return (c1 < c2); // strange order, whatever the column number...
}
BOOL CALLBACK EnumProc2(HWND hwnd, LPARAM lParam)
{
	auto p_wnd = CWnd::FromHandle(hwnd);
	const auto p_translate = reinterpret_cast<CSize*>(lParam);

	auto p_dlg = (CPivotDlg*)p_wnd->GetParent();
	if (!p_dlg) return FALSE;

	CRect rect;
	p_wnd->GetWindowRect(rect);

	p_dlg->ScreenToClient(rect);
	if (hwnd == p_dlg->m_Grid.GetSafeHwnd())
	{
		if (((rect.top >= 7 && p_translate->cy > 0) || rect.Height() > 20) &&
			((rect.left >= 7 && p_translate->cx > 0) || rect.Width() > 20))
			p_wnd->MoveWindow(rect.left, rect.top,
				rect.Width() + p_translate->cx,
				rect.Height() + p_translate->cy, FALSE);
		else
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy,
				rect.Width(), rect.Height(), FALSE);
	}

	else
	{
		if (p_wnd->GetDlgCtrlID() == IDC_SIZEBOX)
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy,
				rect.Width(), rect.Height(), FALSE);
		else
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top, rect.Width(), rect.Height(), FALSE);
	}
	p_dlg->Invalidate();

	return TRUE;
}

void CPivotDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (cx <= 1 || cy <= 1)
		return;

	CSize translate(cx - m_OldSize.cx, cy - m_OldSize.cy);
	::EnumChildWindows(GetSafeHwnd(), EnumProc2, reinterpret_cast<LPARAM>(&translate));
	m_OldSize = CSize(cx, cy);
	auto p_wnd = GetDlgItem(IDC_SIZEBOX);
	if (p_wnd)
		p_wnd->ShowWindow((nType == SIZE_MAXIMIZED) ? SW_HIDE : SW_SHOW);
}