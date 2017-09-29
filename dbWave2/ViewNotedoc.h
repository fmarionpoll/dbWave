#pragma once

// NotedocView.h : interface of the CNoteDocView class
//
/////////////////////////////////////////////////////////////////////////////


class CProjectFilesCntrItem;

class CNoteDocView : public CRichEditView
{
protected: // create from serialization only
	CNoteDocView();
	DECLARE_DYNCREATE(CNoteDocView)

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
	virtual ~CNoteDocView();
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
inline CNoteDoc* CNoteDocView::GetDocument()
   { return (CNoteDoc*)m_pDocument; }
#endif
