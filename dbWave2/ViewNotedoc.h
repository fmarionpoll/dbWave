#pragma once
#include "NoteDoc.h"

class ViewNoteDoc : public CRichEditView
{
	ViewNoteDoc();
	DECLARE_DYNCREATE(CViewNoteDoc)

protected:
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;

	// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
	CNoteDoc* GetDocument();
#endif
#ifndef _DEBUG  // debug version in NotedocView.cpp
	CNoteDoc* ViewNoteDoc::GetDocument() { return (CNoteDoc*)m_pDocument; }
#endif
	~ViewNoteDoc() override;

	// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg void OnToolsOpendatafiles();

	DECLARE_MESSAGE_MAP()
};
