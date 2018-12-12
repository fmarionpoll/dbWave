#pragma once

// NotedocView.h : interface of the CViewNoteDoc class
//
/////////////////////////////////////////////////////////////////////////////


class CProjectFilesCntrItem;

class CViewNoteDoc : public CRichEditView
{
protected: // create from serialization only
	CViewNoteDoc();
	DECLARE_DYNCREATE(CViewNoteDoc)

// Attributes
public:
	CNoteDoc* GetDocument();

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CViewNoteDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg void OnToolsOpendatafiles();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in NotedocView.cpp
inline CNoteDoc* CViewNoteDoc::GetDocument()
   { return (CNoteDoc*)m_pDocument; }
#endif
