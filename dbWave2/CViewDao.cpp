#include "stdafx.h"
#include "dbWaveDoc.h"
#include "CViewDao.h"



IMPLEMENT_DYNAMIC(CViewDAO, CDaoRecordView)

CViewDAO::CViewDAO(LPCTSTR lpszTemplateName)
	: CDaoRecordView(lpszTemplateName)
{
	m_bEnableActiveAccessibility = FALSE;
}

CViewDAO::CViewDAO(UINT nIDTemplate)
	: CDaoRecordView(nIDTemplate)
{
	m_bEnableActiveAccessibility = FALSE;
}

CViewDAO::~CViewDAO()
{
}

BEGIN_MESSAGE_MAP(CViewDAO, CDaoRecordView)
END_MESSAGE_MAP()


// CdaoView drawing
void CViewDAO::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//CDaoRecordset* CViewDAO::OnGetRecordset()
//{
//	return GetDocument()->GetDB_Recordset();
//}
// CdaoView diagnostics
#ifdef _DEBUG
void CViewDAO::AssertValid() const
{
	CDaoRecordView::AssertValid();
}


CdbWaveDoc* CViewDAO::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}

#ifndef _WIN32_WCE
void CViewDAO::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}
#endif
#endif //_DEBUG

BOOL CViewDAO::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

CDaoRecordset* CViewDAO::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

void CViewDAO::OnSize(UINT nType, int cx, int cy)
{
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

BOOL CViewDAO::OnMove(UINT nIDMoveCommand)
{
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewDAO::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
	CDaoRecordView::OnDestroy();
}

BOOL CViewDAO::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	return TRUE;
}

void CViewDAO::OnBeginPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CViewDAO::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CViewDAO::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if (pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(p_dc, pInfo);
}

