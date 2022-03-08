#include "StdAfx.h"
#include "resource.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"

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
	m_bEnableActiveAccessibility = FALSE;
}

CViewNoteDoc::~CViewNoteDoc()
{
}

BOOL CViewNoteDoc::PreCreateWindow(CREATESTRUCT& cs)
{
	return CRichEditView::PreCreateWindow(cs);
}

void CViewNoteDoc::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();
	SetMargins(CRect(720, 720, 720, 720));
}

BOOL CViewNoteDoc::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CViewNoteDoc::OnDestroy()
{
	CRichEditView::OnDestroy();
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
	return static_cast<CNoteDoc*>(m_pDocument);
}
#endif //_DEBUG

void CViewNoteDoc::OnToolsOpendatafiles()
{
	auto p_document = GetDocument();
	CString csname = p_document->GetPathName();
	p_document->OpenProjectFiles(csname);
}
