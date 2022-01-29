#pragma once
#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc : public CRichEditDoc
{
protected: // create from serialization only
	CNoteDoc();
	DECLARE_DYNCREATE(CNoteDoc)

	// Overrides
public:
	BOOL OnNewDocument() override;
	void Serialize(CArchive& ar) override;

	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const override;
	BOOL OpenProjectFiles(CString& cspathname);

	// Implementation
public:
	~CNoteDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
	int extractList(CRichEditCtrl& pEdit, CStringArray& csFilesOK, CStringArray& csDescriptors);
	BOOL openFileList(CString& cspathname, CStringArray& cs_arrayfiles, CStringArray& csListDescriptors, int nColumns);
	BOOL addFileName(CString& resToken, CStringArray& csArrayOK, CStringArray& csArrayTested);
	void displayFilesImported(CRichEditCtrl& pEdit, CStringArray& pArrayTested);
	int extractColumnsFromRow(CString& cs, CStringArray& pResult);

	BOOL isFilePresent(CString csFilename)
	{
		CFileStatus status;
		return CFile::GetStatus(csFilename, status);
	}

	void addRowToArray(CStringArray& csRow, CStringArray& csOut);


	DECLARE_MESSAGE_MAP()
};
