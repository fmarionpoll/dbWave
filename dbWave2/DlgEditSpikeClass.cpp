#include "StdAfx.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgEditSpikeClass, CDialog)

DlgEditSpikeClass::DlgEditSpikeClass(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

DlgEditSpikeClass::~DlgEditSpikeClass()
= default;

void DlgEditSpikeClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_iClass);
}

BEGIN_MESSAGE_MAP(DlgEditSpikeClass, CDialog)
END_MESSAGE_MAP()
