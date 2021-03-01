#include "StdAfx.h"
#include "resource.h"
#include "NoteDoc.h"
#include "CViewNotedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewNoteDoc, CRichEditView)

BEGIN_MESSAGE_MAP(CViewNoteDoc, CRichEditView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_TOOLS_OPENDATAFILES, OnToolsOpendatafiles)
	ON_COMMAND(ID_FILE_PRINT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CRichEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

CViewNoteDoc::CViewNoteDoc()
{
	// TODO: add construction code here
	m_bEnableActiveAccessibility = FALSE;
}

CViewNoteDoc::~CViewNoteDoc()
{
}

BOOL CViewNoteDoc::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CRichEditView::PreCreateWindow(cs);
}

void CViewNoteDoc::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();

	// Set the printing margins (720 twips = 1/2 inch).
	SetMargins(CRect(720, 720, 720, 720));
}

BOOL CViewNoteDoc::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CViewNoteDoc::OnDestroy()
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

#ifdef _DEBUG
void CViewNoteDoc::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CViewNoteDoc::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}

CNoteDoc* CViewNoteDoc::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNoteDoc)));
	return (CNoteDoc*)m_pDocument;
}
#endif //_DEBUG

void CViewNoteDoc::OnToolsOpendatafiles()
{
	CNoteDoc* p_document = (CNoteDoc*)GetDocument();
	CString csname = p_document->GetPathName();
	p_document->OpenProjectFiles(csname);
}