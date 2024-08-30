#include "StdAfx.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgEditSpikeClass, CDialog)

DlgEditSpikeClass::DlgEditSpikeClass(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgEditSpikeClass::~DlgEditSpikeClass()
= default;

void DlgEditSpikeClass::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT1, m_i_class);
}

BEGIN_MESSAGE_MAP(DlgEditSpikeClass, CDialog)
END_MESSAGE_MAP()
