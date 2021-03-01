// CdaoView.cpp : implementation file
//

#include "pch.h"
#include "CdaoView.h"


// CdaoView

IMPLEMENT_DYNCREATE(CdaoView, CView)

CdaoView::CdaoView()
{

}

CdaoView::~CdaoView()
{
}

BEGIN_MESSAGE_MAP(CdaoView, CView)
END_MESSAGE_MAP()


// CdaoView drawing

void CdaoView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CdaoView diagnostics

#ifdef _DEBUG
void CdaoView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CdaoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CdaoView message handlers
