// dbMultiDocTemplate.cpp : implementation file
//

#include "StdAfx.h"
#include "dbMultiDocTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbMultiDocTemplate

CdbMultiDocTemplate::CdbMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass,
                                         CRuntimeClass* pViewClass)
	: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
};

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
	CString str_temp;
	AfxExtractSubString(str_temp, m_strDocStrings, i);

	if (i == filterExt)
	{
		// string contains more than one extension?
		auto n_find_pos = str_temp.Find(_T(";"));
		while (n_find_pos != -1)
		{
			auto str_left = str_temp.Left(n_find_pos + 1);
			auto str_right = _T("*") + str_temp.Right(str_temp.GetLength() - n_find_pos - 1);
			str_temp = str_left + str_right;
			n_find_pos = str_temp.Find(_T(";"), n_find_pos + 1);
		}
	}
	rString = str_temp;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CMultiDocTemplate::Confidence CdbMultiDocTemplate::MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch)
{
	rpDocMatch = nullptr;
	CString cs_path_name = lpszPathName;

	// go through all documents to see if this document is already opened
	auto pos = GetFirstDocPosition();
	while (pos != nullptr)
	{
		const auto p_document = GetNextDoc(pos);
		if (p_document->GetPathName() == cs_path_name)
		{
			rpDocMatch = p_document;
			return yesAlreadyOpen;
		}
	} // end while

	// not open - then see if it matches either suffix
	CString str_filter_ext;
	if (GetDocString(str_filter_ext, filterExt) && !str_filter_ext.IsEmpty())
	{
		// make sure there is a dot in the filename
		const auto n_dot = cs_path_name.ReverseFind(_T('.'));
		if (n_dot <= 0)
			return noAttempt; // no extension found, exit

		// loop over the different extensions stored into the template filter
		auto cs_path_name_ext = cs_path_name.Right(cs_path_name.GetLength() - n_dot - 1);
		cs_path_name_ext.MakeLower();
		ASSERT(str_filter_ext[0] == _T('.'));
		auto n_semi = str_filter_ext.Find(';');

		while (n_semi != -1)
		{
			auto ext = str_filter_ext.Left(n_semi); // get extension
			ext.MakeLower();

			// check for a match against extension
			if (cs_path_name_ext.Find(ext) == 0)
				return yesAttemptNative; // extension matches

			// update filter string
			str_filter_ext = str_filter_ext.Mid(n_semi + 2);
			// search for next filter extension
			n_semi = str_filter_ext.Find(';');
		}
	}

	// otherwise we will guess it may work
	return yesAttemptForeign;
}
