#pragma once

// NoteDoc.h : interface of the CNoteDoc class
//
/////////////////////////////////////////////////////////////////////////////

#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc : public CRichEditDoc
{
protected: // create from serialization only
	CNoteDoc();
	DECLARE_DYNCREATE(CNoteDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	void Serialize(CArchive& ar) override;

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const;
	BOOL OpenProjectFiles(CString& cspathname);


// Implementation
public:
	virtual ~CNoteDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	
	DECLARE_MESSAGE_MAP()
};

