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
	                  enum DocStringIndex index) const override; // get one of the info strings
	Confidence MatchDocType(LPCTSTR lpszPathName,
	                        CDocument*& rpDocMatch) override;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CdbMultiDocTemplate)
public:
protected:
	//}}AFX_VIRTUAL

	// Implementation
public:
	~CdbMultiDocTemplate() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
