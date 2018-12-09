// NotedocView.cpp : implementation of the CNoteDocView class
//

#include "StdAfx.h"
#include "resource.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoteDocView

IMPLEMENT_DYNCREATE(CNoteDocView, CRichEditView)

BEGIN_MESSAGE_MAP(CNoteDocView, CRichEditView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_TOOLS_OPENDATAFILES, OnToolsOpendatafiles)
	ON_COMMAND(ID_FILE_PRINT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CRichEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoteDocView construction/destruction

CNoteDocView::CNoteDocView()
{
	// TODO: add construction code here
	m_bEnableActiveAccessibility=FALSE;
}

CNoteDocView::~CNoteDocView()
{
}

BOOL CNoteDocView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CRichEditView::PreCreateWindow(cs);
}

void CNoteDocView::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();

	// Set the printing margins (720 twips = 1/2 inch).
	SetMargins(CRect(720, 720, 720, 720));
}

/////////////////////////////////////////////////////////////////////////////
// CNoteDocView printing

BOOL CNoteDocView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}


void CNoteDocView::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
   CRichEditView::OnDestroy();

   /*
   // the following lines throw an exception on exit
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
   {
	  pActiveItem->Deactivate();
	  ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
   }*/
}


/////////////////////////////////////////////////////////////////////////////
// CNoteDocView diagnostics

#ifdef _DEBUG
void CNoteDocView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CNoteDocView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}

CNoteDoc* CNoteDocView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNoteDoc)));
	return (CNoteDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNoteDocView message handlers

void CNoteDocView::OnToolsOpendatafiles() 
{
	CNoteDoc* pDoc = (CNoteDoc*) GetDocument();
	CString csname = pDoc->GetPathName();
	pDoc->OpenProjectFiles(csname);
}
