#include "StdAfx.h"
#include "dbWave.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CDlgEditSpikeClass, CDialog)

CDlgEditSpikeClass::CDlgEditSpikeClass(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
	  , m_iClass(0)
{
}

CDlgEditSpikeClass::~CDlgEditSpikeClass()
{
}

void CDlgEditSpikeClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_iClass);
}

BEGIN_MESSAGE_MAP(CDlgEditSpikeClass, CDialog)
END_MESSAGE_MAP()
