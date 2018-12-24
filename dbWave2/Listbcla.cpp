// listbcla.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "resource.h"
#include "Listbcla.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CListBClaSizeDlg dialog


CListBClaSizeDlg::CListBClaSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListBClaSizeDlg::IDD, pParent)
{
	m_rowheight = 0;
	m_superpcol = 0;
	m_textcol = 0;
	m_intercolspace = 0;
}


void CListBClaSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ROWHEIGHT, m_rowheight);
	DDX_Text(pDX, IDC_SUPERPCOL, m_superpcol);
	DDX_Text(pDX, IDC_TEXTCOL, m_textcol);
	DDX_Text(pDX, IDC_INTERCOLSPACE, m_intercolspace);
}

BEGIN_MESSAGE_MAP(CListBClaSizeDlg, CDialog)
	
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CListBClaSizeDlg message handlers

