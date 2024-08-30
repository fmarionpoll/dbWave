
#include "StdAfx.h"
#include "resource.h"
#include "DlgListBClaSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgListBClaSize::DlgListBClaSize(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgListBClaSize::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_ROWHEIGHT, m_row_height);
	DDX_Text(p_dx, IDC_SUPERPCOL, m_super_p_col);
	DDX_Text(p_dx, IDC_TEXTCOL, m_text_col);
	DDX_Text(p_dx, IDC_INTERCOLSPACE, m_inter_col_space);
}

BEGIN_MESSAGE_MAP(DlgListBClaSize, CDialog)

END_MESSAGE_MAP()


