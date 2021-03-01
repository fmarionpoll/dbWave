#include "stdafx.h"
#include "dbWaveDoc.h"
#include "CViewdbWaveRecord.h"




IMPLEMENT_DYNCREATE(CViewdbWaveRecord, CDaoRecordView)

CViewdbWaveRecord::CViewdbWaveRecord()
{

}

CViewdbWaveRecord::~CViewdbWaveRecord()
{
}

BEGIN_MESSAGE_MAP(CViewdbWaveRecord, CDaoRecordView)
END_MESSAGE_MAP()


// CdaoView drawing

void CViewdbWaveRecord::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//CDaoRecordset* CViewdbWaveRecord::OnGetRecordset()
//{
//	return GetDocument()->GetDB_Recordset();
//}

// CdaoView diagnostics

#ifdef _DEBUG
void CViewdbWaveRecord::AssertValid() const
{
	CDaoRecordView::AssertValid();
}


CdbWaveDoc* CViewdbWaveRecord::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}

#ifndef _WIN32_WCE
void CViewdbWaveRecord::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}
#endif
#endif //_DEBUG

BOOL CViewdbWaveRecord::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

