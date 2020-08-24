#pragma once
#include "NoteDoc.h"

class CViewNoteDoc : public CRichEditView
{
	CViewNoteDoc();
	DECLARE_DYNCREATE(CViewNoteDoc)

protected:
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;

	// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	CNoteDoc* GetDocument();
#endif
#ifndef _DEBUG  // debug version in NotedocView.cpp
	CNoteDoc* CViewNoteDoc::GetDocument() { return (CNoteDoc*)m_pDocument; }
#endif
	~CViewNoteDoc() override;

	// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg void OnToolsOpendatafiles();
	DECLARE_MESSAGE_MAP()
};
