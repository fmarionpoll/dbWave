#pragma once


class CdbMultiDocTemplate : public CMultiDocTemplate
{
	// Construction
public:
	CdbMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

	// Attributes
public:

	// Operations
public:
	BOOL GetDocString(CString& rString,
		enum DocStringIndex index) const; // get one of the info strings
	CMultiDocTemplate::Confidence MatchDocType(LPCTSTR lpszPathName,
		CDocument*& rpDocMatch);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CdbMultiDocTemplate)
public:
protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CdbMultiDocTemplate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
