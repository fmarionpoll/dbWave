#pragma once
#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc : public CRichEditDoc
{
protected: // create from serialization only
	CNoteDoc();
	DECLARE_DYNCREATE(CNoteDoc)

	// Overrides
public:
	virtual BOOL OnNewDocument();
	void		Serialize(CArchive& ar) override;

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const;
	BOOL		OpenProjectFiles(CString& cspathname);

	// Implementation
public:
	virtual		~CNoteDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStringArray*	extractListOfFilesSimple(CString& cspathname, CRichEditCtrl& pEdit);
	BOOL			openListOfFilesSimple(CString& cspathname, CStringArray* cs_arrayfiles);
	BOOL			addFileName(CString resToken, CStringArray* pArrayOK, CStringArray* pArrayTested);
	void			displayFilesImported(CRichEditCtrl& pEdit, CStringArray* pArrayTested);
	CStringArray*	analyzeLine(CString cs);

	DECLARE_MESSAGE_MAP()
};
