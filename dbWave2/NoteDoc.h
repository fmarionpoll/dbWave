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
	int			extractList(CRichEditCtrl& pEdit, CStringArray& csFilesOK, CStringArray& csDescriptors);
	BOOL		openFileList(CString& cspathname, CStringArray& cs_arrayfiles, CStringArray& csListDescriptors, int nColumns);
	BOOL		addFileName(CString& resToken, CStringArray& csArrayOK, CStringArray& csArrayTested);
	void		displayFilesImported(CRichEditCtrl& pEdit, CStringArray& pArrayTested);
	int			extractColumnsFromRow(CString& cs, CStringArray& pResult);
	inline BOOL isFilePresent(CString csFilename) { CFileStatus status; return CFile::GetStatus(csFilename, status); }
	void		addRowToArray(CStringArray& csRow, CStringArray& csOut);


	DECLARE_MESSAGE_MAP()
};
