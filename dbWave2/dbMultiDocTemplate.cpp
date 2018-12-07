// dbMultiDocTemplate.cpp : implementation file
//

#include "stdafx.h"
#include "dbMultiDocTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbMultiDocTemplate

CdbMultiDocTemplate::CdbMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) 
	: CMultiDocTemplate (nIDResource, pDocClass, pFrameClass, pViewClass) {};


CdbMultiDocTemplate::~CdbMultiDocTemplate()
{
}

BEGIN_MESSAGE_MAP(CdbMultiDocTemplate, CMultiDocTemplate)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdbMultiDocTemplate diagnostics

#ifdef _DEBUG
void CdbMultiDocTemplate::AssertValid() const
{
	CMultiDocTemplate::AssertValid();
}

void CdbMultiDocTemplate::Dump(CDumpContext& dc) const
{
	CMultiDocTemplate::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// get one of the info strings

BOOL CdbMultiDocTemplate::GetDocString(CString& rString, enum DocStringIndex i) const
{
	CString strTemp, strLeft, strRight;
	int nFindPos;
	AfxExtractSubString(strTemp, m_strDocStrings, (int) i);

	if (i == CDocTemplate::filterExt)
	{
		// string contains more than one extension?
		nFindPos = strTemp.Find(_T(";"));		
		while (nFindPos != -1)
		{
			strLeft= strTemp.Left(nFindPos+1);
			strRight= _T("*") + strTemp.Right(strTemp.GetLength() - nFindPos -1);
			strTemp = strLeft + strRight;
			nFindPos = strTemp.Find(_T(";"), nFindPos+1);
		}
	}
	rString = strTemp;
	return TRUE;
} 

/////////////////////////////////////////////////////////////////////////////

CMultiDocTemplate::Confidence CdbMultiDocTemplate::MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch)
{
	rpDocMatch = nullptr;
	CString csPathName = lpszPathName;

	// go through all documents to see if this document is already opened
	POSITION pos = GetFirstDocPosition();
	while (pos != nullptr)
	{
		CDocument* pDoc = GetNextDoc(pos);
		if (pDoc->GetPathName() == csPathName)
		{		
			rpDocMatch = pDoc;
			return yesAlreadyOpen;
		}
	}	// end while

	// not open - then see if it matches either suffix
	CString strFilterExt ;
	if (GetDocString(strFilterExt, CDocTemplate::filterExt) && !strFilterExt.IsEmpty())
	{	
		// make sure there is a dot in the filename
		int nDot = csPathName.ReverseFind(_T('.'));
		if (nDot <= 0)
			return noAttempt;	// no extension found, exit

		// loop over the different extensions stored into the template filter
		CString csPathNameExt = csPathName.Right(csPathName.GetLength() - nDot -1);
		csPathNameExt.MakeLower();
		ASSERT(strFilterExt[0] == _T('.'));
		int nSemi = strFilterExt.Find(';');
		CString ext;
		while (nSemi != -1)
		{
			CString ext = strFilterExt.Left(nSemi);	// get extension
			ext.MakeLower();

			// check for a match against extension
			if (csPathNameExt.Find(ext) == 0)
				return yesAttemptNative; // extension matches

			// update filter string
			strFilterExt = strFilterExt.Mid(nSemi+2);
			// search for next filter extension
			nSemi = strFilterExt.Find(';');
		}
	}

	// otherwise we will guess it may work
	return yesAttemptForeign;
}

