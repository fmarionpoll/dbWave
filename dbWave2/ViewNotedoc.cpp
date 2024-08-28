#include "StdAfx.h"
#include "resource.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewNoteDoc, CRichEditView)

BEGIN_MESSAGE_MAP(ViewNoteDoc, CRichEditView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_TOOLS_OPEN_DATA_FILES, on_tools_open_data_files)
	ON_COMMAND(ID_FILE_PRINT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CRichEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

ViewNoteDoc::ViewNoteDoc()
{
	m_bEnableActiveAccessibility = FALSE;
}

ViewNoteDoc::~ViewNoteDoc()
{
}

BOOL ViewNoteDoc::PreCreateWindow(CREATESTRUCT& cs)
{
	return CRichEditView::PreCreateWindow(cs);
}

void ViewNoteDoc::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();
	SetMargins(CRect(720, 720, 720, 720));
}

BOOL ViewNoteDoc::OnPreparePrinting(CPrintInfo* p_info)
{
	return DoPreparePrinting(p_info);
}

void ViewNoteDoc::OnDestroy()
{
	CRichEditView::OnDestroy();
}

#ifdef _DEBUG
void ViewNoteDoc::AssertValid() const
{
	CRichEditView::AssertValid();
}

void ViewNoteDoc::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}

CNoteDoc* ViewNoteDoc::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNoteDoc)));
	return static_cast<CNoteDoc*>(m_pDocument);
}
#endif //_DEBUG

void ViewNoteDoc::on_tools_open_data_files()
{
	auto p_document = GetDocument();
	CString cs_name = p_document->GetPathName();
	p_document->open_project_files(cs_name);
}
