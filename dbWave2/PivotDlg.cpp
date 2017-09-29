// PivotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "resource.h"
#include "GridCtrl\GridCtrl.h"
#include "GridCtrl\GridCell.h"
#include "GridCtrl\GridCellCombo_FMP.h"
#include "GridCtrl\GridCellNumeric.h"
#include "GridCtrl\GridCellCheck.h"
#include "afxdialogex.h"
#include "PivotDlg.h"

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
	return (c1<c2); // strange order, whatever the column number...
}
BOOL CALLBACK EnumProc2(HWND hwnd, LPARAM lParam)
{
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	CSize* pTranslate = (CSize*) lParam;

	CPivotDlg* pDlg = (CPivotDlg*) pWnd->GetParent();
	if (!pDlg) return FALSE;

	CRect rect;
	pWnd->GetWindowRect(rect);

	pDlg->ScreenToClient(rect);
	if (hwnd == pDlg->m_Grid.GetSafeHwnd())
	{
		if (  ((rect.top >= 7 && pTranslate->cy > 0) || rect.Height() > 20) &&
			  ((rect.left >= 7 && pTranslate->cx > 0) || rect.Width() > 20)   )
			pWnd->MoveWindow(rect.left, rect.top, 
							 rect.Width()+pTranslate->cx, 
							 rect.Height()+pTranslate->cy, FALSE);
		else
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top+pTranslate->cy, 
							 rect.Width(), rect.Height(), FALSE);
	}

	else 
	{
		if (pWnd->GetDlgCtrlID() == IDC_SIZEBOX)
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top+pTranslate->cy, 
							 rect.Width(), rect.Height(), FALSE);
		else
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top, rect.Width(), rect.Height(), FALSE);
	}
	pDlg->Invalidate();

	return TRUE;
}


void CPivotDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (cx <= 1 || cy <= 1 ) 
		return;

	CSize Translate(cx - m_OldSize.cx, cy - m_OldSize.cy);
	::EnumChildWindows(GetSafeHwnd(), EnumProc2, (LPARAM)&Translate);
	m_OldSize = CSize(cx,cy);
	CWnd *pWnd = GetDlgItem(IDC_SIZEBOX);
	if (pWnd)
		pWnd->ShowWindow( (nType == SIZE_MAXIMIZED)? SW_HIDE : SW_SHOW);
}
