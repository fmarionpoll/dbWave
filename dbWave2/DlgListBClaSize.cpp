
#include "StdAfx.h"
#include "resource.h"
#include "DlgListBClaSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgListBClaSize::DlgListBClaSize(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgListBClaSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ROWHEIGHT, m_rowheight);
	DDX_Text(pDX, IDC_SUPERPCOL, m_superpcol);
	DDX_Text(pDX, IDC_TEXTCOL, m_textcol);
	DDX_Text(pDX, IDC_INTERCOLSPACE, m_intercolspace);
}

BEGIN_MESSAGE_MAP(DlgListBClaSize, CDialog)

END_MESSAGE_MAP()


